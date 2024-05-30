#ifndef __tJobPool__
#define __tJobPool__

namespace Sig { namespace Threads
{
	class base_export tJob : public tRefCounter
	{
	public:
		virtual ~tJob( ) { }
		virtual void fDoWorkInThread( ) = 0;
		virtual void fOnBegin( ) { } // called from main thread before threaded work begins
		virtual void fOnComplete( ) { } // called from main thread after threaded work is done
	};

	define_smart_ptr( base_export, tRefCounterPtr, tJob );

	class tJobThread;
	class tSemaphore;

	class base_export tJobPool : public tRefCounter
	{
	public:
		typedef tGrowableArray<tJob*> tJobQueue;
		struct tCategory
		{
			tJobQueue mJobs;
		};
	private:
		tDynamicArray<tJobThread*>	mJobThreads;
		tDynamicArray<tSemaphore*>	mWorkCompleteSemaphores;
		tDynamicArray<tCategory>	mCategories;
		tCategory*					mCurrentWork;
		f32							mLastJobTimeMs;
		if_logging( tGrowableArray<tJobThread*> mLastJobThreads; )
	public:
		explicit tJobPool( u32 categoryCount, u32 threadCount = ~0 ); // ~0 means use all available hw threads
		~tJobPool( );

		void fClear( );
		void fAddJob( u32 category, tJob* job, b32 processImmediately = false );
		u32  fBeginWork( u32 category, b32 secondaryThreadsOnly = false );
		u32  fBeginWork( tCategory& category, b32 secondaryThreadsOnly = false );
		u32  fWaitForWorkToComplete( );
		f32  fLastJobTimeMs( ) const { return mLastJobTimeMs; }
	};

	define_smart_ptr( base_export, tRefCounterPtr, tJobPool );

}}


#endif//__tJobPool__
