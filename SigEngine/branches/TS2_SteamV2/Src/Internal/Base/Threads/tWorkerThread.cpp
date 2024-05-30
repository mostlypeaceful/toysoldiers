#include "BasePch.hpp"
#include "tWorkerThread.hpp"

namespace Sig { namespace Threads
{
	Threads::tThreadReturn thread_call tWorkerThread::fThreadMain( void* p )
	{
		sigassert( p );
		tWorkerThread& wt = *( tWorkerThread* )p;

		wt.fOnThreadStartup( );

		while( wt.mRunning )
		{
			// make the thread go to sleep until there is actually work to be done
			wt.mWaitForInput.fWaitUntilSignaled( );

			// tick the job thread
			if( wt.mRunning )
				wt.fOnThreadTick( );
		}

		wt.mWaitToDestroy.fSignal( );

		wt.fOnThreadShutdown( );

		thread_return( 0 );
	}

	tWorkerThread::tWorkerThread( const char* threadName, u32 explicitHwThread, b32 suspended )
		: mRunning( true )
	{
		// Worker threads can start suspended to allow derived classes to complete their constructor before calling fResume
		mThread.fStart( fThreadMain, threadName, this, explicitHwThread, suspended );
	}

	tWorkerThread::~tWorkerThread( )
	{
		mRunning = false;

		if( mThread.fRunning( ) )
		{
			mWaitForInput.fSignal( );
			mWaitToDestroy.fWaitUntilSignaled( );
		}
	}

	void tWorkerThread::fChangeHWThread( u32 hwThread )
	{
		mThread.fChangeHWThread( hwThread );
	}

	void tWorkerThread::fResume( )
	{
		mThread.fResume( );
	}
}}

