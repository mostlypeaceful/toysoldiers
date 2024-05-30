#include "BasePch.hpp"
#include "tAsyncFileReaderQueue.hpp"


namespace Sig
{
	tAsyncFileReaderQueue::tAsyncFileReaderQueue( )
		: Threads::tWorkerThread( "AsyncFileReaderQueue", 5 )
		, mOpenQueue( 1024 )
		, mReadQueue( 1024 )
	{
	}

	void tAsyncFileReaderQueue::fEnqueueForOpen( tAsyncFileReader* fileReader )
	{
		const b32 ret = mOpenQueue.fPush( fileReader );
		sigassert( ret && "reached max. limit on open queue" );
		mWaitForInput.fSignal( );
	}

	void tAsyncFileReaderQueue::fEnqueueForRead( tAsyncFileReader* fileReader )
	{
		const b32 ret = mReadQueue.fPush( fileReader );
		sigassert( ret && "reached max. limit on read queue" );
		mWaitForInput.fSignal( );
	}

	void tAsyncFileReaderQueue::fOnThreadTick( )
	{
		tAsyncFileReader* po = 0, *pr = 0;
		while( mReadQueue.fPop( pr ) || mOpenQueue.fPop( po ) )
		{
			if( po )
			{
				po->fOpenFileInThread( );
				po = 0;
			}

			if( pr )
			{
				pr->fReadFileInThread( );
				pr = 0;
			}
		}
	}

}
