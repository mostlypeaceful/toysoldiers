//------------------------------------------------------------------------------
// \file tShapeEntity.cpp - 06 Sep 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tSceneGraph.hpp"
#include "tShapeEntity.hpp"

#include "Math/tIntersectionRayObb.hpp"
#include "Math/tIntersectionRaySphere.hpp"
#include "Math/tIntersectionAabbObb.hpp"
#include "Math/tIntersectionAabbSphere.hpp"
#include "Math/tIntersectionSphereObb.hpp"
#include "Math/tIntersectionSphereFrustum.hpp"

#include "Physics/tCollisionShapes.hpp"
#include "Physics/tPhysicsWorld.hpp"

#include "Gfx/tPotentialVisibilitySet.hpp"

namespace Sig
{
	namespace
	{
		Math::tVec3f fNormalFromPointOnBox( const Math::tVec3f& point, const Math::tObbf& obb )
		{
			for( u32 i = 0; i < 3; ++i )
			{
				const Math::tPlanef p0 = Math::tPlanef( obb.mAxes[i], obb.mCenter + obb.mAxes[i] * obb.mExtents[i] );
				if( fAbs( p0.fSignedDistance( point ) ) < 0.1f )
					return p0.fGetNormalUnit( );

				const Math::tPlanef p1 = Math::tPlanef( -obb.mAxes[i], obb.mCenter - obb.mAxes[i] * obb.mExtents[i] );
				if( fAbs( p1.fSignedDistance( point ) ) < 0.1f )
					return p1.fGetNormalUnit( );
			}

			return Math::tVec3f::cYAxis;
		}
	} // Anonymous Namespace

	//------------------------------------------------------------------------------
	// tShapeEntityDef
	//------------------------------------------------------------------------------
	tShapeEntityDef::tShapeEntityDef( )
		: mShapeType( cShapeTypeBox )
		, mStateMask( 0xFFFF )
		, mHull( NULL )
	{
	}

	//------------------------------------------------------------------------------
	tShapeEntityDef::tShapeEntityDef( tNoOpTag )
		: tEntityDef( cNoOpTag )
		, mHull( cNoOpTag )
		, mPotentialVisibility( cNoOpTag )
	{
	}

	//------------------------------------------------------------------------------
	tShapeEntityDef::~tShapeEntityDef( )
	{
	}

	//------------------------------------------------------------------------------
	void tShapeEntityDef::fCollectEntities( const tCollectEntitiesParams& paramsParent ) const
	{
		const tEntityCreationFlags creationFlags = paramsParent.mCreationFlags | fCreationFlags( );
		b32 createCollision = creationFlags.fCreateCollision( );

		tShapeEntity* entity = NULL;
		
		switch( mShapeType )
		{
		case cShapeTypeCylinder: //Hack here, and hack in tBoxEntity::tBoxEntity( )
		case cShapeTypeCapsule:
		case cShapeTypeBox: entity = NEW tBoxEntity( this, mBounds, mObjectToLocal, createCollision ); break;
		case cShapeTypeSphere: entity = NEW tSphereEntity( this, mBounds, mObjectToLocal, createCollision ); break;
		case cShapeTypeConvexHull: entity = NEW tConvexHullEntity( this, mBounds, mObjectToLocal, createCollision ); break;
		default: sig_nodefault( );
		}

		fApplyPropsAndSpawnWithScript( *entity, tCollectEntitiesParams( paramsParent.mParent, creationFlags ) );
	}

	//------------------------------------------------------------------------------
	// tShapeEntity
	//------------------------------------------------------------------------------
	const u32 tShapeEntity::cSpatialSetIndex = tSceneGraph::fNextSpatialSetIndex( );

	//------------------------------------------------------------------------------
	void tShapeEntity::fSetCollisionEnabled( tEntity& root, b32 enabled )
	{
		tShapeEntity* shape = root.fDynamicCast< tShapeEntity >( );
		if( shape )
			shape->fSetEnabled( enabled );

		for( u32 i = 0; i < root.fChildCount( ); ++i )
			fSetCollisionEnabled( *root.fChild( i ), enabled );
	}

