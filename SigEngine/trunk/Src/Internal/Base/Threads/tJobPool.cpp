#include "BasePch.hpp"
#include "tJobPool.hpp"
#include "tWorkerThread.hpp"
#include "Time.hpp"

namespace Sig { namespace Threads
{
	class base_export tJobThread : public tWorkerThread
	{
	private:
		tJob**			mJobs;
		u32				mStartIndex;
		u32				mStepInterval;
		u32				mNumJobs;
		tSemaphore		mWorkComplete;
		Time::tStopWatch mWorkTimer;

	public:
		explicit tJobThread( u32 hwThread )
			: tWorkerThread( "WorkerThread", hwThread )
			, mJobs( 0 )
			, mStartIndex( 0 )
			, mStepInterval( 0 )
			, mNumJobs( 0 )
			, mWorkComplete( true, true )
			, mWorkTimer( false )
		{
		}

		tSemaphore& fWorkCompleteSemaphore( )
		{
			return mWorkComplete;
		}

		void fBeginWork( tJob** jobs, u32 startIndex, u32 stepInterval, u32 numJobs )
		{
			mJobs = jobs;
			mStartIndex = startIndex;
			mStepInterval = stepInterval;
			mNumJobs = numJobs;

			mWorkTimer.fRestart( );

			mWorkComplete.fReset( );
			mWaitForInput.fSignal( );
		}

		virtual void fOnThreadTick( )
		{
			for( u32 i = mStartIndex; i < mNumJobs; i += mStepInterval )
				mJobs[ i ]->fDoWorkInThread( );
			mJobs = 0;
			mWorkTimer.fStop( );
			mWorkComplete.fSignal( );
		}

		f32 fLastJobTimeMs( ) const
		{
			return mWorkTimer.fGetElapsedMs( );
		}
	};

	tJobPool::tJobPool( u32 categoryCount, u32 threadCount )
		: mCategories( categoryCount )
		, mCurrentWork( 0 )
		, mLastJobTimeMs( 0.f )
	{
		if( threadCount == ~0 )
			threadCount = tThread::fHardwareThreadCount( );
		mJobThreads.fNewArray( threadCount );
		mWorkCompleteSemaphores.fNewArray( threadCount );
		for( u32 i = 0; i < mJobThreads.fCount( ); ++i )
		{
			mJobThreads[ i ] = NEW tJobThread( i );
			mWorkCompleteSemaphores[ i ] = &mJobThreads[ i ]->fWorkCompleteSemaphore( );
		}
	}

	tJobPool::~tJobPool( )
	{
		fClear( );
		for( u32 i = 0; i < mJobThreads.fCount( ); ++i )
			delete mJobThreads[ i ];
	}

	void tJobPool::fClear( )
	{
		fWaitForWorkToComplete( );
		for( u32 icat = 0; icat < mCategories.fCount( ); ++icat )
		{
			tCategory& c = mCategories[ icat ];
			for( u32 ijob = 0; ijob < c.mJobs.fCount( ); ++ijob )
			{
				if( c.mJobs[ ijob ]->fRefCount( ) == 0 )
					delete c.mJobs[ ijob ];
			}
			c.mJobs.fDeleteArray( );
		}
	}

	void tJobPool::fAddJob( u32 category, tJob* job, b32 processImmediately )
	{
		if( processImmediately )
		{
			job->fOnBegin( );
			job->fDoWorkInThread( );
			job->fOnComplete( );
			if( job->fRefCount( ) == 0 )
				delete job;
		}
		else
			mCategories[ category ].mJobs.fPushBack( job );
	}

	u32 tJobPool::fBeginWork( u32 category, b32 secondaryThreadsOnly )
	{
		return fBeginWork( mCategories[ category ], secondaryThreadsOnly );
	}

	u32 tJobPool::fBeginWork( tCategory& c, b32 secondaryThreadsOnly )
	{
		sigassert( !mCurrentWork );
		mCurrentWork = &c;

		const u32 totalJobCount = c.mJobs.fCount( );

		// tell each job we're beginning (last chance to do something in main thread)
		for( u32 i = 0; i < totalJobCount; ++i )
			c.mJobs[ i ]->fOnBegin( );

		// distribute the work to the threads
		const u32 startIndex = ( secondaryThreadsOnly && mJobThreads.fCount( ) > 1 ) ? 1 : 0;
		const u32 effectiveThreadCount = mJobThreads.fCount( ) - startIndex;
		//const u32 jobsPerThread = ( totalJobCount / effectiveThreadCount ) + ( ( totalJobCount % effectiveThreadCount ) ? 1 : 0 );
		for( u32 i = startIndex; i < mJobThreads.fCount( ); ++i )
		{
			mJobThreads[ i ]->fBeginWork( c.mJobs.fBegin( ), i - startIndex, effectiveThreadCount, c.mJobs.fCount( ) );
			if_logging( mLastJobThreads.fPushBack( mJobThreads[ i ] ); )
		}

		return totalJobCount;
	}

	u32 tJobPool::fWaitForWorkToComplete( )
	{
		profile_pix("tJobPool::fWaitForWorkToComplete");
		if( !mCurrentWork )
			return 0;

		// wait for all jobs to complete
		tSemaphore::fWaitUntilMultipleSignaled( mWorkCompleteSemaphores.fBegin( ), mWorkCompleteSemaphores.fCount( ) );

		// accumulate true job time
#ifdef sig_logging
		mLastJobTimeMs = 0.f;
		for( u32 i = 0; i < mLastJobThreads.fCount( ); ++i )
			mLastJobTimeMs += mLastJobThreads[ i ]->fLastJobTimeMs( );
		mLastJobThreads.fSetCount( 0 );
#endif

		// notify each job that it's done and that we're back in the main thread
		tJobQueue& jobs = mCurrentWork->mJobs;
		const u32 totalJobCount = jobs.fCount( );
		for( u32 i = 0; i < totalJobCount; ++i )
		{
			jobs[ i ]->fOnComplete( );

			// if no more ref counts, we clean up
			if( jobs[ i ]->fRefCount( ) == 0 )
				delete jobs[ i ];
		}

		jobs.fSetCount( 0 );
		mCurrentWork = 0;
		return totalJobCount;
	}

}}

