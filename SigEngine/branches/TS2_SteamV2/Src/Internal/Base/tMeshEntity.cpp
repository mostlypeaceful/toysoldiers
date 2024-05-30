#include "BasePch.hpp"
#include "tMeshEntity.hpp"
#include "tShapeEntity.hpp"
#include "tMesh.hpp"
#include "tSceneGraph.hpp"

// graphics
#include "Gfx/tGeometryFile.hpp"
#include "Gfx/tMaterial.hpp"

namespace Sig
{
	devvar( bool, Debug_SceneGraph_BVH_RebuildKDTreesOnLoad, false );

	tMeshEntityDef::tMeshEntityDef( )
		: mMesh( 0 )
		, mStateIndex( 0 )
		, mStateType( cStateTypeState )
		, pad0( 0 )
		, mSortOffset( 0.f )
	{
	}

	tMeshEntityDef::tMeshEntityDef( tNoOpTag )
		: tEntityDef( cNoOpTag )
	{
	}

	tMeshEntityDef::~tMeshEntityDef( )
	{
	}

	void tMeshEntityDef::fOnFileLoaded( )
	{
		tEntityDef::fOnFileLoaded( );
		mMesh->fOnFileLoaded( );
	}

	void tMeshEntityDef::fOnFileUnloading( )
	{
		tEntityDef::fOnFileUnloading( );
		mMesh->fOnFileUnloading( );
	}

	void tMeshEntityDef::fCollectEntities( tEntity& parent, const tEntityCreationFlags& creationFlagsParent ) const
	{
		const tEntityCreationFlags creationFlags = creationFlagsParent | fCreationFlags( );

		const tResourcePtr& geoResource = mMesh->mGeometryFile->fGetResourcePtr( );
		const Gfx::tGeometryFile* geoFile = geoResource->fCast<Gfx::tGeometryFile>( );
		sigassert( geoFile );

		for( u32 isubmesh = 0; isubmesh < mMesh->mSubMeshes.fCount( ); ++isubmesh )
		{
			const tSubMesh& sb = mMesh->mSubMeshes[ isubmesh ];

			Gfx::tRenderBatchData batch;
			batch.mRenderState = &sb.mMaterial->fGetRenderState( );
			batch.mMaterial = sb.mMaterial;
			batch.mVertexFormat = sb.mVertexFormat;
			batch.mGeometryBuffer = &geoFile->mGeometryPointers[ sb.mGeometryBufferIndex ].mVRamBuffer;
			batch.mIndexBuffer = &geoFile->mIndexListPointers[ sb.mIndexBufferIndex ].mVRamBuffer;
			batch.mVertexCount = batch.mGeometryBuffer->fVertexCount( );
			batch.mBaseVertexIndex = 0;
			batch.mPrimitiveCount = batch.mIndexBuffer->fPrimitiveCount( );
			batch.mBaseIndexIndex = 0;
			batch.mPrimitiveType = batch.mIndexBuffer->fIndexFormat( ).mPrimitiveType;

			tMeshEntity* entity = NEW tMeshEntity( Gfx::tRenderBatch::fCreate( batch ), this, &sb, mBounds, mObjectToLocal );
			fApplyProperties( *entity, creationFlags );
			entity->fSpawn( parent );
			entity->fAcquirePropertiesFromAncestors( );
		}
	}

	tMeshEntity::tMeshEntity( 
		const Gfx::tRenderBatchPtr& batchPtr,
		const tMeshEntityDef* entityDef,
		const tSubMesh* subMesh,
		const Math::tAabbf& objectSpaceBox,
		const Math::tMat3f& objectToWorld )
		: Gfx::tRenderableEntity( batchPtr, objectSpaceBox )
		, mEntityDef( entityDef )
		, mSubMesh( subMesh )
		, mStateChangeEnabled( true )
	{
		fSetStateMask( (1<<mEntityDef->mStateIndex) );

		fSetCameraDepthOffset( mEntityDef->mSortOffset );

		if( mEntityDef->mStateType == tMeshEntityDef::cStateTypeTransition || mEntityDef->mStateIndex > 0 )
			fSetDisabled( true );

		if( mEntityDef->mMesh->fIsSkinned( ) )
			mSkinMap.fReset( NEW tSkinMap( mEntityDef->mMesh ) );

		fMoveTo( objectToWorld );

		if( Debug_SceneGraph_BVH_RebuildKDTreesOnLoad )
			const_cast<tSubMesh*>( mSubMesh )->mPolySoupKDTree.fConstruct( mSubMesh->mTriangles, mSubMesh->mVertices, true );
	}

