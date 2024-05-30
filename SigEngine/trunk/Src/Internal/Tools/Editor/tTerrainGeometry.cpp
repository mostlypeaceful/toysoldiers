//------------------------------------------------------------------------------
// \file tTerrainGeometry.cpp - 02 Sep 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "ToolsPch.hpp"
#include "tTerrainGeometry.hpp"
#include "tHeightFieldMeshEntity.hpp"
#include "Gfx/tMaterialFile.hpp"
#include "Gfx/tHeightFieldMaterial.hpp"
#include "Gfx/tHeightFieldTransitionMaterial.hpp"
#include "Gfx/tDisplayList.hpp"
#include "Gfx/tDevice.hpp"
#include "Dx360Util.hpp"
#include "MeshSimplify.hpp"
#include "tLogicThreadPool.hpp"
#include "tProjectFile.hpp"

namespace Sig
{
	tFilePathPtr fMaterialTypeToPath( tTerrainGeometry::tMaterialType hfMatType )
	{
		switch( hfMatType )
		{
		case tTerrainGeometry::cMaterialTypeDefault: return Gfx::tHeightFieldMaterial::fMaterialFilePath( );
		case tTerrainGeometry::cMaterialTypeTransition: return Gfx::tHeightFieldTransitionMaterial::fMaterialFilePath( );
		default: sig_nodefault( );
		}
		return tFilePathPtr( );		
	}


	tTerrainGeometry::tTerrainGeometry( 
		const tResourceDepotPtr& resourceDepot,
		const Gfx::tDevicePtr& device, 
		const Math::tVec2f& worldSpaceLengths, 
		const Math::tVec2i& vertexRes,
		const Math::tVec2i& materialRes,
		const tResourcePtr& diffuseMap,
		const tResourcePtr& normalMap )
		: tHeightFieldMesh( worldSpaceLengths, vertexRes )
		, mHFMatType( cMaterialTypeDefault )
	{
		fAcquireMaterial( device, resourceDepot, mHFMatType, materialRes, diffuseMap, normalMap );

		const u32 vertCount = fRez( ).fTotalVertCount( );
		const u32 numQuadsPerChunk = fRez( ).fNumQuadsPerChunk( );
		const u32 trisPerChunk = 2 * numQuadsPerChunk;
		const u32 idsPerChunk = 3 * trisPerChunk;

		sigassert( mTerrainMaterial->fVertexFormat( ).fFullyEqual( Gfx::tHeightFieldMaterialBase::cVertexFormat ) );
		const Gfx::tVertexFormat& vtxFormat = Gfx::tHeightFieldMaterialBase::cVertexFormat;
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

		fRefreshGraphicsBlock( 
			0,
			0,
			mRez.fNumLogicalVertsX( ) - 1,
			mRez.fNumLogicalVertsZ( ) - 1,
			0,
			mVRamVerts.fVertexCount( ) );
	}

