#include "ToolsPch.hpp"
#include "tTerrainGeometry.hpp"
#include "tHeightFieldMeshEntity.hpp"
#include "Gfx/tMaterialFile.hpp"
#include "Gfx/tHeightFieldMaterial.hpp"
#include "Gfx/tDisplayList.hpp"
#include "Gfx/tDevice.hpp"
#include "Dx360Util.hpp"
#include "MeshSimplify.hpp"
#include "tLogicThreadPool.hpp"

namespace Sig
{
	tTerrainGeometry::tTerrainGeometry( 
		const tResourceDepotPtr& resourceDepot,
		const Gfx::tDevicePtr& device, 
		const Math::tVec2f& worldSpaceLengths, 
		const Math::tVec2i& vertexRes,
		const Math::tVec2i& materialRes,
		const tResourcePtr& diffuseMap,
		const tResourcePtr& normalMap )
		: tHeightFieldMesh( worldSpaceLengths, vertexRes )
	{
		fAcquireMaterial( device, resourceDepot, materialRes, diffuseMap, normalMap );

		const u32 vertCount = fRez( ).fTotalVertCount( );
		const u32 numQuadsPerChunk = fRez( ).fNumQuadsPerChunk( );
		const u32 trisPerChunk = 2 * numQuadsPerChunk;
		const u32 idsPerChunk = 3 * trisPerChunk;

		const Gfx::tVertexFormat& vtxFormat = Gfx::tHeightFieldMaterial::cVertexFormat;
		sigassert( sizeof( tHeightFieldRenderVertex ) == vtxFormat.fVertexSize( ) );

		Gfx::tGeometryBufferVRam& gpuVerts = fGetGpuVerts( );
		gpuVerts.fAllocate( 
			device, 
			vtxFormat, 
			vertCount, 
			Gfx::tGeometryBufferVRam::cAllocDynamic );

		tHeightFieldRenderVertex* renderVerts = fLockRenderVerts( );
		fInitializeRenderVerts( renderVerts, vertCount );
		fUnlockRenderVerts( renderVerts );


		const u32 totalNumIds = idsPerChunk * fRez( ).fNumChunks( );
		const u32 totalNumTris = trisPerChunk * fRez( ).fNumChunks( );
		Gfx::tIndexBufferVRam& gpuIndices = fGetGpuIndices( );
		gpuIndices.fAllocate( 
			device, 
			Gfx::tIndexFormat( Gfx::tIndexFormat::cStorageU16, Gfx::tIndexFormat::cPrimitiveTriangleList ), 
			totalNumIds, 
			totalNumTris, 
			Gfx::tIndexBufferVRam::cAllocDynamic );

		Sig::u16* renderIds = fLockRenderIds( );
		fInitializeRenderIndices( renderIds, totalNumIds );
		fUnlockRenderIds( renderIds );

		// make sure we do this last, after allocating verts and indices
		fRegisterWithDevice( device.fGetRawPtr( ) );
	}

	tTerrainGeometry::~tTerrainGeometry( )
	{
		mTerrainMaterialFile->fUnload( this );
		
		const u32 gcCount = mGroundCover.fCount( );
		for( u32 gc = 0; gc < gcCount; ++gc)
			delete mGroundCover[ gc ];
	}

	void tTerrainGeometry::fOnDeviceReset( Gfx::tDevice* device )
	{
		const u32 vertCount = fRez( ).fTotalVertCount( );
		const u32 numQuadsPerChunk = fRez( ).fNumQuadsPerChunk( );
		const u32 trisPerChunk = 2 * numQuadsPerChunk;
		const u32 idsPerChunk = 3 * trisPerChunk;

		tHeightFieldRenderVertex* renderVerts = fLockRenderVerts( );
		fInitializeRenderVerts( renderVerts, vertCount );
		fUnlockRenderVerts( renderVerts );

		const u32 totalNumIds = idsPerChunk * fRez( ).fNumChunks( );
		Sig::u16* renderIds = fLockRenderIds( );
		fInitializeRenderIndices( renderIds, totalNumIds );
		fUnlockRenderIds( renderIds );

		if (mVRamVerts.fVertexCount() > 0)
        {
			fRefreshGraphicsBlock(
				0,
				0,
				mRez.fNumLogicalVertsX() - 1,
				mRez.fNumLogicalVertsZ() - 1,
				0,
				mVRamVerts.fVertexCount() - 1);
        }
	}

	void tTerrainGeometry::fCollectEntities( tEntity& parent, const tEntityCreationFlags& creationFlags ) const
	{
		const Gfx::tGeometryBufferVRam& gpuVerts = fGetGpuVerts( );
		const Gfx::tIndexBufferVRam& gpuIndices = fGetGpuIndices( );

		const u32 numQuadsPerChunk = fRez( ).fNumQuadsPerChunk( );
		const u32 trisPerChunk = 2 * numQuadsPerChunk;
		const u32 idsPerChunk = 3 * trisPerChunk;

		Gfx::tRenderBatchData batch;
		batch.mRenderState = &Gfx::tRenderState::cDefaultColorOpaque;
		batch.mMaterial = mTerrainMaterial.fGetRawPtr( );
		batch.mVertexFormat = &gpuVerts.fVertexFormat( );
		batch.mGeometryBuffer = &gpuVerts;
		batch.mIndexBuffer = &gpuIndices;
		batch.mVertexCount = fRez( ).fChunkVertCount( );
		batch.mBaseVertexIndex = 0;
		batch.mPrimitiveCount = trisPerChunk;
		batch.mBaseIndexIndex = 0;
		batch.mPrimitiveType = gpuIndices.fIndexFormat( ).mPrimitiveType;
		
		for( u32 ichunkz = 0; ichunkz < fRez( ).fNumChunksZ( ); ++ichunkz )
		{
			for( u32 ichunkx = 0; ichunkx < fRez( ).fNumChunksX( ); ++ichunkx )
			{
				const Math::tAabbf objectSpaceBox = fRez( ).fChunkBounds( ichunkx, ichunkz );
				tHeightFieldMeshEntity* entity = new tHeightFieldMeshEntity( Gfx::tRenderBatch::fCreate( batch ), this, objectSpaceBox );
				entity->fSpawnImmediate( parent );

				batch.mBaseVertexIndex += fRez( ).fChunkVertCount( );
				batch.mBaseIndexIndex  += idsPerChunk;
			}
		}
	}

