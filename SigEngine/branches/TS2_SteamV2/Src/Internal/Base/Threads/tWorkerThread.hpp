#ifndef __tWorkerThread__
#define __tWorkerThread__
#include "tThread.hpp"
#include "tSemaphore.hpp"


namespace Sig { namespace Threads
{
	class base_export tWorkerThread
	{
	private:
		b32			mRunning;
		tThread		mThread;
		tSemaphore	mWaitToDestroy;
	protected:
		tSemaphore	mWaitForInput;
	private:
		static Threads::tThreadReturn thread_call fThreadMain( void* );
	public:
		explicit tWorkerThread( const char* threadName, u32 explicitHwThread = ~0, b32 suspended = false );
		virtual ~tWorkerThread( );
		void fChangeHWThread( u32 hwThread );
		void fResume( );
	private:
		virtual void fOnThreadStartup( ) { }
		virtual void fOnThreadTick( ) = 0;
		virtual void fOnThreadShutdown( ) { }
	};

}}


#endif//__tWorkerThread__
