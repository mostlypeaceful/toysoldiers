//------------------------------------------------------------------------------
// \file tGeometryFile.cpp - 10 Aug 2011
// \author jwittner & cbramwell
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tGeometryFile.hpp"
#include "tDevice.hpp"
#include "tDirectResourceLoader.hpp"

namespace Sig { namespace Gfx
{
	namespace
	{
		static const u32 cMaxChildReads = 10;
	}

	//------------------------------------------------------------------------------
	// tGeometryFile
	//------------------------------------------------------------------------------
	define_lip_version( tGeometryFile, 2, 2, 2 );

	const char* tGeometryFile::fGetFileExtension( )
	{
		return ".geob";
	}

	//------------------------------------------------------------------------------
	tGeometryFile::tGeometryFile( )
		: mHeaderSize( 0 )
	{
	}

	//------------------------------------------------------------------------------
	tGeometryFile::tGeometryFile( tNoOpTag )
		: tLoadInPlaceFileBase( cNoOpTag )
		, mGeometryPointers( cNoOpTag )
		, mIndexListPointers( cNoOpTag )
		, mIndexGroups( cNoOpTag )
	{
	}

	//------------------------------------------------------------------------------
	tGeometryFile::~tGeometryFile( )
	{
	}

	//------------------------------------------------------------------------------
	void tGeometryFile::fOnFileLoaded( const tResource& ownerResource )
	{
		Memory::tAllocStamp stamp = vram_alloc_stamp( cAllocStampContextGeometry );
		stamp.mUserString.fReset( ownerResource.fGetPath( ).fCStr( ) );
		const Memory::tVramContextScope cVramContext( stamp );

		// Get the loader
		tDirectResourceLoader * loader;
		{
			tResourceLoader * resLoader = ownerResource.fGetLoader( );
			loader = resLoader ? resLoader->fDynamicCast<tDirectResourceLoader>( ) : NULL;
			log_assert( loader, ownerResource.fGetPath( ) << ": tGeometryFile should be loaded with a tDirectResourceLoader!" );
		}

		loader->fSetChildReadsCompleteCB(
			make_delegate_memfn(
				tDirectResourceLoader::tOnChildReadsComplete,
				tGeometryFile,
				fOnFileLoaded ) );

		// We work front to back kicking reads so we can be assured we've already updated all the 
		// all the previous loads to the ready state before we run out of reads
		u32 readsLeft = cMaxChildReads;

		// Create all our buffers for the gfx card
		const tDevicePtr& device = tDevice::fGetDefaultDevice( );
		sigassert( mGeometryPointers[ 0 ].mVRamBuffer.fVertexCount( ) > 0 );
		for(u32 i = 0; i < mGeometryPointers.fCount( ); ++i)
		{
			tGeometryPointer& ptr =  mGeometryPointers[ i ];
			
			if( ptr.mState == cBufferStateLoading )
			{
				ptr.mVRamBuffer.fDeepUnlock( );
				ptr.mState = cBufferStateReady;
			}
			else if( ptr.mState == cBufferStateNull )
			{
				
				sigassert( ptr.mState == cBufferStateNull );

				// Do the allocation
				ptr.mVRamBuffer.fAllocateInPlace( device, NULL );

				tAsyncFileReader * fileReader = loader->fCreateChildReader( );

				// Start the chunk read
				fileReader->fRead( 
						tAsyncFileReader::tReadParams( 
							tAsyncFileReader::tRecvBuffer( ptr.mVRamBuffer.fDeepLock( ), ptr.mBufferSize, true ),
						ptr.mBufferSize,
						fileReader->mFileOffset + ptr.mBufferOffset,
						false //decompressAfterRead
					) 
				);
				ptr.mState = cBufferStateLoading;

				if( !--readsLeft )
					return;
			}
		}

		for(u32 i = 0; i < mIndexListPointers.fCount( ); ++i)
		{
			tIndexListPointer& ptr =  mIndexListPointers[ i ];

			if( ptr.mState == cBufferStateLoading )
			{
				ptr.mVRamBuffer.fDeepUnlock( );
				ptr.mState = cBufferStateReady;
			}
			else if( ptr.mState == cBufferStateNull )
			{
				sigassert( ptr.mState == cBufferStateNull );

				// Do the allocation
				ptr.mVRamBuffer.fAllocateInPlace( device, NULL );

				tAsyncFileReader * fileReader = loader->fCreateChildReader( );

				// Start the chunk read
				fileReader->fRead( 
						tAsyncFileReader::tReadParams( 
							tAsyncFileReader::tRecvBuffer( ptr.mVRamBuffer.fDeepLock( ), ptr.mBufferSize, true ),
						ptr.mBufferSize,
						fileReader->mFileOffset + ptr.mBufferOffset, // readByteOffset
						false //decompressAfterRead
					) 
				);
				ptr.mState = cBufferStateLoading;

				if( !--readsLeft )
					return;
			}
		}
	}

	//------------------------------------------------------------------------------
	void tGeometryFile::fOnFileUnloading( const tResource& ownerResource )
	{
		// Remove everything from the gfx card
		for(u32 i = 0; i < mGeometryPointers.fCount( ); ++i)
		{
			sigassert( mGeometryPointers[ i ].mState == cBufferStateReady );
			mGeometryPointers[ i ].mState = cBufferStateNull;
			mGeometryPointers[ i ].mVRamBuffer.fDeallocateInPlace( );
		}
		for(u32 i = 0; i < mIndexListPointers.fCount( ); ++i)
		{
			sigassert( mIndexListPointers[ i ].mState == cBufferStateReady );
			mIndexListPointers[ i ].mState = cBufferStateNull;
			mIndexListPointers[ i ].mVRamBuffer.fDeallocateInPlace( );
		}
	}