	tHeightFieldRenderVertex* tTerrainGeometry::fLockRenderVerts( u32 startVertex, u32 numVerts )
	{
		fGetGpuVerts( ).fDeepLock( );
		return ( tHeightFieldRenderVertex* )fGetGpuVerts( ).fQuickLock( startVertex, numVerts );
	}
	void tTerrainGeometry::fUnlockRenderVerts( tHeightFieldRenderVertex* lockedVerts )
	{
		fGetGpuVerts( ).fDeepUnlock( /*( Sig::byte* )lockedVerts*/ );
	}
	u16* tTerrainGeometry::fLockRenderIds( u32 startIndex, u32 numIds )
	{
		fGetGpuIndices( ).fDeepLock( );
		return ( Sig::u16* )fGetGpuIndices( ).fQuickLock( startIndex, numIds );
	}
	void tTerrainGeometry::fUnlockRenderIds( u16* lockedIds )
	{
		fGetGpuIndices( ).fDeepUnlock( /*( Sig::byte* )lockedIds*/ );
	}

	//------------------------------------------------------------------------------
	tTerrainGeometry::tGroundCoverInternal * tTerrainGeometry::fFindGroundCover( u32 id )
	{
		const u32 gcCount = mGroundCover.fCount( );
		for( u32 gc = 0; gc < gcCount; ++gc )
		{
			if( mGroundCover[ gc ]->mCoverId == id )
				return mGroundCover[ gc ];
		}

		return 0;
	}

	//------------------------------------------------------------------------------
	template<class t>
	static void fResize( 
		tDynamicArray<t> & samples, 
		u32 srcDimX, u32 srcDimZ, 
		u32 destDimX, u32 destDimZ )
	{
		if( srcDimX == destDimX && srcDimZ == destDimZ )
			return;

		tDynamicArray<t> newSamples( destDimX * destDimZ );
		for( u32 x = 0; x < destDimX; ++x )
		{
			u32 oldx = fRound<u32>( ( x / (f32)( destDimX - 1 ) ) * ( srcDimX - 1 ) );
			for( u32 z = 0; z < destDimZ; ++z )
			{
				u32 oldz = fRound<u32>( ( z / (f32)( destDimZ - 1 ) ) * ( srcDimZ - 1 ) );
				newSamples[z * destDimX + x] = samples[ oldz * srcDimX + oldx ];
			}
		}

		samples.fSwap( newSamples );
	}

	//------------------------------------------------------------------------------
	b32 tTerrainGeometry::fUpdateGroundCoverDims( tGroundCoverInternal & cover, f32 unitSize, u32 maxUnitSpawns )
	{
		sigassert( !cover.mEditors );

		const u32 dimx = fRound<u32>( fRez( ).fWorldLengthX( ) / unitSize );
		const u32 dimz = fRound<u32>( fRez( ).fWorldLengthZ( ) / unitSize );

		// Some array dims have changed
		if( dimx != cover.mDimX || dimz != cover.mDimZ || maxUnitSpawns != cover.mMaxUnitSpawns )
		{
			fResize( cover.mMask, cover.mDimX, cover.mDimZ, dimx, dimz );
			cover.mHeights.fNewArray( ( dimx * dimz ) * maxUnitSpawns );
			cover.mHeights.fFill( 0 );

			cover.mUnitSize = unitSize;
			cover.mMaxUnitSpawns = maxUnitSpawns;
			cover.mDimX = dimx;
			cover.mDimZ = dimz;
			return true;
		}
		else if( unitSize != cover.mUnitSize )
		{
			// The heights are now invalid
			cover.mUnitSize = unitSize;
			return true;
		}

		return false;
	}

	void tTerrainGeometry::fAcquireMaterial( 
		const Gfx::tDevicePtr& device, 
		const tResourceDepotPtr& resourceDepot,
		const Math::tVec2i& materialRes,
		const tResourcePtr& diffuseMap,
		const tResourcePtr& normalMap )
	{
		// load terrain material
		mTerrainMaterialFile = resourceDepot->fQuery( tResourceId::fMake<Gfx::tMaterialFile>( Gfx::tHeightFieldMaterial::fMaterialFilePath( ) ) );
		mTerrainMaterialFile->fLoadDefault( this );
		mTerrainMaterialFile->fBlockUntilLoaded( );

		Gfx::tTextureFile::tLockedMip mip;

		mMaskTexture.fAllocate( device, materialRes.x, materialRes.y, Gfx::tTextureFile::cFormatA8R8G8B8 );
		mip = mMaskTexture.fLock( );
		for( u32 y = 0; y < mMaskTexture.fHeight( ); ++y )
			for( u32 x = 0; x < mMaskTexture.fWidth( ); ++x )
				*mip.fGetTexel<u32>( x, y ) = Gfx::tTextureVRam::fPackColorR8G8B8A8( Math::tVec4f( 1.f, 0.0f, 0.0f, 0.f ) );
		mMaskTexture.fUnlock( );

		mMtlIdsTexture.fAllocate( device, materialRes.x, materialRes.y, Gfx::tTextureFile::cFormatR5G6B5 );
		mip = mMtlIdsTexture.fLock( );
		for( u32 y = 0; y < mMtlIdsTexture.fHeight( ); ++y )
			for( u32 x = 0; x < mMtlIdsTexture.fWidth( ); ++x )
				*mip.fGetTexel<u16>( x, y ) = Gfx::tTextureVRam::fPackColorR5G6B5( 0, 0, 0 );
		mMtlIdsTexture.fUnlock( );

		fUpdateDynamicTextureReferences( diffuseMap, normalMap );
	}

