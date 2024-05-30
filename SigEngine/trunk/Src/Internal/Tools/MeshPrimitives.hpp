#ifndef __MeshPrimitives__
#define __MeshPrimitives__

#include "Threads/tMutex.hpp"

namespace Sig { namespace MeshSimplify
{
	class tFace;
	class tVert;

	// These are just for clarity. I've been going insane keeping these separate.
	typedef u32 tFaceID;
	typedef u32 tVertID;
	typedef tGrowableArray< tFaceID > tFaceList;
	typedef tGrowableArray< tVertID > tVertList;

	class tQuadric
	{
		f64	a2, ab, ac, ad;
		f64		b2, bc, bd;
		f64			c2, cd;
		f64				d2;

		f64 mAreaWeight;

	public:
		tQuadric( f64 initVal = 0.f ) { fSetAll( initVal ); }
		tQuadric( f64 a, f64 b, f64 c, f64 d, f64 area ) { fSetPlaneConstraint( a, b, c, d ); mAreaWeight = area; }
		tQuadric( const Math::tVec3d& v, f64 area ) { fSetVertConstraint( v ); }

		// Access different pieces.
		Math::tMat3d	fGetTensor( ) const; // Component A
		Math::tVec3d	fGetVector( ) const { return Math::tVec3d(ad, bd, cd); } // Component b
		f64				fGetOffset( ) const { return d2; } // Component c
		Math::tMat4d	fGetMatrix( ) const; // Whole matrix

		f64				fGetArea( ) const { return mAreaWeight; }

		tQuadric& operator+=( const tQuadric& Q );
		tQuadric& operator*=( f64 scalar );
		tQuadric& operator/=( f64 scalar );

		void fSetPlaneConstraint( f64 a, f64 b, f64 c, f64 d );
		void fSetVertConstraint( const Math::tVec3d& v );
		void fSetAll( f64 val ) { a2 = ab = ac = ad = b2 = bc = bd = c2 = cd = d2 = val; }

		/// 
		/// \brief Computes the position that causes the least amount of error for this vertex.
		b32 fGetOptimalPos( Math::tVec3d& v ) const;

		/// 
		/// \brief Computes the amount of error this quadric would cause for the vertex.
		f64 fEvaluate( const Math::tVec3d& v ) const;
	};

	class tMeshPrim
	{
		b32 mValid;
		u32	mMark;

	public:
		tMeshPrim( ) : mValid( true ), mMark( 0 ) { }

		void fMarkInvalid( ) { sigassert( mValid ); mValid = false; }
		b32 fIsValid( ) const { return mValid; }

		void fMark( u32 mark ) { mMark = mark; }
		u32 fGetMark( ) const { return mMark; }
		void fAddMark( u32 add ) { mMark += add; }
	};

	class tContractPair
	{
		define_class_pool_new_delete_mt( tContractPair, 1024 );
	private:
		tVertID			mV[2];
		Math::tVec3d	mCandidate;
		f32				mCost;

	public:
		tContractPair( tVertID v0, tVertID v1 ) : mCost( -1.f ) { mV[0] = v0; mV[1] = v1; }

		tVertID			fV0( ) const { return mV[0]; }
		tVertID			fV1( ) const { return mV[1]; }
		Math::tVec3d	fGetCandidate( ) const { return mCandidate; }
		inline f32		fGetCost( ) const { return mCost; }

		void fSetVerts( tVertID v0, tVertID v1 );
		tVertID fGetOtherVert( tVertID v ) const;

		void fComputeCandidateAndCost( const tQuadric& Q, const Math::tVec3d& v0, const Math::tVec3d& v1 );
		void fAddPenaltyCost( f32 penalty ) { mCost += penalty; }
	};

	struct tDecimateEvent
	{
		tVertID			v0, v1;
		Math::tVec3d	mCandidate;

		tFaceList		mChangedFaces;
		tFaceList		mDeadFaces;
	};

	class tVert : public tMeshPrim
	{
		Math::tVec3d						mPos;
		tFaceList							mNeighborFaces;
		tGrowableArray< tContractPair* >	mPairs;
		b16									mLocked;
		b16									mDirty;

	public:
		tQuadric		mQ;

		tVert( ) { }
		tVert( const Math::tVec3d& p ) : mPos( p ), mLocked( false ), mDirty( false ), mQ( 0.f ) { }
		tVert( const Math::tVec3f& p) : mPos( Math::tVec3d( p[0], p[1], p[2] ) ), mLocked( false ), mDirty( false ), mQ( 0.f ) { }

		// Neighbor manipulations.
		void				fAddNeighbor( tFaceID i ) { mNeighborFaces.fPushBack( i ); }
		const tFaceList&	fGetNeighbors( ) const { return mNeighborFaces; }
		void				fRemapNeighbor( tFaceID from, tFaceID to );
		b32					fRemoveNeighbor( tFaceID remove ) { return mNeighborFaces.fFindAndErase( remove ); }
		void				fClearNeighbors( ) { mNeighborFaces.fSetCount( 0 ); }

		// Pair manipulations.
		void				fAddPair( tContractPair* newPair ) { mPairs.fPushBack( newPair ); }
		u32					fNumPairs( ) const { return mPairs.fCount( ); }
		tContractPair*		fGetPair( u32 i ) const { sigassert( i < mPairs.fCount( ) ); return mPairs[i]; }
		b32					fRemovePair( tContractPair* removePair ) { return mPairs.fFindAndErase(removePair); }
		void				fClearPairs( ) { mPairs.fSetCount( 0 ); }

		Math::tVec3d		fGetPos( ) const { return mPos; }
		void				fSetPos( const Math::tVec3d& newPos ) { sigassert( !mLocked ); mPos = newPos; mDirty = true; }

		void				fSetLocked( b32 lock ) { mLocked = lock; }
		b32					fLocked( ) const { return mLocked; }

		b32					fDirty( ) const { return mDirty; }
	};


	class tFace : public tMeshPrim
	{
		tVertID		mVerts[3];

	public:
		tFace( ) { }
		tFace( tVertID v0, tVertID v1, tVertID v2 )
		{
			mVerts[0] = v0;
			mVerts[1] = v1;
			mVerts[2] = v2;
		}

		tVertID  operator[]( int i ) const { return mVerts[i]; }

		void fRemapVert( tVertID from, tVertID to );
		u32 fFindVert( tVertID v ) const
		{ 
			for( u32 i = 0; i < 3; ++i )
			{
				if( mVerts[i] == v ) 
					return i;
			}

			return -1;
		}
	};
} }

#endif //__MeshPrimitives__
