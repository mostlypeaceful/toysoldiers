//------------------------------------------------------------------------------
// \file tGameSessionSearch_pc.cpp - 19 Oct 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
#include "tGameSessionSearch.hpp"

namespace Sig
{
#if defined( use_steam )
	//------------------------------------------------------------------------------
	// tSearchData
	//------------------------------------------------------------------------------
	tGameSessionSearch::tSearchData::tSearchData( )
	{
		mApiCall = k_uAPICallInvalid;
	}

	//------------------------------------------------------------------------------
	b32 tGameSessionSearch::tSearchData::fOverlapComplete( b32 & success, b32 wait )
	{
		if( mApiCall != k_uAPICallInvalid )
		{
			bool failed = false;
			bool completed = false;
			do
			{
				completed = SteamUtils( )->IsAPICallCompleted( mApiCall, &failed );
			} while ( wait && !completed );
			if( !completed )
				return false;

			success = !failed;
			mLastError = SteamUtils( )->GetAPICallFailureReason( mApiCall );
			mApiCall = k_uAPICallInvalid;
		}
		return true;
	}

	//------------------------------------------------------------------------------
	// tGameSessionSearchResult
	//------------------------------------------------------------------------------
	tGameSessionSearchResult::tGameSessionSearchResult( const tGameSessionInfo & info )
	{
		mInfo = info;
		mTotalSlots = 0;
		mFilledSlots = 0;

		if( mInfo.fIsValid( ) )
		{
			mTotalSlots = SteamMatchmaking( )->GetLobbyMemberLimit( mInfo.mId );
			mFilledSlots = SteamMatchmaking( )->GetNumLobbyMembers( mInfo.mId );
		}
	}

	//------------------------------------------------------------------------------
	// tGameSessionSearch
	//------------------------------------------------------------------------------
	tGameSessionSearch::tGameSessionSearch( )
		: mState( cStateNull )
		, mCallbackRequestLobbyList( this, &tGameSessionSearch::fOnRequestLobbyList )
	{
	}

	//------------------------------------------------------------------------------
	void tGameSessionSearch::fOnRequestLobbyList(LobbyMatchList_t *pParam)
	{
		log_line( Log::cFlagSession, __FUNCTION__ << " returned " << pParam->m_nLobbiesMatching << " lobbies" );
		mData.mResultsCount = pParam->m_nLobbiesMatching;
	}

	//------------------------------------------------------------------------------
	void tGameSessionSearch::fCancel( )
	{
		if( mState == cStateSearching )
		{
			mState = cStateFail;
		}
		
		fReset( );
	}

	//------------------------------------------------------------------------------
	void tGameSessionSearch::fReset( )
	{
		sigassert( mState != cStateSearching && "Reset called during active session search" );

		if( mState == cStateNull )
			return;

		mState = cStateNull;

		fDestroy( );
	}

	//------------------------------------------------------------------------------
	void tGameSessionSearch::fTick( )
	{
		if( mState == cStateSearching )
		{
			b32 success = false;
			if( mData.fOverlapComplete( success, false ) && mData.mResultsCount != ~0 )
			{
				if( success )
				{
					mData.mResults.fSetCapacity( mData.mResultsCount );
					for( u32 i = 0; i < mData.mResultsCount; ++i )
					{
						auto lobby = SteamMatchmaking( )->GetLobbyByIndex( i );
						tGameSessionInfo searchInfo = tGameSessionInfo( lobby );
						tGameSessionSearchResult searchResult = tGameSessionSearchResult( searchInfo );
						mData.mResults.fPushBack( searchResult );
					}
					mState = cStateSuccess;
				}
				else
				{
					mState = cStateFail;
				}
			}
		}
	}

	//------------------------------------------------------------------------------
	b32 tGameSessionSearch::fBegin( 
		u32 userIndex,
		u32 gameType,
		u32 gameMode,
		u32 procedure, 
		u32 maxResultCount, 
		u32 numUsers )
	{
		log_line( Log::cFlagSession, __FUNCTION__ );
		mState = cStateSearching;
		SteamMatchmaking( )->AddRequestLobbyListNumericalFilter( "gameType", *( int* )&gameType, k_ELobbyComparisonEqual );
		SteamMatchmaking( )->AddRequestLobbyListNumericalFilter( "gameMode", *( int* )&gameMode, k_ELobbyComparisonEqual );
		for( u32 i = 0; i < mData.mProperties.fCount( ); ++i )
		{
			auto userProp = mData.mProperties[ i ];
			auto key = StringUtil::fToString( userProp.mId );
			int value;
			if( userProp.mData.fType( ) == tUserData::cTypeS32 )
				value = userProp.mData.fGetS32( );
			else if( userProp.mData.fType( ) == tUserData::cTypeS64 )
				value = ( int )userProp.mData.fGetS64( );
			switch( userProp.mFilter )
			{
			case cSearchFilterExact:
				SteamMatchmaking( )->AddRequestLobbyListNumericalFilter( key.c_str( ), value, k_ELobbyComparisonEqual );
				log_line( Log::cFlagSession, "Search property " << key << " value " << value << " (exact)" );
				break;
			case cSearchFilterNear:
				SteamMatchmaking( )->AddRequestLobbyListNearValueFilter( key.c_str( ), value );
				log_line( Log::cFlagSession, "Search property " << key << " value " << value << " (near)" );
				break;
			}
		}
		SteamMatchmaking( )->AddRequestLobbyListResultCountFilter( maxResultCount );
		mData.mResultsCount = ~0;
		mData.mApiCall = SteamMatchmaking( )->RequestLobbyList( );
		return mData.mApiCall != k_uAPICallInvalid;
	}