	void tTerrainGeometry::fUpdateDynamicTextureReferences( const tResourcePtr& diffuseMap, const tResourcePtr& normalMap )
	{
		tFixedArray<Math::tNoOpVec4f,8> oldTilingFactors;
		if( mTerrainMaterial )
			oldTilingFactors = mTerrainMaterial->mTileFactors;
		else
		{
			for( u32 i = 0; i < oldTilingFactors.fCount( ); ++i )
				oldTilingFactors[ i ] = Math::tVec4f( 0.5f );
		}

		Gfx::tHeightFieldMaterial* hfmtl = new Gfx::tHeightFieldMaterial( );
		mTerrainMaterial.fReset( hfmtl );
		mTerrainMaterial->fSetMaterialFileResourcePtrOwned( *mTerrainMaterialFile->fGetOwner( ), mTerrainMaterialFile->fGetPath( ) );

		const Gfx::tTextureFile* diffuseMapTex = diffuseMap ? diffuseMap->fCast<Gfx::tTextureFile>( ) : 0;
		const Gfx::tTextureFile* normalMapTex = normalMap ? normalMap->fCast<Gfx::tTextureFile>( ) : 0;
		if( diffuseMapTex && diffuseMapTex->mIsAtlas )
		{
			hfmtl->mWorldSpaceDims = Math::tVec2f( fRez( ).fWorldLengthX( ), fRez( ).fWorldLengthZ( ) );
			hfmtl->mTextureAtlasDims = Math::tVec2u( diffuseMapTex->mSubTexCountX, diffuseMapTex->mSubTexCountY );
			hfmtl->mSubDiffuseRectDims = Math::tVec2f( diffuseMapTex->mSubTexWidth, diffuseMapTex->mSubTexHeight );
		}
		if( normalMapTex && normalMapTex->mIsAtlas )
			hfmtl->mSubNormalRectDims = Math::tVec2f( normalMapTex->mSubTexWidth, normalMapTex->mSubTexHeight );

		hfmtl->mMaskMap.fSetRaw( mMaskTexture.fGetPlatformHandle( ) );
		hfmtl->mMtlIdsMap.fSetRaw( mMtlIdsTexture.fGetPlatformHandle( ) );

		if( diffuseMapTex )
			hfmtl->mDiffuseMap.fSetRaw( diffuseMapTex->mPlatformHandle );
		if( normalMapTex )
			hfmtl->mNormalMap.fSetRaw( normalMapTex->mPlatformHandle );

		hfmtl->mMtlIdsMap.fSetSamplingModes( Gfx::tTextureFile::cFilterModeNone, Gfx::tTextureFile::cAddressModeClamp );
		hfmtl->mMaskMap.fSetSamplingModes( Gfx::tTextureFile::cFilterModeWithMip, Gfx::tTextureFile::cAddressModeClamp );
		hfmtl->mDiffuseMap.fSetSamplingModes( Gfx::tTextureFile::cFilterModeWithMip, Gfx::tTextureFile::cAddressModeClamp );
		hfmtl->mNormalMap.fSetSamplingModes( Gfx::tTextureFile::cFilterModeWithMip, Gfx::tTextureFile::cAddressModeClamp );

		if( normalMapTex )
		{
			hfmtl->mLightingPass.mVS = Gfx::tHeightFieldMaterial::tLightingPass::cVSDiffuseNormalMap;
			hfmtl->mLightingPass.mPS = Gfx::tHeightFieldMaterial::tLightingPass::cPSDiffuseNormalMap;
		}

		mTerrainMaterial->mTileFactors = oldTilingFactors;
	}

	void tTerrainGeometry::fUpdateMaterialTilingFactors( const tDynamicArray<f32>& tilingFactors )
	{
		if( !mTerrainMaterial )
			return;

		// set material tiling factors
		for( u32 i = 0; i < tilingFactors.fCount( ); i += 4 )
		{
			for( u32 j = 0; j < 4; ++j )
			{
				const u32 absoluteIndex = i + j;
				if( absoluteIndex < tilingFactors.fCount( ) )
					mTerrainMaterial->mTileFactors[ i / 4 ].fAxis( j ) = tilingFactors[ absoluteIndex ];
			}
		}
	}

	void tTerrainGeometry::fBeginEditingVerts( tEditableVertices& editableVerts, f32 minLocalX, f32 minLocalZ, f32 maxLocalX, f32 maxLocalZ ) const
	{
		const u32 minXLogicalIdx = fRez( ).fWorldXLogicalVertex( minLocalX );
		const u32 maxXLogicalIdx = fRez( ).fWorldXLogicalVertex( maxLocalX );
		const u32 minZLogicalIdx = fRez( ).fWorldZLogicalVertex( minLocalZ );
		const u32 maxZLogicalIdx = fRez( ).fWorldZLogicalVertex( maxLocalZ );

		const u32 xdim = maxXLogicalIdx - minXLogicalIdx;
		const u32 zdim = maxZLogicalIdx - minZLogicalIdx;

		editableVerts.fSetDimensions( xdim, zdim, minLocalX, minLocalZ, maxLocalX, maxLocalZ );

		editableVerts.mMinRenderVtxIdx  = ~0;
		editableVerts.mMaxRenderVtxIdx  =  0;
		editableVerts.mMinHeight		= +Math::cInfinity;
		editableVerts.mMaxHeight		= -Math::cInfinity;
		editableVerts.mAvgHeight		=  0.f;

		for( u32 z = 0; z < zdim; ++z )
		{
			for( u32 x = 0; x < xdim; ++x )
			{
				tEditableVertex& ev = editableVerts.fIndex( x, z );

				ev.mLogicalX = minXLogicalIdx + x;
				ev.mLogicalZ = minZLogicalIdx + z;
				ev.mLocalX = fRez( ).fLogicalVertexWorldX( ev.mLogicalX );
				ev.mLocalZ = fRez( ).fLogicalVertexWorldZ( ev.mLogicalZ );

				const tLogicalVertex& lv = mLogicalVerts[ fRez( ).fLogicalVertIndex( ev.mLogicalX, ev.mLogicalZ ) ];

				ev.mLocalHeight = lv.fGetHeight( );
				ev.mQuadState = lv.fGetQuadState( );

				// track min, max, and average height
				editableVerts.mMinHeight = fMin( ev.mLocalHeight, editableVerts.mMinHeight );
				editableVerts.mMaxHeight = fMax( ev.mLocalHeight, editableVerts.mMaxHeight );
				editableVerts.mAvgHeight += ev.mLocalHeight;

				// track min and max render vertices
				for( u32 i = 0; i < lv.fGetNumRenderVertexIds( ); ++i )
				{
					const u32 rvtxId = lv.fGetRenderVertexId( i );
					editableVerts.mMinRenderVtxIdx = fMin( rvtxId, editableVerts.mMinRenderVtxIdx );
					editableVerts.mMaxRenderVtxIdx = fMax( rvtxId, editableVerts.mMaxRenderVtxIdx );
				}
			}
		}

		// average height across all verts
		editableVerts.mAvgHeight *= 1.f / ( zdim * xdim );
	}

