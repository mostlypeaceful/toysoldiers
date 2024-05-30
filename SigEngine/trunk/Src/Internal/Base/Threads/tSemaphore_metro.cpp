#include "BasePch.hpp"
#if defined( platform_metro )
#include "tSemaphore.hpp"
#include "MetroUtil.hpp"

namespace Sig { namespace Threads
{

	tSemaphore::tSemaphore( b32 signaled, b32 manualReset )
	{
		DWORD flags = 0;

		if ( signaled )
			flags |= CREATE_EVENT_INITIAL_SET;

		if ( manualReset )
			flags |= CREATE_EVENT_MANUAL_RESET;

		const DWORD access = SYNCHRONIZE | EVENT_MODIFY_STATE; // http://msdn.microsoft.com/en-us/library/windows/apps/ms686670.aspx

		mSemaphoreHandle = CreateEventExW( NULL, NULL, flags, access );
	}

	tSemaphore::~tSemaphore( )
	{
		if( mSemaphoreHandle )
			CloseHandle( mSemaphoreHandle );
		mSemaphoreHandle = 0;
	}

	void tSemaphore::fSignal( )
	{
		if( mSemaphoreHandle )
			SetEvent( mSemaphoreHandle );
	}

	void tSemaphore::fReset( )
	{
		if( mSemaphoreHandle )
			ResetEvent( mSemaphoreHandle );
	}

	void tSemaphore::fWaitUntilSignaled( )
	{
		if( mSemaphoreHandle )
		{
			DWORD result = WaitForSingleObjectEx( mSemaphoreHandle, INFINITE, false );
			log_assert( result == WAIT_OBJECT_0, "Error waiting for single object: " << MetroUtil::fErrorCodeToString(GetLastError()) );
		}
	}

	void tSemaphore::fWaitUntilMultipleSignaled( tSemaphore* waitOn[], u32 numToWaitOn, b32 waitForAll )
	{
		tFixedArray<HANDLE,16> temp;
		sigassert( numToWaitOn <= temp.fCount( ) );
		for( u32 i = 0; i < numToWaitOn; ++i )
			temp[ i ] = waitOn[ i ]->mSemaphoreHandle;

		//WaitForMultipleObjects( numToWaitOn, temp.fBegin( ), waitForAll, INFINITE );
		DWORD result = WaitForMultipleObjectsEx( numToWaitOn, temp.fBegin( ), waitForAll, INFINITE, false );
		log_assert( WAIT_OBJECT_0 <= result && result < (WAIT_OBJECT_0+numToWaitOn), "Error waiting for single object: " << MetroUtil::fErrorCodeToString(GetLastError())  );
	}

}}
#endif//#if defined( platform_msft )
