//------------------------------------------------------------------------------
// \file Time.hpp - 11 Jan 2011
// \author colins
// \author mrickert	tDateTime refactoring
//
// Copyright Signal Studios 2011-2013, All Rights Reserved
//------------------------------------------------------------------------------


#include "BasePch.hpp"

namespace Sig { namespace Time
{
	namespace
	{
		const char * fAsciiDateTime( )
		{
			time_t rawtime;
			time( &rawtime );
			tm* ti = localtime( &rawtime );

			char * asc = asctime( ti ); // returns 'Www Mmm dd hh:mm:ss yyyy' with a \n at the end :(

			//docs say it is exactly 26 chars. stripping the newline with that info
			asc[24] = 0;
			return asc;
		}
	}



	/// General construction & initialization

	tDateTime::tDateTime( b32 utc )
	{
		*this = utc ? fNowUTC( ) : fNowLocal( );
	}

	tDateTime tDateTime::fFromJulianDateMilitaryTime( u32 year, u32 month, u32 day, u32 hour, u32 minute, u32 second )
	{
		tDateTime t( cNoOpTag );

		sigassert( cMinYear <= year && year <= cMaxYear );
		sigassert( 1 <= month	&& month	<= 12 );
		sigassert( 1 <= day		&& day		<= 31 );
		sigassert( 0 <= hour	&& hour		<= 23 );
		sigassert( 0 <= minute	&& minute	<= 59 );
		sigassert( 0 <= second	&& second	<= 59 ); // Possible extremely occasional false positive due to leap seconds.

		t.mTimeInfo.tm_isdst = -1; // C run-time library code compute whether standard time or daylight saving time is in effect.
		t.mTimeInfo.tm_year = year - 1900; // tm_year is since 1900, year is not.
		t.mTimeInfo.tm_mon = month - 1; // tm_mon is since January, month is not.
		t.mTimeInfo.tm_mday = day;
		t.mTimeInfo.tm_hour = hour;
		t.mTimeInfo.tm_min = minute;
		t.mTimeInfo.tm_sec = second;

		t.fUpdateRawTime( );

		return t;
	}

	tDateTime tDateTime::fFromUnixTimeUTC( u64 secondsSinceEpoch )
	{
		tDateTime t( cNoOpTag );

		if( sizeof(time_t) < sizeof(u64) )
			log_warning_once( "tDateTime::fFromUnixTimeUTC: System only handles 32-bit unix timestamps, time may overflow incorrectly (Y2K38 issue)" );

		t.mRawTime = (time_t)secondsSinceEpoch;
		t.mTimeInfo = *gmtime( &t.mRawTime );
		mktime( &t.mTimeInfo );
		return t;
	}

	tDateTime tDateTime::fTodayLocal( )
	{
		return fNowLocal( ).fDate( );
	}

	tDateTime tDateTime::fTodayUTC( )
	{
		return fNowUTC( ).fDate( );
	}

	tDateTime tDateTime::fNowLocal( )
	{
		tDateTime t( cNoOpTag ); // avoid recursion as tDateTime's default ctor uses this
		t.mRawTime = time( NULL );
		t.mTimeInfo = *localtime( &t.mRawTime );
		mktime( &t.mTimeInfo );
		return t;
	}

	tDateTime tDateTime::fNowUTC( )
	{
		tDateTime t( cNoOpTag ); // avoid recursion as tDateTime's default ctor uses this
		t.mRawTime = time( NULL );
		t.mTimeInfo = *gmtime( &t.mRawTime );
		mktime( &t.mTimeInfo );
		return t;
	}

	tDateTime tDateTime::fMin( )
	{
		return fFromJulianDateMilitaryTime( cMinYear, 1, 1, 0, 0, 0 );
	}

	tDateTime tDateTime::fMax( )
	{
		return fFromJulianDateMilitaryTime( cMaxYear, 12, 31, 23, 59, 59 );
	}



	/// Display and formatting

	std::string tDateTime::fDateTimeString( const char* format ) const
	{
		if( !format )
			format = "%m_%d_%H%M";

		char buffer[1024];
		return std::string( buffer, strftime( buffer, array_length( buffer ), format, &mTimeInfo ) );
	}

