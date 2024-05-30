#include "BasePch.hpp"
#if defined( platform_ios )
#include "tThread.hpp"

namespace Sig { namespace Threads
{
	namespace
	{
		// if this fails, we need to change the type of our thread IDs, or build a lut or binary tree of some description.
		static_assert( sizeof(pthread_t) <= sizeof(u32) );
		u32 fToThreadId( const pthread_t& pt )
		{
			u32 id = 0;
			fMemCpy( &id, &pt, sizeof(pthread_t) );
			return id;
		}
	}
	
	u32 tThread::gMainThreadId = 0;
	u32 tThread::gHardwareThreadCount = 1;	
	void tThread::fComputeHardwareThreadCount( )
	{
		gHardwareThreadCount = 1; // ???
		log_line( Log::cFlagThread, "Hardware Thread Count: " << gHardwareThreadCount );
	}

#ifdef sig_assert
	u32 tThread::fMainThreadId( )
	{
		log_warning_unimplemented( );
		return gMainThreadId;
	}
#endif//sig_assert

	u32 tThread::fCurrentThreadId( )
	{
		return fToThreadId(pthread_self());
	}

	class tThreadCountInitializer { public: tThreadCountInitializer( ) { tThread::fComputeHardwareThreadCount( ); } } gAutoThreadInitializer;

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
		mThreadId = fToThreadId(threadObj);
		pthread_setname_np(threadName);

		log_line( Log::cFlagThread, "Thread '" << threadName << "' created" );

		mThreadHandle = ( u64 )threadObj;
		return true;
	}

	void tThread::fChangeHWThread( u32 hwThread )
	{
		if( hwThread != mHwThreadId )
		{
			mHwThreadId = hwThread; // equivalent to the pc version...  but does this even make any sense?
		}
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
