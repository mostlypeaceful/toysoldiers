#include "BasePch.hpp"
#if defined( platform_msft )
#include "tThread.hpp"
#include <process.h>

namespace Sig { namespace Threads
{
#ifdef platform_xbox360
	u32 tThread::gHardwareThreadCount = 6;
	void tThread::fComputeHardwareThreadCount( )
	{
		// nothing to do for 360, already known
		log_line( Log::cFlagThread, "Hardware Thread Count: " << gHardwareThreadCount );
	}
#else
	u32 tThread::gHardwareThreadCount = 1;	
	void tThread::fComputeHardwareThreadCount( )
	{
		// for pc, we have to query for the number of processors
		SYSTEM_INFO sysInf = { 0 };
		GetSystemInfo( &sysInf );
		gHardwareThreadCount = sysInf.dwNumberOfProcessors;
		log_line( Log::cFlagThread, "Hardware Thread Count: " << gHardwareThreadCount );
	}

	class tThreadCountInitializer { public: tThreadCountInitializer( ) { tThread::fComputeHardwareThreadCount( ); } } gAutoThreadInitializer;
#endif

	//------------------------------------------------------------------------------
	u32 tThread::fCurrentThreadId( )
	{
		return GetCurrentThreadId( );
	}

	//------------------------------------------------------------------------------
	b32 tThread::fRunning( ) const
	{
		DWORD exitCode=0;
		return 
				mThreadHandle
			&&	GetExitCodeThread( ( HANDLE )mThreadHandle, &exitCode )
			&&	exitCode == STILL_ACTIVE;
	}

	//-----PULLED FROM: mk:@MSITStore:C:\Program%20Files%20(x86)\Microsoft%20Xbox%20360%20SDK\doc\1033\xbox360sdk.chm::/SetThreadName.htm
	typedef struct tagTHREADNAME_INFO {
		DWORD dwType;     // Must be 0x1000
		LPCSTR szName;    // Pointer to name (in user address space)
		DWORD dwThreadID; // Thread ID (-1 for caller thread)
		DWORD dwFlags;    // Reserved for future use; must be zero
	} THREADNAME_INFO;

	void fSetThreadName( DWORD dwThreadID, LPCSTR szThreadName )
	{
		THREADNAME_INFO info;

		info.dwType = 0x1000;
		info.szName = szThreadName;
		info.dwThreadID = dwThreadID;
		info.dwFlags = 0;

		__try
		{
			RaiseException( 0x406D1388, 0, sizeof(info)/sizeof(DWORD), (DWORD *)&info );
		}
		__except( GetExceptionCode()==0x406D1388 ? EXCEPTION_CONTINUE_EXECUTION : EXCEPTION_EXECUTE_HANDLER )
		{
		}
	}
	//------------------------


	b32 tThread::fStartThreadForPlatform( tThreadMain threadMain, void* threadParam, u32 hwThread, const char* threadName, b32 suspended )
	{
		sigassert( !mThreadHandle );

		const b32 explicitHwThread = ( hwThread < fHardwareThreadCount( ) );
		const u32 creationFlags = suspended ? CREATE_SUSPENDED : 0;
#ifdef platform_xbox360
		if( explicitHwThread )
			creationFlags = CREATE_SUSPENDED;
#endif

		HANDLE hThread = ( HANDLE )_beginthreadex( 
			0, 
			0,
			threadMain, 
			threadParam, 
			creationFlags, 
			&mThreadId );

		if( !hThread || hThread == INVALID_HANDLE_VALUE )
			return false;

		//fSetThreadName( mThreadId, threadName );

#ifdef platform_xbox360
		if( explicitHwThread )
		{
			log_line( Log::cFlagThread, "Thread '" << threadName << "' created using explicit hw thread (" << hwThread << ")" );
			mHwThreadId = hwThread;
			XSetThreadProcessor( hThread, hwThread );
			if( !suspended )
				ResumeThread( hThread );
		}
#else
		log_line( Log::cFlagThread, "Thread '" << threadName << "' created" );
#endif

		mThreadHandle = ( u64 )hThread;
		return true;
	}

	void tThread::fResume( )
	{
		HANDLE hThread = ( HANDLE )mThreadHandle;
		if( hThread )
			ResumeThread( hThread );
	}

	void tThread::fChangeHWThread( u32 hwThread )
	{
		if( hwThread != mHwThreadId )
		{
			mHwThreadId = hwThread;
#ifdef platform_xbox360
			XSetThreadProcessor( ( HANDLE )mThreadHandle, hwThread );
#endif
		}
	}

	void tThread::fCloseThreadHandle( )
	{
		HANDLE hThread = ( HANDLE )mThreadHandle;
		if( hThread )
		{
			CloseHandle( hThread );
			mThreadHandle = 0;
		}
	}

}}
#endif//#if defined( platform_msft )
