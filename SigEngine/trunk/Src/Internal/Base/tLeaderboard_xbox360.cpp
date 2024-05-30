#include "BasePch.hpp"
#if defined( platform_xbox360 )
#include "tLeaderboard.hpp"
#include "Scripts/tScriptFile.hpp"

namespace Sig 
{
#ifdef sig_devmenu
	namespace
	{
		void fResetAllLeaderBoards( tDevCallback::tArgs& args )
		{
			tLeaderboard::fResetLeaderBoards( );
		}
		devcb( AAACheats_Leaderboard_ResetAllLeaderboards, "ResetAll", make_delegate_cfn( tDevCallback::tFunction, fResetAllLeaderBoards ) );
	}
#endif//sig_devmenu

	//------------------------------------------------------------------------------
	// tLeaderboardDesc
	//------------------------------------------------------------------------------
	u32 tLeaderboardDesc::fBoardId( ) const
	{
		return mSpec.dwViewId;
	}

	//------------------------------------------------------------------------------
	// tReadData
	//------------------------------------------------------------------------------
	tLeaderboard::tReadData::tReadData( )
		: mEnumerator( INVALID_HANDLE_VALUE )
		, mRequestingUser( tUser::cInvalidUserId )
	{
		fZeroOut( mOverlapped );
	}