	void tTerrainGeometry::fCollectEntities( const tCollectEntitiesParams& params ) const
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
				const u32 chunkIndex = fRez( ).fChunkIndex( ichunkx, ichunkz );
				const Math::tAabbf objectSpaceBox = fRez( ).fChunkBounds( ichunkx, ichunkz );
				tHeightFieldMeshEntity* entity = new tHeightFieldMeshEntity( chunkIndex, Gfx::tRenderBatch::fCreate( batch ), this, objectSpaceBox );
				entity->fSpawnImmediate( params.mParent );

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
		u32 destDimX, u32 destDimZ,
		b32 preserveSpacing, 
		b32 preserveLeft, 
		b32 preserveTop, 
		const t & defValue )
	{
		if( srcDimX == destDimX && srcDimZ == destDimZ )
			return;

		tDynamicArray<t> newSamples( destDimX * destDimZ );

		// Normalized spacing samples
		if( !preserveSpacing )
		{
			for( u32 x = 0; x < destDimX; ++x )
			{
				u32 oldx = fRound<u32>( ( x / (f32)( destDimX - 1 ) ) * ( srcDimX - 1 ) );
				for( u32 z = 0; z < destDimZ; ++z )
				{
					u32 oldz = fRound<u32>( ( z / (f32)( destDimZ - 1 ) ) * ( srcDimZ - 1 ) );
					newSamples[z * destDimX + x] = samples[ oldz * srcDimX + oldx ];
				}
			}
		}

		// Direct spacing samples
		else
		{
			u32 srcXStart, srcZStart, destXStart, destZStart;
			u32 xCopies, zCopies;

			// We've shrunk
			if( destDimX < srcDimX || destDimZ < srcDimZ )
			{
				sigassert( destDimX <= srcDimX && destDimZ <= srcDimZ );

				srcXStart = preserveLeft ? 0 : srcDimX - destDimX;
				srcZStart = preserveTop ? 0 : srcDimZ - destDimZ;
				destXStart = 0;
				destZStart = 0;

				xCopies = destDimX;
				zCopies = destDimZ;
			}
			else // We've grown
			{
				sigassert( destDimX >= srcDimX && destDimZ >= srcDimZ );

				srcXStart = 0;
				srcZStart = 0;
				destXStart = preserveLeft ? 0 : destDimX - srcDimX;
				destZStart = preserveTop ? 0 : destDimZ - srcDimZ;

				xCopies = srcDimX;
				zCopies = srcDimZ;
			}

			// Fill with default value
			newSamples.fFill( defValue );

			// Copy over the overlap
			for( u32 x = 0; x < xCopies; ++x )
			{
				const u32 srcX = x + srcXStart;
				const u32 dstX = x + destXStart;

				for( u32 z = 0; z < zCopies; ++z )
				{
					const u32 srcZ = z + srcZStart;
					const u32 dstZ = z + destZStart;

					newSamples[ dstZ * destDimX + dstX ] = samples[ srcZ * srcDimX + srcX ];
				}
			}
		}

		samples.fSwap( newSamples );
	}

	//------------------------------------------------------------------------------
	b32 tTerrainGeometry::fUpdateGroundCoverDims( 
		tGroundCoverInternal & cover, f32 unitSize, u32 maxUnitSpawns, b32 preserveSpacing, b32 preserveLeft, b32 preserveTop )
	{
		sigassert( !cover.mEditors );

		const u32 dimx = fRound<u32>( fRez( ).fWorldLengthX( ) / unitSize );
		const u32 dimz = fRound<u32>( fRez( ).fWorldLengthZ( ) / unitSize );

		// Some array dims have changed
		if( dimx != cover.mDimX || dimz != cover.mDimZ || maxUnitSpawns != cover.mMaxUnitSpawns )
		{
			fResize( cover.mMask, cover.mDimX, cover.mDimZ, dimx, dimz, 
				preserveSpacing, preserveLeft, preserveTop, 1.f );

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
		tMaterialType hfType,
		const Math::tVec2i& materialRes,
		const tResourcePtr& diffuseMap,
		const tResourcePtr& normalMap )
	{
		mHFMatType = hfType;

		// load terrain material
		mTerrainMaterialFile = resourceDepot->fQuery( tResourceId::fMake<Gfx::tMaterialFile>( fMaterialTypeToPath( mHFMatType ) ) );
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

		sigassert( mTerrainMaterial->fVertexFormat( ).fFullyEqual( Gfx::tHeightFieldMaterialBase::cVertexFormat ) );
	}

	void tTerrainGeometry::fUpdateDynamicTextureReferences( const tResourcePtr& diffuseMap, const tResourcePtr& normalMap )
	{
		tFixedArray<Math::tVec4f,8> oldTilingFactors;
		if( mTerrainMaterial )
			oldTilingFactors = mTerrainMaterial->mTileFactors;
		else
		{
			for( u32 i = 0; i < oldTilingFactors.fCount( ); ++i )
				oldTilingFactors[ i ] = Math::tVec4f( 0.5f );
		}

		Gfx::tHeightFieldMaterialBase* hfmtl = NULL;
		switch( mHFMatType )
		{
		case cMaterialTypeDefault: hfmtl = NEW_TYPED( Gfx::tHeightFieldMaterial )( ); break;
		case cMaterialTypeTransition: hfmtl = NEW_TYPED( Gfx::tHeightFieldTransitionMaterial )( ); break;
		default: sig_nodefault( );
		};
		mTerrainMaterial.fReset( hfmtl );
		mTerrainMaterial->fSetMaterialFileResourcePtrOwned( *mTerrainMaterialFile->fGetOwner( ), mTerrainMaterialFile->fGetPath( ) );

		const Gfx::tTextureFile* diffuseMapTex = diffuseMap ? diffuseMap->fCast<Gfx::tTextureFile>( ) : 0;
		const Gfx::tTextureFile* normalMapTex = normalMap ? normalMap->fCast<Gfx::tTextureFile>( ) : 0;
		if( diffuseMapTex && diffuseMapTex->mIsAtlas )
		{
			hfmtl->mWorldSpaceDims = Math::tVec2f( fRez( ).fWorldLengthX( ), fRez( ).fWorldLengthZ( ) );
			hfmtl->mSubDiffuseRectDims = Math::tVec2f( diffuseMapTex->mSubTexWidth, diffuseMapTex->mSubTexHeight );
			hfmtl->mDiffuseCount_NormalCount.x = (f32)diffuseMapTex->mSubTexCountX;
			hfmtl->mDiffuseCount_NormalCount.y = (f32)diffuseMapTex->mSubTexCountY;
		}
		if( normalMapTex && normalMapTex->mIsAtlas )
		{
			hfmtl->mSubNormalRectDims = Math::tVec2f( normalMapTex->mSubTexWidth, normalMapTex->mSubTexHeight );
			hfmtl->mDiffuseCount_NormalCount.z = (f32)normalMapTex->mSubTexCountX;
			hfmtl->mDiffuseCount_NormalCount.w = (f32)normalMapTex->mSubTexCountY;
		}

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
			hfmtl->fSetLightingPassToUseNormalMap( );

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

	void tTerrainGeometry::fCopyMaterialTilingFactors( const tTerrainGeometry& other )
	{
		if( other.mTerrainMaterial && mTerrainMaterial )
			mTerrainMaterial->mTileFactors = other.mTerrainMaterial->mTileFactors;
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
		return fUpdateGroundCoverDims( id, unitSize, maxUnitSpawns, false, false, false );
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
	b32 tTerrainGeometry::fUpdateGroundCoverDims( 
		u32 id, f32 unitSize, u32 maxUnitSpawns, b32 preserveSpacing, b32 preserveLeft, b32 preserveTop )
	{
		return fUpdateGroundCoverDims( 
			*fFindGroundCover( id ), unitSize, maxUnitSpawns, preserveSpacing, preserveLeft, preserveTop );
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
		// For sanity
		sigassert( sizeof( tCompressedHeightFieldRenderVertex ) == Gfx::tHeightFieldMaterialBase::cCompressedVertexFormat.fVertexSize( ) );

		tGrowableArray< Math::tVec3f > rawVerts;
		tGrowableArray< tGrowableArray< Math::tVec3u > > splitTris;

		// Grab the triangles
		fDumpRawTriangles( rawVerts, splitTris );
		//splitTris.fSetCount( 1 );
		//fDumpRawTriangles( rawVerts, splitTris[ 0 ] );
		u32 subMeshCount = splitTris.fCount( );
		const u32 vertCount = rawVerts.fCount( );

		// Build the render verts
		tDynamicArray<tCompressedHeightFieldRenderVertex> renderVerts;
		{
			// First step - build the allTris list
			tGrowableArray< Math::tVec3u > allTris;
			for( u32 m = 0; m < subMeshCount; ++m )
				allTris.fJoin( splitTris[ m ] );

			// Then build the render verts table	
			fConstructVerts( 
				vertCount, rawVerts.fBegin( ), 
				allTris.fCount( ), allTris.fBegin( ), renderVerts );
		}

		// Build the submesh data
		tGrowableArray<tHeightFieldMeshEntityDef::tChunkDescription> chunks; chunks.fReserve( subMeshCount );
		tGrowableArray< Gfx::tGeometryBufferSysRam > geos; geos.fReserve( subMeshCount );
		tGrowableArray< Gfx::tIndexBufferSysRam > ibs; ibs.fReserve( subMeshCount );
		{
			tDynamicArray< u32 > vertMap( vertCount );
			tGrowableArray< u32 > vertsToUse; vertsToUse.fReserve( vertCount );

			u32 realSubMeshCount = 0;
			for( u32 m = 0; m < subMeshCount; ++m )
			{
				Math::tAabbf bounds; bounds.fInvalidate( );

				// Remap the verts and build a list of verts to copy
				vertMap.fFill( ~0 );
				vertsToUse.fSetCount( 0 );
				const tGrowableArray<Math::tVec3u> & tris = splitTris[ m ];
				const u32 triCount = tris.fCount( );
				for( u32 t = 0; t < triCount; ++t )
				{
					const Math::tVec3u & tri = tris[ t ];
					for( u32 v = 0; v < tri.cDimension; ++ v )
					{
						u32 id = tri[ v ];
						if( vertMap[ id ] == ~0 )
						{
							vertMap[ id ] = vertsToUse.fCount( );
							vertsToUse.fPushBack( id );
							bounds |= rawVerts[ id ];
						}
					}
				}

				const u32 subVertCount = vertsToUse.fCount( );
				if( !subVertCount ) // No verts means no tris
					continue;

				++realSubMeshCount; // Track the real submesh count

				Gfx::tGeometryBufferSysRam & geo = geos.fPushBack( );
				Gfx::tIndexBufferSysRam & ib = ibs.fPushBack( );

				// Build the bounds
				tHeightFieldMeshEntityDef::tChunkDescription & chunk = chunks.fPushBack( );
				chunk.mBounds = bounds;

				// Allocate our geo
				geo.fAllocate( 
					Gfx::tHeightFieldMaterialBase::cCompressedVertexFormat, 
					subVertCount );

				// Copy the vertices into the geo buffer
				for( u32 v = 0; v < subVertCount; ++v )
				{
					const tCompressedHeightFieldRenderVertex & src = renderVerts[ vertsToUse[ v ] ];
					fMemCpy( geo.fGetVertexBase( v ), &src, sizeof( src ) ); 
				}

				// Allocate our ib
				ib.fAllocate( 
					Gfx::tIndexFormat::fCreateAppropriateFormat( 
						Gfx::tIndexFormat::cPrimitiveTriangleList, subVertCount ),
					tris.fCount( ) * 3 );

				// Map the indices into the ib
				for( u32 t = 0; t < triCount; ++t )
				{
					const Math::tVec3u & tri = tris[ t ];
					const u32 mapped[3] = { vertMap[ tri.x ], vertMap[ tri.y ], vertMap[ tri.z ] };

					ib.fSetIndices( t * 3, mapped, 3 );
				}
			}

			// Use the real count
			subMeshCount = realSubMeshCount;
		}

		// Assign the chunks
		entityDefOut->mChunks.fInitialize( chunks.fBegin( ), chunks.fCount( ) );

		// Add to the sigml converter
		
		tFilePathPtr resourcePath;
		{
			std::stringstream ss;
			ss << "." << sigmlConverter.fGetMeshDataCount( ) << Gfx::tGeometryFile::fGetFileExtension( );
			resourcePath = tFilePathPtr::fSwapExtension( sigmlConverter.fGetResourcePath( ), ss.str( ).c_str( ) );
		}

		const tProjectFile & projectFile = tProjectFile::fInstance( );

		//entityDefOut->mGeometryFile = sigmlConverter.fAddMeshData( subMeshCount, resourcePath, geos.fBegin( ), ibs.fBegin( ) );
		const f32 highLodRatio = 1.f - optimizeTarget;
		const f32 mediumLodRatio = 0.5f;
		const f32 lowLodRatio = 0.25f;
		entityDefOut->mGeometryFile = sigmlConverter.fAddProgressiveMeshData( 
			highLodRatio, mediumLodRatio, lowLodRatio,
			subMeshCount, resourcePath, geos.fBegin( ), ibs.fBegin( ) );

		entityDefOut->mBounds = fComputeBounds( );
		entityDefOut->fSetLogicalVerts( fGetLogicalVerts( ) );
		entityDefOut->fSetQuadTree( fGetQuadTree( ) );
	}

	void tHeightFieldGeometryConverter::fDumpRawTriangles( 
		tGrowableArray< Math::tVec3f >& rawVerts, 
		tGrowableArray< Math::tVec3u >& rawTris )
	{
		// grab raw vertex/triangle list for entire height field
		rawVerts.fSetCapacity( fRez( ).fLogicalVertCount( ) );
		rawTris.fSetCapacity( fRez( ).fTotalTriangleCount( ) );

		const u32 ixMax = fRez( ).fNumLogicalVertsX( );
		const u32 izMax = fRez( ).fNumLogicalVertsZ( );

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

				if( ix < ixMax - 1 && iz < izMax - 1 && lv.fGetQuadState( ) != tHeightFieldMesh::cQuadRemoved )
				{
					// compute the proper triangulation for this quad and store indices
					u32 tempIds[6] = {0};
					fComputeQuadRenderIndicesGeneralized( fRez( ).fNumLogicalVertsX( ), lv.fGetQuadState( ), ix, iz, tempIds );
					
					rawTris.fPushBack( Math::tVec3u( tempIds[0], tempIds[1], tempIds[2] ) );
					rawTris.fPushBack( Math::tVec3u( tempIds[3], tempIds[4], tempIds[5] ) );
				}
			}
		}
	}

	//------------------------------------------------------------------------------
	void tHeightFieldGeometryConverter::fDumpRawTriangles(
		tGrowableArray< Math::tVec3f >& rawVerts,
		tGrowableArray< tGrowableArray< Math::tVec3u > >& splitTris )
	{
		// grab raw vertex/triangle list for entire height field
		rawVerts.fSetCapacity( fRez( ).fLogicalVertCount( ) );

		const u32 ixMax = fRez( ).fNumLogicalVertsX( );
		const u32 izMax = fRez( ).fNumLogicalVertsZ( );

		const u32 cPartition = 16u;
		const u32 xSplits = fMax<s32>( ( ixMax - 2 ), 0 ) / cPartition + 1;
		const u32 zSplits = fMax<s32>( ( izMax - 2 ), 0 ) / cPartition + 1;

		const u32 approxCapacity = fRez( ).fTotalTriangleCount( ) / 4;
		splitTris.fSetCount( xSplits * zSplits );
		for( u32 i = 0; i < splitTris.fCount( ); ++i )
			splitTris[i].fSetCapacity( approxCapacity );

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

				if( ix < ixMax - 1 && iz < izMax - 1 && lv.fGetQuadState( ) != tHeightFieldMesh::cQuadRemoved )
				{
					// compute the proper triangulation for this quad and store indices
					u32 tempIds[6] = { 0 };
					fComputeQuadRenderIndicesGeneralized( fRez( ).fNumLogicalVertsX( ), lv.fGetQuadState( ), ix, iz, tempIds );

					// store tris
					const u32 ixPlace = ix / cPartition;
					const u32 izPlace = iz / cPartition;
					const u32 listPos = ixPlace + izPlace * xSplits;

					splitTris[ listPos ].fPushBack( Math::tVec3u( tempIds[0], tempIds[1], tempIds[2] ) );
					splitTris[ listPos ].fPushBack( Math::tVec3u( tempIds[3], tempIds[4], tempIds[5] ) );
				}
			}
		}
	}

	//------------------------------------------------------------------------------
	void tHeightFieldGeometryConverter::fConstructVerts( 
		u32 vertCount, const Math::tVec3f verts[], 
		u32 triCount, const Math::tVec3u tris[],
		tDynamicArray<tCompressedHeightFieldRenderVertex> & out )
	{
		tDynamicArray< tHeightFieldRenderVertex > buildVerts( vertCount );

		// First construct the builder verts
		for( u32 v = 0; v < vertCount; ++v )
			buildVerts[ v ] = tHeightFieldRenderVertex( verts[ v ], Math::tVec3f::cZeroVector );

		// Then build the vertex normals by combining all normals and then normalizing
		for( u32 t = 0; t < triCount; ++t )
		{
			const Math::tVec3u & triIds = tris[ t ];
			const Math::tTrianglef tri = Math::tTrianglef( 
				verts[ triIds.x ], verts[ triIds.y ], verts[ triIds.z ] );

			const Math::tVec3f triNormal = tri.fComputeUnitNormal( );
			buildVerts[ triIds.x ].N += triNormal;
			buildVerts[ triIds.y ].N += triNormal;
			buildVerts[ triIds.z ].N += triNormal;
		}

		// Normalize the verts safely
		for( u32 v = 0; v < vertCount; ++v )
			buildVerts[ v ].N.fNormalizeSafe( Math::tVec3f::cYAxis );

		// Now compress the verts into the render vert buffer
		out.fNewArray( vertCount );
		for( u32 v = 0; v < vertCount; ++v )
		{
			const tHeightFieldRenderVertex & vert = buildVerts[ v ];
			out[ v ] = tCompressedHeightFieldRenderVertex(
				Dx360Util::fConvertVec4fToHalf( Math::tVec4f( vert.P, vert.N.z ) ),
				Dx360Util::fConvertVec2fToHalf( Math::tVec2f( vert.N.x, vert.N.y ) ) );
		}
	}

	tHeightFieldRenderVertex* tHeightFieldGeometryConverter::fLockRenderVerts( u32 startVertex, u32 numVerts )
	{
		return NULL;
	}

	void tHeightFieldGeometryConverter::fUnlockRenderVerts( tHeightFieldRenderVertex* lockedVerts )
	{
	}

	u16* tHeightFieldGeometryConverter::fLockRenderIds( u32 startIndex, u32 numIds )
	{
		return NULL;
	}

	void tHeightFieldGeometryConverter::fUnlockRenderIds( u16* lockedIds )
	{
	}

}
