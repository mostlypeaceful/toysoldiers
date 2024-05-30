//------------------------------------------------------------------------------
// \file tHeightFieldMeshEntity.cpp - 15 Aug 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tHeightFieldMeshEntity.hpp"
#include "tSceneGraph.hpp"
#include "Physics/tCollisionShapes.hpp"
#include "Physics/tPhysicsWorld.hpp"

#include "Gfx/tMaterial.hpp"
#include "Gfx/tGeometryFile.hpp"

namespace Sig
{

	//------------------------------------------------------------------------------
	// tHeightFieldMeshEntityDef
	//------------------------------------------------------------------------------
	tHeightFieldMeshEntityDef::tHeightFieldMeshEntityDef( )
		: tHeightFieldMesh( Math::tVec2f( 256.f, 256.f ), Math::tVec2i( 256, 256 ) )
		, mGeometryFile( 0 )
		, mMaterial( 0 )
	{
	}

	//------------------------------------------------------------------------------
	tHeightFieldMeshEntityDef::tHeightFieldMeshEntityDef( const Math::tVec2f& worldSpaceLengths, const Math::tVec2i& vertexRes )
		: tHeightFieldMesh( worldSpaceLengths, vertexRes )
		, mGeometryFile( 0 )
		, mMaterial( 0 )
	{
	}

	//------------------------------------------------------------------------------
	tHeightFieldMeshEntityDef::tHeightFieldMeshEntityDef( tNoOpTag )
		: tHeightFieldMesh( cNoOpTag )
		, mChunks( cNoOpTag )
		, mGroundCoverDefs( cNoOpTag )
	{
	}

	//------------------------------------------------------------------------------
	tHeightFieldMeshEntityDef::~tHeightFieldMeshEntityDef( )
	{
		delete mMaterial;
	}

	//------------------------------------------------------------------------------
	void tHeightFieldMeshEntityDef::fCollectEntities( const tCollectEntitiesParams& paramsParent ) const
	{
		const tEntityCreationFlags creationFlags = paramsParent.mCreationFlags | fCreationFlags( );

		tEntity* meshReferenceFrame = NEW tHeightFieldMeshReferenceFrameEntity( this, mObjectToLocal );

		// children are spawned before main spawn
		for( u32 m = 0; m < mChunks.fCount( ); ++m )
		{
			const Math::tAabbf objectSpaceBox = mChunks[ m ].mBounds;
			tHeightFieldMeshEntity* entity = NEW tHeightFieldMeshEntity( m, Gfx::tRenderBatchPtr( ), this, objectSpaceBox );
			fInitLOD( entity );

			fApplyProperties( *entity, creationFlags );
			entity->fSpawn( *meshReferenceFrame );
		}

		fApplyPropsAndSpawnWithScript( *meshReferenceFrame, tCollectEntitiesParams( paramsParent.mParent, creationFlags ) );

		// Collect ground cover clouds
		const u32 gcCount = mGroundCoverDefs.fCount( );
		for( u32 gc = 0; gc < gcCount; ++gc )
		{
			Gfx::tGroundCoverCloud * entity = 
				NEW Gfx::tGroundCoverCloud( &mGroundCoverDefs[ gc ] );

			fApplyProperties( *entity, creationFlags );
			entity->fSpawn( *meshReferenceFrame );
		}
	}

	//------------------------------------------------------------------------------
	void tHeightFieldMeshEntityDef::fSelectLOD( Gfx::tRenderableEntity* entity, f32 ratio, b32 shadows, b32 normals ) const
	{
		tHeightFieldMeshEntity* mesh = entity->fStaticCast<tHeightFieldMeshEntity>( );
		Gfx::tGeometryFile* geoFile = mGeometryFile->fGetResourcePtr( )->fCast<Gfx::tGeometryFile>( );
		sigassert( geoFile );

		const u32 chunkIndex = mesh->fChunkIndex( );
		const u32 userFlags = mesh->fUserFlags( );
		tEntityDef::fSetLOD( entity, geoFile, chunkIndex, userFlags, mMaterial, ratio, shadows, normals );
	}

