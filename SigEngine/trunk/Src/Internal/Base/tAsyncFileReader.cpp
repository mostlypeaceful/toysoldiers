#include "BasePch.hpp"
#include "tAsyncFileReader.hpp"
#include "tAsyncFileReaderQueue.hpp"

namespace Sig
{
	tAsyncFileReader::tAsyncFileReader( )
		: mPlatformFileHandle( 0 )
		, mFileSize( 0 )
		, mState( cStateNull )
		, mOwnsFile( true )
		, mFileOffset( 0 )
	{
	}

	tAsyncFileReader::~tAsyncFileReader( )
	{
		sigassert( !fInBusyState( ) );

		if( mOwnsFile )
			fCloseFileForPlatform( );
		fCleanupDanglingAllocations( );
	}

	tAsyncFileReaderPtr tAsyncFileReader::fCreate( const tFilePathPtr& path )
	{
		tAsyncFileReaderPtr fileReadPtr( NEW tAsyncFileReader );

		fileReadPtr->mFileName	= path;
		fileReadPtr->mState		= cStateOpening;

		tAsyncFileReaderQueue::fInstance( ).fEnqueueForOpen( fileReadPtr.fGetRawPtr( ) );

		return fileReadPtr;
	}

	void tAsyncFileReader::fRead( const tReadParams& readParams )
	{
		sigassert( fInValidStateForNewRead( ) );
		sigassert( readParams.mRecvBuffer.fGetBuffer( ) );
		sigassert( readParams.mReadByteOffset + readParams.mReadByteCount <= mFileSize );

		fCleanupDanglingAllocations( );

		mReadParams = readParams;
		mState = cStateReading;

		tAsyncFileReaderQueue::fInstance( ).fEnqueueForRead( this );
	}

	tAsyncFileReaderPtr tAsyncFileReader::fSpawnChild( u32 rawFileOffset ) const
	{
		sigassert( !fInBusyState( ) ); // TODO not sure this assertion is necessary; in fact, it seems valid to spawn a child even when busy
		sigassert( !fInFailedState( ) );
		sigassert( mPlatformFileHandle != 0 );

		tAsyncFileReaderPtr fileReadPtr( NEW tAsyncFileReader );

		fileReadPtr->mPlatformFileHandle = mPlatformFileHandle;
		fileReadPtr->mFileOffset = rawFileOffset;
		fileReadPtr->mFileSize = mFileSize;
		fileReadPtr->mState = cStateOpenSuccess;
		fileReadPtr->mOwnsFile = false;
		fileReadPtr->mFileName = mFileName;

		return fileReadPtr;
	}

	void tAsyncFileReader::fBlockUntilOpen( )
	{
		// mState must remain volatile for this to function
		while( mState == tAsyncFileReader::cStateOpening );
	}

	void tAsyncFileReader::fBlockUntilReadComplete( )
	{
		// mState must remain volatile for this to function
		while( mState == tAsyncFileReader::cStateReading );
	}

	b32	tAsyncFileReader::fInValidStateForNewRead( ) const
	{
		return	(	fGetState( ) == cStateOpenSuccess || 
					fGetState( ) == cStateReadSuccess || 
					fGetState( ) == cStateReadFailure	);
	}

	b32	tAsyncFileReader::fInBusyState( ) const
	{
		return	(	fGetState( ) == cStateOpening || 
					fGetState( ) == cStateReading	);
	}

	b32 tAsyncFileReader::fInFailedState( ) const
	{
		return	(	fGetState( ) == cStateOpenFailure || 
					fGetState( ) == cStateReadFailure	);
	}

	byte* tAsyncFileReader::fForgetBuffer( u32* optionalNumBytesAllocatedOut )
	{
		sigassert( !fInBusyState( ) );

		if( optionalNumBytesAllocatedOut )
			*optionalNumBytesAllocatedOut = mReadParams.mRecvBuffer.fGetBytesAllocated( );
		return mReadParams.mRecvBuffer.fForgetBuffer( );
	}

	void tAsyncFileReader::fOpenFileInThread( )
	{
		if( fCreateFileForPlatform( ) )
			mState = cStateOpenSuccess;
		else
		{
			//log_warning( "Error trying to open file [" << mFileName << "]; this probably means the specified file doesn't exist." );
			mState = cStateOpenFailure;
		}
	}

	void tAsyncFileReader::fReadFileInThread( )
	{
		if( fReadFileInThreadForPlatform( ) )
			mState = cStateReadSuccess;
		else
			mState = cStateReadFailure;
	}

	void tAsyncFileReader::fCleanupDanglingAllocations( )
	{
		sigassert( !fInBusyState( ) );

		mReadParams.mRecvBuffer.fForgetBuffer( );
	}


	///
	/// \section tAsyncFileReader::tRecvBuffer
	///

}
