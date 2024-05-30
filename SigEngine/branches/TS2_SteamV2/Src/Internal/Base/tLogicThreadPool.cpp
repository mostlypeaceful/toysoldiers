#include "BasePch.hpp"
#include "tLogicThreadPool.hpp"
#include "Threads/tWorkerThread.hpp"
#include "Time.hpp"

namespace Sig { namespace Threads
{
	class tLogicThread : public tWorkerThread
	{
	private:
		tLogic::tRunListId mRunListId;
		f32				mDt;
		tLogic* const*	mLogicObjects;
		u32				mStepStart;
		u32				mStepInterval;
		u32				mStepCount;
		volatile b32	mThreadStarted;
		Threads::tDistributedForLoopCallback* mDistributedForLoop;
		tSemaphore		mWorkComplete;
		Time::tStopWatch mWorkTimer;
	public:
		explicit tLogicThread( u32 hwThread )
			: tWorkerThread( "LogicThread", hwThread, true )
			, mRunListId( tLogic::cRunListCount )
			, mDt( 0.f )
			, mLogicObjects( 0 )
			, mStepStart( 0 )
			, mStepInterval( 0 )
			, mStepCount( 0 )
			, mThreadStarted( false )
			, mDistributedForLoop( 0 )
			, mWorkComplete( true, true )
			, mWorkTimer( false )
		{
			// All members have been initialised, so now we can resume the worker thread
			fResume( );
		}

		tSemaphore& fWorkCompleteSemaphore( )
		{
			return mWorkComplete;
		}

		b32 fThreadStarted( ) const { return mThreadStarted; }

		void fDistributeForLoop( Threads::tDistributedForLoopCallback& cb, u32 startIndex, u32 stepInterval, u32 forLoopCount )
		{
			sigassert( !mLogicObjects );

			mDistributedForLoop = &cb;
			mStepStart = startIndex;
			mStepInterval = stepInterval;
			mStepCount = forLoopCount;

			mWorkTimer.fRestart( );

			mWorkComplete.fReset( );
			mWaitForInput.fSignal( );
		}

		void fBeginWork( tLogic::tRunListId runListId, f32 dt, tLogic* const* logics, u32 startIndex, u32 stepInterval, u32 numLogicObjects )
		{
			sigassert( !mDistributedForLoop );

			mRunListId = runListId;
			mDt = dt;
			mLogicObjects = logics;
			mStepStart = startIndex;
			mStepInterval = stepInterval;
			mStepCount = numLogicObjects;

			//sync_event_v_c( Math::tVec4u( runListId, mStepStart, mStepInterval, mStepCount ), tSync::cSCThread );

			mWorkTimer.fRestart( );

			mWorkComplete.fReset( );
			mWaitForInput.fSignal( );
		}

		virtual void fOnThreadStartup( )
		{
			sync_register_thread( false );
			mThreadStarted = true;
		}

		virtual void fOnThreadTick( )
		{
			//sync_event_v_c( Math::tVec3u( mRunListId, mStepStart ), tSync::cSCThread );

			if( mLogicObjects )
			{
				// distribute ticks on logic objects
				for( u32 i = mStepStart; i < mStepCount; i += mStepInterval )
				{
					//sync_event_v_c( mLogicObjects[ i ]->fGuid( ), tSync::cSCThread );
					mLogicObjects[ i ]->fOnTick( mRunListId, mDt );
				}

				mRunListId = tLogic::cRunListCount;
				mLogicObjects = 0;
			}
			else if( mDistributedForLoop )
			{
				// distribute user for loop
				for( u32 i = mStepStart; i < mStepCount; i += mStepInterval )
					if( !(*mDistributedForLoop)( i ) )
						break;

				mDistributedForLoop = 0;
			}
			else
			{
				sigassert( !"invalid logic worker thread" );
			}

			mWorkTimer.fStop( );
			mWorkComplete.fSignal( );
		}

		virtual void fOnThreadShutdown( )
		{
			sync_deregister_thread( );
			mThreadStarted = false;
		}

		f32 fLastJobTimeMs( ) const
		{
			return mWorkTimer.fGetElapsedMs( );
		}
	};

}}

namespace Sig
{
	devvar_clamp( s32, Debug_Threads_MaxRunListThreads, 0, 0, 64, 0 );

	namespace
	{
		static const u32 cNumDedicatedPrimaryThreads = 2;
	}

	//------------------------------------------------------------------------------
	tLogicThreadPool::tLogicThreadPool( u32 threadCount )
		: mThreadAllowedFlags( 0 )
		, mLastJobTimeMs( 0.f )
	{
		if( threadCount == ~0 )
			threadCount = Threads::tThread::fHardwareThreadCount( );
		if( Debug_Threads_MaxRunListThreads > 0 )
			threadCount = fMin<s32>( Debug_Threads_MaxRunListThreads, threadCount );
		mLogicThreads.fNewArray( threadCount );
		mWorkCompleteSemaphores.fNewArray( threadCount );
		for( u32 i = 0; i < mLogicThreads.fCount( ); ++i )
		{
			mLogicThreads[ i ] = NEW Threads::tLogicThread( i );
			mThreadAllowedFlags |= ( 1 << i );
			mWorkCompleteSemaphores[ i ] = &mLogicThreads[ i ]->fWorkCompleteSemaphore( );

			// Spin until thread starts up to ensure they're registered deterministically
			if( sync_enabled( ) )
			{
				while( !mLogicThreads[ i ]->fThreadStarted( ) );
			}
		}
	}

