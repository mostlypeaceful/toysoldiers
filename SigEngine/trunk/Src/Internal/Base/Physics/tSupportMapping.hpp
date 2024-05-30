#ifndef __tSupportMapping__
#define __tSupportMapping__

#include "Math/tConvexHull.hpp"

namespace Sig { namespace Physics
{
	enum tTypes
	{
		cSphereSupport,
		cPointSphereSupport,
		cPointCapsuleSupport,
		cCylinderSupport,
		cAabbSupport,
		cObbSupport,
		cHullSupport,
		cTriangleSupport,
		cRaySupport,
		cSupportCount
	};

	class tSupportMapping : public tRefCounter
	{
	public:
		f32	mExtraRadius;

		tSupportMapping( f32 extraRadius = 0.f, u32 type = cSphereSupport );
		virtual ~tSupportMapping( );

		struct tSupport
		{
			Math::tVec3f mP;
			u32 mID;

			tSupport( const Math::tVec3f& p = Math::tVec3f::cZeroVector, u32 id = ~0 )
				: mP( p ), mID( id )
			{ }
		};
		
		tSupport fWorldSupport( const Math::tVec3f& d )
		{
			tSupport s = fLocalSupport( mWorldXform.fInverseXformVector( d ) );
			s.mP = mWorldXform.fXformPoint( s.mP );
			return s;
		}

		Math::tVec3f fWorldCenter( ) const 
		{ 
			return mWorldXform.fXformPoint( fLocalCenter( ) ); 
		}

		void fWorldIndexedSupport( tSupport& s ) const
		{
			fLocalIndexedSupport( s );
			s.mP = mWorldXform.fXformPoint( s.mP );
		}

		virtual void fSetWorldXform( const Math::tMat3f& xform )
		{
			mWorldXform = xform; 
		}

		virtual b32 fIndexable( ) const { return true; }

		// Debugging
#ifdef sig_devmenu
		u32 mType;
		static tFixedArray< u32, cSupportCount > gCounts;
#endif

	protected:
		// Support happens in local space.
		Math::tMat3f mWorldXform;

		virtual tSupport fLocalSupport( const Math::tVec3f& d ) const = 0;
		virtual void fLocalIndexedSupport( tSupport& s ) const
		{
			//not defined
			sigassert( !"Don't try to index data that isnt indexed, make up a search dir." );
		}

		virtual Math::tVec3f fLocalCenter( ) const { return Math::tVec3f::cZeroVector; }
	};

	typedef tRefCounterPtr<tSupportMapping> tSupportMappingPtr;

	// These all handle the objects in local space.
	class tSphereSupportMapping : public tSupportMapping
	{
	public:
		f32 mRadius;

		tSphereSupportMapping( f32 radius ) 
			: tSupportMapping( 0.f, cSphereSupport )
			, mRadius( radius ) 
		{ }

		virtual tSupport fLocalSupport( const Math::tVec3f& d ) const 
		{ 
			Math::tVec3f normD = d;
			normD.fNormalizeSafe( Math::tVec3f::cYAxis );
			return tSupport( normD * mRadius ); 
		}

		virtual b32 fIndexable( ) const { return false; }
	};

	class tPointSphereSupportMapping : public tSupportMapping
	{
	public:
		Math::tVec3f mCenter;

		tPointSphereSupportMapping( const Math::tVec3f& center, f32 radius ) 
			: tSupportMapping( radius, cPointSphereSupport ) 
			, mCenter( center )
		{ }

		virtual tSupport fLocalSupport( const Math::tVec3f& d ) const 
		{ 
			return tSupport( mCenter, 0 ); 
		}

		virtual void fLocalIndexedSupport( tSupport& s ) const
		{
			s.mP = mCenter;
		}

		virtual Math::tVec3f fLocalCenter( ) const 
		{ 
			return mCenter; 
		}
	};

	class tPointCapsuleSupportMapping : public tSupportMapping
	{
	public:
		Math::tVec3f mP1;
		Math::tMat3f mLocalXform;

		tPointCapsuleSupportMapping( const Math::tCapsule& capsule ) 
			: tSupportMapping( capsule.mRadius, cPointCapsuleSupport ) 
			, mP1( Math::tVec3f::cYAxis * capsule.mHalfHeight )
			, mLocalXform( capsule.fGetTransform( ) )
		{ }

		void fUpdate( const Math::tCapsule& capsule )
		{
			mExtraRadius = capsule.mRadius;
			mP1 = Math::tVec3f::cYAxis * capsule.mHalfHeight;
			mLocalXform = capsule.fGetTransform( );
		}

		virtual tSupport fLocalSupport( const Math::tVec3f& d ) const 
		{ 
			f32 dot = mP1.fDot( d );
			if( dot < 0 )
				return tSupport( -mP1, 1 );
			else
				return tSupport( mP1, 0 );
		}

		virtual void fLocalIndexedSupport( tSupport& s ) const
		{
			if( s.mID == 0 )
				s.mP = mP1;
			else
				s.mP = -mP1;
		}

		virtual void fSetWorldXform( const Math::tMat3f& xform )
		{
			mWorldXform = xform * mLocalXform; 
		}
	};

