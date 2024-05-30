#ifndef __Time__
#define __Time__
#ifndef __Core__
#error This file must be included via Core.hpp!
#endif//__Core__

namespace Sig { namespace Time
{
	typedef u64 tStamp;

	///
	/// \brief Capture the current time in a platform-independent time stamp.
	base_export tStamp	fGetStamp( );

	///
	/// \brief Get the elapsed seconds between two time stamps.
	base_export f32 fGetElapsedS( tStamp start, tStamp end );
	
	///
	/// \brief Get the elapsed milliseconds between two time stamps.
	base_export f32 fGetElapsedMs( tStamp start, tStamp end );

	///
	/// \brief Tracks elapsed time.
	class base_export tStopWatch
	{
	private:

		mutable tStamp	mLastTime;
		mutable f32		mElapsed;
		b32				mRunning;

	public:

		///
		/// \brief Automatically starts timing by default; pass false for 
		/// the stop watch to be created in pause mode.
		inline tStopWatch( b32 start=true ) 
			: mLastTime( fGetStamp( ) )
			, mElapsed( 0.f )
			, mRunning( start )
		{
		}

		///
		/// \brief Reset the elapsed time; note that you can reset it
		/// to any value you like, but it defaults to 0.
		inline void fResetElapsedS( f32 newElapsed=0.f )
		{
			mLastTime = fGetStamp( );
			mElapsed = newElapsed;
		}

		///
		/// \brief Get the elapsed time in seconds.
		/// \note This method does not stop the stop watch.
		inline f32 fGetElapsedS( ) const
		{
			const tStamp now = fGetStamp( );

			if( mRunning )
				mElapsed += Sig::Time::fGetElapsedS( mLastTime, now );

			mLastTime = now;

			return mElapsed;
		}

		///
		/// \brief Get the elapsed time in milliseconds.
		/// \note This method does not stop the stop watch.
		inline f32 fGetElapsedMs( ) const
		{
			return fGetElapsedS( ) * 1000.f;
		}

		///
		/// \brief Stop the stop watch; this will freeze the
		/// elapsed time at the point of this function call; you
		/// can call start to continue, or reset it, as you choose.
		inline void fStop( )
		{
			fGetElapsedS( );
			mRunning = false;
		}

		///
		/// \brief Start timing, accumulating elapsed time from
		/// whatever the previous elapsed time was.
		inline void fStart( )
		{
			fGetElapsedS( );
			mRunning = true;
		}

		///
		/// \brief Start timing, reseting elapsed time from
		/// whatever the previous elapsed time was.
		inline void fRestart( )
		{
			mElapsed = 0.f;
			mLastTime = fGetStamp( );
			mRunning = true;
		}

		///
		/// \brief Toggle the stop watch's start/stop status.
		inline void fToggleStopStart( )
		{
			fGetElapsedS( );
			mRunning = !mRunning;
		}

		///
		/// \brief See if the stop watch is currently running (time is elapsing).
		inline b32 fRunning( ) const
		{
			return mRunning;
		}
	};

}}


#endif//__Time__