	tDateTime tDateTime::fParseSignalToolsTextTime( const char* asciiTime )
	{
		std::stringstream ss;
		ss << asciiTime;
		ss >> std::noskipws;

		const char* const cDateSeperators = "-/\\.";
		const char* const cSpaceSeperators = " \t\r\n";
		const char* const cTimeSeperarators = ":";

		u32 year=1, month=1, day=1; // date -- mandatory in text
		u32 hour=0, minute=0, second=0; // time -- optional in text
		char chEndYear, chEndMonth, chEndDay, chEndHour, chEndMinute, chEndSecond;

		// Must always at minimum parse a full date
		sigcheckfail( (ss >> year	>> chEndYear	) && StringUtil::fIsAnyOf( cDateSeperators, chEndYear	)	, return tDateTime::fMin( ) );
		sigcheckfail( (ss >> month	>> chEndMonth	) && StringUtil::fIsAnyOf( cDateSeperators, chEndMonth	)	, return tDateTime::fMin( ) );
		sigcheckfail( (ss >> day					)															, return tDateTime::fMin( ) );

		if( ss >> chEndDay )
		{
			// More text after the date? At minimum parse a HH:MM
			sigcheckfail( StringUtil::fIsAnyOf( cSpaceSeperators, chEndDay ), return tDateTime::fMin( ) );
			sigcheckfail( (ss >> hour	>> chEndHour) && StringUtil::fIsAnyOf( cTimeSeperarators, chEndHour )	, return tDateTime::fMin( ) );
			sigcheckfail( (ss >> minute				)															, return tDateTime::fMin( ) );

			if( ss >> chEndMinute )
			{
				// More text after the minute? Parse the seconds, and nothing more.
				sigcheckfail( StringUtil::fIsAnyOf( cTimeSeperarators, chEndMinute ), return tDateTime::fMin( ) );
				sigcheckfail( (ss >> second), return tDateTime::fMin( ) );
				log_sigcheckfail( !(ss >> chEndSecond), "Text after seconds invalid", return tDateTime::fMin( ) );
			}
		}

		return tDateTime::fFromJulianDateMilitaryTime( year, month, day, hour, minute, second );
	}

	tStringPtr tDateTime::fToSignalToolsTextTime( ) const
	{
		return tStringPtr( fDateTimeString( "%Y-%m-%d %H:%M:%S" ).c_str() );
	}


	/// Comparison

	b32 operator==( const tDateTime& lhs, const tDateTime& rhs )
	{
		return lhs.mRawTime == rhs.mRawTime;
	}

	b32 operator!=( const tDateTime& lhs, const tDateTime& rhs )
	{
		return !( lhs == rhs );
	}

	b32 operator<( const tDateTime& lhs, const tDateTime& rhs )
	{
		return lhs.fSecondsDiff( rhs ) < 0.0;
	}

	b32 operator>( const tDateTime& lhs, const tDateTime& rhs )
	{
		return lhs.fSecondsDiff( rhs ) > 0.0;
	}

	b32 operator<=( const tDateTime& lhs, const tDateTime& rhs )
	{
		return lhs.fSecondsDiff( rhs ) <= 0.0;
	}

	b32 operator>=( const tDateTime& lhs, const tDateTime& rhs )
	{
		return lhs.fSecondsDiff( rhs ) >= 0.0;
	}

	f64 tDateTime::fSecondsDiff( const tDateTime& rhs ) const
	{
		return difftime( mRawTime, rhs.mRawTime );
	}



	/// Manipulation and querying

	void tDateTime::fSetYear( u32 year )
	{
		sigassert( cMinYear <= year && year <= cMaxYear );
		mTimeInfo.tm_year = year - 1900; // tm_year is years since 1900
		fUpdateRawTime( );
	}

	void tDateTime::fSetMonth( u32 month )
	{
		sigassert( 1 <= month && month <= 12 );
		mTimeInfo.tm_mon = month - 1; // tm_mon is days since January
		fUpdateRawTime( );
	}

	void tDateTime::fSetDay( u32 day )
	{
		sigassert( 1 <= day && day <= 31 );
		mTimeInfo.tm_mday = day;
		fUpdateRawTime( );
	}

	void tDateTime::fSetHour( u32 hour )
	{
		sigassert( 0 <= hour && hour <= 23 );
		mTimeInfo.tm_hour = hour;
		fUpdateRawTime( );
	}

	void tDateTime::fSetMinute( u32 minute )
	{
		sigassert( 0 <= minute && minute <= 59 );
		mTimeInfo.tm_min = minute;
		fUpdateRawTime( );
	}

	void tDateTime::fSetSecond( u32 second )
	{
		sigassert( 0 <= second && second <= 59 ); // Possible extremely occasional false positive due to leap seconds
		mTimeInfo.tm_sec = second;
		fUpdateRawTime( );
	}

	tDateTime tDateTime::fDate( ) const
	{
		tDateTime t( *this );

		// 0 out time
		t.mTimeInfo.tm_hour = 0;
		t.mTimeInfo.tm_min = 0;
		t.mTimeInfo.tm_sec = 0;
		t.fUpdateRawTime( );

		return t;
	}

	u64 tDateTime::fToUnixTime( ) const
	{
		return (u64)mRawTime; // time_t is unix time
	}



	/// Internal details

	tDateTime::tDateTime( tNoOpTag )
	{
	}

	void tDateTime::fUpdateRawTime( )
	{
		mRawTime = mktime( &mTimeInfo );
		sigassert( mRawTime != -1 && "mkTime( ) failed; mTimeInfo is invalid" );
	}
}}

