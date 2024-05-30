//------------------------------------------------------------------------------
// \file tMeshEntity.cpp - 15 Aug 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tMeshEntity.hpp"
#include "tShapeEntity.hpp"
#include "tMesh.hpp"
#include "tSceneGraph.hpp"
#include "tSceneRefEntity.hpp"
#include "Physics/tCollisionShapes.hpp"
#include "Physics/tPhysicsWorld.hpp"

// graphics
#include "Gfx/tGeometryFile.hpp"
#include "Gfx/tMaterial.hpp"

namespace Sig
{
	devvar( bool, Debug_SceneGraph_BVH_RebuildKDTreesOnLoad, false );

	//------------------------------------------------------------------------------
	// tMeshEntityDef
	//------------------------------------------------------------------------------
	tMeshEntityDef::tMeshEntityDef( )
		: mMesh( 0 )
		, mStateMask( (1<<0) )
		, mStateType( cStateTypeState )
		, pad0( 0 )
		, mSortOffset( 0.f )
	{
	}

	//------------------------------------------------------------------------------
	tMeshEntityDef::tMeshEntityDef( tNoOpTag )
		: tEntityDef( cNoOpTag )
	{
	}

	//------------------------------------------------------------------------------
	tMeshEntityDef::~tMeshEntityDef( )
	{
	}

	//------------------------------------------------------------------------------
	void tMeshEntityDef::fOnFileLoaded( )
	{
		tEntityDef::fOnFileLoaded( );
		mMesh->fOnFileLoaded( );
	}

	//------------------------------------------------------------------------------
	void tMeshEntityDef::fOnFileUnloading( )
	{
		tEntityDef::fOnFileUnloading( );
		mMesh->fOnFileUnloading( );
	}

	//------------------------------------------------------------------------------
	void tMeshEntityDef::fCollectEntities( const tCollectEntitiesParams& paramsParent ) const
	{
		tEntityCreationFlags creationFlags = paramsParent.mCreationFlags | fCreationFlags( );

		const b32 wantsCollision = creationFlags.fCreateCollision( );

		for( u32 isubmesh = 0; isubmesh < mMesh->mSubMeshes.fCount( ); ++isubmesh )
		{
			const tSubMesh& sb = mMesh->mSubMeshes[ isubmesh ];

			// Creation stuff
			sigassert( paramsParent.mOwnerResource.fGetRawPtr( ) && "tMeshEntity needs to hold on to a resource!" );
			tMeshEntity* entity = NEW tMeshEntity( Gfx::tRenderBatchPtr( ), this, &sb, mBounds, mObjectToLocal, creationFlags.mVisibilitySet.fVisibilitySet( ), paramsParent.mOwnerResource.fGetRawPtr( ) );
			fApplyProperties( *entity, creationFlags );
			entity->fSpawn( paramsParent.mParent );
			entity->fAcquirePropertiesFromAncestors( );

			// Post creation stuff
			fInitLOD( entity );
			if( wantsCollision && entity->fStateEnabled( 0 ) )
				entity->fSetCollision( NEW Physics::tCollisionShapeMesh( *entity ) );
		}
	}

	//------------------------------------------------------------------------------
	void tMeshEntityDef::fSelectLOD( Gfx::tRenderableEntity* entity, f32 ratio, b32 shadows, b32 normals ) const
	{
		tMeshEntity* mesh = entity->fStaticCast<tMeshEntity>( );
		Gfx::tGeometryFile * geoFile = mMesh->mGeometryFile->fGetResourcePtr( )->fCast<Gfx::tGeometryFile>( );
		sigassert( geoFile );

		const tSubMesh& sb = *mesh->fSubMesh( );
		sigassert( sb.mGeometryBufferIndex == sb.mIndexBufferIndex );

		tEntityDef::fSetLOD( entity, geoFile, sb.mGeometryBufferIndex, 0, sb.mMaterial, ratio, shadows, normals );
	}


