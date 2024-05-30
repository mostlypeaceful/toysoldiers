#include "BasePch.hpp"
#include "tThread.hpp"

namespace Sig { namespace Threads
{

	tThread::tThread( )
		: mThreadHandle( 0 )
		, mThreadId( 0 )
		, mHwThreadId( ~0 )
	{
	}

	tThread::~tThread( )
	{
		fCloseThreadHandle( );
	}

	void tThread::fStart( tThreadMain threadMain, const char* threadName, void* threadParam, u32 hwThread, b32 suspended )
	{
		sigassert( threadMain );
		sigassert( !fRunning( ) );
		fCloseThreadHandle( );
		if( !fStartThreadForPlatform( threadMain, threadParam, hwThread, threadName, suspended ) )
		{
			log_warning( Log::cFlagThread, "Couldn't start new thread named: " << threadName );
		}
	}

}}