	//------------------------------------------------------------------------------
	u32 tGeometryFile::fComputeStorage( std::string & display ) const
	{
		u32 geoVRamUsed = 0, geoVRamMax = 0;
		u32 indiVRamUsed = 0, indiVRamMax = 0;

		fVramUsage( geoVRamUsed, geoVRamMax, indiVRamUsed, indiVRamMax );

		const u32 totalVRamUsed = geoVRamUsed + indiVRamUsed;
		const u32 totalVRamMax = geoVRamMax + indiVRamMax;
		const u32 mainUsed = fMainUsage( );

		std::stringstream ss;
		ss << std::fixed << std::setprecision( 2 ) << 
			"Total - " << Memory::fToMB<f32>( mainUsed + totalVRamMax )  << " MB " <<
			"Main - " << Memory::fToKB<f32>( mainUsed )					<< " KB " <<
			"Vid - "  << Memory::fToKB<f32>( totalVRamUsed ) << " / " << Memory::fToKB<f32>( totalVRamMax ) << " KB " <<
			"Vtx - "  << Memory::fToKB<f32>( geoVRamUsed )   << " / " << Memory::fToKB<f32>( geoVRamMax )	<< " KB " << 
			"Idx - "  << Memory::fToKB<f32>( indiVRamUsed )  << " / " << Memory::fToKB<f32>( indiVRamMax )  << " KB";

		display = ss.str( );

		return mHeaderSize + totalVRamMax;
	}

	//------------------------------------------------------------------------------
	const tGeometryBufferVRam * tGeometryFile::fRequestGeometry( u32 index )
	{
		tGeometryFile::tGeometryPointer & ptr = mGeometryPointers[ index ];
		sigassert( ptr.mState == cBufferStateReady );

		++ptr.mRefCount;
		return &ptr.mVRamBuffer;
	}

	//------------------------------------------------------------------------------
	const tIndexBufferVRam * tGeometryFile::fRequestIndices( 
		u32 subMesh, f32 lodRatio, tGeometryFile::tIndexWindow & window )
	{
		u32 idx;
		fSelectLod( subMesh, lodRatio, idx, window );

		tGeometryFile::tIndexListPointer & indexList = mIndexListPointers[ idx ];
		sigassert( indexList.mState == cBufferStateReady );
		
		++indexList.mRefCount;
		return &indexList.mVRamBuffer;
	}

	//------------------------------------------------------------------------------
	void tGeometryFile::fSelectLod( 
		u32 subMesh, f32 ratio, u32 & idx, tGeometryFile::tIndexWindow & window )
	{
		const tGeometryFile::tIndexLodGroup & group = mIndexGroups[ subMesh ];
		u32 windowIndex = fRound<u32>( ( group.mTotalWindowCount - 1 ) * ratio );

		// Determine the ideal lod and sub lod window index
		u32 idealLod;
		for( idealLod = 0; idealLod < group.mLods.fCount( ); ++idealLod )
		{
			if( windowIndex >= group.mLods[ idealLod ].mWindows.fCount( ) )
				windowIndex -= group.mLods[ idealLod ].mWindows.fCount( );
			else
				break;
		}

		sigassert( idealLod < group.mLods.fCount( ) && "Sanity!" );

		// Store the ideal index
		idx = idealLod + group.mPointerStart;

		// May not be the ideal buffer but we've found one to render
		window = group.mLods[ idealLod ].mWindows[ windowIndex ];
	}

	//------------------------------------------------------------------------------
	u32 tGeometryFile::fLODMemoryInBytes( ) const
	{
		u32 result = 0;

		const u32 indexSize = mIndexListPointers[ 0 ].mVRamBuffer.fIndexFormat( ).mSize;

		for( u32 i = 0; i < mIndexGroups.fCount( ); ++i )
		{
			//doesnt count highest lod
			for( s32 a = 0; a < (s32)mIndexGroups[ i ].mLods.fCount( ) - 1; ++a )
			{
				const tIndexLod& lod = mIndexGroups[ i ].mLods[ a ];
				for( u32 w = 0; w < lod.mWindows.fCount( ); ++w )
					result += lod.mWindows[ w ].mNumFaces * 3 * indexSize;
			}
		}

		return result;
	}

	//------------------------------------------------------------------------------
	u32 tGeometryFile::fVramUsage( u32& geoVRamUsed, u32& geoVRamMax, u32& indiVRamUsed, u32& indiVRamMax ) const
	{
		geoVRamUsed = 0; geoVRamMax = 0;
		indiVRamUsed = 0; indiVRamMax = 0;

		// Geometry
		const u32 geomCount = mGeometryPointers.fCount( );
		for( u32 g = 0; g < geomCount; ++g )
		{
			geoVRamMax += mGeometryPointers[ g ].mBufferSize;
			if( mGeometryPointers[ g ].mVRamBuffer.fPlatformHandle( ) )
				geoVRamUsed += mGeometryPointers[ g ].mBufferSize;
		}

		// Indices
		const u32 ibCount = mIndexListPointers.fCount( );
		for( u32 i = 0; i < ibCount; ++i )
		{
			indiVRamMax += mIndexListPointers[ i ].mBufferSize;
			if( mIndexListPointers[ i ].mVRamBuffer.fPlatformHandle( ) )
				indiVRamUsed += mIndexListPointers[ i ].mBufferSize;
		}

		return geoVRamUsed + indiVRamUsed;
	}

}}