	//------------------------------------------------------------------------------
	// tMeshEntity
	//------------------------------------------------------------------------------
	tMeshEntity::tMeshEntity( 
		const Gfx::tRenderBatchPtr& batchPtr,
		const tMeshEntityDef* entityDef,
		const tSubMesh* subMesh,
		const Math::tAabbf& objectSpaceBox,
		const Math::tMat3f& objectToWorld,
		Gfx::tVisibilitySetRef& visibility,
		tResource* resource )
		: Gfx::tRenderableEntity( batchPtr, objectSpaceBox )
		, mEntityDef( entityDef )
		, mSubMesh( subMesh )
		, mStateChangeEnabled( true )
		, mFromResource( resource )
	{
		fSetStateMask( mEntityDef->mStateMask );

		fSetCameraDepthOffset( mEntityDef->mSortOffset );

		if( mEntityDef->mStateType == tMeshEntityDef::cStateTypeTransition || !fStateEnabled( 0 ) )
			fSetDisabled( true );

		if( mEntityDef->mMesh->fIsSkinned( ) )
			mSkinMap.fReset( NEW tSkinMap( mEntityDef->mMesh ) );

		fMoveTo( objectToWorld );

		if( Debug_SceneGraph_BVH_RebuildKDTreesOnLoad )
			const_cast<tSubMesh*>( mSubMesh )->mPolySoupKDTree.fConstruct( mSubMesh->mTriangles, mSubMesh->mVertices, true );

		mVisibilitySet.fReset( &visibility );
	}

	//------------------------------------------------------------------------------
	tMeshEntity::~tMeshEntity( )
	{

	}

	//------------------------------------------------------------------------------
	void tMeshEntity::fSetCollision( Physics::tCollisionShape* collision )
	{
		if( mCollisionShape ) mCollisionShape->fUserData( ).fRelease( );
		mCollisionShape.fReset( collision );
		if( mCollisionShape ) mCollisionShape->fUserData( ).fReset( this );
	}

	//------------------------------------------------------------------------------
	void tMeshEntity::fCollectTris( const Math::tObbf& obb, tGrowableArray<Math::tTrianglef>& trisOut ) const
	{
		const u32 startTransformingAt = trisOut.fCount( );

		mSubMesh->fCollectTris( obb.fTransform( fWorldToObject( ) ).fToAabb( ), trisOut );

		// TODO cull tris that aren't in obb

		for( u32 i = startTransformingAt; i < trisOut.fCount( ); ++i )
			trisOut[ i ] = trisOut[ i ].fTransform( fObjectToWorld( ) );
	}

	//------------------------------------------------------------------------------
	void tMeshEntity::fCollectTris( const Math::tAabbf& aabb, tGrowableArray<Math::tTrianglef>& trisOut ) const
	{
		const u32 startTransformingAt = trisOut.fCount( );

		mSubMesh->fCollectTris( aabb.fTransform( fWorldToObject( ) ), trisOut );

		for( u32 i = startTransformingAt; i < trisOut.fCount( ); ++i )
			trisOut[ i ] = trisOut[ i ].fTransform( fObjectToWorld( ) );
	}

	//------------------------------------------------------------------------------
	void tMeshEntity::fRayCast( const Math::tRayf& ray, Math::tRayCastHit& hit ) const
	{
		tPolySoupRayCastHit polySoupHit;
		const Math::tMat3f & worldToObject = fWorldToObject( );
		const Math::tRayf rayInLocal = ray.fTransform( worldToObject );

		mSubMesh->fTestRay( rayInLocal, polySoupHit );

		if( polySoupHit.fHit( ) )
		{
			hit = polySoupHit;

			// transform normal back to world
			hit.mN = fObjectToWorld( ).fXformVector( hit.mN );

			sync_event_v_c( rayInLocal.mExtent, tSync::cSCRaycast );
			sync_event_v_c( rayInLocal.mOrigin, tSync::cSCRaycast );
		}
	}

	//------------------------------------------------------------------------------

	//devvar_p( f32, Lod_MeshNormalDrop, 100.f, 1 );
	devvar_p( f32, Lod_MeshShadowDrop, 500.f, 1 );
	devvar_p( f32, Lod_MeshHigh, 25.f, 1 );
	devvar_p( f32, Lod_MeshLow, 250.f, 1 );

	u32 tMeshEntity::gLODForceMesh = 0;
	devvarptr_clamp( u32, Lod_ForceMesh, tMeshEntity::gLODForceMesh, 0, 3, 0 );

	//------------------------------------------------------------------------------
	void tMeshEntity::fUpdateLOD( const Math::tVec3f & eye )
	{
		if( fForceHighLOD( ) )
		{
			mEntityDef->fSelectLOD( this, 1.0f, true, true );
			return;
		}

		f32 d = fWorldSpaceBox( ).fDistanceToPoint( eye );

		const f32 minD = ( mLODMediumOverride != 0.f ) ? mLODMediumOverride : Lod_MeshHigh;
		const f32 maxD = ( mLODFarOverride != 0.f ) ? mLODFarOverride : Lod_MeshLow;

		d = fClamp( d, minD, maxD );

		if( tEntityDef::fLODDistChanged( mLastLODDistance, d ) || gLODForceMesh != 0 )
		{
			mLastLODDistance = d;

			f32 ratio = 1.f - ( ( d - minD ) / ( maxD - minD ) );

			if( gLODForceMesh == 1 ) ratio = 0.0f;
			else if( gLODForceMesh == 2 ) ratio = 0.5f;
			else if( gLODForceMesh == 3 ) ratio = 1.0f;

#ifdef target_tools 
			if( mLODMediumOverride == 0.f && mLODFarOverride == 0.f )
				ratio = 1.f;
#endif

			mEntityDef->fSelectLOD( this, ratio, ratio <= Lod_MeshShadowDrop, true/*ratio <= Lod_MeshNormalDrop*/ );
		}
	}

