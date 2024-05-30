#include "BasePch.hpp"
#if defined( platform_ios )
#include "tThread.hpp"

namespace Sig { namespace Threads
{
	u32 tThread::gHardwareThreadCount = 1;	
	void tThread::fComputeHardwareThreadCount( )
	{
		gHardwareThreadCount = 1; // ???
		log_line( Log::cFlagThread, "Hardware Thread Count: " << gHardwareThreadCount );
	}

	u32 tThread::fCurrentThreadId( )
	{
		log_warning_unimplemented( 0 );
		return 0;
	}

	class tThreadCountInitializer { public: tThreadCountInitializer( ) { tThread::fComputeHardwareThreadCount( ); } } gAutoThreadInitializer;

	b32 tThread::fRunning( ) const
	{
		return false;
	}

	b32 tThread::fStartThreadForPlatform( tThreadMain threadMain, void* threadParam, u32 hwThread, const char* threadName )
	{
		sigassert( !mThreadHandle );

		//const b32 explicitHwThread = ( hwThread < fHardwareThreadCount( ) );
		//const u32 creationFlags = 0;

		pthread_t threadObj = NULL;
		const int result = pthread_create( &threadObj, NULL, threadMain, threadParam );
		
		if( result != 0 || !threadObj )
			return false;
		
		// TODO acquire mThreadId?
		// TODO fSetThreadName( mThreadId, threadName );?

		log_line( Log::cFlagThread, "Thread '" << threadName << "' created" );

		mThreadHandle = ( u64 )threadObj;
		return true;
	}

	void tThread::fCloseThreadHandle( )
	{
		pthread_t threadObj = ( pthread_t )mThreadHandle;
		if( threadObj )
		{
			pthread_detach( threadObj );
			mThreadHandle = 0;
		}
	}

}}
#endif//#if defined( platform_ios )