	//------------------------------------------------------------------------------
	tHeightFieldRenderVertex* tHeightFieldMeshEntityDef::fLockRenderVerts( u32 startVertex, u32 numVerts )
	{
		// TODO
		return 0;
	}

	//------------------------------------------------------------------------------
	void tHeightFieldMeshEntityDef::fUnlockRenderVerts( tHeightFieldRenderVertex* lockedVerts )
	{
		// TODO
	}

	//------------------------------------------------------------------------------
	u16* tHeightFieldMeshEntityDef::fLockRenderIds( u32 startIndex, u32 numIds )
	{
		// TODO
		return 0;
	}

	//------------------------------------------------------------------------------
	void tHeightFieldMeshEntityDef::fUnlockRenderIds( u16* lockedIds )
	{
		// TODO
	}


	//------------------------------------------------------------------------------
	// tHeightFieldMeshEntity
	//------------------------------------------------------------------------------
	tHeightFieldMeshEntity::tHeightFieldMeshEntity(
		const u32 chunkIndex,
		const Gfx::tRenderBatchPtr& batchPtr,
		const tHeightFieldMesh * mesh,
		const Math::tAabbf& objectSpaceBox )
		: Gfx::tRenderableEntity( batchPtr, objectSpaceBox )
		, mMesh( mesh )
		, mChunkIndex( chunkIndex )
		, mUserFlags( 0 )
	{
	}

	//------------------------------------------------------------------------------
	tHeightFieldMeshEntity::~tHeightFieldMeshEntity( )
	{
	}

	//------------------------------------------------------------------------------
	void tHeightFieldMeshEntity::fCollectTris( const Math::tObbf& obb, tGrowableArray<Math::tTrianglef>& trisOut ) const
	{
		const u32 startTransformingAt = trisOut.fCount( );

		mMesh->fCollectTris( obb.fTransform( fWorldToObject( ) ).fToAabb( ), trisOut );

		// TODO cull tris that aren't in obb

		for( u32 i = startTransformingAt; i < trisOut.fCount( ); ++i )
			trisOut[ i ] = trisOut[ i ].fTransform( fObjectToWorld( ) );
	}

	//------------------------------------------------------------------------------
	void tHeightFieldMeshEntity::fCollectTris( const Math::tAabbf& aabb, tGrowableArray<Math::tTrianglef>& trisOut ) const
	{
		const u32 startTransformingAt = trisOut.fCount( );

		mMesh->fCollectTris( aabb.fTransform( fWorldToObject( ) ), trisOut );

		for( u32 i = startTransformingAt; i < trisOut.fCount( ); ++i )
			trisOut[ i ] = trisOut[ i ].fTransform( fObjectToWorld( ) );
	}

	//------------------------------------------------------------------------------
	void tHeightFieldMeshEntity::fRayCast( const Math::tRayf& ray, Math::tRayCastHit& hit ) const
	{
		const Math::tRayf rayInLocal = ray.fTransform( fWorldToObject( ) );
		mMesh->fTestRay( rayInLocal, hit );

		if( hit.fHit( ) )
			hit.mN = fObjectToWorld( ).fXformVector( hit.mN ); // transform normal back to world
	}

	//------------------------------------------------------------------------------
	b32 tHeightFieldMeshEntity::fIntersects( const Math::tFrustumf& v ) const
	{
		Math::tFrustumf vInObject;
		fToObjectSpace( vInObject, v );
		return mMesh->fTestFrustum( vInObject );
	}

	//------------------------------------------------------------------------------
	void tHeightFieldMeshEntity::fFakeRayCastUpDown( const Math::tRayf& ray, Math::tRayCastHit& hit ) const
	{
		sigassert( ray.mExtent.x == 0.f && ray.mExtent.z == 0.f );

		const f32 heightAtGround = fSampleHeight( ray.mOrigin );
		const f32 distToGround = heightAtGround - ray.mOrigin.y;
		if( ray.mExtent.y * distToGround < 0.f ) // different direction
		{
			hit = Math::tRayCastHit( );
			return;
		}

		const f32 t = fAbs( distToGround ) / fAbs( ray.mExtent.y );
		hit = Math::tRayCastHit( t, Math::tVec3f::cYAxis );
	}

