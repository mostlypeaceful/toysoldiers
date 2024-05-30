//------------------------------------------------------------------------------
// \file Time.hpp - 11 Jan 2011
// \author colins
// \author mrickert
//
// Copyright Signal Studios 2011-2013, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __Time__
#define __Time__

#ifndef __Core__
#error This file must be included via Core.hpp!
#endif//__Core__

#include "time.h"

namespace Sig { namespace Time
{
	typedef u64 tStamp;

	//Return time as ascii string. This performs no allocations.
	const char * fAsciiDateTime( );

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

	class base_export tDateTime
	{
		sig_make_stringstreamable( tDateTime, fDateTimeString( ).c_str( ) );

	private:
		tm		mTimeInfo;
		time_t	mRawTime;

	private:
		static const u32 cMinYear = 1970;
		static const u32 cMaxYear = 2999;



	public:
		tDateTime( b32 utc = false );		///< Initialize to 'now' -- I'd recommend one of the static funcs bellow instead for explicitness.

		/// \brief Initializes a date time based on Julian Dates + Military Time.  Note that Julian is 1-based, Time is 0-based.  For example,
		///		fMin( ) uses fFromJulianDateMilitaryTime( 1970,  1,  1,  0,  0,  0 );
		///		fMax( ) uses fFromJulianDateMilitaryTime( 2999, 12, 31, 23, 59, 59 );
		///
		/// \param year		[1970-2999]: Julian year
		/// \param month	[1-12]: Julian month of the year (e.g. 1=January, 12=December)
		/// \param day		[1-31]: Julian day of the month (e.g. 1="The first of ...")
		/// \param hour		[0-23]: Military/24 Hour style hour of the day (e.g. 0=Midnight, 6=6AM, 12=Noon, 18=6PM)
		/// \param minute	[0-59]: Military/24 Hour style minute of the hour
		/// \param second	[0-59*]: Military/24 Hour style second of the minute (60 occasionally technically valid thanks to leap seconds)
		static tDateTime fFromJulianDateMilitaryTime( u32 year, u32 month, u32 day, u32 hour, u32 minute, u32 second );

		/// \brief secondsSinceEpoch Seconds since January 1st 1970 UTC, ignoring leap seconds.
		static tDateTime fFromUnixTimeUTC( u64 secondsSinceEpoch );

		static tDateTime fTodayLocal( );	///< Returns today's date (in local time) at midnight
		static tDateTime fTodayUTC( );		///< Returns today's date (in UTC) at midnight
		static tDateTime fNowLocal( );		///< Returns the current time (in local time)
		static tDateTime fNowUTC( );		///< Returns the current time (in UTC)
		static tDateTime fMin( );			///< Returns a date sufficiently in the past that it should be less than all used dates.
		static tDateTime fMax( );			///< Returns a date sufficiently in the future that it should be past all used dates.



	public:
		///
		/// \brief Date and Time string. Defaults to MM_DD_HHmm format
		std::string fDateTimeString( const char* format = NULL ) const;

		/// \brief Note that this ignores the user and system locales in favor of consistency between systems
		/// (as this will be fed data from various text format files checked into version control)
		///
		/// The format is year-month-day hour:minute:second
		static tDateTime fParseSignalToolsTextTime( const char* asciiTime );

		tStringPtr fToSignalToolsTextTime( ) const;


	public:
		/// We're trying to 'correct' for tm's relative schizophrenia by making everything consistently based on
		/// their display representations instead of a random mishmash of display (tm_mday) and offset (tm_mon) by
		/// consistently using Julian calendar conventions for date, and 'military style' 0 based time.

		inline u32 fYear( )		const { return mTimeInfo.tm_year+1900;	} ///< [1970,3000] Years since 'Year 0'/1BCE
		inline u32 fMonth( )	const { return mTimeInfo.tm_mon+1;		} ///< [1,12] Month of the year
		inline u32 fDay( )		const { return mTimeInfo.tm_mday;		} ///< [1,31] Day of the month
		inline u32 fHour( )		const { return mTimeInfo.tm_hour;		} ///< [0,23] Hours since midnight
		inline u32 fMinute( )	const { return mTimeInfo.tm_min;		} ///< [0,59] Minutes after the hour
		inline u32 fSecond( )	const { return mTimeInfo.tm_sec;		} ///< [0,59*] Seconds after the minute (* Leap seconds technically make 60 occasionally valid)

		inline u32 fMonthIndex( )	const { return fMonth( )	- 1; } ///< [0,11] 0 based month of the year (e.g. for indexing arrays)
		inline u32 fDayIndex( )		const { return fDay( )		- 1; } ///< [0,30] 0 based day of the month (e.g. for indexing arrays)

		void fSetYear	( u32 year		); ///< [1970-2999]: Sets the Julian-style year since 'Year 9'/1 BCE
		void fSetMonth	( u32 month		); ///< [1-12]: Sets the Julian-style month of the year (e.g. 1=January, 12=December)
		void fSetDay	( u32 day		); ///< [1-31]: Sets the Julian-style day of the month (e.g. 1=1st of the month)
		void fSetHour	( u32 hour		); ///< [0-23]: Sets the Military/24-Hour style hour of the day (0=Midnight, 6=6am, 12=Noon, 18=6pm)
		void fSetMinute	( u32 minute	); ///< [0-59]: Sets the Military/24-Hour style minute of the hour
		void fSetSecond	( u32 second	); ///< [0-59*]: Sets the Military/24-Hour style second of the minute.  (* Leap seconds technically make 60 occasionally valid)

		inline void fSetMonthIndex	( u32 monthIndex )	{ fSetMonth	( monthIndex	+ 1 ); } ///< [0-11]: Changes the time based on a 0-based month of the year index.
		inline void fSetDayIndex	( u32 dayIndex )	{ fSetDay	( dayIndex		+ 1 ); } ///< [0-30]: Changes the time based on a 0-based day of the month index.

		tDateTime fDate( ) const; ///< Returns a consistent value for a given day or date (Midnight at the start of that day).

		u64 fToUnixTime( ) const; ///< Returns the number of seconds since Jan 1 1970, minus leap seconds.



	public:
		friend b32 operator==( const tDateTime& lhs, const tDateTime& rhs );
		friend b32 operator!=( const tDateTime& lhs, const tDateTime& rhs );
		friend b32 operator< ( const tDateTime& lhs, const tDateTime& rhs );
		friend b32 operator> ( const tDateTime& lhs, const tDateTime& rhs );
		friend b32 operator<=( const tDateTime& lhs, const tDateTime& rhs );
		friend b32 operator>=( const tDateTime& lhs, const tDateTime& rhs );

		///
		/// \brief Difference in seconds (this - rhs)
		f64 fSecondsDiff( const tDateTime& rhs ) const;



	private:
		tDateTime( tNoOpTag ); ///< Probably not serialization safe, don't make public.
		void fUpdateRawTime( ); ///< Update mRawTime from mTimeInfo
	};
}}


#endif//__Time__
