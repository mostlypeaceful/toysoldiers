#include "BasePch.hpp"
#include "tHeightFieldMeshEntity.hpp"

#include "Gfx/tMaterial.hpp"
#include "Gfx/tGeometryFile.hpp"

namespace Sig
{
	tHeightFieldMeshEntityDef::tHeightFieldMeshEntityDef( )
		: tHeightFieldMesh( Math::tVec2f( 256.f, 256.f ), Math::tVec2i( 256, 256 ) )
		, mGeometryFile( 0 )
		, mMaterial( 0 )
	{
	}

	tHeightFieldMeshEntityDef::tHeightFieldMeshEntityDef( const Math::tVec2f& worldSpaceLengths, const Math::tVec2i& vertexRes )
		: tHeightFieldMesh( worldSpaceLengths, vertexRes )
		, mGeometryFile( 0 )
		, mMaterial( 0 )
	{
	}

	tHeightFieldMeshEntityDef::tHeightFieldMeshEntityDef( tNoOpTag )
		: tHeightFieldMesh( cNoOpTag )
		, mChunkDescs( cNoOpTag )
		, mGroundCoverDefs( cNoOpTag )
	{
	}

	tHeightFieldMeshEntityDef::~tHeightFieldMeshEntityDef( )
	{
		delete mMaterial;
	}