	//------------------------------------------------------------------------------
	tLogicThreadPool::~tLogicThreadPool( )
	{
		fClear( );
		for( u32 i = 0; i < mLogicThreads.fCount( ); ++i )
			delete mLogicThreads[ i ];
	}



	//------------------------------------------------------------------------------
	void tLogicThreadPool::fDistributeForLoop( Threads::tDistributedForLoopCallback& cb, u32 loopCount, b32 secondaryThreadsOnly )
	{
		// ensure existing work is done
		fWaitForWorkToComplete( );

		if( loopCount > 0 )
		{
			u32 threadFlags, threadCount;
			fGetThreadLoopVars( secondaryThreadsOnly, threadFlags, threadCount );
			const u32 totalThreadCount = mLogicThreads.fCount( );
			for( u32 i = 0, job = 0; i < totalThreadCount; ++i )
			{
				if( threadFlags & ( 1 << i ) )
				{
					mLogicThreads[ i ]->fDistributeForLoop( cb, job++, threadCount, loopCount );
					if_logging( mLastJobThreads.fPushBack( mLogicThreads[ i ] ); )
				}
			}

			// wait for all threads to complete
			Threads::tSemaphore::fWaitUntilMultipleSignaled( mWorkCompleteSemaphores.fBegin( ), mWorkCompleteSemaphores.fCount( ) );
		}
	}

	//------------------------------------------------------------------------------
	void tLogicThreadPool::fBeginWork( tLogic::tRunListId runListId, f32 dt, const tLogic::tRunList& runList, b32 secondaryThreadsOnly, b32 waitToComplete )
	{
		// ensure existing work is done
		fWaitForWorkToComplete( );
		sigassert( !mCurrentRunList );

		if( runList.fCount( ) > 0 )
		{
			mCurrentRunList = &runList;

			u32 threadFlags, threadCount;
			fGetThreadLoopVars( secondaryThreadsOnly, threadFlags, threadCount );
			const u32 totalThreadCount = mLogicThreads.fCount( );

			//sync_event_v_c( Math::tVec3u( threadFlags, threadCount, totalThreadCount ), tSync::cSCThread );

			for( u32 i = 0, job = 0; i < totalThreadCount; ++i )
			{
				if( threadFlags & ( 1 << i ) )
				{
					mLogicThreads[ i ]->fBeginWork( runListId, dt, runList.fBegin( ), job++, threadCount, runList.fCount( ) );
					if_logging( mLastJobThreads.fPushBack( mLogicThreads[ i ] ); )
				}
			}

			// only wait for work to complete if specified by caller
			if( waitToComplete )
				fWaitForWorkToComplete( );
		}
	}

	//------------------------------------------------------------------------------
	void tLogicThreadPool::fWaitForWorkToComplete( )
	{
		if( !mCurrentRunList )
			return;

		// wait for all threads to complete
		Threads::tSemaphore::fWaitUntilMultipleSignaled( mWorkCompleteSemaphores.fBegin( ), mWorkCompleteSemaphores.fCount( ) );

		mCurrentRunList = 0;

		// accumulate true job time
#ifdef sig_logging
		mLastJobTimeMs = 0.f;
		for( u32 i = 0; i < mLastJobThreads.fCount( ); ++i )
			mLastJobTimeMs += mLastJobThreads[ i ]->fLastJobTimeMs( );
		mLastJobThreads.fSetCount( 0 );
#endif
	}

	//------------------------------------------------------------------------------
	void tLogicThreadPool::fClear( )
	{
		fWaitForWorkToComplete( );
	}

	//------------------------------------------------------------------------------
	void tLogicThreadPool::fDisallowHwThread( u32 threadId )
	{
		if( threadId < cNumDedicatedPrimaryThreads )
		{
			log_warning( 0, "Cannot disallow primary threads, use \"secondaryThreadsOnly\" argument instead" );
			return;
		}

		if( threadId >= mLogicThreads.fCount( ) )
			return;

		mThreadAllowedFlags &= ~( 1 << threadId );
	}

	//------------------------------------------------------------------------------
	void tLogicThreadPool::fAllowHwThread( u32 threadId )
	{
		if( threadId < cNumDedicatedPrimaryThreads )
			return;

		if( threadId >= mLogicThreads.fCount( ) )
			return;

		mThreadAllowedFlags |= ( 1 << threadId );
	}

	//------------------------------------------------------------------------------
	void tLogicThreadPool::fGetThreadLoopVars( 
		b32 secondaryThreadsOnly, u32 & threadFlags, u32 & threadCount )
	{
		threadFlags = mThreadAllowedFlags;

		if( secondaryThreadsOnly )
		{
			for( u32 i = 0; i < cNumDedicatedPrimaryThreads; ++i )
			{
				u32 newThreadFlags = threadFlags & ~( 1 << i );

				// If disabling this primary thread means we have no allowed threads
				// then do not disable this or any more primary threads
				if( !newThreadFlags )
					break;

				threadFlags = newThreadFlags;
			}
		}

		// Count how many threads are enabled
		u32 threadFlagCounter = threadFlags;
		for( threadCount = 0; threadFlagCounter; ++threadCount )
			threadFlagCounter &= threadFlagCounter - 1;
	}

}