	//------------------------------------------------------------------------------
	tShapeEntity::tShapeEntity( const tShapeEntityDef* entityDef, const Math::tAabbf& objectSpaceBox, const Math::tMat3f& objectToWorld, b32 createCollision )
		: tSpatialEntity( objectSpaceBox, entityDef->mStateMask )
		, mEntityDef( entityDef )
		, mShapeType( entityDef->mShapeType )
		, mEnabled( fTestBits( entityDef->mStateMask, (1<<0) ) )
		, mCreateCollision( createCollision )
	{
		fMoveTo( objectToWorld );
	}

	//------------------------------------------------------------------------------
	tShapeEntity::tShapeEntity( const Math::tAabbf& objectSpaceBox, tShapeEntityDef::tShapeType shape, b32 createCollision )
		: tSpatialEntity( Math::tAabbf( -1.f, +1.f ) )
		, mEntityDef( NULL )
		, mShapeType( shape )
		, mEnabled( true )
		, mCreateCollision( createCollision )
	{
		Math::tMat3f objectToWorld = Math::tMat3f::cIdentity;
		objectToWorld.fSetTranslation( objectSpaceBox.fComputeCenter( ) );
		objectToWorld.fScaleLocal( 0.5f * objectSpaceBox.fComputeDiagonal( ) );

		fMoveTo( objectToWorld );
	}

	//------------------------------------------------------------------------------
	tShapeEntity::~tShapeEntity( )
	{
	}

	//------------------------------------------------------------------------------
	void tShapeEntity::fOnSpawn( )
	{
		if( mCreateCollision )
			fCreateCollision( );

		if( mEntityDef )
		{
			for( u32 i = 0; i < mEntityDef->mPotentialVisibility.fCount( ); ++i )
			{
				const tShapeEntityDef::tPVSInformation& pvs = mEntityDef->mPotentialVisibility[ i ];
				fSceneGraph( )->fPotentialVisibilitySet( )->fRegisterConvexHull( this, pvs.mSetName->fGetStringPtr( ), pvs.mInvert );
			}
		}
		
		tSpatialEntity::fOnSpawn( );
	}

	//------------------------------------------------------------------------------
	void tShapeEntity::fOnDelete( )
	{
		tSpatialEntity::fOnDelete( );

		if( mCollisionShape )
		{
			mCollisionShape->fUserData( ).fRelease( );
			mCollisionShape->fRemoveFromOwner( );
		}
	}

	//------------------------------------------------------------------------------
	void tShapeEntity::fUpdateCollision( )
	{
		if( mCollisionShape )
		{
			Physics::tCollisionShape::fUpdateTransform( mCollisionShape.fGetRawPtr( ), this, Math::tMat3f::cIdentity );
			if( mEnabled && mCollisionShape->mOwner )
				mCollisionShape->mOwner->fShapeMoved( mCollisionShape.fGetRawPtr( ) );
		}
	}

	//------------------------------------------------------------------------------
	void tShapeEntity::fPropagateSkeleton( Anim::tAnimatedSkeleton& skeleton )
	{
		fPropagateSkeletonInternal( skeleton, mEntityDef );
	}

	//------------------------------------------------------------------------------
	void tShapeEntity::fSetEnabled( b32 enabled ) 
	{ 
		mEnabled = enabled;
		if( mCollisionShape )
		{
			if( enabled )
			{
				if( fSceneGraph( ) && !mCollisionShape->fOwner( ) )
					fSceneGraph( )->fPhysics( )->fMainStaticBody( )->fAddShape( mCollisionShape.fGetRawPtr( ) );
			}
			else
				mCollisionShape->fRemoveFromOwner( );
		}
	}