	void tTerrainGeometry::fEndEditingVerts( const tEditableVertices& editableVerts )
	{
		const u32 minx = editableVerts.fIndex( 0, 0 ).mLogicalX;
		const u32 minz = editableVerts.fIndex( 0, 0 ).mLogicalZ;
		const u32 maxx = editableVerts.fIndex( editableVerts.fDimX( ) - 1, 0 ).mLogicalX;
		const u32 maxz = editableVerts.fIndex( 0, editableVerts.fDimZ( ) - 1 ).mLogicalZ;

		// copy height values back to logical grid
		for( u32 z = 0; z < editableVerts.fDimZ( ); ++z )
		{
			for( u32 x = 0; x < editableVerts.fDimX( ); ++x )
			{
				const tEditableVertex& ev = editableVerts.fIndex( x, z );
				tLogicalVertex& lv = mLogicalVerts[ fRez( ).fLogicalVertIndex( ev.mLogicalX, ev.mLogicalZ ) ];

				// copy height and quad state back to logical vertex structure
				lv.fSetHeight( ev.mLocalHeight );
				lv.fSetQuadState( ev.mQuadState );
			}
		}

		fRefreshLogicalBlock( minx, minz, maxx, maxz );
		fRefreshGraphicsBlock( minx, minz, maxx, maxz, editableVerts.mMinRenderVtxIdx, editableVerts.mMaxRenderVtxIdx );
	}

	//------------------------------------------------------------------------------
	b32 tTerrainGeometry::fRestoreGroundCover( u32 id, f32 unitSize, u32 maxUnitSpawns )
	{
		return fUpdateGroundCoverDims( id, unitSize, maxUnitSpawns );
	}

	//------------------------------------------------------------------------------
	void tTerrainGeometry::fAddGroundCover( u32 id, f32 unitSize, u32 maxUnitSpawns )
	{
		sigassert( !fFindGroundCover( id ) );

		const u32 dimx = fRound<u32>( fRez( ).fWorldLengthX( ) / unitSize );
		const u32 dimz = fRound<u32>( fRez( ).fWorldLengthZ( ) / unitSize );

		tGroundCoverInternal * cover = new tGroundCoverInternal( );
		cover->mCoverId = id;
		cover->mDimX = dimx;
		cover->mDimZ = dimz;
		cover->mUnitSize = unitSize;
		cover->mMask.fNewArray( cover->mDimX * cover->mDimZ );
		cover->mMask.fFill( 1.f );
		
		cover->mMaxUnitSpawns = maxUnitSpawns;
		cover->mHeights.fNewArray( ( cover->mDimX * cover->mDimZ ) * cover->mMaxUnitSpawns );
		cover->mHeights.fFill( 0 );

		mGroundCover.fPushBack( cover );
	}

	//------------------------------------------------------------------------------
	void tTerrainGeometry::fRemoveGroundCover( u32 id )
	{
		const u32 gcCount = mGroundCover.fCount( );
		for( u32 gc = 0; gc < gcCount; ++gc )
		{
			if( mGroundCover[ gc ]->mCoverId != id )
				continue;

			delete mGroundCover[ gc ];
			mGroundCover.fErase( gc );

			return;
		}
	}

	//------------------------------------------------------------------------------
	void tTerrainGeometry::fBeginEditingGroundCoverMask( 
		u32 id, tEditableGroundCoverMask & editableMask,
		f32 minLocalX, f32 minLocalZ, f32 maxLocalX, f32 maxLocalZ )
	{
		tGroundCoverInternal * cover = fFindGroundCover( id );
		sigassert( cover );

		++cover->mEditors;

		const f32 unitSize = cover->mUnitSize;

		const u32 minX = fClamp( 
			fRoundDown<u32>( ( fRez( ).fWorldXUV( minLocalX ) * fRez( ).fWorldLengthX( ) ) / unitSize ), 
			0u, cover->mDimX );
		const u32 minZ = fClamp( 
			fRoundDown<u32>( ( fRez( ).fWorldZUV( minLocalZ ) * fRez( ).fWorldLengthZ( ) ) / unitSize ), 
			0u, cover->mDimZ );
		const u32 maxX = fClamp( 
			fRoundDown<u32>( ( fRez( ).fWorldXUV( maxLocalX ) * fRez( ).fWorldLengthX( ) ) / unitSize ) + 1, 
			0u, cover->mDimX );
		const u32 maxZ = fClamp( 
			fRoundDown<u32>( ( fRez( ).fWorldZUV( maxLocalZ ) * fRez( ).fWorldLengthZ( ) ) / unitSize ) + 1, 
			0u, cover->mDimZ );

		const u32 xdim = maxX - minX;
		const u32 zdim = maxZ - minZ;

		editableMask.fSetDimensions( 
			xdim, zdim, 
			0.5f * unitSize + minX * unitSize - 0.5f * fRez( ).fWorldLengthX( ), 
			0.5f * unitSize + minZ * unitSize - 0.5f * fRez( ).fWorldLengthZ( ), 
			0.5f * unitSize + maxX * unitSize - 0.5f * fRez( ).fWorldLengthX( ), 
			0.5f * unitSize + maxZ * unitSize - 0.5f * fRez( ).fWorldLengthZ( ) );

		editableMask.mGroundCoverId = cover->mCoverId;
		editableMask.mStartX = minX;
		editableMask.mStartZ = minZ;

		if( xdim && zdim )
		{
			for( u32 zD = 0; zD < zdim; ++zD )
			{
				fMemCpy( 
					&editableMask.fIndex( 0, zD ), 
					&cover->mMask[ ( minZ + zD ) * cover->mDimX + minX ],
					cover->mMask.fElementSizeOf( ) * xdim );
			}
		}
	}

