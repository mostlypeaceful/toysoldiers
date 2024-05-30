#ifndef __tGJK__
#define __tGJK__

#include "tSupportMapping.hpp"
#include "tContactPoint.hpp"

namespace Sig { namespace Physics
{

	struct tSimplexVertex
	{
		tSupportMapping::tSupport mA;
		tSupportMapping::tSupport mB;
		Math::tVec3f mMinkowski;

		tSimplexVertex( ) 
		{ }

		tSimplexVertex( const tSupportMapping::tSupport& a, const tSupportMapping::tSupport& b, const Math::tVec3f& minkowski )
			: mA( a )
			, mB( b )
			, mMinkowski( minkowski )
		{ }

		b32 operator == ( const tSimplexVertex& vert ) const
		{
			return mA.mID == vert.mA.mID && mB.mID == vert.mB.mID;
		}

		b32 operator == ( const Math::tVec3f& minkowski ) const
		{
			return mMinkowski.fEqual( minkowski );
		}
	};

	struct tSimplex
	{
		typedef tFixedGrowingArray< tSimplexVertex, 4 > tVertList;
		tVertList mVerts;
		Math::tVec3f mSearchDir;
		Math::tVec3f mClosestPt;

		void fClear( ) { mVerts.fSetCount( 0 ); }
		b32  fContainsPt( const tSimplexVertex& vert ) const { return mVerts.fIndexOf( vert ) != -1; }
		b32  fContainsPt( const Math::tVec3f& minkowski ) const { return mVerts.fIndexOf( minkowski ) != -1; }

		if_devmenu( b32 operator == ( const tSimplex& other ) const; )
	};

	// for debugging
	struct tGJKHistory
	{
		tGrowableArray< tSimplex > mEvents;
		void fLog( );
		void fClear( ) { mEvents.fSetCount( 0 ); }
	};

	class tGJK
	{
	public:
		tGJK( tSupportMapping* a, tSupportMapping* b, b32 forRaycast = false );

		const tSupportMappingPtr& fA( ) const { return mA; }
		const tSupportMappingPtr& fB( ) const { return mB; }
		void fReset( );

		// call this is if the bodies have moved and you want to retest as fast as possible.
		void fResume( );

		void fCompute( );

		// Feedback
		const Math::tVec3f& fClosestPtA( ) const { return mClosestPtA; }
		const Math::tVec3f& fClosestPtB( ) const { return mClosestPtB; }
		b32 fIntersects( ) const { return mIntersects; }

		// These are only valid if fIntersects is true.
		f32 fPenetration( ) const { return mPenetration; }
		const Math::tVec3f fNormalBToA( ) const { return mNormal; }
		tContactPoint fMakeContactPt( const Math::tVec3f* normalOverride = NULL ) const;

		// These should probably be moved to tTriangle, but are dependent on a lot of hidden functions in this cpp
		static Math::tVec3f fBarycentricTri( const Math::tVec3f& a, const Math::tVec3f& b, const Math::tVec3f& c );

		// only for debugging.
		static void fTest( );
		void fDrawSimplexes( );

	private:
		tSupportMappingPtr mA;
		tSupportMappingPtr mB;
		b32 mResumable;
		b32 mForRaycast;

		tSimplex mSimplex;
		b32 mIntersects;
		b32 mTerminate;
		f32 mClosestDist; //used for termination, to ensure we're getting closer.
		Math::tVec3f mClosestPtA;
		Math::tVec3f mClosestPtB;
		f32 mPenetration;
		Math::tVec3f mNormal;

		if_devmenu( tGJKHistory mHistory; )

		tSimplexVertex fSupport( const Math::tVec3f& worldD );

		// return false if origin is within simplex.
		void fUpdateSimplex( const tSimplexVertex& newVert );
		void fEvolveSimplex( );
		b32 fShouldUpdate( const Math::tVec3f& newVert );

		// a final step to compute the actual closest pts on the shapes.
		void fUpdateClosestPts( );

	};


	class tGJKRaycast
	{
	public:
		tGJKRaycast( tSupportMapping* a, const Math::tRayf& ray );

		void fReset( );

		void fCompute( );

		tSupportMapping* fShape( ) const { return mGJK.fA( ).fGetRawPtr( ); }
		Math::tRayf& fRay( ) { return mRay; }

		//results
		b32 mIntersects;
		Math::tVec3f mPoint;
		Math::tVec3f mNormal;
		f32 mT;

		b32 mDebugRender;

	private:
		tPointSphereSupportMapping* mRaySupport; //owned by mGJK
		tGJK mGJK;
		Math::tRayf mRay;

		Math::tVec4f mDebugColor;
	};

}}

#endif//__tGJK__