	//------------------------------------------------------------------------------
	void tShapeEntity::fCreateCollision( )
	{
		if( mCollisionShape )
			mCollisionShape->fRemoveFromOwner( );

		mCollisionShape.fReset( Physics::tCollisionShape::fFromShapeEntity( this, Math::tMat3f::cIdentity ) ); //these are assumed to be static and fixed in world space.
		mCollisionShape->fUserData( ).fReset( this );
		
		fSetEnabled( mEnabled );
	}

	//------------------------------------------------------------------------------
	Math::tObbf tShapeEntity::fParentRelativeBox( ) const
	{
		return Math::tObbf( Math::tAabbf( -1.f, +1.f ), fParentRelative( ) );
	}

	//------------------------------------------------------------------------------
	Math::tSpheref tShapeEntity::fParentRelativeSphere( ) const
	{
		const Math::tMat3f& xform = fParentRelative( );
		return Math::tSpheref(
			xform.fGetTranslation( ),
			fMax( xform.fXAxis( ).fLength( ), xform.fYAxis( ).fLength( ), xform.fZAxis( ).fLength( ) ) );
	}

	//------------------------------------------------------------------------------
	const Math::tObbf& tShapeEntity::fBox( ) const 
	{ 
		// will assert if this is not a tBoxEntity
		return fStaticCast<tBoxEntity>( )->fBox( ); 
	}

	//------------------------------------------------------------------------------
	const Math::tSpheref& tShapeEntity::fSphere( ) const 
	{ 
		// will assert if this is not a tSphereEntity
		return fStaticCast<tSphereEntity>( )->fSphere( ); 
	}

	//------------------------------------------------------------------------------
	// tSphereEntity
	//------------------------------------------------------------------------------
	tSphereEntity::tSphereEntity( 
		const tShapeEntityDef* entityDef, 
		const Math::tAabbf& objectSpaceBox, 
		const Math::tMat3f& objectToWorld,
		b32 createCollision )
		: tShapeEntity( entityDef, objectSpaceBox, objectToWorld, createCollision )
		, mSphere( fComputeSphere( ) )
	{
		sigassert( entityDef->mShapeType == tShapeEntityDef::cShapeTypeSphere );
	}

	//------------------------------------------------------------------------------
	tSphereEntity::tSphereEntity( const Math::tSpheref& sphere, b32 createCollision )
		: tShapeEntity( Math::tAabbf( sphere ), tShapeEntityDef::cShapeTypeSphere, createCollision )
		, mSphere( fComputeSphere( ) )
	{
	}

	//------------------------------------------------------------------------------
	void tSphereEntity::fOnMoved( b32 recomputeParentRelative )
	{
		tShapeEntity::fOnMoved( recomputeParentRelative );
		mSphere = fComputeSphere( );
		fUpdateCollision( );
	}

	//------------------------------------------------------------------------------
	void tSphereEntity::fRayCast( const Math::tRayf& ray, Math::tRayCastHit& hit ) const
	{
		if( !fEnabled( ) )
			return;

		Math::tIntersectionRaySphere<f32> test( ray, mSphere );
		if( test.fIntersects( ) )
		{
			hit.mT = test.fT( );
			hit.mN = ( ray.fPointAtTime( hit.mT ) - mSphere.fCenter( ) ).fNormalizeSafe( Math::tVec3f::cYAxis );
		}
	}

	//------------------------------------------------------------------------------
	b32 tSphereEntity::fIntersects( const Math::tFrustumf& v ) const
	{
		if( !fEnabled( ) )
			return false;

		return Math::tIntersectionSphereFrustum<f32>( mSphere, v ).fIntersects( );
	}

	//------------------------------------------------------------------------------
	b32 tSphereEntity::fIntersects( const Math::tAabbf& v ) const
	{
		if( !fEnabled( ) )
			return false;

		return Math::tIntersectionAabbSphere<f32>( v, mSphere ).fIntersects( );
	}