	void tMeshEntity::fCollectTris( const Math::tObbf& obb, tGrowableArray<Math::tTrianglef>& trisOut ) const
	{
		if( fIgnore( ) ) return;

		const u32 startTransformingAt = trisOut.fCount( );

		mSubMesh->fCollectTris( obb.fTransform( fWorldToObject( ) ).fToAabb( ), trisOut );

		// TODO cull tris that aren't in obb

		for( u32 i = startTransformingAt; i < trisOut.fCount( ); ++i )
			trisOut[ i ] = trisOut[ i ].fTransform( fObjectToWorld( ) );
	}

	void tMeshEntity::fCollectTris( const Math::tAabbf& aabb, tGrowableArray<Math::tTrianglef>& trisOut ) const
	{
		if( fIgnore( ) ) return;

		const u32 startTransformingAt = trisOut.fCount( );

		mSubMesh->fCollectTris( aabb.fTransform( fWorldToObject( ) ), trisOut );

		for( u32 i = startTransformingAt; i < trisOut.fCount( ); ++i )
			trisOut[ i ] = trisOut[ i ].fTransform( fObjectToWorld( ) );
	}

	void tMeshEntity::fRayCast( const Math::tRayf& ray, Math::tRayCastHit& hit ) const
	{
		if( fIgnore( ) ) return;

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

	b32 tMeshEntity::fIntersects( const Math::tFrustumf& v ) const
	{
		if( fIgnore( ) ) return false;

		Math::tFrustumf vInObject;
		fToObjectSpace( vInObject, v );
		return mSubMesh->fTestFrustum( vInObject );
	}

	b32	tMeshEntity::fIntersects( const Math::tAabbf& v ) const
	{
		if( fIgnore( ) ) return false;
		return Gfx::tRenderableEntity::fIntersects( v );
	}

	b32 tMeshEntity::fIntersects( const Math::tObbf& v ) const
	{
		if( fIgnore( ) ) return false;
		return Gfx::tRenderableEntity::fIntersects( v );
	}

	b32 tMeshEntity::fIntersects( const Math::tSpheref& v ) const
	{
		if( fIgnore( ) ) return false;
		return Gfx::tRenderableEntity::fIntersects( v );
	}

	void tMeshEntity::fPropagateSkeleton( tAnimatedSkeleton& skeleton )
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
		struct tComputeMeshStateCount
		{
			mutable s16 mHighestStateState;
			mutable s16 mHighestTransitionState;
			tComputeMeshStateCount( ) : mHighestStateState( -1 ), mHighestTransitionState( -1 ) { }
			b32 operator()( tEntity& entity ) const
			{
				tMeshEntity* meshEnt = entity.fDynamicCast< tMeshEntity >( );
				if( !meshEnt ) return false;

				if( meshEnt->fEntityDef( ).mStateType == tMeshEntityDef::cStateTypeTransition )
					mHighestTransitionState = fMax<s16>( mHighestTransitionState, meshEnt->fEntityDef( ).mStateIndex );
				else
					mHighestStateState = fMax<s16>( mHighestStateState, meshEnt->fEntityDef( ).mStateIndex );
				return false;
			}
		};
		static void fCombinedObjectSpaceBoxRecursive( const tEntity& ent, const Math::tMat3f& toObjectSpace, Math::tAabbf& box )
		{
			tMeshEntity* spatialEnt = ent.fDynamicCast<tMeshEntity>( );
			if( spatialEnt && !spatialEnt->fDisabled( ) )
				box |= spatialEnt->fObjectSpaceBox( ).fTransform( toObjectSpace * ent.fObjectToWorld( ) );

			for( u32 i = 0; i < ent.fChildCount( ); ++i )
				fCombinedObjectSpaceBoxRecursive( *ent.fChild( i ), toObjectSpace, box );
		}
	}

