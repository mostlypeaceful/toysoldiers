#ifndef __tThread__
#define __tThread__

namespace Sig { namespace Threads
{
	
#if defined( platform_msft )
#	define thread_call __stdcall
#	define thread_return( exitCode ) return ::Sig::Threads::tThread::fThreadReturning(( u32 )exitCode)
	typedef u32 tThreadReturn;
#else
#	define thread_call
#	define thread_return( exitCode ) return ::Sig::Threads::tThread::fThreadReturning(( void* )exitCode)
	typedef void* tThreadReturn;
#endif
	
	typedef tThreadReturn (thread_call *tThreadMain)( void* threadParam );
	
	class tThreadCountInitializer;

	///
	/// \brief Encapsulates running a secondary (or tertiary, etc.) thread.
	/// \note Beware of the usual thread gotches, such as race conditions,
	/// deadlock, memory allocation, etc.!
	class base_export tThread : public tUncopyable
	{
		friend class tThreadCountInitializer;
	private:
		static u32 gMainThreadId;
		static u32 gHardwareThreadCount;
		static void fComputeHardwareThreadCount( );

	private:
		u64			mThreadHandle;
		u32			mThreadId;
		u32			mHwThreadId;

	public:
		static u32 fHardwareThreadCount( ) { return gHardwareThreadCount; }
		if_assert( static u32 fMainThreadId( ); ) //to aid sigassert_is_main_thread
		static u32 fCurrentThreadId( );
		static tThreadReturn fThreadReturning( tThreadReturn r ); ///< Lets us keep track of what threads are finishing on platforms where there's no native API support (ios, metro).  Returns it's input.

		tThread( );
		~tThread( );

		///
		/// \brief Query for whether the thread is still running.
		b32 fRunning( ) const;

		///
		/// \brief Start the thread using supplied thread main.
		/// \param threadMain The function to be called in the thread.
		/// \param threadParam This value will be passed to your thread main.
		void fStart( tThreadMain threadMain, const char* threadName, void* threadParam=0, u32 hwThread=~0 );

		///
		/// \brief Change the HW thread index at run time (i.e. after the thread was created) - happens asynchronously
		void fChangeHWThread( u32 hwThread );

	private:

		b32		fStartThreadForPlatform( tThreadMain threadMain, void* threadParam, u32 hwThread, const char* threadName );
		void	fCloseThreadHandle( );
	};


}}



#endif//__tThread__