	//------------------------------------------------------------------------------
	b32 tSphereEntity::fIntersects( const Math::tObbf& v ) const
	{
		if( !fEnabled( ) )
			return false;

		return Math::tIntersectionSphereObb<f32>( mSphere, v ).fIntersects( );
	}

	//------------------------------------------------------------------------------
	b32 tSphereEntity::fIntersects( const Math::tSpheref& v ) const
	{
		if( !fEnabled( ) )
			return false;

		return mSphere.fIntersects( v );
	}

	//------------------------------------------------------------------------------
	b32 tSphereEntity::fContains( const Math::tVec3f& point ) const
	{
		if( !fEnabled( ) )
			return false;
		
		return mSphere.fContains( point );
	}

	//------------------------------------------------------------------------------
	b32 tSphereEntity::fContains2D( const Math::tVec3f& point ) const
	{
		if( !fEnabled( ) )
			return false;
		
		return mSphere.fContains2D( point );
	}

	//------------------------------------------------------------------------------
	b32 tSphereEntity::fIntersects( const tShapeEntity& otherShape ) const
	{
		if( !fEnabled( ) )
			return false;

		return ((tSpatialEntity*)&otherShape)->fIntersects( mSphere );
	}

	//------------------------------------------------------------------------------
	Math::tVec3f tSphereEntity::fClosestPoint( const Math::tVec3f& point ) const
	{
		f32 distance; 
		Math::tVec3f delta = point - mSphere.fCenter( );
		delta.fNormalizeSafe( Math::tVec3f::cZeroVector, distance );
		return mSphere.fCenter( ) + delta * fMin( distance, mSphere.fRadius( ) );	
	}

	//------------------------------------------------------------------------------
	Math::tVec3f tSphereEntity::fSupport( const Math::tVec3f& dir ) const
	{
		// Assumes dir is normalized
		return mSphere.mCenter + dir * mSphere.mRadius;
	}

#ifdef sig_devmenu
	//------------------------------------------------------------------------------
	void tSphereEntity::fDebugRender( const Math::tVec4f& color, b32 doBasis ) const
	{
		fSceneGraph( )->fDebugGeometry( ).fRenderOnce( fComputeSphere( ), color );

		if( doBasis )
			fSceneGraph( )->fDebugGeometry( ).fRenderOnce( fObjectToWorld( ), 1.f );
	}
#endif//sig_devmenu

	//------------------------------------------------------------------------------
	Math::tSpheref tSphereEntity::fComputeSphere( ) const
	{
		const Math::tMat3f& xform = fObjectToWorld( );
		return Math::tSpheref(
			xform.fGetTranslation( ),
			fMax( xform.fXAxis( ).fLength( ), 
				  xform.fYAxis( ).fLength( ), 
				  xform.fZAxis( ).fLength( ) ) );
	}

	//------------------------------------------------------------------------------
	// tBoxEntity
	//------------------------------------------------------------------------------
	tBoxEntity::tBoxEntity( 
		const tShapeEntityDef* entityDef, 
		const Math::tAabbf& objectSpaceBox, 
		const Math::tMat3f& objectToWorld,
		b32 createCollision  )
		: tShapeEntity( entityDef, objectSpaceBox, objectToWorld, createCollision )
		, mBox( fComputeBox( ) )
	{
		sigassert( entityDef->mShapeType == tShapeEntityDef::cShapeTypeBox || entityDef->mShapeType == tShapeEntityDef::cShapeTypeCapsule || entityDef->mShapeType == tShapeEntityDef::cShapeTypeCylinder );
	}

	//------------------------------------------------------------------------------
	tBoxEntity::tBoxEntity( const Math::tAabbf& box, b32 createCollision  )
		: tShapeEntity( box, tShapeEntityDef::cShapeTypeBox, createCollision ) 
		, mBox( fComputeBox( ) )
	{
	}

	//------------------------------------------------------------------------------
	void tBoxEntity::fOnMoved( b32 recomputeParentRelative )
	{
		tShapeEntity::fOnMoved( recomputeParentRelative );
		mBox = fComputeBox( );
		fUpdateCollision( );
	}

