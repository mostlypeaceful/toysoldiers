//------------------------------------------------------------------------------
// \file tDirectResourceLoader.cpp - 11 Aug 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tDirectResourceLoader.hpp"
#include "tLoadInPlaceResourceBuffer.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	tDirectResourceLoader::tDirectResourceLoader( u32 headerSizeOffset, tResource * resource )
		: tResourceLoader( resource )
		, mHeaderSize( 0 )
		, mHeaderSizeOffset( headerSizeOffset )
		, mLoadState( cLoadStateOpening )
	{
	}

	//------------------------------------------------------------------------------
	void tDirectResourceLoader::fInitiate( )
	{
		// Allocate and begin opening the file
		mFileReader = tAsyncFileReader::fCreate( fGetResource( )->fAbsolutePhysicalPath( ) );

		fSetResourceBuffer( NEW_TYPED( tLoadInPlaceResourceBuffer )( ) );
		fSetSelfOnResource( );
	}

	//------------------------------------------------------------------------------
	tResourceLoader::tLoadResult tDirectResourceLoader::fUpdate( )
	{
		if( mLoadState < cLoadStateFinished )
		{
			// check if file is busy opening or reading
			if( mFileReader->fInBusyState( ) )
				return cLoadPending;

			// check for failed state
			if( mFileReader->fInFailedState( ) )
			{
				if( mFileReader->fGetState( ) == tAsyncFileReader::cStateOpenFailure )
					fOnOpenFailure( );
				else if( mFileReader->fGetState( ) == tAsyncFileReader::cStateReadFailure )
					fOnReadFailure( );
				return cLoadFailure;
			}

			// it's not busy and not failed, so it must be finished with either reading or writing
			if( mFileReader->fGetState( ) == tAsyncFileReader::cStateOpenSuccess )
				fOnOpenSuccess( );
			else if( mFileReader->fGetState( ) == tAsyncFileReader::cStateReadSuccess )
			{
				switch( mLoadState )
				{
				case cLoadStateHeaderSize:
					fOnReadHeaderSizeSuccess( );
					break;
				case cLoadStateHeader:
					fOnReadHeaderSuccess( );
					break;
				}
			}

			// check for cancellation after other checks, as we still need to handle
			// the state of the file and proper cleanup regardless of cancellation
			if( fGetCancel( ) )
			{
				tGenericBuffer* resBuffer = fGetResourceBuffer( );
				if( resBuffer )
					resBuffer->fFree( );
				return cLoadCancel;
			}

			// we want our file to remain open for the tDirectResourceLoader so we can process child reads
			return mFileReader->fGetState( ) == tAsyncFileReader::cStateReadSuccess ? cLoadSuccess : cLoadPending;
		}

		else if( mLoadState == cLoadStateChildReads )
		{
			sigassert( mFileReader && mFileReader->fGetState( ) == tAsyncFileReader::cStateReadSuccess );

			// Check for active child reads
			u32 finishedReads = 0;
			const u32 childReaderCount = mChildReads.fCount( );
			for( u32 c = 0; c < childReaderCount; ++c )
			{
				if( mChildReads[ c ]->fInBusyState( ) )
					return cLoadPending;

				sigassert( !mChildReads[ c ]->fInFailedState( ) && "Need to gracefully handle failed child reads!" );

				if( mChildReads[ c ]->fGetState( ) == tAsyncFileReader::cStateReadSuccess ||
					mChildReads[ c ]->fGetState( ) == tAsyncFileReader::cStateOpenSuccess )
					++finishedReads;
			}

			// If we had child reads and they're all done, clear the children
			// and fire the callback
			if( finishedReads == childReaderCount )
			{
				mLoadState = cLoadStateChildReadsFinished;
				mChildReads.fSetCount( 0 );
				if( !mChildReadsCompleteCb.fNull( ) )
					mChildReadsCompleteCb( *fGetResource( ) );
			}

			return mLoadState == cLoadStateChildReadsFinished ? cLoadSuccess : cLoadPending;
		}

		sigassert( 0 && "How did we get here while in the cLoadStateFinished?");
		return cLoadSuccess;
	}

	//------------------------------------------------------------------------------
	s32	tDirectResourceLoader::fGetLoadStage( )
	{
		switch( mLoadState )
		{
		case cLoadStateHeaderSize:
		case cLoadStateHeader:
			return 1;	// Still loading header, need more time
		case cLoadStateFinished: return 0; // This is the initial load finish, needs mPostLoad called.
		case cLoadStateChildReads: return 1; // This is the second load state, needs more time.
		case cLoadStateChildReadsFinished: return -1; // This is the last load state, we need to wrap up.
		default: sig_nodefault( );
		}

		return 0;
	}

	//------------------------------------------------------------------------------
	tAsyncFileReader * tDirectResourceLoader::fCreateChildReader( )
	{
		sigassert( mFileReader );
		sigassert( mLoadState >= cLoadStateFinished );

		mLoadState = cLoadStateChildReads;

		tAsyncFileReaderPtr child = fCreateChildReaderInternal( );

		mChildReads.fPushBack( child );
		return child.fGetRawPtr( );
	}

	//------------------------------------------------------------------------------
	void tDirectResourceLoader::fOnOpenSuccess( )
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
				mHeaderSizeOffset, // readByteOffset
				false //decompressAfterRead
			) 
		);
	}
	
	//------------------------------------------------------------------------------
	void tDirectResourceLoader::fOnOpenFailure( )
	{
		fCleanupAfterFailure( );
	}

	//------------------------------------------------------------------------------
	void tDirectResourceLoader::fOnReadHeaderSizeSuccess( )
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
				0,		// readByteOffset
				false	// decompressAfterRead
			) 
		);
	}

	//------------------------------------------------------------------------------
	void tDirectResourceLoader::fOnReadHeaderSuccess( )
	{
		sigassert( mLoadState == cLoadStateHeader );

#ifdef sig_assert
		u32 numBytesAllocated=0;
		byte* readBuffer = mFileReader->fForgetBuffer( &numBytesAllocated );
		tGenericBuffer* resBuffer = fGetResourceBuffer( );
		sigassert( mHeaderSizeOffset < numBytesAllocated + sizeof( mHeaderSize ) );
		sigassert( readBuffer && readBuffer == resBuffer->fGetBuffer( ) );
		sigassert( numBytesAllocated > 0 && numBytesAllocated == resBuffer->fGetBufferSize( ) );
		sigassert( *(const u32*)( readBuffer + mHeaderSizeOffset ) == mHeaderSize );
#else
		mFileReader->fForgetBuffer( );
#endif

		mLoadState = cLoadStateFinished;
		fCleanupAfterSuccess( );
	}

	//------------------------------------------------------------------------------
	void tDirectResourceLoader::fOnReadFailure( )
	{
		fCleanupAfterFailure( );
	}

	//------------------------------------------------------------------------------
	void tDirectResourceLoader::fCleanupAfterSuccess( )
	{
#ifdef sig_devmenu
		if( mFileReader )
			fSetFileTimeStamp( mFileReader->fGetLastModifiedTimeStamp( ) );
#endif
		//mFileReader.fRelease( );
	}

	//------------------------------------------------------------------------------
	void tDirectResourceLoader::fCleanupAfterFailure( )
	{
		if( tGenericBuffer * buffer = fGetResourceBuffer( ) )
			buffer->fFree( );
		mFileReader.fRelease( );
	}

	//------------------------------------------------------------------------------
	tAsyncFileReaderPtr tDirectResourceLoader::fCreateChildReaderInternal( )
	{
		return mFileReader->fSpawnChild( 0 );
	}
}
