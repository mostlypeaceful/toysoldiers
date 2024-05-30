#ifndef __tSemaphore__
#define __tSemaphore__

namespace Sig { namespace Threads
{

	///
	/// \brief A thread-safe signalling device; allows a thread to sleep
	/// until the semaphore is signaled (without spinning in a while loop).
	class base_export tSemaphore
	{
#if defined( platform_pcdx9 ) || defined( platform_pcdx10 ) || defined( platform_xbox360 )
		HANDLE mSemaphoreHandle;
#elif defined( platform_apple )
		pthread_cond_t mCondition;
		pthread_mutex_t mMutex;
#endif// win32 and xbox360 platforms

	public:

		tSemaphore( b32 signaled=false, b32 manualReset=false );
		~tSemaphore( );

		void fSignal( );
		void fReset( );
		void fWaitUntilSignaled( );
		static void fWaitUntilMultipleSignaled( tSemaphore* waitOn[], u32 numToWaitOn, b32 waitForAll = true );
	};

}}



#endif//__tSemaphore__