	//------------------------------------------------------------------------------
	void tTerrainGeometry::fEndEditingGroundCoverMask( 
		const tEditableGroundCoverMask & editableMask )
	{
		tGroundCoverInternal * cover = fFindGroundCover( editableMask.mGroundCoverId );

		sigassert( cover && cover->mEditors );

		--cover->mEditors;

		const u32 xdim = editableMask.fDimX( );
		const u32 zdim = editableMask.fDimZ( );

		if( xdim && zdim )
		{
			for( u32 zS = 0; zS < zdim; ++zS )
			{
				fMemCpy( 
					&cover->mMask[ ( editableMask.mStartZ + zS ) * cover->mDimX + editableMask.mStartX ], 
					&editableMask.fIndex( 0, zS ), 
					cover->mMask.fElementSizeOf( ) * xdim );
			}
		}
	}

	//------------------------------------------------------------------------------
	b32 tTerrainGeometry::fUpdateGroundCoverDims( u32 id, f32 unitSize, u32 maxUnitSpawns )
	{
		return fUpdateGroundCoverDims( *fFindGroundCover( id ), unitSize, maxUnitSpawns );
	}

	//------------------------------------------------------------------------------
	void tTerrainGeometry::fBeginEditingGroundCoverHeights( 
		u32 id, tEditableGroundCoverHeights & editableHeights )
	{
		tGroundCoverInternal * cover = fFindGroundCover( id );
		sigassert( cover );

		cover->mEditors++;

		const f32 unitSize = cover->mUnitSize;

		editableHeights.mGroundCoverId = id;
		editableHeights.mStartX = 0;
		editableHeights.mStartZ = 0;
		editableHeights.fSetDimensions( 
			cover->mDimX, 
			cover->mDimZ,
			0.5f * unitSize - 0.5f * fRez( ).fWorldLengthX( ), 
			0.5f * unitSize - 0.5f * fRez( ).fWorldLengthZ( ), 
			0.5f * unitSize + cover->mDimX * unitSize - 0.5f * fRez( ).fWorldLengthX( ), 
			0.5f * unitSize + cover->mDimZ * unitSize - 0.5f * fRez( ).fWorldLengthZ( ), 
			cover->mMaxUnitSpawns );

		sigassert( editableHeights.mItems.fCount( ) == cover->mHeights.fCount( ) );
		fMemCpy( 
			editableHeights.mItems.fBegin( ), 
			cover->mHeights.fBegin( ), 
			cover->mHeights.fTotalSizeOf( ) );
	}

	//------------------------------------------------------------------------------
	void tTerrainGeometry::fBeginEditingGroundCoverHeights( 
		u32 id, tEditableGroundCoverHeights & editableHeights, 
		f32 minLocalX, f32 minLocalZ, f32 maxLocalX, f32 maxLocalZ )
	{
		tGroundCoverInternal * cover = fFindGroundCover( id );
		sigassert( cover );

		const u32 unitSize = cover->mUnitSize;

		cover->mEditors++;

		const u32 minX = fClamp( 
			fRoundDown<u32>( ( fRez( ).fWorldXUV( minLocalX ) * fRez( ).fWorldLengthX( ) ) / unitSize ), 
			0u, cover->mDimX );
		const u32 minZ = fClamp( 
			fRoundDown<u32>( ( fRez( ).fWorldZUV( minLocalZ ) * fRez( ).fWorldLengthZ( ) ) / unitSize ), 
			0u, cover->mDimZ );
		const u32 maxX = fClamp( 
			fRoundDown<u32>( ( fRez( ).fWorldXUV( maxLocalX ) * fRez( ).fWorldLengthX( ) ) / unitSize ) + 1, 
			0u, cover->mDimX );
		const u32 maxZ = fClamp( 
			fRoundDown<u32>( ( fRez( ).fWorldZUV( maxLocalZ ) * fRez( ).fWorldLengthZ( ) ) / unitSize ) + 1, 
			0u, cover->mDimZ );

		const u32 xdim = maxX - minX;
		const u32 zdim = maxZ - minZ;

		editableHeights.fSetDimensions( 
			xdim, zdim, 
			0.5f * unitSize + minX * unitSize - 0.5f * fRez( ).fWorldLengthX( ), 
			0.5f * unitSize + minZ * unitSize - 0.5f * fRez( ).fWorldLengthZ( ), 
			0.5f * unitSize + maxX * unitSize - 0.5f * fRez( ).fWorldLengthX( ), 
			0.5f * unitSize + maxZ * unitSize - 0.5f * fRez( ).fWorldLengthZ( ),
			cover->mMaxUnitSpawns );

		editableHeights.mGroundCoverId = cover->mCoverId;
		editableHeights.mStartX = minX;
		editableHeights.mStartZ = minZ;

		if( xdim && zdim )
		{
			for( u32 zD = 0; zD < zdim; ++zD )
			{
				fMemCpy( 
					&editableHeights.fIndex( 0, zD ), 
					&cover->mHeights[ ( ( minZ + zD ) * cover->mDimX + minX ) * cover->mMaxUnitSpawns ],
					( cover->mMaxUnitSpawns * cover->mHeights.fElementSizeOf( ) ) * xdim  );
			}
		}
	}