	//------------------------------------------------------------------------------
	void tMeshEntity::fPropagateSkeleton( Anim::tAnimatedSkeleton& skeleton )
	{
		sigassert( mEntityDef && mEntityDef->mMesh );
		if( mEntityDef->mMesh->fIsSkinned( ) )
			mSkinMap.fReset( NEW tSkinMap( skeleton, mEntityDef->mMesh ) );
		else
			mSkinMap.fRelease( );

		tEntity::fPropagateSkeletonInternal( skeleton, mEntityDef );
	}

	namespace
	{
		///
		/// \class tComputeMeshStateCount
		/// \brief 
		struct tComputeMeshStateCount
		{
			mutable s16 mHighestStateState;
			mutable s16 mHighestTransitionState;
			tComputeMeshStateCount( ) : mHighestStateState( -1 ), mHighestTransitionState( -1 ) { }
			b32 operator()( tEntity& entity ) const
			{
				tMeshEntity* meshEnt = entity.fDynamicCast< tMeshEntity >( );
				if( !meshEnt ) return false;

				s32 highestIndex = fHighestSetBitIndex( meshEnt->fEntityDef( ).mStateMask );
				if( meshEnt->fEntityDef( ).mStateType == tMeshEntityDef::cStateTypeTransition )
					mHighestTransitionState = fMax<s16>( mHighestTransitionState, highestIndex );
				else
					mHighestStateState = fMax<s16>( mHighestStateState, highestIndex );
				return false;
			}
		};

		//------------------------------------------------------------------------------
		static void fCombinedObjectSpaceBoxRecursive( const tEntity& ent, const Math::tMat3f& toObjectSpace, Math::tAabbf& box )
		{
			tMeshEntity* spatialEnt = ent.fDynamicCast<tMeshEntity>( );
			if( spatialEnt && !spatialEnt->fDisabled( ) )
				box |= spatialEnt->fObjectSpaceBox( ).fTransform( toObjectSpace * ent.fObjectToWorld( ) );

			for( u32 i = 0; i < ent.fChildCount( ); ++i )
				fCombinedObjectSpaceBoxRecursive( *ent.fChild( i ), toObjectSpace, box );
		}
	}

	//------------------------------------------------------------------------------
	void tMeshEntity::fStateCount( tEntity& parent, s32& statesOut, s32& transitionsOut )
	{
		tComputeMeshStateCount counter;
		parent.fForEachDescendent( counter );

		statesOut = counter.mHighestStateState;
		transitionsOut = counter.mHighestTransitionState;
	}

	//------------------------------------------------------------------------------
	void tMeshEntity::fEnableStateChanges( tEntity& entity, b32 enable )
	{
		tMeshEntity* meshEnt = entity.fDynamicCast< tMeshEntity >( );
		if( meshEnt )
			meshEnt->fSetStateChangeEnabled( enable );
		for( u32 i = 0; i < entity.fChildCount( ); ++i )
			fEnableStateChanges( *entity.fChild( i ), enable );
	}

	//------------------------------------------------------------------------------
	Math::tAabbf tMeshEntity::fCombinedObjectSpaceBox( tEntity& parent )
	{
		Math::tAabbf box;
		box.fInvalidate( );
		fCombinedObjectSpaceBoxRecursive( parent, parent.fWorldToObject( ), box );
		return box;
	}

	//------------------------------------------------------------------------------
	Math::tAabbf tMeshEntity::fCombinedWorldSpaceBox( tEntity& parent )
	{
		return fCombinedObjectSpaceBox( parent ).fTransform( parent.fObjectToWorld( ) );
	}

	//------------------------------------------------------------------------------
	void tMeshEntity::fOnSpawn( )
	{
		sigassert( mSubMesh );

		if( mCollisionShape )
			fSceneGraph( )->fPhysics( )->fMainStaticBody( )->fAddShape( mCollisionShape.fGetRawPtr( ) );

		// Puts renderable into spatial set
		tRenderableEntity::fOnSpawn( );
	}

