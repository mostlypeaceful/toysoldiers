//------------------------------------------------------------------------------
// \file tAchievements_pc.cpp - 21 Dec 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tAchievements.hpp"
#include "tWin32Window.hpp"
#include "tGameAppBase.hpp"

namespace Sig
{
	const u32 tAchievementsReader::cDetailsLabel = ( 1 << 0 );
	const u32 tAchievementsReader::cDetailsDescription = ( 1 << 1 );
	const u32 tAchievementsReader::cDetailsUnachieved = ( 1 << 2 );

#if defined( use_steam )

	namespace Achievements
	{
		tDelegate< const tStringPtr& ( u32 ) > sAchievementIdToValueString;
	}

	//------------------------------------------------------------------------------
	// tAchievementsWriter::tWriteData
	//------------------------------------------------------------------------------
	tAchievementsWriter::tWriteData::tWriteData( )
		: mCallbackUserAchievementStored( this, &tWriteData::fOnUserAchievementStored )
		, mCallbackUserStatsStored( this, &tWriteData::fOnUserStatsStored )
	{
		fClear( );
	}

	//------------------------------------------------------------------------------
	tAchievementsWriter::tWriteData::~tWriteData( )
	{
		fCancel( );
	}

	//------------------------------------------------------------------------------
	void tAchievementsWriter::tWriteData::fCancel( )
	{
		fClear( );
	}

	//------------------------------------------------------------------------------
	b32 tAchievementsWriter::tWriteData::fOverlapComplete( b32 & success )
	{
		if( mState >= cStateDone )
		{
			success = ( mState == cStateDone );
			return true;
		};

		return false;
	}

	//------------------------------------------------------------------------------
	void tAchievementsWriter::tWriteData::fWrite( tDynamicArray< tStringPtr >& achievements )
	{
		u32 count = achievements.fCount( );
		for( u32 i = 0; i < count; ++i )
		{
			auto name = achievements[ i ];
			SteamUserStats( )->SetAchievement( name.fCStr( ) );
		}
		SteamUserStats( )->StoreStats( );
		mState = cStateWriting;
	}

	//------------------------------------------------------------------------------
	void tAchievementsWriter::tWriteData::fClear( )
	{
		mState = cStateNone;
	}

	//------------------------------------------------------------------------------
	void tAchievementsWriter::tWriteData::fOnUserStatsStored( UserStatsStored_t *pResult )
	{
		// Apparently we might receive stats for other titles, according to the documentation
		if( pResult->m_nGameID == SteamUtils( )->GetAppID( ) )
		{
			if( k_EResultOK == pResult->m_eResult )
				mState = cStateDone;
			else
				mState = cStateError;
		}
	}

	//------------------------------------------------------------------------------
	void tAchievementsWriter::tWriteData::fOnUserAchievementStored( UserAchievementStored_t *pResult )
	{
	}

	//------------------------------------------------------------------------------
	// tAchievementsWriter
	//------------------------------------------------------------------------------
	tAchievementsWriter::tAchievementsWriter( 
		u32 localUserIndex, u32 count, const u32 achievements[] )
		: mState( cStateNull )
	{
		mToWrite.fNewArray( count );
		for( u32 i = 0; i < count; ++i )
		{
			mToWrite[ i ] = Achievements::sAchievementIdToValueString( achievements[ i ] );
		}
	}

	//------------------------------------------------------------------------------
	void tAchievementsWriter::fBegin( )
	{
		mWriteData.fWrite( mToWrite );
		mState = cStateWriting;
		mStarted = true;
	}

	//------------------------------------------------------------------------------
	b32 tAchievementsWriter::fFinished( )
	{
		b32 success = false;
		if( mWriteData.fOverlapComplete( success ) )
		{
			mStarted = false;
			mState = success ? cStateSuccess : cStateFail;
			mErrored = !success;
		}
		return mState >= cStateSuccess;
	}

	//------------------------------------------------------------------------------
	void tAchievementsWriter::fFinish( b32 wait )
	{
		sigassert( !wait && "Blocking wait not permitted on PC" );
	}

	//------------------------------------------------------------------------------
	// tAchievementsReader::tReadData
	//------------------------------------------------------------------------------
	tAchievementsReader::tReadData::tReadData( )
		: mCallbackUserStatsReceived( this, &tReadData::fOnUserStatsReceived )
	{
		fClear( );
	}

	//------------------------------------------------------------------------------
	tAchievementsReader::tReadData::~tReadData( )
	{
		fCancel( );
	}

	//------------------------------------------------------------------------------
	void tAchievementsReader::tReadData::fCancel( )
	{
		//if( mCallbackUserStatsReceived.IsActive( ) )
		//	mCallbackUserStatsReceived.Cancel( );
		fClear( );
	}

	//------------------------------------------------------------------------------
	b32 tAchievementsReader::tReadData::fOverlapComplete( b32 & success, u32 & count )
	{
		if( mState >= cStateDone )
		{
			success = ( mState == cStateDone );
			return true;
		};

		return false;
	}