	void tMeshEntity::fStateCount( tEntity& parent, s32& statesOut, s32& transitionsOut )
	{
		tComputeMeshStateCount counter;
		parent.fForEachDescendent( counter );

		statesOut = counter.mHighestStateState;
		transitionsOut = counter.mHighestTransitionState;
	}

	void tMeshEntity::fEnableStateChanges( tEntity& entity, b32 enable )
	{
		tMeshEntity* meshEnt = entity.fDynamicCast< tMeshEntity >( );
		if( meshEnt )
			meshEnt->fSetStateChangeEnabled( enable );
		for( u32 i = 0; i < entity.fChildCount( ); ++i )
			fEnableStateChanges( *entity.fChild( i ), enable );
	}

	Math::tAabbf tMeshEntity::fCombinedObjectSpaceBox( tEntity& parent )
	{
		Math::tAabbf box;
		box.fInvalidate( );
		fCombinedObjectSpaceBoxRecursive( parent, parent.fWorldToObject( ), box );
		return box;
	}

	Math::tAabbf tMeshEntity::fCombinedWorldSpaceBox( tEntity& parent )
	{
		return fCombinedObjectSpaceBox( parent ).fTransform( parent.fObjectToWorld( ) );
	}

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
	{
	}

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
			if( entityDef.mStateIndex == 0 )
				mTransitionPieces.fPushBack( tMeshEntityPtr( meshEntity ) );
		}
		else
		{
			if( meshEntity->fEntityDef( ).mStateType == tMeshEntityDef::cStateTypeState )
			{
				mHighestState = fMax<s16>( mHighestState, entityDef.mStateIndex );
				meshEntity->fStateMaskEnable( mNewStateIndex );
			}
			else if( !mJustChangeState )
			{
				sigassert( entityDef.mStateType == tMeshEntityDef::cStateTypeTransition );

				//original behavior   //if( entityDef.mStateIndex == mPrevStateIndex ) 
				
				if( mCollectAllTransition )
				{
					if( fInBounds<s16>( entityDef.mStateIndex, mPrevStateIndex, mNewStateIndex -1 ) ) 
						mTransitionPieces.fPushBack( tMeshEntityPtr( meshEntity ) );
				}
				else
				{
					// minimal, just the state before the current, behavior
					if( entityDef.mStateIndex == mNewStateIndex - 1 )
						mTransitionPieces.fPushBack( tMeshEntityPtr( meshEntity ) );
				}
			}
		}

		return true;
	}

	void tMeshEntity::tChangeMeshState::fChangeStateRecursive( tEntity& root, b32 firstChange ) const
	{
		if( fProcess( root ) || firstChange )
		{
			for( u32 i = 0; i < root.fChildCount( ); ++i )
				fChangeStateRecursive( *root.fChild( i ), false );
		}
	}

	void tMeshEntity::tChangeMeshState::fChangeState( tEntity& root ) const
	{
		fChangeStateRecursive( root, true );

		if( !mJustChangeState && mAllowDefaultTransition && mHighestState == 0 && mTransitionPieces.fCount( ) == 0 )
		{
			// go back through and get all default transition pieces
			mGatherDefaultTransitionPieces = true;
			fChangeStateRecursive( root, true );
		}
	}

	void tMeshEntity::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tMeshEntity,Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		classDesc
			.StaticFunc(_SC("CombinedWorldSpaceBox"),	&tMeshEntity::fCombinedWorldSpaceBox)
			;

		vm.fRootTable( ).Bind( _SC("MeshEntity"), classDesc );
	}

}