	//------------------------------------------------------------------------------
	void tTerrainGeometry::fEndEditingGroundCoverHeights( 
		const tEditableGroundCoverHeights & editableHeights )
	{
		tGroundCoverInternal * cover = fFindGroundCover( editableHeights.mGroundCoverId );
		sigassert( cover && cover->mEditors );
		sigassert( cover->mMaxUnitSpawns == editableHeights.fPerItem( ) );

		--cover->mEditors;

		const u32 xdim = editableHeights.fDimX( );
		const u32 zdim = editableHeights.fDimZ( );

		if( xdim && zdim )
		{
			for( u32 zS = 0, zD = editableHeights.mStartZ; zS < zdim; ++zS, ++zD )
			{
				fMemCpy( 
					&cover->mHeights[ ( zD * cover->mDimX + editableHeights.mStartX ) * cover->mMaxUnitSpawns ], 
					&editableHeights.fIndex( 0, zS ), 
					( cover->mMaxUnitSpawns * cover->mHeights.fElementSizeOf( ) ) * xdim );
			}
		}
	}

	//------------------------------------------------------------------------------
	void tTerrainGeometry::fQueryGroundCoverMask( 
		u32 id, u32 minX, u32 minZ, u32 maxX, u32 maxZ, tDynamicArray< f32 > & mask )
	{
		tGroundCoverInternal * cover = fFindGroundCover( id );

		const u32 dimX = maxX - minX;
		const u32 dimZ = maxZ - minZ;

		// No range or no cover
		if( !cover || !dimX || !dimZ )
		{
			mask.fDeleteArray( );
			return;
		}

		sigassert( minX <= cover->mDimX );
		sigassert( maxX <= cover->mDimX );
		sigassert( minZ <= cover->mDimZ );
		sigassert( maxZ <= cover->mDimZ );

		mask.fNewArray( dimX * dimZ );
		for( u32 zD = 0, zS = minZ; zD < dimZ; ++zD, ++zS )
		{
			fMemCpy( 
				&mask[ zD * dimX ], 
				&cover->mMask[ zS * cover->mDimX + minX ], 
				mask.fElementSizeOf( ) * dimX );
		}
	}

	//------------------------------------------------------------------------------
	void tTerrainGeometry::fQueryGroundCoverHeights( u32 id, tDynamicArray< f32 > & heights )
	{
		tGroundCoverInternal * cover = fFindGroundCover( id );
		if( !cover )
			heights.fDeleteArray( );
		else
			fQueryGroundCoverHeights( id, 0, 0, cover->mDimX, cover->mDimZ, heights );
	}

	//------------------------------------------------------------------------------
	void tTerrainGeometry::fQueryGroundCoverHeights( 
		u32 id, u32 minX, u32 minZ, u32 maxX, u32 maxZ, tDynamicArray< f32 > & heights )
	{
		tGroundCoverInternal * cover = fFindGroundCover( id );
		
		const u32 dimX = maxX - minX;
		const u32 dimZ = maxZ - minZ;

		// No range or no cover
		if( !cover || !dimX || !dimZ )
		{
			heights.fDeleteArray( );
			return;
		}

		sigassert( minX <= cover->mDimX );
		sigassert( maxX <= cover->mDimX );
		sigassert( minZ <= cover->mDimZ );
		sigassert( maxZ <= cover->mDimZ );

		heights.fNewArray( ( dimX * dimZ ) * cover->mMaxUnitSpawns );
		for( u32 zD = 0, zS = minZ; zD < dimZ; ++zD, ++zS )
		{
			fMemCpy( 
				&heights[ ( zD * dimX ) * cover->mMaxUnitSpawns ], 
				&cover->mHeights[ ( zS * cover->mDimX + minX ) * cover->mMaxUnitSpawns ], 
				( cover->mHeights.fElementSizeOf( ) * cover->mMaxUnitSpawns ) * dimX );
		}
	}

	//------------------------------------------------------------------------------
	void tTerrainGeometry::fSaveGroundCover( tGroundCover & gc )
	{
		tGroundCoverInternal * cover = fFindGroundCover( gc.mCoverId );

		if( !cover )
			gc.mCoverId = ~0;
		else
			gc = *cover;
	}

	//------------------------------------------------------------------------------
	void tTerrainGeometry::fSaveGroundCover( tDynamicArray<tGroundCover> & groundCover )
	{
		const u32 gcCount = mGroundCover.fCount( );
		groundCover.fNewArray( gcCount );

		for( u32 gc = 0; gc < gcCount; ++gc )
			groundCover[ gc ] = *mGroundCover[ gc ];
	}

	//------------------------------------------------------------------------------
	void tTerrainGeometry::fRestoreGroundCover( const tGroundCover & gc )
	{
		tGroundCoverInternal * cover = fFindGroundCover( gc.mCoverId );
		sigassert( cover );

		*cover = gc;
	}

	//------------------------------------------------------------------------------
	void tTerrainGeometry::fRestoreGroundCover( 
		const tDynamicArray<tGroundCover> & groundCover )
	{
		const u32 gcCount = groundCover.fCount( );
		mGroundCover.fSetCapacity( gcCount );

		for( u32 gc = 0; gc < gcCount; ++gc )
		{
			const tGroundCover & externalCover = groundCover[ gc ];
			tGroundCoverInternal * cover = new tGroundCoverInternal( );
			*cover = externalCover;

			mGroundCover.fPushBack( cover );
		}
	}

}

#include "tSigmlConverter.hpp"
#include "Gfx/tGeometryFile.hpp"

namespace Sig
{
	class tTerrainGeometrySimplifier
	{
	public:
		b32 fRunIteration( u32 index )
		{
			MeshSimplify::fSimplify( *mRawVerts, (*mSplitTris)[ index ], mOptimizeTarget, true, false );
			return true;
		}

