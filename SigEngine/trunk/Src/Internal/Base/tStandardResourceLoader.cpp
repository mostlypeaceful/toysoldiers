#include "BasePch.hpp"
#include "tStandardResourceLoader.hpp"
#include "tResource.hpp"
#include "tLoadInPlaceResourceBuffer.hpp"
#include "FileSystem.hpp"


namespace Sig
{

	tStandardResourceLoader::tStandardResourceLoader( tResource* res )
		: tResourceLoader( res )
	{
	}

	void tStandardResourceLoader::fInitiate( )
	{
		mFileReader = tAsyncFileReader::fCreate( fGetResource( )->fAbsolutePhysicalPath( ) );
		fSetResourceBuffer( NEW tLoadInPlaceResourceBuffer( ) );
		fSetSelfOnResource( );
	}

	tResourceLoader::tLoadResult tStandardResourceLoader::fUpdate( )
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
			fOnReadSuccess( );

		// check for cancellation after other checks, as we still need to handle
		// the state of the file and proper cleanup regardless of cancellation
		if( fGetCancel( ) )
		{
			tGenericBuffer* resBuffer = fGetResourceBuffer( );
			if( resBuffer )
				resBuffer->fFree( );
			return cLoadCancel;
		}

		return mFileReader.fNull( ) ? cLoadSuccess : cLoadPending;
	}

	void tStandardResourceLoader::fOnOpenSuccess( )
	{
		if( fGetCancel( ) )
			return; // don't allocate or begin reading if we've been cancelled

		const u32 resBufferSize = mFileReader->fGetFileSize( );

		tGenericBuffer* resBuffer = fGetResourceBuffer( );
		resBuffer->fAlloc( resBufferSize, fMakeStamp( resBufferSize ) );

		mFileReader->fRead( tAsyncFileReader::tReadParams( 
				tAsyncFileReader::tRecvBuffer( resBuffer->fGetBuffer( )/*buffer*/, resBufferSize/*bytesAllocated*/ ),
				resBufferSize/*readByteCount*/,
				mFileReader->mFileOffset/*readByteOffset*/,
			false/*decompressAfterRead*/) );
	}
	
	void tStandardResourceLoader::fOnOpenFailure( )
	{
		fCleanupAfterFailure( );
	}

	void tStandardResourceLoader::fOnReadSuccess( )
	{
#ifdef sig_assert
		u32 numBytesAllocated=0;
		byte* readBuffer = mFileReader->fForgetBuffer( &numBytesAllocated );
		tGenericBuffer* resBuffer = fGetResourceBuffer( );
		sigassert( readBuffer && readBuffer == resBuffer->fGetBuffer( ) );
		sigassert( numBytesAllocated > 0 && numBytesAllocated == resBuffer->fGetBufferSize( ) );
#else
		mFileReader->fForgetBuffer( );
#endif

		fCleanupAfterSuccess( );
	}

	void tStandardResourceLoader::fOnReadFailure( )
	{
		fCleanupAfterFailure( );
	}

	void tStandardResourceLoader::fCleanupAfterSuccess( )
	{
#ifdef sig_devmenu
		if( mFileReader )
			fSetFileTimeStamp( mFileReader->fGetLastModifiedTimeStamp( ) );
#endif
		mFileReader.fRelease( );
	}

	void tStandardResourceLoader::fCleanupAfterFailure( )
	{
		if( fGetResourceBuffer( ) )
			fGetResourceBuffer( )->fFree( );
		mFileReader.fRelease( );
	}

}

