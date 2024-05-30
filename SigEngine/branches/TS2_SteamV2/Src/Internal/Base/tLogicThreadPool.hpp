#ifndef __tLogicRunList__
#define __tLogicRunList__
#include "tLogic.hpp"

namespace Sig
{
	namespace Threads
	{
		class tLogicThread;
		class tSemaphore;

		///
		/// \brief Return false to terminate the distributed for loop.
		typedef tDelegate<b32 ( u32 i )> tDistributedForLoopCallback;
	}

	class base_export tLogicThreadPool : public tRefCounter
	{
		tDynamicArray<Threads::tLogicThread*>	mLogicThreads;
		tDynamicArray<Threads::tSemaphore*>		mWorkCompleteSemaphores;
		const tLogic::tRunList*					mCurrentRunList;
		u32										mThreadAllowedFlags;
		f32										mLastJobTimeMs;
		if_logging( tGrowableArray<Threads::tLogicThread*> mLastJobThreads; )

	public:
		explicit tLogicThreadPool( u32 threadCount = ~0 ); // ~0 means use all available hw thread
		~tLogicThreadPool( );
		void fDistributeForLoop( Threads::tDistributedForLoopCallback& cb, u32 loopCount, b32 secondaryThreadsOnly = false );
		void fBeginWork( tLogic::tRunListId runListId, f32 dt, const tLogic::tRunList& runList, b32 secondaryThreadsOnly = false, b32 waitToComplete = true );
		void fWaitForWorkToComplete( );
		void fClear( );

		void fDisallowHwThread( u32 threadId );
		void fAllowHwThread( u32 threadId );

		f32  fLastJobTimeMs( ) const { return mLastJobTimeMs; }

	private:

		void fGetThreadLoopVars( b32 secondaryThreadsOnly, u32 & threadFlags, u32 & threadCount ); 

	};

	define_smart_ptr( base_export, tRefCounterPtr, tLogicThreadPool );

}

#endif//__tLogicRunList__
