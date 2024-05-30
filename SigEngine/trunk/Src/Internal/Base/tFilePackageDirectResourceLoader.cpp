//------------------------------------------------------------------------------
// \file tFilePackageDirectResourceLoader.cpp - 04 Oct 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tFilePackageDirectResourceLoader.hpp"
#include "tLoadInPlaceResourceBuffer.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	tFilePackageDirectResourceLoader::tFilePackageDirectResourceLoader(
		tResource* resource,
		const tAsyncFileReaderPtr& fileReader,
		u32 headerSizeOffset,
		u32 fileOffset, 
		u32 fileSizeActual,
		u32 fileSizeUncompressed, 
		b32 decompress )
		: tDirectResourceLoader( headerSizeOffset, resource )
		, mFileOffset( fileOffset )
		, mFileSizeActual( fileSizeActual )
		, mFileSizeUncompressed( fileSizeUncompressed )
		, mDecompress( decompress )
	{
		sigassert( !mDecompress && "Decompression not available on streaming resources" );
		sigassert( mFileSizeUncompressed >= mFileSizeActual );
		mFileReader = fileReader;
	}

	//------------------------------------------------------------------------------
	void tFilePackageDirectResourceLoader::fInitiate( )
	{
		fSetFileTimeStamp( 0 );
		fSetResourceBuffer( NEW_TYPED( tLoadInPlaceResourceBuffer )( ) );
		fSetSelfOnResource( );
	}

	//------------------------------------------------------------------------------
	void tFilePackageDirectResourceLoader::fOnOpenSuccess( )
	{
		sigassert( mLoadState == cLoadStateOpening );

		if( fGetCancel( ) )
			return; // don't allocate or begin reading if we've been cancelled

		mLoadState = cLoadStateHeaderSize;

		// Read the header size
		mFileReader->fRead( 
			tAsyncFileReader::tReadParams( 
				tAsyncFileReader::tRecvBuffer( (byte*)&mHeaderSize, sizeof( mHeaderSize ) ),
				sizeof( mHeaderSize ),
				mFileOffset + mHeaderSizeOffset, // readByteOffset
				mDecompress //decompressAfterRead - asserted false in constructor
			)
		);
	}

	//------------------------------------------------------------------------------
	void tFilePackageDirectResourceLoader::fOnReadHeaderSizeSuccess( )
	{
		sigassert( mLoadState == cLoadStateHeaderSize );

#ifdef sig_assert
		u32 numBytesAllocated=0;
		byte* readBuffer = mFileReader->fForgetBuffer( &numBytesAllocated );
		sigassert( readBuffer == (void*)&mHeaderSize );
		sigassert( numBytesAllocated > 0 && numBytesAllocated == sizeof( mHeaderSize ) );
#else
		mFileReader->fForgetBuffer( );
#endif

		// Don't start another read if the file has been cancelled
		if( fGetCancel( ) )
			return;
		
		mLoadState = cLoadStateHeader;

		tGenericBuffer* resBuffer = fGetResourceBuffer( );
		resBuffer->fAlloc( mHeaderSize, fMakeStamp( mHeaderSize ) );

		// Read the header
		mFileReader->fRead( 
			tAsyncFileReader::tReadParams( 
				tAsyncFileReader::tRecvBuffer( resBuffer->fGetBuffer( ), mHeaderSize ),
				mHeaderSize,
				mFileOffset, // readByteOffset
				mDecompress	// decompressAfterRead - asserted false in constructor
			) 
		);
	}

	//------------------------------------------------------------------------------
	tAsyncFileReaderPtr tFilePackageDirectResourceLoader::fCreateChildReaderInternal( )
	{
		return mFileReader->fSpawnChild( mFileOffset );
	}
}