	//------------------------------------------------------------------------------
	void tMeshEntity::fOnDelete( )
	{
		tRenderableEntity::fOnDelete( );

		if( mCollisionShape )
		{
			mCollisionShape->fRemoveFromOwner( );
			mCollisionShape.fRelease( );
		}
	}

	//------------------------------------------------------------------------------
	// tMeshEntity::tChangeMeshState
	//------------------------------------------------------------------------------
	tMeshEntity::tChangeMeshState::tChangeMeshState( s16 prevIdx, s16 newIdx, b32 justChangeState, b32 allowDefaultTransition, u32 ignoreFlagsMask, b32 ignoreLogics )
		: mPrevStateIndex( prevIdx )
		, mNewStateIndex( newIdx )
		, mHighestState( 0 )
		, mGatherDefaultTransitionPieces( false )
		, mJustChangeState( justChangeState )
		, mAllowDefaultTransition( allowDefaultTransition )
		, mIgnoreFlagsMask( ignoreFlagsMask )
		, mIgnoreLogics( ignoreLogics )
		, mCollectAllTransition( false )
		, mCollectTransitionMask( 0 )
	{
	}

	//------------------------------------------------------------------------------
	b32 tMeshEntity::tChangeMeshState::fProcess( tEntity& entity ) const
	{
		if( mIgnoreLogics && entity.fHasLogic( ) ) 
			return false;

		const b32 ignore = entity.fHasGameTagsAny( mIgnoreFlagsMask );

		tMeshEntity* meshEntity = entity.fDynamicCast< tMeshEntity >( );
		if( !meshEntity )
		{
			if( ignore )
				return false;

			if( !mGatherDefaultTransitionPieces )
			{
				tStateableEntity* stateable = entity.fDynamicCast< tStateableEntity >( );
				if( stateable )
					stateable->fStateMaskEnable( mNewStateIndex );
			}

			return true; // keep going if we were not ignored
		}
			
		// we are a mesh
		if( ignore || !meshEntity->fStateChangeEnabled( ) )
			return true;

		const tMeshEntityDef& entityDef = meshEntity->fEntityDef( );

		if( mGatherDefaultTransitionPieces )
		{
			if( entityDef.fStateIndexSet( 0 ) )
				mTransitionPieces.fPushBack( tMeshEntityPtr( meshEntity ) );
		}
		else
		{
			if( meshEntity->fEntityDef( ).mStateType == tMeshEntityDef::cStateTypeState )
			{
				mHighestState = fMax<s16>( mHighestState, fHighestSetBitIndex( entityDef.mStateMask ) );
				meshEntity->fStateMaskEnable( mNewStateIndex );
			}
			else if( !mJustChangeState )
			{
				sigassert( entityDef.mStateType == tMeshEntityDef::cStateTypeTransition );

				//original behavior   //if( entityDef.mStateIndex == mPrevStateIndex ) 
				
				if( entityDef.mStateMask & mCollectTransitionMask ) 
					mTransitionPieces.fPushBack( tMeshEntityPtr( meshEntity ) );
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------
	void tMeshEntity::tChangeMeshState::fChangeStateRecursive( tEntity& root, b32 firstChange ) const
	{
		if( fProcess( root ) || firstChange )
		{
			for( u32 i = 0; i < root.fChildCount( ); ++i )
				fChangeStateRecursive( *root.fChild( i ), false );
		}
	}

	//------------------------------------------------------------------------------
	void tMeshEntity::tChangeMeshState::fChangeState( tEntity& root ) const
	{
		if( mCollectAllTransition )
		{
			mCollectTransitionMask = 0;
			for( s32 i = mPrevStateIndex; i <= mNewStateIndex - 1; ++i )
				mCollectTransitionMask |= (1<<i);
		}
		else if( mNewStateIndex )
		{
			// minimal, just the state before the current, behavior
			mCollectTransitionMask = (1<< (mNewStateIndex - 1));
		}

		fChangeStateRecursive( root, true );

		if( !mJustChangeState && mAllowDefaultTransition && mHighestState == 0 && mTransitionPieces.fCount( ) == 0 )
		{
			// go back through and get all default transition pieces
			mGatherDefaultTransitionPieces = true;
			fChangeStateRecursive( root, true );
		}
	}

	//------------------------------------------------------------------------------
	void tMeshEntity::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tMeshEntity,Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		classDesc
			.StaticFunc(_SC("CombinedWorldSpaceBox"),	&tMeshEntity::fCombinedWorldSpaceBox)
			;

		vm.fRootTable( ).Bind( _SC("MeshEntity"), classDesc );
	}

}
