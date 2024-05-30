#include "UnitTestsPch.hpp"
#include "Threads/tThread.hpp"
#include "Threads/tLocklessQueue.hpp"


using namespace Sig;

tGrowableArray<u32> gPushed;
tGrowableArray<u32> gPopped;

static u32 thread_call fLocklessQueueTestThreadMain( void* p )
{
	Threads::tLocklessQueue<u32>& q = *( Threads::tLocklessQueue<u32>* )p;

	Time::tStopWatch timer;

	while( timer.fGetElapsedS( ) < 2.5f )
	{
		u32 v;
		if( q.fPop( v ) )
		{
			log_line( 0, "pop " << v );
			gPopped.fPushBack( v );
			fSleep( 10 );
		}
	}
	return 0;
}

define_unittest( TestLocklessQueue )
{
	Threads::tLocklessQueue<u32> q( 32 );

	const u32 startSize = q.fGetCapacity( );

	Threads::tThread thread;
	thread.fStart( fLocklessQueueTestThreadMain, "LocklessQueueThreadedMain", &q );

	Time::tStopWatch timer;

	while( timer.fGetElapsedS( ) < 5.f )
	{
		const u32 toPush = rand();
		if( q.fPush( toPush ) )
		{
			log_line( 0, "push " << toPush );
			gPushed.fPushBack( toPush );
			fSleep( 0 );
		}
	}

	const u32 endSize = q.fGetCapacity( );

	fAssert( gPushed.fCount( ) >= gPopped.fCount( ) );

	gPushed.fSetCount( gPopped.fCount( ) );
	for( u32 i = 0; i < gPushed.fCount( ); ++i )
	{
		fAssertEqual( gPushed[i], gPopped[i] );
	}
}
