#include "BasePch.hpp"
#if defined( platform_ios )
#include "tSemaphore.hpp"

namespace Sig { namespace Threads
{

	tSemaphore::tSemaphore( b32 signaled, b32 manualReset )
		: mSignaled(signaled)
		, mManualReset(manualReset)
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
		pthread_mutex_lock(&mMutex);
		mSignaled = true;
		const int result = pthread_cond_signal( &mCondition );
		log_assert( result == 0, "Invalid result" );
		pthread_mutex_unlock(&mMutex);
	}

	void tSemaphore::fReset( )
	{		
		mSignaled = false;
	}

	void tSemaphore::fWaitUntilSignaled( )
	{
		pthread_mutex_lock(&mMutex);
		while(!mSignaled)
		{
			const int result = pthread_cond_wait(&mCondition, &mMutex);
			log_assert( result == 0, "Invalid result" );
		}
		if (!mManualReset)
			mSignaled = false;
		pthread_mutex_unlock(&mMutex);
	}
	
	b32 tSemaphore::fIsSignaled( ) const
	{
		return mSignaled;
	}

	void tSemaphore::fWaitUntilMultipleSignaled( tSemaphore* waitOn[], u32 numToWaitOn, b32 waitForAll )
	{
		for ( u32 i = 0 ; i < numToWaitOn ; ++i )
			waitOn[i]->fWaitUntilSignaled( );
		// TODO: Verify this is sane.
	}

}}
#endif//#if defined( platform_ios )