		void fSimplify( f32 optTarget, tGrowableArray< Math::tVec3f >* rawVerts, tGrowableArray< tGrowableArray< Math::tVec3u > >* splitTris )
		{
			mOptimizeTarget = optTarget;
			mRawVerts = rawVerts;
			mSplitTris = splitTris;

			const b32 mt = true;
			if( mt )
			{
				tLogicThreadPool pool;
				Threads::tDistributedForLoopCallback callback = make_delegate_memfn( Threads::tDistributedForLoopCallback, tTerrainGeometrySimplifier, fRunIteration );
				pool.fDistributeForLoop( callback, mSplitTris->fCount( ) );
			}
			else
			{
				for( u32 index = 0; index < mSplitTris->fCount( ); ++index )
					MeshSimplify::fSimplify( *mRawVerts, (*mSplitTris)[ index ], mOptimizeTarget, true, false );
			}
		}

	private:
		f32 mOptimizeTarget;
		tGrowableArray< Math::tVec3f >* mRawVerts;
		tGrowableArray< tGrowableArray< Math::tVec3u > >* mSplitTris;
	};

	tHeightFieldGeometryConverter::tHeightFieldGeometryConverter( 
		const Math::tVec2f& worldSpaceLengths, 
		const Math::tVec2i& vertexRes,
		const tHeightField& heightField )
		: tHeightFieldMesh( worldSpaceLengths, vertexRes )
	{
		tHeightFieldMesh::fRestoreHeightField( heightField, false );
	}

	void tHeightFieldGeometryConverter::fConvertToEntityDef( tHeightFieldMeshEntityDef* entityDefOut, tSigmlConverter& sigmlConverter, const f32 optimizeTarget )
	{
		tGrowableArray< Math::tVec3f > rawVerts;
		tGrowableArray< Math::tVec3u > allTris;
		tGrowableArray< tGrowableArray< Math::tVec3u > > splitTris;
		
		const b32 useMtOpt = optimizeTarget > 0.f;

		fDumpRawTriangles( rawVerts, allTris, splitTris, useMtOpt );

		if( useMtOpt )
		{
			Time::tStopWatch optTimer;

			tTerrainGeometrySimplifier sampler;
			sampler.fSimplify( optimizeTarget, &rawVerts, &splitTris );

			// Mash back and simplify once more.
			for( u32 i = 0; i < splitTris.fCount( ); ++i )
				allTris.fJoin( splitTris[i] );

			MeshSimplify::fSimplify( rawVerts, allTris, 0.001f, false, true );

			log_line( 0, "Mesh simplification overall time = " << optTimer.fGetElapsedS( ) << "s" );
		}

		// before creating gpu-buffers, split up the geometry into general zones by triangle count (to ensure we don't create inefficient render calls)
		// TODO
		//const u32 maxTrisPerRenderBuffer = 10*1024;

		// now we allocate a pseudo-gpu vertex buffer (it's actually in main RAM, but this will ultimately be copied to GPU in-game)
		const u32 totalVerts = rawVerts.fCount( );
		const Gfx::tVertexFormat& vtxFormat = Gfx::tHeightFieldMaterial::cCompressedVertexFormat;
		sigassert( sizeof( tCompressedHeightFieldRenderVertex ) == vtxFormat.fVertexSize( ) );
		mSysRamVerts.fAllocate( vtxFormat, totalVerts );

		// now we do the pseudo-gpu index buffer (similar idea to the vertex bufer)
		const u32 totalNumTris = allTris.fCount( );
		const u32 totalNumIds = totalNumTris * 3;
		const Gfx::tIndexFormat idxFormat = Gfx::tIndexFormat::fCreateAppropriateFormat( Gfx::tIndexFormat::cPrimitiveTriangleList, rawVerts.fCount( ) );
		mSysRamIndices.fAllocate( idxFormat, totalNumIds );

		// lock pseudo-gpu buffers for writing
		tCompressedHeightFieldRenderVertex* renderVerts = ( tCompressedHeightFieldRenderVertex* )fLockRenderVerts( );
		Sig::byte* renderIds = ( Sig::byte* )fLockRenderIds( );

		// convert raw verts to height field verts
		tDynamicArray< tHeightFieldRenderVertex > heightFieldVerts( totalVerts );
		for( u32 i = 0; i < totalVerts; ++i )
			heightFieldVerts[ i ] = tHeightFieldRenderVertex( rawVerts[ i ], Math::tVec3f::cZeroVector );
		// quick-n-dirty vertex normal computation, works fine
		for( u32 i = 0; i < allTris.fCount( ); ++i )
		{
			const Math::tTrianglef tri = Math::tTrianglef( rawVerts[ allTris[ i ].x ], rawVerts[ allTris[ i ].y ], rawVerts[ allTris[ i ].z ] );
			const Math::tVec3f triNormal = tri.fComputeUnitNormal( );
			heightFieldVerts[ allTris[ i ].x ].N += triNormal;
			heightFieldVerts[ allTris[ i ].y ].N += triNormal;
			heightFieldVerts[ allTris[ i ].z ].N += triNormal;
		}
		for( u32 i = 0; i < totalVerts; ++i )
			heightFieldVerts[ i ].N.fNormalizeSafe( Math::tVec3f::cYAxis );

		// compress heightfield verts into gpu stream
		for( u32 i = 0; i < totalVerts; ++i )
		{
			renderVerts[ i ] = tCompressedHeightFieldRenderVertex(
				Dx360Util::fConvertVec4fToHalf( Math::tVec4f( heightFieldVerts[ i ].P, heightFieldVerts[ i ].N.z ) ),
				Dx360Util::fConvertVec2fToHalf( Math::tVec2f( heightFieldVerts[ i ].N.x, heightFieldVerts[ i ].N.y ) ) );
		}

		// copy/store indices into gpu stream
		if( idxFormat.mSize == sizeof( u32 ) )
			fMemCpy( renderIds, allTris.fBegin( ), totalNumIds * idxFormat.mSize );
		else if( idxFormat.mSize == sizeof( u16 ) )
		{
			for( u32 i = 0; i < totalNumIds; ++i )
			{
				const u32* src = ( u32* )allTris.fBegin( );
				(( u16* )renderIds)[ i ] = ( u16 )src[ i ];
			}
		}
		else
		{
			sigassert( !"invalid index format" );
		}

		// unlock pseudo-gpu buffers for writing
		fUnlockRenderVerts( ( tHeightFieldRenderVertex* )renderVerts );
		fUnlockRenderIds( ( u16* )renderIds );

		// store chunk data
		entityDefOut->mChunkDescs.fNewArray( 1 );
		entityDefOut->mChunkDescs.fFront( ) = tHeightFieldMeshEntityDef::tChunkDesc( totalNumTris, totalVerts );

		fAddToSigmlConverter( entityDefOut, sigmlConverter );
	}

