#include "BasePch.hpp"
#include "tAsyncFileReader.hpp"
#include "tAsyncFileReaderQueue.hpp"

namespace Sig
{
	tAsyncFileReader::tAsyncFileReader( )
		: mPlatformFileHandle( 0 )
		, mFileSize( 0 )
		, mState( cStateNull )
		//, mCancel( false )
		, mOwnsFile( true )
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

	tAsyncFileReaderPtr tAsyncFileReader::fSpawnChild( ) const
	{
		sigassert( !fInBusyState( ) ); // TODO not sure this assertion is necessary; in fact, it seems valid to spawn a child even when busy
		sigassert( !fInFailedState( ) );
		sigassert( mPlatformFileHandle != 0 );

		tAsyncFileReaderPtr fileReadPtr( NEW tAsyncFileReader );

		fileReadPtr->mPlatformFileHandle = mPlatformFileHandle;
		fileReadPtr->mFileSize = mFileSize;
		fileReadPtr->mState = cStateOpenSuccess;
		//fileReadPtr->mCancel = false;
		fileReadPtr->mOwnsFile = false;
		fileReadPtr->mFileName = mFileName;

		return fileReadPtr;
	}

	void tAsyncFileReader::fBlockUntilOpen( )
	{
		while( !fInFailedState( ) && fGetState( ) == tAsyncFileReader::cStateOpening )
			fSleep( 1 );
	}

	void tAsyncFileReader::fBlockUntilReadComplete( )
	{
		while( !fInFailedState( ) && fGetState( ) == tAsyncFileReader::cStateReading )
			fSleep( 1 );
	}

	b32	tAsyncFileReader::fInValidStateForNewRead( ) const
	{
		return	( 
				fGetState( ) == cStateOpenSuccess
			||	fGetState( ) == cStateReadSuccess
			||	fGetState( ) == cStateReadFailure
				);
	}

	b32	tAsyncFileReader::fInBusyState( ) const
	{
		return	( 
				fGetState( ) == cStateOpening
			||	fGetState( ) == cStateReading
			//||	mCancel
				);
	}

	b32 tAsyncFileReader::fInFailedState( ) const
	{
		return	( 
				fGetState( ) == cStateOpenFailure
			||	fGetState( ) == cStateReadFailure
				);
	}

	//void tAsyncFileReader::fCancel( )
	//{
	//	// TODO consider getting rid of cancel,
	//	// as its inherently a very friend-UNFRINEDLY operation,
	//	// having to do with memory allocation/deallocation, atomic state,
	//	// etc... note that this function is not really safe to use right now
	//	if( !fInBusyState( ) )
	//		return; // nothing to cancel
	//	mCancel = true;
	//	if( mState == cStateOpening )
	//	{
	//		mFileSize = 0;
	//		mFileName = tFilePathPtr( );
	//	}
	//	else if( mState == cStateReading )
	//	{
	//		fCleanupDanglingAllocations( );
	//		mReadParams = tReadParams( );
	//	}
	//	else
	//	{
	//		sigassert( !"invalid state" );
	//	}
	//}

	byte* tAsyncFileReader::fForgetBuffer( u32* optionalNumBytesAllocatedOut )
	{
		sigassert( !fInBusyState( ) );

		if( optionalNumBytesAllocatedOut )
			*optionalNumBytesAllocatedOut = mReadParams.mRecvBuffer.fGetBytesAllocated( );
		return mReadParams.mRecvBuffer.fForgetBuffer( );
	}

	b32	tAsyncFileReader::fHandleCancelForOpen( )
	{
		//if( !mCancel )
			return false;
		//sigassert( mState == cStateOpening || mState == cStateOpenFailure || mState == cStateOpenSuccess );
		//mCancel = false;
		//mState = cStateNull;
		//return true;
	}

	b32 tAsyncFileReader::fHandleCancelForRead( )
	{
		//if( !mCancel )
			return false;
		//sigassert( mState == cStateReading || mState == cStateReadFailure || mState == cStateReadSuccess );
		//mCancel = false;
		//mState = cStateOpenSuccess;
		//return true;
	}

	void tAsyncFileReader::fOpenFileInThread( )
	{
		//if( !mCancel )
		{
			if( fCreateFileForPlatform( ) )
				mState = cStateOpenSuccess;
			else
			{
				//log_warning( Log::cFlagFile, "Error trying to open file [" << mFileName << "]; this probably means the specified file doesn't exist." );
				mState = cStateOpenFailure;
			}
		}

		//fHandleCancelForOpen( );
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