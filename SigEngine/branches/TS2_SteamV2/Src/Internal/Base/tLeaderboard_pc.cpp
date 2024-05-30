#include "BasePch.hpp"
#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
#include "tLeaderboard.hpp"
#include "tGameAppBase.hpp"
#if defined( use_steam )
#include "tGameSession.hpp"
#endif

namespace Sig 
{
	//------------------------------------------------------------------------------
	// tLeaderboardDesc
	//------------------------------------------------------------------------------
	u32 tLeaderboardDesc::fBoardId( ) const
	{
		return mSpec.dwViewId;
	}

#if defined( use_steam )
	static tLeaderboardPtr sCurrent;

	//------------------------------------------------------------------------------
	// tReadData
	//------------------------------------------------------------------------------
	tLeaderboard::tReadData::tReadData( )
	{
		fClear( );
	}

	//------------------------------------------------------------------------------
	tLeaderboard::tReadData::~tReadData( )
	{
		fCancel( );
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::tReadData::fClear( )
	{
		for( u32 i = 0; i < mViews.fCount( ); ++i )
		{
			mViews[ i ].dwNumRows = 0;
		}
		mViewsCount = 0;
		mCurrentBoard = 0;
		mRequestingUser = tUser::cInvalidUserId;
		mFriendReadStart = 0;
		mUserIds.fDeleteArray( );
		mUserIdCount = 0;
		mUser = nullptr;
		mRankStart = 0;
		mToRead = 0;
		mState = cStateNone;
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::tReadData::fCancel( )
	{
		if( mCallbackFindLeaderboard.IsActive( ) )
			mCallbackFindLeaderboard.Cancel( );
		if( mCallbackGetLeaderboardEntries.IsActive( ) )
			mCallbackGetLeaderboardEntries.Cancel( );
		fClear( );
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::tReadData::fReadByRank( u32 rankStart, u32 toRead )
	{
		fClear( );
		mRequestingUser = tUser::cInvalidUserId;
		mRankStart = rankStart;
		mToRead = toRead;
		mState = cStateWaiting;
		mRequestType = cRequestTypeByRank;
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::tReadData::fReadByRankAround( tUser * user, u32 toRead )
	{
		fClear( );
		mRequestingUser = tUser::cInvalidUserId;
		mToRead = toRead;
		mState = cStateWaiting;
		mRequestType = cRequestTypeByRankAround;
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::tReadData::fReadByFriends( tUser * user, u32 friendStart )
	{
		fClear( );
		mRequestingUser = user->fPlatformId( );
		mFriendReadStart = friendStart;
		mState = cStateWaiting;
		mRequestType = cRequestTypeByFriends;
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::tReadData::fReadByPlatformId( const tPlatformUserId userIds[], u32 userIdCount )
	{
		fClear( );
		mRequestingUser = tUser::cInvalidUserId;
		mUserIds.fNewArray( userIdCount );
		for( u32 i = 0; i < userIdCount; ++i )
			mUserIds[ i ] = userIds[ i ];
		mUserIdCount = userIdCount;
		mState = cStateWaiting;
		mRequestType = cRequestTypeByPlatformId;
	}

	//------------------------------------------------------------------------------
	b32 tLeaderboard::tReadData::fOverlapComplete( b32 & success )
	{
		if( mState == cStateWaiting )
		{
			if( !tGameSession::fIsWriteActive( ) )
			{
				switch( mRequestType )
				{
				case cRequestTypeByRank:
					{
						auto handle = tLeaderboard::fGetHandle( mBoards[ mCurrentBoard ]->mSpec.mViewName );
						if( handle )
						{
							log_line( Log::cFlagNone, __FUNCTION__ << " Found cached leaderboard handle for " << mBoards[ mCurrentBoard ]->mSpec.mViewName );
							LeaderboardFindResult_t result;
							result.m_bLeaderboardFound = 1;
							result.m_hSteamLeaderboard = handle;
							fOnFindLeaderboardByRank( &result, false );
						}
						else
						{
							log_line( Log::cFlagNone, __FUNCTION__ << " Finding/creating leaderboard " << mBoards[ mCurrentBoard ]->mSpec.mViewName );
							auto apiCall = SteamUserStats( )->FindOrCreateLeaderboard( mBoards[ mCurrentBoard ]->mSpec.mViewName, k_ELeaderboardSortMethodDescending, k_ELeaderboardDisplayTypeNumeric );
							mCallbackFindLeaderboard.Set( apiCall, this, &tReadData::fOnFindLeaderboardByRank );
						}
					}
					break;
				case cRequestTypeByRankAround:
					{
						auto handle = tLeaderboard::fGetHandle( mBoards[ mCurrentBoard ]->mSpec.mViewName );
						if( handle )
						{
							log_line( Log::cFlagNone, __FUNCTION__ << " Found cached leaderboard handle for " << mBoards[ mCurrentBoard ]->mSpec.mViewName );
							LeaderboardFindResult_t result;
							result.m_bLeaderboardFound = 1;
							result.m_hSteamLeaderboard = handle;
							fOnFindLeaderboardByRankAround( &result, false );
						}
						else
						{
							log_line( Log::cFlagNone, __FUNCTION__ << " Finding/creating leaderboard " << mBoards[ mCurrentBoard ]->mSpec.mViewName );
							auto apiCall = SteamUserStats( )->FindOrCreateLeaderboard( mBoards[ mCurrentBoard ]->mSpec.mViewName, k_ELeaderboardSortMethodDescending, k_ELeaderboardDisplayTypeNumeric );
							mCallbackFindLeaderboard.Set( apiCall, this, &tReadData::fOnFindLeaderboardByRankAround );
						}
					}
					break;
				case cRequestTypeByFriends:
					{
						auto handle = tLeaderboard::fGetHandle( mBoards[ mCurrentBoard ]->mSpec.mViewName );
						if( handle )
						{
							log_line( Log::cFlagNone, __FUNCTION__ << " Found cached leaderboard handle for " << mBoards[ mCurrentBoard ]->mSpec.mViewName );
							LeaderboardFindResult_t result;
							result.m_bLeaderboardFound = 1;
							result.m_hSteamLeaderboard = handle;
							fOnFindLeaderboardByFriends( &result, false );
						}
						else
						{
							log_line( Log::cFlagNone, __FUNCTION__ << " Finding/creating leaderboard " << mBoards[ mCurrentBoard ]->mSpec.mViewName );
							auto apiCall = SteamUserStats( )->FindOrCreateLeaderboard( mBoards[ mCurrentBoard ]->mSpec.mViewName, k_ELeaderboardSortMethodDescending, k_ELeaderboardDisplayTypeNumeric );
							mCallbackFindLeaderboard.Set( apiCall, this, &tReadData::fOnFindLeaderboardByFriends );
						}
					}
					break;
				case cRequestTypeByPlatformId:
					{
						auto handle = tLeaderboard::fGetHandle( mBoards[ mCurrentBoard ]->mSpec.mViewName );
						if( handle )
						{
							log_line( Log::cFlagNone, __FUNCTION__ << " Found cached leaderboard handle for " << mBoards[ mCurrentBoard ]->mSpec.mViewName );
							LeaderboardFindResult_t result;
							result.m_bLeaderboardFound = 1;
							result.m_hSteamLeaderboard = handle;
							fOnFindLeaderboardByPlatformId( &result, false );
						}
						else
						{
							log_line( Log::cFlagNone, __FUNCTION__ << " Finding/creating leaderboard " << mBoards[ mCurrentBoard ]->mSpec.mViewName );
							auto apiCall = SteamUserStats( )->FindOrCreateLeaderboard( mBoards[ mCurrentBoard ]->mSpec.mViewName, k_ELeaderboardSortMethodDescending, k_ELeaderboardDisplayTypeNumeric );
							mCallbackFindLeaderboard.Set( apiCall, this, &tReadData::fOnFindLeaderboardByPlatformId );
						}
					}
					break;
				}
				mState = cStateFinding;
			}
		}

		if( mState >= cStateDone )
		{
			success = ( mState == cStateDone );
			return true;
		};

		return false;
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::tReadData::fAddEntriesToView( LeaderboardScoresDownloaded_t *params )
	{
		mViews[ mCurrentBoard ].dwViewId = mBoards[ mCurrentBoard ]->mSpec.dwViewId;
		mViews[ mCurrentBoard ].dwNumRows = 0;
		mViews[ mCurrentBoard ].dwTotalViewRows = SteamUserStats( )->GetLeaderboardEntryCount( params->m_hSteamLeaderboard );
		mViewsCount = mCurrentBoard + 1;
		auto board = tLeaderboard::fFindLeaderboardByViewId( mViews[ mCurrentBoard ].dwViewId );
		for( int i = 0; i < params->m_cEntryCount; ++i )
		{
			LeaderboardEntry_t entry;
			int details[ cUserAttributesInStatsSpec ];
			if( SteamUserStats( )->GetDownloadedLeaderboardEntry( params->m_hSteamLeaderboardEntries, i, &entry, details, cUserAttributesInStatsSpec ) )
			{
				tUserStatsRow row;
				row.dwRank = entry.m_nGlobalRank;
				row.mUserId = tPlatformUserId( entry.m_steamIDUser );
				row.szGamertag = tStringPtr( SteamFriends( )->GetFriendPersonaName( entry.m_steamIDUser ) );
				row.pColumns.fNewArray( mBoards[ mCurrentBoard ]->mSpec.dwNumColumnIds );
				row.dwNumColumns = row.pColumns.fCount( );
				for( u32 j = 0; j < mBoards[ mCurrentBoard ]->mSpec.dwNumColumnIds; ++j )
				{
					tUserStatsColumn col;
					row.pColumns[ j ].wColumnId = mBoards[ mCurrentBoard ]->mSpec.wColumnId[ j ];
					// Steam stores columns as s32. Xbox 360 stores them as s64. The game wants them as u32 cast to s64.
					if( mBoards[ mCurrentBoard ]->mColumnDescs[ j ].mUserData == 1 /* GameFlags::cLEADERBOARD_COLUMN_TYPE_TIME in GameApp */ )
					{
						// Column is a float
						if( j < ( u32 )entry.m_cDetails )
						{
							s32 v = details[ j ];
							row.pColumns[ j ].Value.fSet( ( f64 )( *( f32* )&v ) );
						}
						else
						{
							row.pColumns[ j ].Value.fSet( ( f64 )0.0 );
						}
					}
					else
					{
						// Column is an integer
						if( j < ( u32 )entry.m_cDetails )
						{
							s32 v = details[ j ];
							row.pColumns[ j ].Value.fSet( ( s64 )( *( u32* )&v ) );
						}
						else
						{
							row.pColumns[ j ].Value.fSet( ( s64 )0 );
						}
					}
				}
				mViews[ mCurrentBoard ].pRows.fPushBack( row );
				++mViews[ mCurrentBoard ].dwNumRows;
			}
		}
		log_line( Log::cFlagNone, __FUNCTION__ << " Added " << mViews[ mCurrentBoard ].dwNumRows << " rows from leaderboard " << mBoards[ mCurrentBoard ]->mSpec.mViewName );
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::tReadData::fOnFindLeaderboardByRank( LeaderboardFindResult_t *pResult, bool bIOFailure )
	{
		if( !pResult->m_bLeaderboardFound )
		{
			log_warning( Log::cFlagNone, "Could not find leaderboard " << mBoards[ mCurrentBoard ]->mSpec.mViewName );
			mState = cStateError;
			return;
		}

		if( bIOFailure )
		{
			log_warning( Log::cFlagNone, "I/O failure finding leaderboard " << mBoards[ mCurrentBoard ]->mSpec.mViewName );
			mState = cStateError;
			return;
		}

		tLeaderboard::fAddHandle( mBoards[ mCurrentBoard ]->mSpec.mViewName, pResult->m_hSteamLeaderboard );
		log_line( Log::cFlagNone, __FUNCTION__ << " Found leaderboard " << mBoards[ mCurrentBoard ]->mSpec.mViewName << ". Beginning download of entries." );
		auto apiCall = SteamUserStats( )->DownloadLeaderboardEntries( pResult->m_hSteamLeaderboard, k_ELeaderboardDataRequestGlobal, mRankStart, mRankStart + mToRead );
		mCallbackGetLeaderboardEntries.Set( apiCall, this, &tReadData::fOnGetLeaderboardEntriesByRank );
		mState = cStateReading;
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::tReadData::fOnFindLeaderboardByRankAround( LeaderboardFindResult_t *pResult, bool bIOFailure )
	{
		if( !pResult->m_bLeaderboardFound )
		{
			log_warning( Log::cFlagNone, "Could not find leaderboard " << mBoards[ mCurrentBoard ]->mSpec.mViewName );
			mState = cStateError;
			return;
		}

		if( bIOFailure )
		{
			log_warning( Log::cFlagNone, "I/O failure finding leaderboard " << mBoards[ mCurrentBoard ]->mSpec.mViewName );
			mState = cStateError;
			return;
		}

		tLeaderboard::fAddHandle( mBoards[ mCurrentBoard ]->mSpec.mViewName, pResult->m_hSteamLeaderboard );
		log_line( Log::cFlagNone, __FUNCTION__ << " Found leaderboard " << mBoards[ mCurrentBoard ]->mSpec.mViewName << ". Beginning download of entries." );
		auto apiCall = SteamUserStats( )->DownloadLeaderboardEntries( pResult->m_hSteamLeaderboard, k_ELeaderboardDataRequestGlobalAroundUser, -( s32 )mToRead, mToRead );
		mCallbackGetLeaderboardEntries.Set( apiCall, this, &tReadData::fOnGetLeaderboardEntriesByRankAround );
		mState = cStateReading;
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::tReadData::fOnFindLeaderboardByFriends( LeaderboardFindResult_t *pResult, bool bIOFailure )
	{
		if( !pResult->m_bLeaderboardFound )
		{
			log_warning( Log::cFlagNone, "Could not find leaderboard " << mBoards[ mCurrentBoard ]->mSpec.mViewName );
			mState = cStateError;
			return;
		}

		if( bIOFailure )
		{
			log_warning( Log::cFlagNone, "I/O failure finding leaderboard " << mBoards[ mCurrentBoard ]->mSpec.mViewName );
			mState = cStateError;
			return;
		}

		tLeaderboard::fAddHandle( mBoards[ mCurrentBoard ]->mSpec.mViewName, pResult->m_hSteamLeaderboard );
		log_line( Log::cFlagNone, __FUNCTION__ << " Found leaderboard " << mBoards[ mCurrentBoard ]->mSpec.mViewName << ". Beginning download of entries." );
		auto apiCall = SteamUserStats( )->DownloadLeaderboardEntries( pResult->m_hSteamLeaderboard, k_ELeaderboardDataRequestFriends, 0, 0 );
		mCallbackGetLeaderboardEntries.Set( apiCall, this, &tReadData::fOnGetLeaderboardEntriesByFriends );
		mState = cStateReading;
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::tReadData::fOnFindLeaderboardByPlatformId( LeaderboardFindResult_t *pResult, bool bIOFailure )
	{
		if( !pResult->m_bLeaderboardFound )
		{
			log_warning( Log::cFlagNone, "Could not find leaderboard " << mBoards[ mCurrentBoard ]->mSpec.mViewName );
			mState = cStateError;
			return;
		}

		if( bIOFailure )
		{
			log_warning( Log::cFlagNone, "I/O failure finding leaderboard " << mBoards[ mCurrentBoard ]->mSpec.mViewName );
			mState = cStateError;
			return;
		}

		tLeaderboard::fAddHandle( mBoards[ mCurrentBoard ]->mSpec.mViewName, pResult->m_hSteamLeaderboard );
		log_line( Log::cFlagNone, __FUNCTION__ << " Found leaderboard " << mBoards[ mCurrentBoard ]->mSpec.mViewName << ". Beginning download of entries." );
		auto apiCall = SteamUserStats( )->DownloadLeaderboardEntriesForUsers( pResult->m_hSteamLeaderboard, ( CSteamID* )mUserIds.fBegin( ), mUserIdCount );
		mCallbackGetLeaderboardEntries.Set( apiCall, this, &tReadData::fOnGetLeaderboardEntriesByPlatformId );
		mState = cStateReading;
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::tReadData::fOnGetLeaderboardEntriesByRank( LeaderboardScoresDownloaded_t *params, bool bIOFailure )
	{
		fAddEntriesToView( params );

		if( mCurrentBoard < mBoardsCount - 1 )
		{
			++mCurrentBoard;
			auto apiCall = SteamUserStats( )->FindOrCreateLeaderboard( mBoards[ mCurrentBoard ]->mSpec.mViewName, k_ELeaderboardSortMethodDescending, k_ELeaderboardDisplayTypeNumeric );
			mCallbackFindLeaderboard.Set( apiCall, this, &tReadData::fOnFindLeaderboardByRank );
		}
		else
		{
			mState = cStateDone;
		}
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::tReadData::fOnGetLeaderboardEntriesByRankAround( LeaderboardScoresDownloaded_t *params, bool bIOFailure )
	{
		fAddEntriesToView( params );

		if( mCurrentBoard < mBoardsCount - 1 )
		{
			++mCurrentBoard;
			auto apiCall = SteamUserStats( )->FindOrCreateLeaderboard( mBoards[ mCurrentBoard ]->mSpec.mViewName, k_ELeaderboardSortMethodDescending, k_ELeaderboardDisplayTypeNumeric );
			mCallbackFindLeaderboard.Set( apiCall, this, &tReadData::fOnFindLeaderboardByRankAround );
		}
		else
		{
			mState = cStateDone;
		}
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::tReadData::fOnGetLeaderboardEntriesByFriends( LeaderboardScoresDownloaded_t *params, bool bIOFailure )
	{
		fAddEntriesToView( params );

		if( mCurrentBoard < mBoardsCount - 1 )
		{
			++mCurrentBoard;
			auto apiCall = SteamUserStats( )->FindOrCreateLeaderboard( mBoards[ mCurrentBoard ]->mSpec.mViewName, k_ELeaderboardSortMethodDescending, k_ELeaderboardDisplayTypeNumeric );
			mCallbackFindLeaderboard.Set( apiCall, this, &tReadData::fOnFindLeaderboardByFriends );
		}
		else
		{
			mState = cStateDone;
		}
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::tReadData::fOnGetLeaderboardEntriesByPlatformId( LeaderboardScoresDownloaded_t *params, bool bIOFailure )
	{
		fAddEntriesToView( params );

		if( mCurrentBoard < mBoardsCount - 1 )
		{
			++mCurrentBoard;
			auto apiCall = SteamUserStats( )->FindOrCreateLeaderboard( mBoards[ mCurrentBoard ]->mSpec.mViewName, k_ELeaderboardSortMethodDescending, k_ELeaderboardDisplayTypeNumeric );
			mCallbackFindLeaderboard.Set( apiCall, this, &tReadData::fOnFindLeaderboardByPlatformId );
		}
		else
		{
			mState = cStateDone;
		}
	}

	//------------------------------------------------------------------------------
	// tLeaderboard
	//------------------------------------------------------------------------------
	tHashTable< const char*, SteamLeaderboard_t > tLeaderboard::sHandleCache;

	//------------------------------------------------------------------------------
	b32 tLeaderboard::fIsReadActive( )
	{
		fProcessCurrent( );
		return !sCurrent.fNull( );
	}

	void tLeaderboard::fResetLeaderBoards( )
	{
		log_warning_unimplemented( 0 );
	}

	//------------------------------------------------------------------------------
	SteamLeaderboard_t tLeaderboard::fGetHandle( const char* name )
	{
		auto handle = sHandleCache.fFind( name );
		if( handle )
			return *handle;
		return 0;
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::fAddHandle( const char* name, SteamLeaderboard_t handle )
	{
		auto handlePtr = sHandleCache.fFind( name );
		if( handlePtr )
			*handlePtr = handle;
		else
			sHandleCache.fInsert( name, handle );
	}

	//------------------------------------------------------------------------------
	b32 tLeaderboard::fFindResult( u32 col, u32 row, tUserData & data ) const
	{
		if( mState != cStateSuccess )
			return false;

		if( !mSelected.mBoard )
		{
			log_warning( 0, "Must select board before accessing data" );
			return false;
		}

		if( !fAquireSelectedView( ) )
			return false;

		sigassert( col < fColumnCount( ) );
		sigassert( row < fRowsAvailable( ) );

		row += mReadData.mFriendReadStart;
	
		switch( col )
		{
		case cColumnIndexRank:
			data.fSet( (s32)mSelected.mView->pRows[ row ].dwRank );
			break;
		case cColumnIndexGamerName:
			data.fSet( 
				mSelected.mView->pRows[ row ].szGamertag.fCStr( ), 
				mSelected.mView->pRows[ row ].szGamertag.fLength( ) );
			break;
		default:
			{
				col -= 2;
				sigassert( col < mSelected.mView->pRows[ row ].dwNumColumns ); // Mostly for sanity
				data.fSetPlatformSpecific( &mSelected.mView->pRows[ row ].pColumns[ col ].Value );
				break;
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------
	b32 tLeaderboard::fFindResultById( u32 columnId, u32 row, tUserData & data ) const
	{
		if( mState != cStateSuccess )
			return false;

		if( !mSelected.mBoard )
		{
			log_warning( 0, "Must select board before accessing data" );
			return false;
		}

		if( !fAquireSelectedView( ) )
			return false;

		if( row >= fRowsAvailable( ) )
		{
			log_warning( 0, "Accessing row beyond rows available" );
			return false;
		}

		row += mReadData.mFriendReadStart;

		switch( columnId )
		{
		case cColumnIdRank:
			data.fSet( (s32)mSelected.mView->pRows[ row ].dwRank );
			return true;
		case cColumnIdGamerName:
			data.fSet( 
				mSelected.mView->pRows[ row ].szGamertag.fCStr( ),
				mSelected.mView->pRows[ row ].szGamertag.fLength( ) );
			return true;
		default:
			{
				for( u32 c = 0; c < mSelected.mView->pRows[ row ].dwNumColumns; ++c )
				{
					if( mSelected.mView->pRows[ row ].pColumns[ c ].wColumnId == columnId )
					{
						data.fSetPlatformSpecific( &mSelected.mView->pRows[ row ].pColumns[ c ].Value );
						return true;
					}
				}
				
				log_warning( 0, "ColumnId " << columnId << " could not be found for selected board" );
				return false;
			}
		}
	}

	//------------------------------------------------------------------------------
	u32 tLeaderboard::fPropertyIdToColumnId( u32 propertyId )
	{
		if( mState != cStateSuccess )
			return ~0;

		if( !mSelected.mBoard )
		{
			log_warning( 0, "Must select board before accessing data" );
			return ~0;
		}

		if( !fAquireSelectedView( ) )
			return ~0;

		for( u32 c = 0; c < mSelected.mBoard->mSpec.dwNumColumnIds; ++c )
		{
			if( mSelected.mBoard->mColumnDescs[ c ].mPropertyId == propertyId )
				return mSelected.mBoard->mSpec.wColumnId[ c ];
		}

		return ~0;
	}

	//------------------------------------------------------------------------------
	u32 tLeaderboard::fIndexOfColumnId( u32 columnId )
	{
		if( mState != cStateSuccess )
			return ~0;

		if( !mSelected.mBoard )
		{
			log_warning( 0, "Must select board before accessing data" );
			return ~0;
		}

		if( !fAquireSelectedView( ) )
			return ~0;

		for( u32 c = 0; c < mSelected.mBoard->mSpec.dwNumColumnIds; ++c )
		{
			if( mSelected.mBoard->mSpec.wColumnId[ c ] == columnId )
				return c;
		}

		return ~0;
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::fReadByRank( u32 rankStart, u32 toRead )
	{
		sigassert( mState != cStateReading && "Cannot start read from read state" );

		// Do we have any boards set?
		if( !mBoardsToReadCount )
		{
			log_warning( 0, "No valid leaderboards set in" << __FUNCTION__ );
			return;
		}

		// Sanity on the read count
		if( toRead > cMaxRowsToRead ) // From XUserCreateStatsEnumeratorByRank docs
		{
			log_warning( 0, "Too many ranks requested: " << toRead << " when only 100 is supported" );
			return;
		}
		
		// Copy the boards into one contiguous array
		for( u32 b = 0; b < mBoardsToReadCount; ++b )
			mReadData.mBoards[ b ] = mBoardsToRead[ b ];
		mReadData.mBoardsCount = mBoardsToReadCount;

		// Ranks are 1 based
		rankStart = fMax( rankStart, 1u );

		mReadData.fReadByRank( rankStart, toRead );

		mState = cStateReading;
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::fReadByRankAround( tUser * user, u32 toRead )
	{
		sigassert( mState != cStateReading && "Cannot start read from read state" );

		// Do we have any boards set?
		if( !mBoardsToReadCount )
		{
			log_warning( 0, "No valid leaderboards set in" << __FUNCTION__ );
			return;
		}

		// Sanity on the read count
		if( toRead > cMaxRowsToRead ) // From XUserCreateStatsEnumerator* docs
		{
			log_warning( 0, "Too many ranks requested: " << toRead << " when only 100 is supported" );
			return;
		}

		// Copy the boards into one contiguous array
		for( u32 b = 0; b < mBoardsToReadCount; ++b )
			mReadData.mBoards[ b ] = mBoardsToRead[ b ];
		mReadData.mBoardsCount = mBoardsToReadCount;

		mReadData.fReadByRankAround( user, toRead );

		mState = cStateReading;
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::fReadByFriends( tUser * user, u32 friendStart )
	{
		sigassert( mState != cStateReading && "Cannot start read from read state" );
		sigassert( user && "Null users are not allowed" );

		if( !user->fIsLocal( ) )
		{
			log_warning( 0, "Friend Leaderboards can only be queried for local users" );
			return;
		}

		tPlatformUserId userId = user->fPlatformId( );

		// Valid user?
		if( userId == tUser::cInvalidUserId )
		{
			log_warning( 0, "Invalid user passed to " << __FUNCTION__ );
			return;
		}

		// Do we have a board?
		if( !mBoardsToReadCount )
		{
			log_warning( 0, "No valid leaderboards set in " << __FUNCTION__ );
			return;
		}

		// Have we already queried for this users friends?
		if( userId == mReadData.mRequestingUser && mState == cStateSuccess )
		{
			// We already have the friends buffer and board results
			mReadData.mFriendReadStart = friendStart;
			return;
		}

		// Copy the boards into one contiguous array
		for( u32 b = 0; b < mBoardsToReadCount; ++b )
			mReadData.mBoards[ b ] = mBoardsToRead[ b ];
		mReadData.mBoardsCount = mBoardsToReadCount;

		mReadData.fReadByFriends( user, friendStart );

		mState = cStateReading;
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::fReadByPlatformId( const tPlatformUserId userIds[], u32 userIdCount )
	{
		sigassert( mState != cStateReading && "Cannot start read from read state" );

		// Do we have any boards set?
		if( !mBoardsToReadCount )
		{
			log_warning( 0, "No valid leaderboards set in" << __FUNCTION__ );
			return;
		}

		if( !userIds || !userIdCount )
		{
			log_warning( 0, "No user ids passed to " << __FUNCTION__ );
			return;
		}

		// Sanity on the read count
		if( userIdCount > cMaxRowsToRead ) // From XUserReadStats
		{
			log_warning( 0, "Too many user reads requested: " << userIdCount << " when only 100 is supported" );
			return;
		}

		// Copy the boards into one contiguous array
		for( u32 b = 0; b < mBoardsToReadCount; ++b )
			mReadData.mBoards[ b ] = mBoardsToRead[ b ];
		mReadData.mBoardsCount = mBoardsToReadCount;

		mReadData.fReadByPlatformId( userIds, userIdCount );

		mState = cStateReading;
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::fProcessCurrent( )
	{
		// If the current instance has a refcount of 1, it has been orphaned
		if( sCurrent && ( sCurrent.fRefCount( ) == 1 ) )
		{
			sCurrent->fCancelRead( );
		}
	}

	//------------------------------------------------------------------------------
	b32	tLeaderboard::fAdvanceRead( )
	{
		if( mState == cStateReading )
		{
			fProcessCurrent( );

			// No current instance? Make this the current instance.
			if ( !sCurrent )
				sCurrent.fReset( this );

			// Advance the current instance
			if( sCurrent == this )
			{
				b32 success;
				if( !mReadData.fOverlapComplete( success ) )
					return false;
				mState = success ? cStateSuccess : cStateFail;
				sCurrent.fRelease( );
				log_line( Log::cFlagNone, __FUNCTION__ << " Completed leaderboard read with " << ( success ? "success" : "failure" ) );
			}
			else
			{
				return false;
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::fCancelRead( )
	{
		bool releaseCurrent = false;
		if( ( sCurrent == this ) && ( sCurrent.fRefCount( ) > 0 ) )
			releaseCurrent = true;
		if( mState != cStateReading )
		{
			if( releaseCurrent )
				sCurrent.fRelease( );

			return;
		}

		log_line( Log::cFlagNone, __FUNCTION__ << " Cancelling leaderboard read" );
		mReadData.fCancel( );
		mState = cStateNull;

		if( releaseCurrent )
		{
			sCurrent.fRelease( );
		}
	}

	//------------------------------------------------------------------------------
	tLocalizedString tLeaderboard::fBoardName( ) const
	{
		sigassert( mSelected.mBoard && "Must have board selected" );
		return tGameAppBase::fInstance( ).fLocString( tStringPtr( mSelected.mBoard->mTitleDisplayId ) );
	}

	//------------------------------------------------------------------------------
	u32 tLeaderboard::fColumnCount( ) const
	{
		if( !mSelected.mBoard )
			return 0;

		// Add 2 for the rank and gamertag
		return 2 + mSelected.mBoard->mSpec.dwNumColumnIds;
	}

	//------------------------------------------------------------------------------
	u32	tLeaderboard::fColumnWidth( u32 col ) const
	{
		sigassert( mSelected.mBoard && "Must have a board selected" );

		switch( col )
		{
		// Rank
		case cColumnIndexRank:
			return 110;

		// Gamer Name
		case cColumnIndexGamerName:
			return 250;

		// Data
		default:
			col -= 2;	
			sigassert( col < mSelected.mBoard->mSpec.dwNumColumnIds && "Column number is out of range" );
			return mSelected.mBoard->mColumnDescs[ col ].mWidth;
		}
	}

	//------------------------------------------------------------------------------
	u32	tLeaderboard::fColumnAlignment( u32 col ) const
	{
		sigassert( mSelected.mBoard && "Must have board selected" );

		switch( col )
		{
		// Rank
		case cColumnIndexRank:
			return Gui::tText::cAlignLeft;

		// Gamer Name
		case cColumnIndexGamerName:
			return Gui::tText::cAlignLeft;
		
		// Data
		default:
			col -= 2;
			sigassert( col < mSelected.mBoard->mSpec.dwNumColumnIds && "Column number is out of range" );
			return mSelected.mBoard->mColumnDescs[ col ].mAlignment;
		}
	}

	//------------------------------------------------------------------------------
	tLocalizedString tLeaderboard::fColumnName( u32 col ) const
	{
		sigassert( mSelected.mBoard && "Must have board selected" );

		tStringPtr stringId; 
		switch( col )
		{
		// Rank
		case cColumnIndexRank:
			stringId = tStringPtr( "X_STRINGID_RANKCOL" );
			break;

		// Gamer Name
		case cColumnIndexGamerName:
			stringId = tStringPtr( "X_STRINGID_GAMERNAMECOL" );
			break;

		// Data
		default:
			col -= 2;
			
			sigassert( col < mSelected.mBoard->mSpec.dwNumColumnIds && "Column number is out of range" );
			stringId = tStringPtr( mSelected.mBoard->mColumnDescs[ col ].mDisplayNameId );
			break;
		}

		return tGameAppBase::fInstance( ).fLocString( stringId );
	}

	//------------------------------------------------------------------------------
	u32 tLeaderboard::fColumnUserData( u32 col ) const
	{
		sigassert( mSelected.mBoard && "Must have board selected" );

		switch( col )
		{
			// Rank
		case cColumnIndexRank:
			return ~0;

			// Gamer Name
		case cColumnIndexGamerName:
			return ~0;

			// Data
		default:
			col -= 2;
			sigassert( col < mSelected.mBoard->mSpec.dwNumColumnIds && "Column number is out of range" );
			return mSelected.mBoard->mColumnDescs[ col ].mUserData;
		}
		return ~0;
	}

	//------------------------------------------------------------------------------
	u32 tLeaderboard::fRowsAvailable( ) const
	{
		if( mState != cStateSuccess )
			return 0;

		if( !mSelected.mBoard )
		{
			log_warning( 0, "Must select board before accessing data" );
			return 0;
		}

		if( !fAquireSelectedView( ) )
			return 0;

		return mSelected.mView->dwNumRows - mReadData.mFriendReadStart;
	}

	//------------------------------------------------------------------------------
	u32 tLeaderboard::fRowsTotal( ) const
	{
		if( mState != cStateSuccess )
			return 0;

		if( !mSelected.mBoard )
		{
			log_warning( 0, "Must select board before accessing data" );
			return 0;
		}

		if( !fAquireSelectedView( ) )
			return 0;

		return mSelected.mView->dwTotalViewRows;
	}

	//------------------------------------------------------------------------------
	u32 tLeaderboard::fRowRank( u32 r ) const
	{
		if( mState != cStateSuccess )
			return 0;

		if( !mSelected.mBoard )
		{
			log_warning( 0, "Must select board before accessing data" );
			return 0;
		}

		if( !fAquireSelectedView( ) )
			return 0;

		r += mReadData.mFriendReadStart; // Is zero for non friend reads
		if( r >= mSelected.mView->dwNumRows )
		{
			log_warning( 0, "Row index out of range" );
			return 0;
		}
		return mSelected.mView->pRows[ r ].dwRank;
	}

	//------------------------------------------------------------------------------
	tPlatformUserId tLeaderboard::fRowUserId( u32 r ) const
	{
		if( mState != cStateSuccess )
			return tUser::cInvalidUserId;

		if( !mSelected.mBoard )
		{
			log_warning( 0, "Must select board before accessing data" );
			return tUser::cInvalidUserId;
		}

		if( !fAquireSelectedView( ) )
			return tUser::cInvalidUserId;

		r += mReadData.mFriendReadStart; // Is zero for non friend reads
		if( r >= mSelected.mView->dwNumRows )
		{
			log_warning( 0, "Row index out of range" );
			return tPlatformUserId( );
		}
		return mSelected.mView->pRows[ r ].mUserId;
	}

	//------------------------------------------------------------------------------
	u32 tLeaderboard::fRowForUserId( tPlatformUserId userId ) const
	{
		if( mState != cStateSuccess )
			return ~0;

		if( !mSelected.mBoard )
		{
			log_warning( 0, "Must select board before accessing data" );
			return ~0;
		}

		if( !fAquireSelectedView( ) )
			return ~0;

		for( u32 r = 0; r < mSelected.mView->dwNumRows; ++r )
		{
			if( mSelected.mView->pRows[ r ].mUserId == userId )
				return r;
		}

		return ~0;
	}

	//------------------------------------------------------------------------------
	std::string tLeaderboard::fRowGamerName( u32 r ) const
	{
		if( mState != cStateSuccess )
			return "???";

		if( !mSelected.mBoard )
		{
			log_warning( 0, "Must select board before accessing data" );
			return "???";
		}

		if( !fAquireSelectedView( ) )
			return "???";
		
		r += mReadData.mFriendReadStart; // Is zero for non friend reads
		if( r >= mSelected.mView->dwNumRows )
		{
			log_warning( 0, "Row index out of range" );
			return std::string( );
		}
		return std::string( mSelected.mView->pRows[ r ].szGamertag.fCStr( ) );
	}

	//------------------------------------------------------------------------------
	b32 tLeaderboard::fAquireSelectedView( ) const
	{
		sigassert( mSelected.mBoard );
		sigassert( mState == cStateSuccess );

		if( mSelected.mView && mSelected.mView->dwViewId == mSelected.mBoard->mSpec.dwViewId )
			return true;
	
		for( u32 b = 0; b < mReadData.mViewsCount; ++b )
		{
			if( mReadData.mViews[ b ].dwViewId == mSelected.mBoard->mSpec.dwViewId )
			{
				mSelected.mView = &mReadData.mViews[ b ];
				return true;
			}
		}

		log_warning( 0, "Selected board could not be found in results" );
		return false;
	}

#else

	//------------------------------------------------------------------------------
	// tLeaderboard
	//------------------------------------------------------------------------------
	void tLeaderboard::fResetLeaderBoards( )
	{
		log_warning_unimplemented( 0 );
	}

	//------------------------------------------------------------------------------
	b32 tLeaderboard::fFindResult( u32 col, u32 row, tUserData & data ) const
	{
		if( mState != cStateSuccess )
			return false;

		if( !mSelected.mBoard )
		{
			log_warning( 0, "Must select board before accessing data" );
			return false;
		}


		return false;
	}

	//------------------------------------------------------------------------------
	b32 tLeaderboard::fFindResultById( u32 columnId, u32 row, tUserData & data ) const
	{
		if( mState != cStateSuccess )
			return false;

		if( !mSelected.mBoard )
		{
			log_warning( 0, "Must select board before accessing data" );
			return false;
		}

		return false;
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::fReadByRank( u32 rankStart, u32 toRead )
	{
		sigassert( mState != cStateReading && "Cannot start read from read state" );

		// Do we have any boards set?
		if( !mBoardsToReadCount )
		{
			log_warning( 0, "No valid leaderboards set in" << __FUNCTION__ );
			return;
		}

		// Sanity on the read count
		if( toRead > cMaxRowsToRead ) // From XUserCreateStatsEnumeratorByRank docs
		{
			log_warning( 0, "Too many ranks requested: " << toRead << " when only 100 is supported" );
			return;
		}
		


		mState = cStateReading;
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::fReadByRankAround( tUser * user, u32 toRead )
	{
		sigassert( mState != cStateReading && "Cannot start read from read state" );

		// Do we have any boards set?
		if( !mBoardsToReadCount )
		{
			log_warning( 0, "No valid leaderboards set in" << __FUNCTION__ );
			return;
		}

		// Sanity on the read count
		if( toRead > cMaxRowsToRead ) // From XUserCreateStatsEnumerator* docs
		{
			log_warning( 0, "Too many ranks requested: " << toRead << " when only 100 is supported" );
			return;
		}



		mState = cStateReading;
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::fReadByFriends( tUser * user, u32 friendStart )
	{
		sigassert( mState != cStateReading && "Cannot start read from read state" );
		sigassert( user && "Null users are not allowed" );

		if( !user->fIsLocal( ) )
		{
			log_warning( 0, "Friend Leaderboards can only be queried for local users" );
			return;
		}

		tPlatformUserId userId = user->fPlatformId( );

		// Valid user?
		if( userId == tUser::cInvalidUserId )
		{
			log_warning( 0, "Invalid user passed to " << __FUNCTION__ );
			return;
		}

		// Do we have a board?
		if( !mBoardsToReadCount )
		{
			log_warning( 0, "No valid leaderboards set in " << __FUNCTION__ );
			return;
		}



		mState = cStateReading;
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::fReadByPlatformId( const tPlatformUserId userIds[], u32 userIdCount )
	{
		sigassert( mState != cStateReading && "Cannot start read from read state" );

		// Do we have any boards set?
		if( !mBoardsToReadCount )
		{
			log_warning( 0, "No valid leaderboards set in" << __FUNCTION__ );
			return;
		}

		if( !userIds || !userIdCount )
		{
			log_warning( 0, "No user ids passed to " << __FUNCTION__ );
			return;
		}

		// Sanity on the read count
		if( userIdCount > cMaxRowsToRead ) // From XUserReadStats
		{
			log_warning( 0, "Too many user reads requested: " << userIdCount << " when only 100 is supported" );
			return;
		}


		mState = cStateReading;
	}

	//------------------------------------------------------------------------------
	b32	tLeaderboard::fAdvanceRead( )
	{
		return true;
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::fCancelRead( )
	{
		if( mState != cStateReading )
			return;

		mState = cStateNull;
	}

	//------------------------------------------------------------------------------
	tLocalizedString tLeaderboard::fBoardName( ) const
	{
		sigassert( mSelected.mBoard && "Must have board selected" );
		return tGameAppBase::fInstance( ).fLocString( tStringPtr( mSelected.mBoard->mTitleDisplayId ) );
	}

	//------------------------------------------------------------------------------
	u32 tLeaderboard::fColumnCount( ) const
	{
		if( !mSelected.mBoard )
			return 0;

		return 2 + mSelected.mBoard->mSpec.dwNumColumnIds;
	}

	//------------------------------------------------------------------------------
	u32	tLeaderboard::fColumnWidth( u32 col ) const
	{
		sigassert( mSelected.mBoard && "Must have a board selected" );

		switch( col )
		{
		// Rank
		case cColumnIndexRank:
			return 110;

		// Gamer Name
		case cColumnIndexGamerName:
			return 250;

		// Data
		default:
			col -= 2;	
			sigassert( col < mSelected.mBoard->mSpec.dwNumColumnIds && "Column number is out of range" );
			return mSelected.mBoard->mColumnDescs[ col ].mWidth;
		}
	}

	//------------------------------------------------------------------------------
	u32	tLeaderboard::fColumnAlignment( u32 col ) const
	{
		sigassert( mSelected.mBoard && "Must have board selected" );

		switch( col )
		{
		// Rank
		case cColumnIndexRank:
			return Gui::tText::cAlignLeft;

		// Gamer Name
		case cColumnIndexGamerName:
			return Gui::tText::cAlignLeft;
		
		// Data
		default:
			col -= 2;
			sigassert( col < mSelected.mBoard->mSpec.dwNumColumnIds && "Column number is out of range" );
			return mSelected.mBoard->mColumnDescs[ col ].mAlignment;
		}
	}

	//------------------------------------------------------------------------------
	tLocalizedString tLeaderboard::fColumnName( u32 col ) const
	{
		sigassert( mSelected.mBoard && "Must have board selected" );

		tStringPtr stringId; 
		switch( col )
		{
		// Rank
		case cColumnIndexRank:
			stringId = tStringPtr( "X_STRINGID_RANKCOL" );
			break;

		// Gamer Name
		case cColumnIndexGamerName:
			stringId = tStringPtr( "X_STRINGID_GAMERNAMECOL" );
			break;

		// Data
		default:
			col -= 2;
			
			sigassert( col < mSelected.mBoard->mSpec.dwNumColumnIds && "Column number is out of range" );
			stringId = tStringPtr( mSelected.mBoard->mColumnDescs[ col ].mDisplayNameId );
			break;
		}

		return tGameAppBase::fInstance( ).fLocString( stringId );
	}

	//------------------------------------------------------------------------------
	u32 tLeaderboard::fColumnUserData( u32 col ) const
	{
		sigassert( mSelected.mBoard && "Must have board selected" );

		switch( col )
		{
			// Rank
		case cColumnIndexRank:
			return ~0;

			// Gamer Name
		case cColumnIndexGamerName:
			return ~0;

			// Data
		default:
			col -= 2;
			sigassert( col < mSelected.mBoard->mSpec.dwNumColumnIds && "Column number is out of range" );
			return mSelected.mBoard->mColumnDescs[ col ].mUserData;
		}
		return ~0;
	}

	//------------------------------------------------------------------------------
	u32 tLeaderboard::fRowsAvailable( ) const
	{
		return 0;
	}

	//------------------------------------------------------------------------------
	u32 tLeaderboard::fRowsTotal( ) const
	{
		return 0;
	}

	//------------------------------------------------------------------------------
	u32 tLeaderboard::fRowRank( u32 r ) const
	{
		return 0;
	}

	//------------------------------------------------------------------------------
	tPlatformUserId tLeaderboard::fRowUserId( u32 r ) const
	{
		return tUser::cInvalidUserId;
	}

	//------------------------------------------------------------------------------
	u32 tLeaderboard::fRowForUserId( tPlatformUserId userId ) const
	{
		return ~0;
	}

	//------------------------------------------------------------------------------
	std::string tLeaderboard::fRowGamerName( u32 r ) const
	{
		return std::string( "???" );
	}

#endif
}
#endif//#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