	void tHeightFieldMeshEntityDef::fCollectEntities( tEntity& parent, const tEntityCreationFlags& creationFlagsParent ) const
	{
		const tEntityCreationFlags creationFlags = creationFlagsParent | fCreationFlags( );

		tEntity* meshReferenceFrame = NEW tHeightFieldMeshReferenceFrameEntity( this, mObjectToLocal );
		fApplyPropsAndSpawnWithScript( *meshReferenceFrame, parent, creationFlags );

		const tResourcePtr& geoResource = mGeometryFile->fGetResourcePtr( );
		const Gfx::tGeometryFile* geoFile = geoResource->fCast<Gfx::tGeometryFile>( );
		sigassert( geoFile );

		Gfx::tRenderBatchData batch;
		batch.mRenderState = &Gfx::tRenderState::cDefaultColorOpaque;
		batch.mMaterial = mMaterial;
		batch.mVertexFormat = &geoFile->mGeometryPointers[ 0 ].mVRamBuffer.fVertexFormat( );
		batch.mGeometryBuffer = &geoFile->mGeometryPointers[ 0 ].mVRamBuffer;
		batch.mIndexBuffer = &geoFile->mIndexListPointers[ 0 ].mVRamBuffer;
		batch.mBaseVertexIndex = 0;
		batch.mBaseIndexIndex = 0;
		batch.mPrimitiveType = geoFile->mIndexListPointers[ 0 ].mVRamBuffer.fIndexFormat( ).mPrimitiveType;

		const u32 totalNumChunks = fRez( ).fNumChunksZ( ) * fRez( ).fNumChunksX( );
		if( totalNumChunks != mChunkDescs.fCount( ) ) // optimized mesh, all one graphics chunk
		{
			sigassert( mChunkDescs.fCount( ) == 1 );

			const u32 chunkIndex = 0;
			if( mChunkDescs[ chunkIndex ].fIsCulled( ) )
				return; // empty chunk

			batch.mVertexCount    = mChunkDescs[ chunkIndex ].mVtxCount;
			batch.mPrimitiveCount = mChunkDescs[ chunkIndex ].mTriCount;

			const Math::tAabbf objectSpaceBox = mBounds;
			tHeightFieldMeshEntity* entity = NEW tHeightFieldMeshEntity( Gfx::tRenderBatch::fCreate( batch ), this, objectSpaceBox );
			fApplyProperties( *entity, creationFlags );
			entity->fSpawn( *meshReferenceFrame );
		}
		else // un-optimized, old-skool
		{
			for( u32 ichunkz = 0; ichunkz < fRez( ).fNumChunksZ( ); ++ichunkz )
			{
				for( u32 ichunkx = 0; ichunkx < fRez( ).fNumChunksX( ); ++ichunkx )
				{
					const u32 chunkIndex = fRez( ).fChunkIndex( ichunkx, ichunkz );
					if( mChunkDescs[ chunkIndex ].fIsCulled( ) )
						continue; // empty chunk

					batch.mVertexCount    = mChunkDescs[ chunkIndex ].mVtxCount;
					batch.mPrimitiveCount = mChunkDescs[ chunkIndex ].mTriCount;

					const Math::tAabbf objectSpaceBox = fComputeChunkBounds( ichunkx, ichunkz );
					tHeightFieldMeshEntity* entity = NEW tHeightFieldMeshEntity( Gfx::tRenderBatch::fCreate( batch ), this, objectSpaceBox );
					fApplyProperties( *entity, creationFlags );
					entity->fSpawn( *meshReferenceFrame );

					batch.mBaseVertexIndex += mChunkDescs[ chunkIndex ].mVtxCount;
					batch.mBaseIndexIndex  += mChunkDescs[ chunkIndex ].mTriCount * 3;
				}
			}
		}

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

	tHeightFieldRenderVertex* tHeightFieldMeshEntityDef::fLockRenderVerts( u32 startVertex, u32 numVerts )
	{
		// TODO
		return 0;
	}

	void tHeightFieldMeshEntityDef::fUnlockRenderVerts( tHeightFieldRenderVertex* lockedVerts )
	{
		// TODO
	}

	u16* tHeightFieldMeshEntityDef::fLockRenderIds( u32 startIndex, u32 numIds )
	{
		// TODO
		return 0;
	}

	void tHeightFieldMeshEntityDef::fUnlockRenderIds( u16* lockedIds )
	{
		// TODO
	}


	tHeightFieldMeshEntity::tHeightFieldMeshEntity(
		const Gfx::tRenderBatchPtr& batchPtr,
		const tHeightFieldMesh* entityDef,
		const Math::tAabbf& objectSpaceBox )
		: Gfx::tRenderableEntity( batchPtr, objectSpaceBox )
		, mEntityDef( entityDef )
	{
	}

	void tHeightFieldMeshEntity::fCollectTris( const Math::tObbf& obb, tGrowableArray<Math::tTrianglef>& trisOut ) const
	{
		if( !mEntityDef || fDisabled( ) || fInvisible( ) ) return;

		const u32 startTransformingAt = trisOut.fCount( );

		mEntityDef->fCollectTris( obb.fTransform( fWorldToObject( ) ).fToAabb( ), trisOut );

		// TODO cull tris that aren't in obb

		for( u32 i = startTransformingAt; i < trisOut.fCount( ); ++i )
			trisOut[ i ] = trisOut[ i ].fTransform( fObjectToWorld( ) );
	}

	void tHeightFieldMeshEntity::fCollectTris( const Math::tAabbf& aabb, tGrowableArray<Math::tTrianglef>& trisOut ) const
	{
		if( !mEntityDef || fDisabled( ) || fInvisible( ) ) return;

		const u32 startTransformingAt = trisOut.fCount( );

		mEntityDef->fCollectTris( aabb.fTransform( fWorldToObject( ) ), trisOut );

		for( u32 i = startTransformingAt; i < trisOut.fCount( ); ++i )
			trisOut[ i ] = trisOut[ i ].fTransform( fObjectToWorld( ) );
	}

	void tHeightFieldMeshEntity::fRayCast( const Math::tRayf& ray, Math::tRayCastHit& hit ) const
	{
		if( !mEntityDef || fDisabled( ) || fInvisible( ) ) return;

		const Math::tRayf rayInLocal = ray.fTransform( fWorldToObject( ) );
		mEntityDef->fTestRay( rayInLocal, hit );

		if( hit.fHit( ) )
			hit.mN = fObjectToWorld( ).fXformVector( hit.mN ); // transform normal back to world
	}

	b32 tHeightFieldMeshEntity::fIntersects( const Math::tFrustumf& v ) const
	{
		if( !mEntityDef || fDisabled( ) || fInvisible( ) ) return false;

		Math::tFrustumf vInObject;
		fToObjectSpace( vInObject, v );
		return mEntityDef->fTestFrustum( vInObject );
	}
	void tHeightFieldMeshEntity::fFakeRayCastUpDown( const Math::tRayf& ray, Math::tRayCastHit& hit ) const
	{
		if( !mEntityDef || fDisabled( ) || fInvisible( ) ) return;

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
	f32 tHeightFieldMeshEntity::fSampleHeight( const Math::tVec3f& worldPos ) const
	{
		const Math::tVec3f localPos = fWorldToObject( ).fXformPoint( worldPos );
		return mEntityDef->fSampleHeight( localPos.x, localPos.z ); // NOTE hack for speed, assumes heightfield at 0,0,0 and unscaled/rotated
	}
	b32 tHeightFieldMeshEntity::fOffHeightField( const Math::tVec3f& worldPos ) const
	{
		const Math::tVec3f localPos = fWorldToObject( ).fXformPoint( worldPos );
		const Math::tAabbf bounds = mEntityDef->fComputeBounds( );
		return !bounds.fContainsXZ( localPos ) || worldPos.y < bounds.mMin.y;
	}

}

