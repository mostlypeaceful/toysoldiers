#include "BasePch.hpp"
#if defined( platform_msft ) && !defined( platform_metro )
#include "tSemaphore.hpp"

namespace Sig { namespace Threads
{

	tSemaphore::tSemaphore( b32 signaled, b32 manualReset )
	{
		mSemaphoreHandle = CreateEvent( 0, manualReset, signaled, 0 );
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
			WaitForSingleObject( mSemaphoreHandle, INFINITE );		
	}

	void tSemaphore::fWaitUntilMultipleSignaled( tSemaphore* waitOn[], u32 numToWaitOn, b32 waitForAll )
	{
		tFixedArray<HANDLE,16> temp;
		sigassert( numToWaitOn <= temp.fCount( ) );
		for( u32 i = 0; i < numToWaitOn; ++i )
			temp[ i ] = waitOn[ i ]->mSemaphoreHandle;

#if defined( platform_xbox360 )
		WaitForMultipleObjects( numToWaitOn, temp.fBegin( ), waitForAll, INFINITE );
#else
		WaitForMultipleObjectsEx( numToWaitOn, temp.fBegin( ), waitForAll, INFINITE, false );
#endif 
	}

}}
#endif//#if defined( platform_msft ) && !defined( platform_metro )
