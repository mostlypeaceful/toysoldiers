#include "BasePch.hpp"
#include "tThread.hpp"
#include "tSemaphore.hpp"
#include "tMutex.hpp"

namespace Sig { namespace Threads
{
	namespace
	{
		tHashTable<u32,tThreadReturn> gFinishedThreads;
		tCriticalSection gFinishedThreadsSema;
	}

	tThreadReturn tThread::fThreadReturning( tThreadReturn r )
	{
		tMutex lock( gFinishedThreadsSema );
		gFinishedThreads.fInsert( tThread::fCurrentThreadId(), r );
		return r;
	}

	tThread::tThread( )
		: mThreadHandle( 0 )
		, mThreadId( 0 )
		, mHwThreadId( ~0 )
	{
	}

	tThread::~tThread( )
	{
		fCloseThreadHandle( );
		tMutex lock( gFinishedThreadsSema );
		gFinishedThreads.fRemove( mThreadId );
	}

#if !(defined( platform_pcdx ) || defined( platform_xbox360 )) // these platforms have their own API based version...
	b32 tThread::fRunning( ) const
	{
		// maybe it makes more sense to have a local boolean that's updated on thread death so we can avoid a lock?
		// then again it might cause lifetime issues for fire-and-forget threads...
		tMutex lock( gFinishedThreadsSema );
		return mThreadHandle && !gFinishedThreads.fFind(mThreadId);
	}
#endif

	void tThread::fStart( tThreadMain threadMain, const char* threadName, void* threadParam, u32 hwThread )
	{
		sigassert( threadMain );
		sigassert( !fRunning( ) );
		fCloseThreadHandle( );
		if( !fStartThreadForPlatform( threadMain, threadParam, hwThread, threadName ) )
		{
			log_warning( "Couldn't start new thread named: " << threadName );
		}
	}

}}