	//------------------------------------------------------------------------------
	void tAchievementsReader::tReadData::fRead( tPlatformUserId target )
	{
		log_line( Log::cFlagNone, __FUNCTION__ << " Beginning to read achievements" );
		SteamAPICall_t apiCall = k_uAPICallInvalid;
		CSteamID targetId = target;
		if( targetId == SteamUser( )->GetSteamID( ) )
			apiCall = SteamUserStats( )->RequestCurrentStats( );
		else
			apiCall = SteamUserStats( )->RequestUserStats( targetId );
		sigassert( apiCall != k_uAPICallInvalid && "Failed to read user stats from Steam" );
		//if( apiCall != k_uAPICallInvalid )
		//{
		//	mCallbackUserStatsReceived.Set( apiCall, this, &tReadData::fOnUserStatsReceived );
		//	mState = cStateReading;
		//}
	}

	//------------------------------------------------------------------------------
	void tAchievementsReader::tReadData::fClear( )
	{
		mState = cStateNone;
	}

	//------------------------------------------------------------------------------
	void tAchievementsReader::tReadData::fOnUserStatsReceived( UserStatsReceived_t *pResult/*, bool bIOFailure*/ )
	{
		// Apparently we might receive stats for other titles, according to the documentation
		if( pResult->m_nGameID == SteamUtils( )->GetAppID( ) )
		{
			log_line( Log::cFlagNone, __FUNCTION__ << " Achievement read completed " << ( k_EResultOK == pResult->m_eResult ? "successfully" : "unsuccessfully" ) );
			if( k_EResultOK == pResult->m_eResult )
				mState = cStateDone;
			else
				mState = cStateError;
		}
	}

	//------------------------------------------------------------------------------
	// tAchievementsReader
	//------------------------------------------------------------------------------
	void tAchievementsReader::fRead( u32 startIdx, u32 toRead, u32 details, tPlatformUserId target )
	{
		sigassert( mState != cStateReading && "Cancel any pending reads before starting a new one" );
		sigassert( mUserIndex < tUser::cMaxLocalUsers );

		mReadData.fRead( target == tUser::cInvalidUserId ? tPlatformUserId( SteamUser( )->GetSteamID( ) ) : target );
		mState = cStateReading;
	}

	//------------------------------------------------------------------------------
	void tAchievementsReader::fCancelRead( )
	{
		if( mState != cStateReading )
			return;
		mReadData.fCancel( );
		mState = cStateNull;
	}

	//------------------------------------------------------------------------------
	b32	tAchievementsReader::fAdvanceRead( )
	{
		if( mState == cStateReading )
		{
			b32 success;
			if( !mReadData.fOverlapComplete( success, mReadCount ) )
				return false;

			mState = success ? cStateSuccess : cStateFailure;
		}

		return true;
	}

	//------------------------------------------------------------------------------
	b32	tAchievementsReader::fIsAwarded( u32 id )
	{
		bool achieved = false;
		auto name = Achievements::sAchievementIdToValueString( id );
		if( SteamUserStats( )->GetAchievement( name.fCStr( ), &achieved ) )
			return achieved;
		log_line( Log::cFlagNone, __FUNCTION__ << " Could not find achievement " << name.fCStr( ) );
		return false;
	}

	//------------------------------------------------------------------------------
	b32 tAchievementsReader::fGetData( u32 id, tAchievementData& out )
	{
		auto achievement = Achievements::sAchievementIdToValueString( id );
		auto name = SteamUserStats( )->GetAchievementDisplayAttribute( achievement.fCStr( ), "name" );
		out.mLabel.fFromCStr( name );
		auto desc = SteamUserStats( )->GetAchievementDisplayAttribute( achievement.fCStr( ), "desc" );
		out.mDescription.fFromCStr( desc );

		out.mImageId = 0;

		return true;
	}

#else
	//------------------------------------------------------------------------------
	// tAchievementsWriter
	//------------------------------------------------------------------------------
	tAchievementsWriter::tAchievementsWriter( u32 localUserIndex, u32 count, const u32 achievements[] )
	{
	}

	//------------------------------------------------------------------------------
	void tAchievementsWriter::fBegin( )
	{
	}

	//------------------------------------------------------------------------------
	b32 tAchievementsWriter::fFinished( )
	{
		return true;
	}

	//------------------------------------------------------------------------------
	void tAchievementsWriter::fFinish( b32 wait )
	{
	}

	//------------------------------------------------------------------------------
	// tAchievementsReader
	//------------------------------------------------------------------------------
	void tAchievementsReader::fRead( u32 startIdx, u32 toRead, u32 details, tPlatformUserId target )
	{
	}

	//------------------------------------------------------------------------------
	void tAchievementsReader::fCancelRead( )
	{
	}

	//------------------------------------------------------------------------------
	b32	tAchievementsReader::fAdvanceRead( )
	{
		return true;
	}

	//------------------------------------------------------------------------------
	b32	tAchievementsReader::fIsAwarded( u32 id )
	{
		return false;
	}

	//------------------------------------------------------------------------------
	b32 tAchievementsReader::fGetData( u32 id, tAchievementData& out )
	{
		return false;
	}
#endif
}