	class tCylinderSupportMapping : public tSupportMapping
	{
	public:
		Math::tMat3f mLocalXform;
		Math::tVec3f mHeightAxis;
		f32 mRadius;

		tCylinderSupportMapping( const Math::tCylinder& cylinder, f32 extraRadius ) 
			: tSupportMapping( extraRadius, cCylinderSupport ) 
			, mLocalXform( cylinder.fGetTransform( ) )
			, mHeightAxis( Math::tVec3f::cYAxis * cylinder.mHalfHeight )
			, mRadius( cylinder.mRadius )
		{ }

		void fUpdate( const Math::tCylinder& cylinder )
		{
			mLocalXform = cylinder.fGetTransform( );
			mHeightAxis = Math::tVec3f::cYAxis * cylinder.mHalfHeight;
			mRadius = cylinder.mRadius;
		}

		virtual tSupport fLocalSupport( const Math::tVec3f& d ) const 
		{ 
			Math::tVec3f subAxis = mHeightAxis.fCross( d.fCross( mHeightAxis ) );
			subAxis.fNormalizeSafe( Math::tVec3f::cXAxis );
			subAxis *= mRadius;

			if( mHeightAxis.fDot( d ) > 0 )
				subAxis += mHeightAxis;
			else
				subAxis -= mHeightAxis;

			return tSupport( subAxis );
		}

		virtual void fSetWorldXform( const Math::tMat3f& xform )
		{
			mWorldXform = xform * mLocalXform; 
		}

		virtual b32 fIndexable( ) const { return false; }
	};

	class tAabbSupportMapping : public tSupportMapping
	{
	public:
		Math::tVec3f mExtents;

		tAabbSupportMapping( const Math::tVec3f& extents, f32 extraRadius, u32 typeOverride = cAabbSupport ) 
			: tSupportMapping( extraRadius, typeOverride )
			, mExtents( extents ) 
		{ }

		virtual tSupport fLocalSupport( const Math::tVec3f& d ) const 
		{ 
			tSupport support( mExtents, 0 );

			if( d.x < 0 )
			{
				support.mID |= 1; 
				support.mP.x *= -1;
			}

			if( d.y < 0 )
			{
				support.mID |= 2; 
				support.mP.y *= -1;
			}

			if( d.z < 0 )
			{
				support.mID |= 4; 
				support.mP.z *= -1;
			}

			return support;
		}

		virtual void fLocalIndexedSupport( tSupport& s ) const
		{
			s.mP = mExtents;
			if( s.mID & 1 ) s.mP.x *= -1.f;
			if( s.mID & 2 ) s.mP.y *= -1.f;
			if( s.mID & 4 ) s.mP.z *= -1.f;
		}
	};

	class tObbSupportMapping : public tAabbSupportMapping
	{
	public:
		Math::tMat3f mLocalXform;

		tObbSupportMapping( const Math::tObbf& box, f32 extraRadius ) 
			: tAabbSupportMapping( box.mExtents, extraRadius, cObbSupport )
		{
			fUpdate( box );
		}

		void fUpdate( const Math::tObbf& box )
		{
			mLocalXform = box.fGetTransform( );
		}

		virtual void fSetWorldXform( const Math::tMat3f& xform )
		{
			mWorldXform = xform * mLocalXform; 
		}
	};

	class tHullSupportMapping : public tSupportMapping
	{
	public:
		const Math::tConvexHull& mHull;
		Math::tMat3f mLocalXform;

		// just holds a reference
		tHullSupportMapping( const Math::tConvexHull* hull, const Math::tMat3f& localXform, f32 extraRadius ) 
			: tSupportMapping( extraRadius, cHullSupport )
			, mHull( *hull )
			, mLocalXform( localXform )
		{ }

		virtual tSupport fLocalSupport( const Math::tVec3f& d ) const 
		{ 
			u32 vert = mHull.fSupportingVertexIndex( d );
			return tSupport( mHull.fVerts( )[ vert ], vert );
		}

		virtual void fLocalIndexedSupport( tSupport& s ) const
		{
			s.mP = mHull.fVerts( )[ s.mID ];
		}

		virtual Math::tVec3f fLocalCenter( ) const 
		{ 
			return mHull.fComputeCenter( ); 
		}

		virtual void fSetWorldXform( const Math::tMat3f& xform )
		{
			mWorldXform = xform * mLocalXform; 
		}
	};

	class  tTriangleSupportMapping: public tSupportMapping
	{
	public:
		const Math::tTrianglef* mTri;

		// just holds a reference
		tTriangleSupportMapping( const Math::tTrianglef* tri, f32 extraRadius ) 
			: tSupportMapping( extraRadius, cTriangleSupport )
			, mTri( tri ) 
		{ }


		virtual tSupport fLocalSupport( const Math::tVec3f& d ) const 
		{ 
			u32 index = mTri->fSupportingCornerIndex( d );
			return tSupport( mTri->fCorner( index ), index );
		}

		virtual void fLocalIndexedSupport( tSupport& s ) const
		{
			s.mP = mTri->fCorner( s.mID );
		}

		virtual Math::tVec3f fLocalCenter( ) const 
		{ 
			return mTri->fComputeCenter( ); 
		}
	};

}}

#endif//__tSupportMapping__