	//------------------------------------------------------------------------------
	b32 tGameSessionSearch::fBegin( u32 userIndex, const tGameSessionInfo & info )
	{
		log_line( Log::cFlagSession, __FUNCTION__ );
		mData.mResultsCount = 1;
		mData.mResults.fSetCapacity( 1 );
		tGameSessionInfo searchInfo = tGameSessionInfo( info.mId );
		tGameSessionSearchResult searchResult = tGameSessionSearchResult( searchInfo );
		mData.mResults.fPushBack( searchResult );
		mState = cStateSuccess;
		return true;
	}

	//------------------------------------------------------------------------------
	void tGameSessionSearch::fSetContext( u32 id, u32 value )
	{
		auto userProp = tUserProperty( id );
		userProp.mData.fSet( *( s32* )&value );
		mData.mContexts.fPushBack( userProp );
	}

	//------------------------------------------------------------------------------
	void tGameSessionSearch::fSetProperty( u32 id, const tUserData & value, tSearchFilter filter )
	{
		auto userProp = tUserProperty( id );
		userProp.mData = value;
		userProp.mFilter = filter;
		mData.mProperties.fPushBack( userProp );
	}

	//------------------------------------------------------------------------------
	u32 tGameSessionSearch::fFindOpenResult( u32 slotsNeeded ) const
	{
		if( mState == cStateSuccess )
		{
			for( u32 i = 0; i < mData.mResults.fCount( ); ++i )
			{
				if( mData.mResults[ i ].fOpenPublicSlots( ) >= slotsNeeded )
					return i;
			}
		}
		return ~0;
	}

	//------------------------------------------------------------------------------
	u32 tGameSessionSearch::fResultCount( ) const
	{
		sigassert( mState == cStateSuccess && "Can only access session search results on successful searches" );
		return mData.mResults.fCount( );
	}

	//------------------------------------------------------------------------------
	const tGameSessionSearchResult & tGameSessionSearch::fResult( u32 idx ) const
	{
		sigassert( mState == cStateSuccess && "Can only access session search results on successful searches" );
		sigassert( idx < mData.mResults.fCount( ) && "Result index out of bounds" );
		return mData.mResults[ idx ];
	}

	//------------------------------------------------------------------------------
	u32 tGameSessionSearch::fResultContext( u32 idx, u32 context ) const
	{
		// Never used. Remains unimplemented.
		log_warning_unimplemented( 0 );
		return ~0;
	}

	//------------------------------------------------------------------------------
	void tGameSessionSearch::fResultProperty( u32 idx, tUserProperty & prop ) const
	{
		// Never used. Remains unimplemented.
		log_warning_unimplemented( 0 );
		prop.mData.fReset( );
	}

	//------------------------------------------------------------------------------
	void tGameSessionSearch::fDestroy( )
	{
		sigassert( mState == cStateNull && "Sanity: Destroy can only act when the state is null" );

		mData.mApiCall = k_uAPICallInvalid;
		mData.mContexts.fDeleteArray( );
		mData.mProperties.fDeleteArray( );
		mData.mResults.fDeleteArray( );
		mData.mResultsCount = 0;
	}
#else//#if defined( use_steam )
	void tGameSessionSearch::fCancel( )
	{
		log_warning_unimplemented( 0 );
	}

	void tGameSessionSearch::fTick( )
	{
		log_warning_unimplemented( 0 );
	}

	b32 tGameSessionSearch::fBegin( 
		u32 userIndex,
		u32 gameType,
		u32 gameMode,
		u32 procedure, 
		u32 maxResultCount, 
		u32 numUsers )
	{
		log_warning_unimplemented( 0 );
		return false;
	}

	b32 tGameSessionSearch::fBegin( u32 userIndex, const tGameSessionInfo & info )
	{
		log_warning_unimplemented( 0 );
		return false;
	}

	void tGameSessionSearch::fSetContext( u32 id, u32 value )
	{
		log_warning_unimplemented( 0 );
	}

	void tGameSessionSearch::fSetProperty( u32 id, const tUserData & value )
	{
		log_warning_unimplemented( 0 );
	}

	u32 tGameSessionSearch::fFindOpenResult( u32 slotsNeeded ) const
	{
		log_warning_unimplemented( 0 );
		return ~0;
	}

	u32 tGameSessionSearch::fResultCount( ) const
	{
		log_warning_unimplemented( 0 );
		return 0;
	}

	const tGameSessionSearchResult & tGameSessionSearch::fResult( u32 idx ) const
	{
		log_warning_unimplemented( 0 );
		static tGameSessionSearchResult result;
		return result;
	}

	u32 tGameSessionSearch::fResultContext( u32 idx, u32 context ) const
	{
		log_warning_unimplemented( 0 );
		return ~0;
	}

	void tGameSessionSearch::fResultProperty( u32 idx, tUserProperty & prop ) const
	{
		log_warning_unimplemented( 0 );
		prop.mData.fReset( );
	}
#endif//#if defined( use_steam )
}
#endif//#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