	//------------------------------------------------------------------------------
	tLeaderboard::tReadData::~tReadData( )
	{
		sigcheckfail_xoverlapped_done_else_wait_cancel( &mOverlapped );
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::tReadData::fCloseEnumerator( )
	{
		if( mEnumerator != INVALID_HANDLE_VALUE )
		{
			XCloseHandle( mEnumerator );
			mEnumerator = INVALID_HANDLE_VALUE;
		}
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::tReadData::fResetOverlapped( )
	{
		fZeroOut( mOverlapped );
	}

	//------------------------------------------------------------------------------
	b32 tLeaderboard::tReadData::fStatsOverlapComplete( b32 & success )
	{
		DWORD ignored;
		DWORD testResult = XGetOverlappedResult( &mOverlapped, &ignored, FALSE );
		if( testResult == ERROR_IO_INCOMPLETE )
			return false;

		success = ( testResult == ERROR_SUCCESS );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tLeaderboard::tReadData::fFriendsOverlapComplete( b32 & success )
	{
		DWORD countIgnored;
		DWORD testResult = XGetOverlappedResult( &mOverlapped, &countIgnored, FALSE );
		if( testResult == ERROR_IO_INCOMPLETE )
			return false;

		DWORD extendedError = XGetOverlappedExtendedError( &mOverlapped );

		success = ( testResult == ERROR_SUCCESS || HRESULT_CODE( extendedError ) == ERROR_NO_MORE_FILES );
		return true;
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::fReadByRank( u32 rankStart, u32 toRead )
	{
		sigassert( mState != cStateReading && "Cannot start read from read state" );
		sigassert( mState != cStateReadingFriends && "Cannot start read from read state" );

		// Do we have any boards set?
		if( !mBoardsToReadCount )
		{
			log_warning( "No valid leaderboards set in" << __FUNCTION__ );
			return;
		}

		// Sanity on the read count
		if( toRead > cMaxRowsToRead ) // From XUserCreateStatsEnumeratorByRank docs
		{
			log_warning( "Too many ranks requested: " << toRead << " when only 100 is supported" );
			return;
		}
		
		// Ranks are 1 based
		rankStart = fMax( rankStart, 1u );

		// Copy the stats specs into one contiguous array
		for( u32 b = 0; b < mBoardsToReadCount; ++b )
			mReadData.mSpecData[ b ] = mBoardsToRead[ b ]->mSpec;

		// Create the stats enumerator
		DWORD bufferSize;
		DWORD result = XUserCreateStatsEnumeratorByRank( 
			0, rankStart, toRead, 
			mBoardsToReadCount, mReadData.mSpecData.fBegin( ), 
			&bufferSize, 
			&mReadData.mEnumerator );

		// Did we succeed?
		if( result != ERROR_SUCCESS )
		{
			log_warning( "Error " << result << " creating stats enumerator" );
			mReadData.fCloseEnumerator( );
			return;
		}

		// Allocate our buffer
		if( mReadData.mBuffer.fCount( ) < bufferSize )
			mReadData.mBuffer.fNewArray( bufferSize );

		// Enumerate results
		result = XEnumerate( 
			mReadData.mEnumerator, 
			mReadData.mBuffer.fBegin( ), 
			mReadData.mBuffer.fCount( ), 
			NULL, 
			&mReadData.mOverlapped );

		// Success?
		log_line( Log::cFlagSession, "XEnumerate" );
		if( result != ERROR_IO_PENDING && result != ERROR_SUCCESS )
		{
			log_warning( "Error " << std::hex << result << " enumerating stats" );
			mReadData.fCloseEnumerator( );
			return;
		}

		mReadData.mRequestingUser = tUser::cInvalidUserId;
		mReadData.mFriendReadStart = 0;
		mState = cStateReading;
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::fReadByRankAround( tUser * user, u32 toRead )
	{
		sigassert( mState != cStateReading && "Cannot start read from read state" );
		sigassert( mState != cStateReadingFriends && "Cannot start read from read state" );
		sigassert( user && "Null users are not allowed" );
		
		// Do we have any boards set?
		if( !mBoardsToReadCount )
		{
			log_warning( "No valid leaderboards set in" << __FUNCTION__ );
			return;
		}

		// Sanity on the read count
		if( toRead > cMaxRowsToRead ) // From XUserCreateStatsEnumerator* docs
		{
			log_warning( "Too many ranks requested: " << toRead << " when only 100 is supported" );
			return;
		}

		// Copy the stats specs into one contiguous array
		for( u32 b = 0; b < mBoardsToReadCount; ++b )
			mReadData.mSpecData[ b ] = mBoardsToRead[ b ]->mSpec;

		// Create the stats enumerator
		DWORD bufferSize;
		DWORD result = XUserCreateStatsEnumeratorByXuid( 
			0, user->fPlatformId( ), toRead, 
			mBoardsToReadCount, mReadData.mSpecData.fBegin( ), 
			&bufferSize, 
			&mReadData.mEnumerator );

		// Did we succeed?
		if( result != ERROR_SUCCESS )
		{
			log_warning( "Error " << result << " creating stats enumerator" );
			mReadData.fCloseEnumerator( );
			return;
		}

		// Allocate our buffer
		if( mReadData.mBuffer.fCount( ) < bufferSize )
			mReadData.mBuffer.fNewArray( bufferSize );

		// Enumerate results
		result = XEnumerate( 
			mReadData.mEnumerator, 
			mReadData.mBuffer.fBegin( ), 
			mReadData.mBuffer.fCount( ), 
			NULL, 
			&mReadData.mOverlapped );

		// Success?
		if( result != ERROR_IO_PENDING && result != ERROR_SUCCESS )
		{
			log_warning( "Error " << std::hex << result << " enumerating stats" );
			mReadData.fCloseEnumerator( );
			return;
		}

		mReadData.mRequestingUser = tUser::cInvalidUserId;
		mReadData.mFriendReadStart = 0;
		mState = cStateReading;
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::fReadByFriends( tUser * user, u32 friendStart )
	{
		sigassert( mState != cStateReading && "Cannot start read from read state" );
		sigassert( mState != cStateReadingFriends && "Cannot start read from read state" );
		sigassert( user && "Null users are not allowed" );

		if( !user->fIsLocal( ) )
		{
			log_warning( "Friend Leaderboards can only be queried for local users" );
			return;
		}

		tPlatformUserId userId = user->fPlatformId( );

		// Valid user?
		if( userId == tUser::cInvalidUserId )
		{
			log_warning( "Invalid user passed to " << __FUNCTION__ );
			return;
		}

		// Do we have a board?
		if( !mBoardsToReadCount )
		{
			log_warning( "No valid leaderboards set in " << __FUNCTION__ );
			return;
		}

		// Have we already queried for this users friends?
		if( userId == mReadData.mRequestingUser && mState == cStateSuccess )
		{
			// We already have the friends buffer and board results
			mReadData.mFriendReadStart = friendStart;
			return;
		}

		// Otherwise we need to read the friends

		// Create an an enumerator to retrieve our friends
		DWORD bufferSize;
		DWORD result = XFriendsCreateEnumerator( 
			user->fLocalHwIndex( ), 0, cMaxRowsToRead, &bufferSize, &mReadData.mEnumerator );

		// Success?
		if( result != ERROR_SUCCESS )
		{
			log_warning( "Error " << std::hex << result << " creating friend enumerator" );
			return;
		}

		// Allocate
		if( mReadData.mFriendBuffer.fCount( ) < bufferSize )
			mReadData.mFriendBuffer.fSetCount( bufferSize );

		// Enumerate friends
		result = XEnumerate( 
			mReadData.mEnumerator, 
			mReadData.mFriendBuffer.fBegin( ), 
			mReadData.mFriendBuffer.fCount( ), 
			NULL,
			&mReadData.mOverlapped );

		// Success?
		if( result != ERROR_IO_PENDING && result != ERROR_SUCCESS )
		{
			log_warning( "Error " << std::hex << result << " enumerating friends" );
			mReadData.fCloseEnumerator( );
			return;
		}

		mReadData.mRequestingUser = userId;
		mReadData.mFriendReadStart = friendStart;
		mState = cStateReadingFriends;
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::fReadByPlatformId( const tPlatformUserId userIds[], u32 userIdCount )
	{
		sigassert( mState != cStateReading && "Cannot start read from read state" );
		sigassert( mState != cStateReadingFriends && "Cannot start read from read state" );

		// Do we have any boards set?
		if( !mBoardsToReadCount )
		{
			log_warning( "No valid leaderboards set in" << __FUNCTION__ );
			return;
		}

		if( !userIds || !userIdCount )
		{
			log_warning( "No user ids passed to " << __FUNCTION__ );
			return;
		}

		// Sanity on the read count
		if( userIdCount > cMaxRowsToRead ) // From XUserReadStats
		{
			log_warning( "Too many user reads requested: " << userIdCount << " when only 100 is supported" );
			return;
		}

		// Copy the stats specs into one contiguous array
		for( u32 b = 0; b < mBoardsToReadCount; ++b )
			mReadData.mSpecData[ b ] = mBoardsToRead[ b ]->mSpec;

		//Query the result buffer size
		DWORD bufferSize = 0;
		DWORD result = XUserReadStats( 
			0,									// Title ID
			userIdCount, 
			userIds, 
			mBoardsToReadCount, 
			mReadData.mSpecData.fBegin( ), 
			&bufferSize, 
			NULL,								// Result buffer
			NULL								// Overlapped
		);

		log_line( Log::cFlagSession, "XUserReadStats, Xuids: " << userIdCount + 1 << " Boards: " << mBoardsToReadCount << " Func: " __FUNCTION__ );
		sigassert( result == ERROR_INSUFFICIENT_BUFFER && "Unknown error getting XUserReadStats buffer size" );
		
		const u32 totalBufferSize = bufferSize + ( sizeof( tPlatformUserId ) * userIdCount );

		// Allocate our buffer
		if( mReadData.mBuffer.fCount( ) < totalBufferSize )
			mReadData.mBuffer.fNewArray( totalBufferSize );

		// Copy the user ids to the read buffer end
		tPlatformUserId * permUserIds = (tPlatformUserId *)( mReadData.mBuffer.fBegin( ) + bufferSize );
		fMemCpy( permUserIds, userIds, sizeof( tPlatformUserId ) * userIdCount );

		// Read the stats
		result = XUserReadStats( 
			0,	// TitleId
			userIdCount, 
			permUserIds, 
			mBoardsToReadCount, 
			mReadData.mSpecData.fBegin( ), 
			&bufferSize, 
			(XUSER_STATS_READ_RESULTS*)mReadData.mBuffer.fBegin( ), 
			&mReadData.mOverlapped );

		if( result != ERROR_IO_PENDING && result != ERROR_SUCCESS )
		{
			log_warning( "Error " << std::hex << result << " reading user stats" );
			return;
		}

		log_line( Log::cFlagSession, "XUserReadStats 2, Xuids: " << userIdCount + 1 << " Boards: " << mBoardsToReadCount << " Func: " __FUNCTION__ );
		mReadData.mRequestingUser = userIds[ 0 ]; // Any valid entry is fine
		mReadData.mFriendReadStart = 0;
		mState = cStateReading;
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::fOnFriendsRead( )
	{
		sigassert( !( mReadData.mFriendBuffer.fCount( ) % sizeof( XONLINE_FRIEND ) ) ); // sanity
		const DWORD totalFriends = ( mReadData.mFriendBuffer.fCount( ) / sizeof( XONLINE_FRIEND ) );

		// Determine the number of real non-request friends.
		tGrowableArray<u32> actualFriends;
		{
			const XONLINE_FRIEND * friends = ( const XONLINE_FRIEND * )mReadData.mFriendBuffer.fBegin( );
			
			for( u32 f = 0; f < totalFriends; ++f )
			{
				if( friends[f].dwFriendState & XONLINE_FRIENDSTATE_FLAG_SENTREQUEST
					|| friends[f].dwFriendState & XONLINE_FRIENDSTATE_FLAG_RECEIVEDREQUEST )
					continue;

				actualFriends.fPushBack( f );
			}
		}

		// Copy the stats specs into one contiguous array
		for( u32 b = 0; b < mBoardsToReadCount; ++b )
			mReadData.mSpecData[ b ] = mBoardsToRead[ b ]->mSpec;

		// Get the necessary buffer size
		// NOTE: We don't have an XUID to pass in, so we're just passing 
		// in (XUID*)1 which isn't used in the size call
		DWORD bufferSize = 0;
		DWORD result = XUserReadStats( 
			0,								// Title ID
			actualFriends.fCount() + 1,		// Num XUIDS
			(XUID *)1,						// XUIDs
			mBoardsToReadCount,				// Boards
			mReadData.mSpecData.fBegin( ),	// Specs
			&bufferSize,					// Result buffer size
			NULL, NULL );
		log_line( Log::cFlagSession, "XUserReadStats, Xuids: " << actualFriends.fCount() + 1 << " Boards: " << mBoardsToReadCount << " Func: " __FUNCTION__ );
		sigassert( result == ERROR_INSUFFICIENT_BUFFER && "Unknown error getting XUserReadStats buffer size" );

		// Allocate a big enough buffer
		const u32 totalBufferSize = bufferSize + ( actualFriends.fCount() + 1 ) * sizeof( tPlatformUserId );
		mReadData.mBuffer.fNewArray( totalBufferSize );

		tPlatformUserId * ids = ( tPlatformUserId * )( mReadData.mBuffer.fBegin( ) + bufferSize );

		// Copy all the ids into the buffer and free the friend buffer
		{
			tPlatformUserId * currId = ids;

			// Copy the requesting user
			*currId++ = mReadData.mRequestingUser;

			// Copy the friends
			const XONLINE_FRIEND * friends = ( const XONLINE_FRIEND * )mReadData.mFriendBuffer.fBegin( );
			for( u32 f = 0; f < actualFriends.fCount(); ++f )
			{
				const u32 friendToGrab = actualFriends[f];
				*currId++ = friends[ friendToGrab ].xuid;
			}
		}

		// Read the stats
		result = XUserReadStats( 
			0,								// TitleID
			actualFriends.fCount() + 1,		// Num XUIDS
			ids,							// XUIDS
			mBoardsToReadCount,				// Board Count
			mReadData.mSpecData.fBegin( ),	// Specs
			&bufferSize,					// Size of buffer
			( XUSER_STATS_READ_RESULTS * )mReadData.mBuffer.fBegin( ), // Buffer
			&mReadData.mOverlapped );		// Overlapped
		log_line( Log::cFlagSession, "XUserReadStats 2, Xuids: " << actualFriends.fCount() + 1 << " Boards: " << mBoardsToReadCount << " Func: " __FUNCTION__ );

		// Success?
		if( result != ERROR_IO_PENDING && result != ERROR_SUCCESS )
		{
			log_warning( "Error " << std::hex << result << " reading user stats" );
			mReadData.fCloseEnumerator( );
			return;
		}

		mState = cStateReading;
	}

	//------------------------------------------------------------------------------
	static bool fUserStatsRowCompare( const XUSER_STATS_ROW & a, const XUSER_STATS_ROW & b )
	{
		if( a.dwRank != 0 && b.dwRank == 0 )
			return true;
		else if( a.dwRank == 0 && b.dwRank != 0 )
			return false;
		else if( a.dwRank == 0 && b.dwRank == 0 )
			return false;

		return a.dwRank <= b.dwRank;
	}

	//------------------------------------------------------------------------------
	b32 tLeaderboard::fAdvanceRead( )
	{
		if( mState == cStateReading )
		{
			b32 success;
			if( !mReadData.fStatsOverlapComplete( success ) )
				return false;

			mReadData.fResetOverlapped( );
			mReadData.fCloseEnumerator( );
			if( success )
			{
				mState = cStateSuccess;

				// If this was a request by XUID then the requesting user id will be valid
				// and we need to resort the results by rank
				if( mReadData.mRequestingUser != tUser::cInvalidUserId )
				{
					const XUSER_STATS_READ_RESULTS * results = ( XUSER_STATS_READ_RESULTS * )mReadData.mBuffer.fBegin( );
					for( u32 b = 0; b < results->dwNumViews; ++b )
					{
						std::sort( 
							results->pViews[ b ].pRows, 
							results->pViews[ b ].pRows + results->pViews[ b ].dwNumRows, 
							fUserStatsRowCompare );

						// Don't include friends who are not ranked, unless explicitly specified
						if( !mRetainZeroRanks )
						{
							while( results->pViews[ b ].dwNumRows && !results->pViews[ b ].pRows[ results->pViews[ b ].dwNumRows - 1 ].dwRank )
								--results->pViews[ b ].dwNumRows;
						}

						// We query for all friends or an explicit list
						results->pViews[ b ].dwTotalViewRows = results->pViews[ b ].dwNumRows;
					}
				}
			}
			else
				mState = cStateFail;
		}
		else if( mState == cStateReadingFriends )
		{
			b32 success;
			if( !mReadData.fFriendsOverlapComplete( success ) )
				return false;

			u32 friendsRead = mReadData.mOverlapped.InternalHigh;
			mReadData.fCloseEnumerator( );
			mReadData.fResetOverlapped( );

			// If we've read the friends, then advance to reading the board
			if( success )
			{
				// Shrink to correct size for friends found
				mReadData.mFriendBuffer.fSetCount( friendsRead * sizeof( XONLINE_FRIEND ) );
				fOnFriendsRead( );
			}
			
			// If we made it to the reading state then pretend we didn't advance
			if( mState == cStateReading )
				return false;

			// Otherwise report that we've failed
			mReadData.mRequestingUser = tUser::cInvalidUserId;
			mReadData.fResetOverlapped( );
			mState = cStateFail;
		}

		return true;
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::fCancelRead( )
	{
		if( mState != cStateReading && mState != cStateReadingFriends )
			return;

		XCancelOverlapped( &mReadData.mOverlapped );
		
		mReadData.fCloseEnumerator( );
		fZeroOut( mReadData.mOverlapped );
		mState = cStateNull;
	}

	//------------------------------------------------------------------------------
	tLocalizedString tLeaderboard::fBoardName( ) const
	{
		sigassert( mSelected.mBoard && "Must have board selected" );
	
		DWORD length = XCONTENT_MAX_DISPLAYNAME_LENGTH;
		wchar_t str[ XCONTENT_MAX_DISPLAYNAME_LENGTH ];

		XResourceGetString( mSelected.mBoard->mTitleDisplayId, str, &length, NULL );

		return tLocalizedString( str );
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

		DWORD stringId; 
		switch( col )
		{
		// Rank
		case cColumnIndexRank:
			stringId = X_STRINGID_RANKCOL;
			break;

		// Gamer Name
		case cColumnIndexGamerName:
			stringId = X_STRINGID_GAMERNAMECOL;
			break;

		// Data
		default:
			col -= 2;
			
			sigassert( col < mSelected.mBoard->mSpec.dwNumColumnIds && "Column number is out of range" );
			stringId = mSelected.mBoard->mColumnDescs[ col ].mDisplayNameId;
			break;
		}

		DWORD length = XCONTENT_MAX_DISPLAYNAME_LENGTH;
		wchar_t str[ XCONTENT_MAX_DISPLAYNAME_LENGTH ];

		XResourceGetString( stringId, str, &length, NULL );

		return tLocalizedString( str );
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
			break;

			// Gamer Name
		case cColumnIndexGamerName:
			return ~0;
			break;

			// Data
		default:
			col -= 2;
			sigassert( col < mSelected.mBoard->mSpec.dwNumColumnIds && "Column number is out of range" );
			return mSelected.mBoard->mColumnDescs[ col ].mUserData;
			break;
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
			log_warning( "Must select board before accessing data" );
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
			log_warning( "Must select board before accessing data" );
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
			log_warning( "Must select board before accessing data" );
			return 0;
		}

		if( !fAquireSelectedView( ) )
			return 0;

		r += mReadData.mFriendReadStart; // Is zero for non friend reads
		sigassert( r < mSelected.mView->dwNumRows && "Row index out of range" );
		return mSelected.mView->pRows[ r ].dwRank;
	}

	//------------------------------------------------------------------------------
	tPlatformUserId tLeaderboard::fRowUserId( u32 r ) const
	{
		if( mState != cStateSuccess )
			return tUser::cInvalidUserId;

		if( !mSelected.mBoard )
		{
			log_warning( "Must select board before accessing data" );
			return tUser::cInvalidUserId;
		}

		if( !fAquireSelectedView( ) )
			return tUser::cInvalidUserId;

		r += mReadData.mFriendReadStart; // Is zero for non friend reads
		sigassert( r < mSelected.mView->dwNumRows && "Row index out of range" );
		return mSelected.mView->pRows[ r ].xuid;
	}

	//------------------------------------------------------------------------------
	std::string tLeaderboard::fRowGamerName( u32 r ) const
	{
		if( mState != cStateSuccess )
			return "???";

		if( !mSelected.mBoard )
		{
			log_warning( "Must select board before accessing data" );
			return "???";
		}

		if( !fAquireSelectedView( ) )
			return "???";
		
		r += mReadData.mFriendReadStart; // Is zero for non friend reads
		sigassert( r < mSelected.mView->dwNumRows && "Row index out of range" );
		return mSelected.mView->pRows[ r ].szGamertag;
	}

	//------------------------------------------------------------------------------
	u32 tLeaderboard::fRowForUserId( tPlatformUserId userId ) const
	{
		if( mState != cStateSuccess )
			return ~0;

		if( !mSelected.mBoard )
		{
			log_warning( "Must select board before accessing data" );
			return ~0;
		}

		if( !fAquireSelectedView( ) )
			return ~0;

		for( u32 r = 0; r < mSelected.mView->dwNumRows; ++r )
		{
			if( mSelected.mView->pRows[ r ].xuid == userId )
				return r;
		}

		return ~0;
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::fResetLeaderBoards( )
	{
		const u32 count = mLeaderboards.fCount( );
		for( u32 l = 0; l < count; ++l )
		{
			XUserResetStatsViewAllUsers( mLeaderboards[ l ].mSpec.dwViewId, NULL );
		}
	}

	//------------------------------------------------------------------------------
	b32 tLeaderboard::fFindResult( u32 col, u32 row, tUserData & data ) const
	{

		if( mState != cStateSuccess )
			return false;

		if( !mSelected.mBoard )
		{
			log_warning( "Must select board before accessing data" );
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
			data.fSetBinary( 
				(const void*)mSelected.mView->pRows[ row ].szGamertag, 
				fNullTerminatedLength( mSelected.mView->pRows[ row ].szGamertag ) );
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
			log_warning( "Must select board before accessing data" );
			return false;
		}

		if( !fAquireSelectedView( ) )
			return false;

		sigassert( row < fRowsAvailable( ) );

		row += mReadData.mFriendReadStart;

		switch( columnId )
		{
		case cColumnIdRank:
			data.fSet( (s32)mSelected.mView->pRows[ row ].dwRank );
			return true;
		case cColumnIdGamerName:
			data.fSetBinary( 
				(const void*)mSelected.mView->pRows[ row ].szGamertag,
				fNullTerminatedLength( mSelected.mView->pRows[ row ].szGamertag ) );
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
				
				log_warning( "ColumnId " << columnId << " could not be found for selected board" );
				return false;
			}
		}
	}

	//------------------------------------------------------------------------------
	b32 tLeaderboard::fAquireSelectedView( ) const
	{
		sigassert( mSelected.mBoard );
		sigassert( mState == cStateSuccess );

		if( mSelected.mView )
			return true;
	
		const XUSER_STATS_READ_RESULTS * results = ( XUSER_STATS_READ_RESULTS * )mReadData.mBuffer.fBegin( );
		for( u32 b = 0; b < results->dwNumViews; ++b )
		{
			if( results->pViews[ b ].dwViewId == mSelected.mBoard->mSpec.dwViewId )
			{
				mSelected.mView = &results->pViews[ b ];
				return true;
			}
		}

		log_warning( "Selected board could not be found in results" );
		return false;
	}
}
#endif//#if defined( platform_xbox360 )