	//------------------------------------------------------------------------------
	void tBoxEntity::fRayCast( const Math::tRayf& ray, Math::tRayCastHit& hit ) const
	{
		if( !fEnabled( ) )
			return;

		Math::tIntersectionRayObb<f32> test( ray, mBox );
		if( test.fIntersects( ) )
		{
			hit.mT = test.fT( );

			const Math::tVec3f p = ray.fPointAtTime( hit.mT );
			hit.mN = fNormalFromPointOnBox( p, mBox );
		}
	}

	//------------------------------------------------------------------------------
	b32 tBoxEntity::fIntersects( const Math::tFrustumf& v ) const
	{
		if( !fEnabled( ) )
			return false;

		//log_warning( "Box entity needs upgraded frustum test!" );
		return tSpatialEntity::fIntersects( v );
	}

	//------------------------------------------------------------------------------
	b32 tBoxEntity::fIntersects( const Math::tAabbf& v ) const
	{
		if( !fEnabled( ) )
			return false;

		return Math::tIntersectionAabbObb<f32>( mBox, v ).fIntersects( );
	}

	//------------------------------------------------------------------------------
	b32 tBoxEntity::fIntersects( const Math::tObbf& v ) const
	{
		if( !fEnabled( ) )
			return false;

		//log_warning( "Box entity needs upgraded obb test!" );
		return tSpatialEntity::fIntersects( v );
	}

	//------------------------------------------------------------------------------
	b32 tBoxEntity::fIntersects( const Math::tSpheref& v ) const
	{
		if( !fEnabled( ) )
			return false;

		return Math::tIntersectionSphereObb<f32>( mBox, v ).fIntersects( );
	}

	//------------------------------------------------------------------------------
	b32 tBoxEntity::fContains( const Math::tVec3f& point ) const
	{
		if( !fEnabled( ) )
			return false;
		
		return mBox.fContains( point );
	}

	//------------------------------------------------------------------------------
	b32 tBoxEntity::fContains2D( const Math::tVec3f& point ) const
	{
		if( !fEnabled( ) )
			return false;
		
		return mBox.fContains2D( point );
	}

	//------------------------------------------------------------------------------
	b32 tBoxEntity::fIntersects( const tShapeEntity& otherShape ) const
	{
		if( !fEnabled( ) )
			return false;

		return ((tSpatialEntity*)&otherShape)->fIntersects( mBox );
	}

	//------------------------------------------------------------------------------
	Math::tVec3f tBoxEntity::fClosestPoint( const Math::tVec3f& point ) const
	{
		return mBox.fClosestPoint( point );	
	}

	//------------------------------------------------------------------------------
	Math::tVec3f tBoxEntity::fSupport( const Math::tVec3f& dir ) const
	{
		// Assumes dir is normalized
		return mBox.mCenter 
			+ mBox.mAxes[0] * mBox.mExtents[0] * mBox.mAxes[0].fDot( dir )
			+ mBox.mAxes[1] * mBox.mExtents[1] * mBox.mAxes[1].fDot( dir )
			+ mBox.mAxes[2] * mBox.mExtents[2] * mBox.mAxes[2].fDot( dir );
	}

#ifdef sig_devmenu
	//------------------------------------------------------------------------------
	void tBoxEntity::fDebugRender( const Math::tVec4f& color, b32 doBasis ) const
	{
		fSceneGraph( )->fDebugGeometry( ).fRenderOnce( fComputeBox( ), color );

		if( doBasis )
			fSceneGraph( )->fDebugGeometry( ).fRenderOnce( fObjectToWorld( ), 1.f );
	}
#endif//sig_devmenu

	//------------------------------------------------------------------------------
	Math::tObbf tBoxEntity::fComputeBox( ) const
	{
		return Math::tObbf( Math::tAabbf( -1.f, +1.f ), fObjectToWorld( ) );
	}


