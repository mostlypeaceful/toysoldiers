#include "BasePch.hpp"
#if defined( platform_ios )
#include "tSemaphore.hpp"

namespace Sig { namespace Threads
{

	tSemaphore::tSemaphore( b32 signaled, b32 manualReset )
	{
		pthread_mutex_init( &mMutex, NULL );
		pthread_cond_init( &mCondition, NULL );
	}

	tSemaphore::~tSemaphore( )
	{
		pthread_cond_destroy( &mCondition );
		pthread_mutex_destroy( &mMutex );
	}

	void tSemaphore::fSignal( )
	{
		log_warning_unimplemented( 0 );
		
		//pthread_mutex_unlock( &mMutex );
		
		const int result = pthread_cond_signal( &mCondition );
		log_assert( result == 0, "Invalid result" );
		
//		if( mSemaphoreHandle )
//			SetEvent( mSemaphoreHandle );
	}

	void tSemaphore::fReset( )
	{
		log_warning_unimplemented( 0 );
		
//		if( mSemaphoreHandle )
//			ResetEvent( mSemaphoreHandle );
	}

	void tSemaphore::fWaitUntilSignaled( )
	{
		log_warning_unimplemented( 0 );
		
		//pthread_mutex_lock( &mMutex );
		
		const int result = pthread_cond_wait( &mCondition, &mMutex );
		log_assert( result == 0, "Invalid result" );
	}

	void tSemaphore::fWaitUntilMultipleSignaled( tSemaphore* waitOn[], u32 numToWaitOn, b32 waitForAll )
	{
		log_warning_unimplemented( 0 );
		
//		tFixedArray<HANDLE,16> temp;
//		sigassert( numToWaitOn <= temp.fCount( ) );
//		for( u32 i = 0; i < numToWaitOn; ++i )
//			temp[ i ] = waitOn[ i ]->mSemaphoreHandle;
//
//		WaitForMultipleObjects( numToWaitOn, temp.fBegin( ), waitForAll, INFINITE );
	}

}}
#endif//#if defined( platform_ios )