	//------------------------------------------------------------------------------
	f32 tHeightFieldMeshEntity::fSampleHeight( const Math::tVec3f& worldPos ) const
	{
		const Math::tVec3f localPos = fWorldToObject( ).fXformPoint( worldPos );
		return mMesh->fSampleHeight( localPos.x, localPos.z );
	}

	//------------------------------------------------------------------------------
	b32 tHeightFieldMeshEntity::fOffHeightField( const Math::tVec3f& worldPos ) const
	{
		const Math::tVec3f localPos = fWorldToObject( ).fXformPoint( worldPos );
		const Math::tAabbf bounds = mMesh->fComputeBounds( );
		return !bounds.fContainsXZ( localPos ) || worldPos.y < bounds.mMin.y;
	}

	devvar_p( f32, Lod_TerrainNormalDrop, 100.f, 1 );
	devvar_p( f32, Lod_TerrainShadowDrop, 500.f, 1 );
	devvar_p( f32, Lod_TerrainHigh, 75.f, 1 );
	devvar_p( f32, Lod_TerrainLow, 500.f, 1 );

	u32 tHeightFieldMeshEntity::gLODForceTerrain = 0;
	devvarptr_clamp( u32, Lod_ForceTerrain, tHeightFieldMeshEntity::gLODForceTerrain, 0, 3, 0 );

	//------------------------------------------------------------------------------
	void tHeightFieldMeshEntity::fUpdateLOD( const Math::tVec3f & eye )
	{
		sigassert_is_main_thread( );
		f32 d = fWorldSpaceBox( ).fDistanceToPoint( eye );

		const f32 minD = Lod_TerrainHigh;
		const f32 maxD = Lod_TerrainLow;

		d = fClamp( d, minD, maxD );
		
		if( tEntityDef::fLODDistChanged( mLastLODDistance, d ) || gLODForceTerrain != 0 )
		{
			mLastLODDistance = d;
			fForceChangeRenderBatch( );
		}
	}

	//------------------------------------------------------------------------------
	void tHeightFieldMeshEntity::fForceChangeRenderBatch( )
	{
		sigassert_is_main_thread( );

		const f32 minD = Lod_TerrainHigh;
		const f32 maxD = Lod_TerrainLow;

		f32 ratio = 1.f - ( ( mLastLODDistance - minD ) / ( maxD - minD ) );
		if( gLODForceTerrain == 1 ) ratio = 0.0f;
		else if( gLODForceTerrain == 2 ) ratio = 0.5f;
		else if( gLODForceTerrain == 3 ) ratio = 1.0f;

#ifdef target_tools 
		ratio = 1.f;
#endif

		mMesh->fSelectLOD( this, ratio, ratio <= Lod_TerrainShadowDrop, ratio <= Lod_TerrainNormalDrop );
	}

	//------------------------------------------------------------------------------
	void tHeightFieldMeshEntity::fSetUserFlags( u32 userFlags )
	{
		sigassert_is_main_thread( );
		if( mUserFlags != userFlags )
		{
			mUserFlags = userFlags;
			fForceChangeRenderBatch( );
		}
	}

	//------------------------------------------------------------------------------
	void tHeightFieldMeshEntity::fOnSpawn( )
	{
		sigassert( mMesh );

		//if( mEntityDef && fTestBits( mEntityDef->mFlags, tHeightFieldMeshEntityDef::cFlagsCreateStaticPhysics ) )
		//right now always doing this, to keep things simple
		{
			mCollisionShape.fReset( NEW Physics::tCollisionShapeHeightfield( *this ) );
			mCollisionShape->fUserData( ).fReset( this );
			fSceneGraph( )->fPhysics( )->fMainStaticBody( )->fAddShape( mCollisionShape.fGetRawPtr( ) );
		}

		tRenderableEntity::fOnSpawn( );
	}

	//------------------------------------------------------------------------------
	void tHeightFieldMeshEntity::fOnDelete( )
	{
		tRenderableEntity::fOnDelete( );

		if( mCollisionShape )
		{
			mCollisionShape->fRemoveFromOwner( );
			mCollisionShape.fRelease( );
		}
	}

}