	//------------------------------------------------------------------------------
	tConvexHullEntity::tConvexHullEntity( const tShapeEntityDef* entityDef, const Math::tAabbf& objectSpaceBox, const Math::tMat3f& objectToWorld, b32 createCollision )
		: tShapeEntity( entityDef, objectSpaceBox, objectToWorld, createCollision )
	{ }

	//------------------------------------------------------------------------------
	b32 tConvexHullEntity::fContains( const Math::tVec3f& point ) const 
	{ 
		sigassert( mEntityDef->mHull );
		return mEntityDef->mHull->fContainsPtGJK( fObjectToWorld( ), point ); 
	}

	//------------------------------------------------------------------------------
	b32 tConvexHullEntity::fContains2D( const Math::tVec3f& point ) const 
	{ 
		sigassert( !"tConvexHullEntity::fContains2D NOT SUPPORTED. DO NOT CALL THIS FUNCTION" );
		return false;
	}

#ifdef sig_devmenu
	//------------------------------------------------------------------------------
	void tConvexHullEntity::fDebugRender( const Math::tVec4f& color, b32 doBasis ) const
	{
		sigassert( mEntityDef->mHull );
		fSceneGraph( )->fDebugGeometry( ).fRenderOnce( *mEntityDef->mHull, fObjectToWorld( ), color );

		if( doBasis )
			fSceneGraph( )->fDebugGeometry( ).fRenderOnce( fObjectToWorld( ), 1.f );
	}
#endif//sig_devmenu


} // ::Sig

namespace Sig
{
	namespace
	{
		static tShapeEntity * fConvertToShapeEntity( tEntity * ent )
		{
			sigassert( ent );
			tShapeEntity * shapeEnt = ent->fDynamicCast<tShapeEntity>( );
			return shapeEnt;
		}
		static tSphereEntity * fConvertToSphereEntity( tEntity * ent )
		{
			sigassert( ent );
			tSphereEntity * sphereEnt = ent->fDynamicCast<tSphereEntity>( );
			return sphereEnt;
		}
	}

	//------------------------------------------------------------------------------
	void tShapeEntity::fExportScriptInterface( tScriptVm& vm )
	{
		// Shape
		{
			Sqrat::DerivedClass<tShapeEntity, tEntity, Sqrat::NoConstructor> classDesc( vm.fSq( ) );
			classDesc
				.Prop(_SC("ShapeType"),		&tShapeEntity::fShapeType)
				.Prop(_SC("Enabled"),		&tShapeEntity::fEnabled, &tShapeEntity::fSetEnabled)
				.StaticFunc(_SC("Convert"), &fConvertToShapeEntity)
				;
			vm.fRootTable( ).Bind(_SC("ShapeEntity"), classDesc);
		}

		// Sphere
		{
			Sqrat::DerivedClass<tSphereEntity, tShapeEntity, Sqrat::NoConstructor> classDesc( vm.fSq( ) );
			classDesc
				.Prop(_SC("Center"), &tSphereEntity::fCenter)
				.Prop(_SC("Radius"), &tSphereEntity::fRadius)
				.StaticFunc(_SC("Convert"), &fConvertToSphereEntity)
				;
			vm.fRootTable( ).Bind(_SC("SphereEntity"), classDesc);

		}

		// Box
		{
			Sqrat::DerivedClass<tBoxEntity, tShapeEntity, Sqrat::NoConstructor> classDesc( vm.fSq( ) );
			classDesc
				;
			vm.fRootTable( ).Bind(_SC("BoxEntity"), classDesc);
		}



		vm.fConstTable( ).Const( "SHAPE_ENTITY_BOX", ( int )tShapeEntityDef::cShapeTypeBox );
		vm.fConstTable( ).Const( "SHAPE_ENTITY_SPHERE", ( int )tShapeEntityDef::cShapeTypeSphere );
	}
} // ::Sig