	void tHeightFieldGeometryConverter::fDumpRawTriangles( 
		tGrowableArray< Math::tVec3f >& rawVerts, 
		tGrowableArray< Math::tVec3u >& rawTris,
		tGrowableArray< tGrowableArray< Math::tVec3u > >& splitSubs,
		b32 fillSplitSubs )
	{
		// grab raw vertex/triangle list for entire height field
		rawVerts.fSetCapacity( fRez( ).fLogicalVertCount( ) );

		const u32 ixMax = fRez( ).fNumLogicalVertsX( );
		const u32 izMax = fRez( ).fNumLogicalVertsZ( );

		const u32 cPartition = 128u;
		const u32 xSplits = fMax<s32>( ( ixMax - 2 ), 0 ) / cPartition + 1;
		const u32 zSplits = fMax<s32>( ( izMax - 2 ), 0 ) / cPartition + 1;

		if( fillSplitSubs )
		{
			const u32 approxCapacity = fRez( ).fTotalTriangleCount( ) / 4;
			splitSubs.fSetCount( xSplits * zSplits );
			for( u32 i = 0; i < splitSubs.fCount( ); ++i )
				splitSubs[i].fSetCapacity( approxCapacity );
		}
		else
		{
			rawTris.fSetCapacity( fRez( ).fTotalTriangleCount( ) );
		}

		for( u32 iz = 0; iz < izMax; ++iz )
		{
			const f32 z = fRez( ).fLogicalVertexWorldZ( iz );
			for( u32 ix = 0; ix < ixMax; ++ix )
			{
				const f32 x = fRez( ).fLogicalVertexWorldX( ix );
				const tLogicalVertex& lv = mLogicalVerts[ fRez( ).fLogicalVertIndex( ix, iz ) ];

				// store vertex pos
				const Math::tVec3f p = Math::tVec3f( x, lv.fGetHeight( ), z );
				rawVerts.fPushBack( p );

				if( ix < fRez( ).fNumLogicalVertsX( )-1 && 
					iz < fRez( ).fNumLogicalVertsZ( )-1 &&
					lv.fGetQuadState( ) != tHeightFieldMesh::cQuadRemoved )
				{
					// compute the proper triangulation for this quad and store indices
					u32 tempIds[6] = {0};
					fComputeQuadRenderIndicesGeneralized( fRez( ).fNumLogicalVertsX( ), lv.fGetQuadState( ), ix, iz, tempIds );

					if( fillSplitSubs )
					{
						// store tris
						const u32 ixPlace = ix / cPartition;
						const u32 izPlace = iz / cPartition;
						const u32 listPos = ixPlace + izPlace * xSplits;

						splitSubs[ listPos ].fPushBack( Math::tVec3u( tempIds[0], tempIds[1], tempIds[2] ) );
						splitSubs[ listPos ].fPushBack( Math::tVec3u( tempIds[3], tempIds[4], tempIds[5] ) );
					}
					else
					{
						rawTris.fPushBack( Math::tVec3u( tempIds[0], tempIds[1], tempIds[2] ) );
						rawTris.fPushBack( Math::tVec3u( tempIds[3], tempIds[4], tempIds[5] ) );
					}
				}
			}
		}
	}

	void tHeightFieldGeometryConverter::fAddToSigmlConverter( tHeightFieldMeshEntityDef* entityDefOut, tSigmlConverter& sigmlConverter )
	{
		tStrongPtr< tSigmlConverter::tMeshData > meshData( new tSigmlConverter::tMeshData );

		std::stringstream ss;
		ss << "." << sigmlConverter.fGetMeshDataCount( ) << Gfx::tGeometryFile::fGetFileExtension( );
		meshData->mResourcePath = tFilePathPtr::fSwapExtension( sigmlConverter.fGetResourcePath( ), ss.str( ).c_str( ) );

		meshData->mGeometryData.fSetCount( 1 );
		meshData->mIndexData.fSetCount( 1 );

		meshData->mGeometryData.fFront( ) = fGetSysRamVerts( );
		meshData->mIndexData.fFront( ).mIndexBuffer = fGetSysRamIndices( );
		meshData->mIndexData.fFront( ).mNumTris = meshData->mIndexData.fFront( ).mIndexBuffer.fIndexCount( ) / 3;

		entityDefOut->mGeometryFile = sigmlConverter.fAddLoadInPlaceResourcePtr( tResourceId::fMake<Gfx::tGeometryFile>( meshData->mResourcePath ) );
		entityDefOut->mBounds = fComputeBounds( );
		entityDefOut->fSetLogicalVerts( fGetLogicalVerts( ) );
		entityDefOut->fSetQuadTree( fGetQuadTree( ) );

		sigmlConverter.fAddMeshData( meshData );
	}

	tHeightFieldRenderVertex* tHeightFieldGeometryConverter::fLockRenderVerts( u32 startVertex, u32 numVerts )
	{
		return ( ( tHeightFieldRenderVertex* )mSysRamVerts.fBegin( ) ) + startVertex;
	}

	void tHeightFieldGeometryConverter::fUnlockRenderVerts( tHeightFieldRenderVertex* lockedVerts )
	{
	}

	u16* tHeightFieldGeometryConverter::fLockRenderIds( u32 startIndex, u32 numIds )
	{
		return ( ( u16* )mSysRamIndices.fGetBuffer( ).fBegin( ) ) + startIndex;
	}

	void tHeightFieldGeometryConverter::fUnlockRenderIds( u16* lockedIds )
	{
	}

}
