#ifndef __tSemaphore__
#define __tSemaphore__

namespace Sig { namespace Threads
{
	///
	/// \brief A thread-safe signalling device; allows a thread to sleep
	/// until the semaphore is signaled (without spinning in a while loop).
	class base_export tSemaphore
	{
		volatile b32 mSignaled;

#if defined( platform_msft )
		HANDLE mSemaphoreHandle;
#elif defined( platform_apple )
		const b32 mManualReset;
		pthread_cond_t mCondition;
		pthread_mutex_t mMutex;
#endif

	public:

		tSemaphore( b32 signaled=false, b32 manualReset=false );
		~tSemaphore( );

		void fSignal( );
		void fReset( );
		void fWaitUntilSignaled( );
		b32  fIsSignaled( ) const;
		static void fWaitUntilMultipleSignaled( tSemaphore* waitOn[], u32 numToWaitOn, b32 waitForAll = true );
	};
}}

#endif//__tSemaphore__
