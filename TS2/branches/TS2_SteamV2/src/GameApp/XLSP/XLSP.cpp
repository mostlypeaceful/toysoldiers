#include "GameAppPch.hpp"
#include "xlsp.h"
#include "stubs.h"
#include "XlspManager.h"
#include "Gui/tText.hpp"
#include "tGameApp.hpp"


using namespace XLSP;


#ifdef platform_xbox360
#define using_xlsp
#endif

namespace Sig
{

#ifdef using_xlsp
	void tXLSP::fInitialize( )
	{
		g_XlspManager.Initialize( );
	}

	void tXLSP::fSetFrontEndPlayer( const tUserPtr& user )
	{
		g_LocalPlayer.mUser = user;
	}

	void tXLSP::fUpdate( f32 dt )
	{
		g_XlspManager.Update( dt );
	}

	void tXLSP::fWriteTestValues( tUser* user )
	{
		sigassert( user );
		g_XlspManager.RequestPostScore(user->fPlatformId( ), cMachineID_Fly, 1, XlspManager::PlatformMobile);
		g_XlspManager.RequestPostScore(user->fPlatformId( ), cMachineID_Hallway, 2, XlspManager::PlatformMobile);
		g_XlspManager.RequestPostScore(user->fPlatformId( ), cMachineID_TrialMG2, 3, XlspManager::PlatformMobile);

	}

	void tXLSP::fWriteTestOwnership( tUser* user )
	{
		sigassert( user );
		g_XlspManager.RequestSetOwnershipRecord( user->fPlatformId( ), cTitleID_MobileOwnership, cContentID_MobileOwnership, XlspManager::PlatformMobile );
	}



	void tXLSPLeaderboard::fRequestFriendHighScores( u32 leaderboardMachine, u32 numResults )
	{
		sigassert( !( mFriendBuffer.fCount( ) % sizeof( XONLINE_FRIEND ) ) ); // sanity
		const DWORD friendsFound = ( mFriendBuffer.fCount( ) / sizeof( XONLINE_FRIEND ) );

		std::list<XUID> friendsIDs;

		const XONLINE_FRIEND * friends = ( const XONLINE_FRIEND * )mFriendBuffer.fBegin( );
		for( u32 f = 0; f < friendsFound; ++f )
			friendsIDs.push_back( friends[ f ].xuid );

		friendsIDs.push_back( mRequestingUser ); // add ourselves to the array
		g_XlspManager.RequestGetScoresForXuids( mRequestingUser, leaderboardMachine, XlspManager::PlatformMobile, friendsIDs );

	}

	b32 tXLSPLeaderboard::fCheckReceivedFriendHighScores( )
	{
		u32 status = g_XlspManager.GetScoresForXuids( mScores );
		if (status == XlspManager::LspStatusUpdated)
		{
			mFriendRequestSuccess = true;
			return true;
		}
		else if (status == XlspManager::LspStatusFailure)
		{
			log_warning( 0, "===== GetScoresForXuids Failed =====");
			return true;
		}

		return false;
	}

	void tXLSPLeaderboard::fRequestMobileHighScores( u32 leaderboardMachine, u32 numResults )
	{
		g_XlspManager.RequestTopScoresPlatform( leaderboardMachine, XlspManager::PlatformMobile, numResults );
	}

	b32 tXLSPLeaderboard::fCheckReceivedHighScores( )
	{
		u32 status = g_XlspManager.GetTopScoresPlatform( mScores );
		if (status == XlspManager::LspStatusUpdated)
		{
			return true;
		}
		else if (status == XlspManager::LspStatusFailure)
		{
			log_warning( 0, "===== GetTopScoresPlatform Failed =====");
			return true;
		}

		return false;
	}

	void tXLSPLeaderboard::fRequestMobileHighScoresAround( u32 leaderboardMachine, tUser* user, u32 numResults )
	{
		g_XlspManager.RequestGetScoresAroundScore(user->fPlatformId( ), leaderboardMachine, numResults, numResults);
	}

	b32 tXLSPLeaderboard::fCheckReceivedHighScoresAround( )
	{
		u32 status = g_XlspManager.GetScoresAroundScore( mScores );
		if (status == XlspManager::LspStatusUpdated)
		{
			// sort scores
			std::sort( mScores.fBegin( ), mScores.fEnd( ) );

			// we have to find our rank and assign other ranks relative to that
			u32 myIndex = 0;
			for( myIndex = 0; myIndex < mScores.fCount( ); ++myIndex )
				if( mScores[ myIndex ].xuid == mRequestingUser )
					break;

			if( myIndex < mScores.fCount( ) )
			{
				s32 myRank = mScores[ myIndex ].rank;
				s32 firstRank = myRank - myIndex;
				for( u32 i = 0; i < myIndex; ++i )
					mScores[ i ].rank = firstRank + i;
				for( u32 i = myIndex + 1; i < mScores.fCount( ); ++i )
					mScores[ i ].rank = myRank + 1 + i;
			}

			return true;
		}
		else if (status == XlspManager::LspStatusFailure)
		{
			log_warning( 0, "===== GetScoresAroundScore Failed =====");
			return true;
		}

		return false;
	}

	b32 tXLSPLeaderboard::fCheckReceivedOwnership( )
	{
		mHasOwnerShip = false;

		std::list< XLSP::XlspManager::OwnershipRecord > records;
		u32 status = g_XlspManager.GetOwnershipRecords( records );
		if (status == XlspManager::LspStatusUpdated)
		{
			mHasOwnerShip = records.size( ) != 0;
			return true;
		}
		else if (status == XlspManager::LspStatusFailure)
		{
			log_warning( 0, "===== GetScoresAroundScore Failed =====");
			return true;
		}

		return false;
	}

	void tXLSPLeaderboard::fRequestMobileOwnership( tUser& user )
	{
		mMode = cModeRequestingOwnership;
		mRequestingUserPtr = &user;
	}

	void tXLSPLeaderboard::fAddBoardToRead( u32 board ) 
	{ 
		mBoardID = board;

		switch( board )
		{
		case GameFlags::cMOBILE_MINIGAME_FLY:
			mBoardID = tXLSP::cMachineID_Fly;
			break;
		case GameFlags::cMOBILE_MINIGAME_HALLWAY:
			mBoardID = tXLSP::cMachineID_Hallway;
			break;
		case GameFlags::cMOBILE_MINIGAME_TRIAL_GAME_2:
			mBoardID = tXLSP::cMachineID_TrialMG2;
			break;
		case GameFlags::cMOBILE_MINIGAME_TOTALS:
			mBoardID = tXLSP::cMachineID_Totals;
			break;
		default:
			log_warning( 0, "XLSP leaderboard not recognized: " << board );
		}
	}

	void tXLSPLeaderboard::fReadByFriends( tUser* user, u32 start ) 
	{ 
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
		if( mBoardID == ~0 )
		{
			log_warning( 0, "No valid leaderboards set in " << __FUNCTION__ );
			return;
		}

		// Have we already queried for this users friends?
		if( userId == mRequestingUser && mFriendRequestSuccess )
		{
			// We already have the friends buffer and board results
			mFriendReadStart = start;
			return;
		}

		// Otherwise we need to read the friends

		// Create an an enumerator to retrieve our friends
		DWORD bufferSize;

		const u32 cMaxRowsToRead = 10;
		DWORD result = XFriendsCreateEnumerator( user->fLocalHwIndex( ), 0, cMaxRowsToRead, &bufferSize, &mEnumerator );

		// Success?
		if( result != ERROR_SUCCESS )
		{
			log_warning( 0, "Error " << std::hex << result << " creating friend enumerator" );
			return;
		}

		// Allocate
		if( mFriendBuffer.fCount( ) < bufferSize )
			mFriendBuffer.fSetCount( bufferSize );

		// Enumerate friends
		result = XEnumerate( 
			mEnumerator, 
			mFriendBuffer.fBegin( ), 
			mFriendBuffer.fCount( ), 
			NULL,
			&mOverlapped );

		// Success?
		if( result != ERROR_IO_PENDING && result != ERROR_SUCCESS )
		{
			log_warning( 0, "Error " << std::hex << result << " enumerating friends" );
			fCloseEnumerator( );
			return;
		}

		mRequestingUser = userId;
		mFriendReadStart = 0;

		mMode = cModeRequestingFriends;
		mReadSize = 100;

		mFriendRequestSuccess = false;
		mRequestingUserPtr = user;
	}

	void tXLSPLeaderboard::fReadByRank( u32 start, u32 amount ) 
	{ 
		mMode = cModeRequestingRank;
		mReadSize = amount;
		mRequestingUserPtr = NULL;
	}

	void tXLSPLeaderboard::fReadByRankAround( tUser * user, u32 toRead ) 
	{ 
		mMode = cModeRequestingRankAround;
		mReadSize = toRead;
		mRequestingUserPtr = user;
		sigassert( user );
		mRequestingUser = user->fPlatformId( );
	}

	b32  tXLSPLeaderboard::fAdvanceRead( ) 
	{ 
		if( mMode == cModeIdle )
			return true;
		else if( mMode == cModeWaitingResult )
		{
			b32 result = false;

			switch( mRequestedMode )
			{
			case cModeRequestingFriends: 
				result = fCheckReceivedFriendHighScores( );
				break;
			case cModeRequestingRank: 
				result = fCheckReceivedHighScores( );
				break;
			case cModeRequestingRankAround: 
				result = fCheckReceivedHighScoresAround( );
				break;
			case cModeRequestingOwnership:
				result = fCheckReceivedOwnership( );
			}

			if( result )
				mMode = cModeIdle;
		}
		else
		{
			//waiting for request
			if( g_XlspManager.fReady( ) )
			{
				switch( mMode )
				{
					case cModeRequestingFriends:
						{
							b32 success = false;
							if( !fFriendsOverlapComplete( success ) )
								return false;

							u32 friendsRead = mOverlapped.InternalHigh;
							fCloseEnumerator( );
							fResetOverlapped( );

							// If we've read the friends, then advance to reading the board
							if( success )
							{
								// Shrink to correct size for friends found
								mFriendBuffer.fSetCount( friendsRead * sizeof( XONLINE_FRIEND ) );

								fRequestFriendHighScores( mBoardID, mReadSize );
							}
							else
							{
								log_warning( 0, "Error reqeuesting friends!" );
								mMode = cModeIdle;
								return true;
							}
						}

						break;
					case cModeRequestingRank:
						fRequestMobileHighScores( mBoardID, mReadSize );
						break;
					case cModeRequestingRankAround:
						fRequestMobileHighScoresAround( mBoardID, mRequestingUserPtr, mReadSize );
						break;
					case cModeRequestingOwnership:
						g_XlspManager.RequestGetOwnershipRecords( mRequestingUserPtr->fPlatformId( ), tXLSP::cTitleID_MobileOwnership, XlspManager::PlatformMobile );
						break;
				}

				mRequestedMode = mMode;
				mMode = cModeWaitingResult;
			}
		}

		return ( mMode == cModeIdle ); 
	}

	b32 tXLSPLeaderboard::fFriendsOverlapComplete( b32 & success )
	{
		DWORD countIgnored;
		DWORD testResult = XGetOverlappedResult( &mOverlapped, &countIgnored, FALSE );
		if( testResult == ERROR_IO_INCOMPLETE )
			return false;

		DWORD extendedError = XGetOverlappedExtendedError( &mOverlapped );

		success = ( testResult == ERROR_SUCCESS || HRESULT_CODE( extendedError ) == ERROR_NO_MORE_FILES );
		return true;
	}

	void tXLSPLeaderboard::fCloseEnumerator( )
	{
		if( mEnumerator != INVALID_HANDLE_VALUE )
		{
			XCloseHandle( mEnumerator );
			mEnumerator = INVALID_HANDLE_VALUE;
		}
	}

	void tXLSPLeaderboard::fResetOverlapped( )
	{
		fZeroOut( mOverlapped );
	}

	void tXLSPLeaderboard::fCancelRead( ) 
	{ 
		mMode = cModeIdle;
	}

	void tXLSPLeaderboard::fSelectBoard( u32 board ) 
	{
	}

	namespace
	{
		static const tStringPtr cMobileRank( "mobile_rank" );
		static const tStringPtr cMobileScore( "mobile_score" );
		static const tStringPtr cMobileTotalScore( "mobile_totalscore" );

		static const tStringPtr cMobileBoardNameFly( "mobile_board_fly" );
		static const tStringPtr cMobileBoardNameHall( "mobile_board_hall" );
		static const tStringPtr cMobileBoardNameMg2( "mobile_board_mg2" );
		static const tStringPtr cMobileBoardNameTotals( "mobile_board_totals" );
	}

	tLocalizedString tXLSPLeaderboard::fBoardName( )
	{
		switch( mBoardID )
		{
		case tXLSP::cMachineID_Fly:
			return tGameApp::fInstance( ).fLocString( cMobileBoardNameFly );
			break;
		case tXLSP::cMachineID_Hallway:
			return tGameApp::fInstance( ).fLocString( cMobileBoardNameHall );
			break;
		case tXLSP::cMachineID_TrialMG2:
			return tGameApp::fInstance( ).fLocString( cMobileBoardNameMg2 );
			break;
		case tXLSP::cMachineID_Totals:
			return tGameApp::fInstance( ).fLocString( cMobileBoardNameTotals );
			break;
		default:
			log_warning( 0, "XLSP leaderboard not recognized: " << mBoardID );
			return tLocalizedString( );
		}
	}

	u32  tXLSPLeaderboard::fColumnWidth( u32 col ) 
	{ 
		return (col == 0) ? 450 : 250; 
	}

	u32  tXLSPLeaderboard::fColumnAlignment( u32 col ) 
	{
		return col == 1 ? Gui::tText::cAlignLeft : Gui::tText::cAlignCenter; 
	}

	tLocalizedString tXLSPLeaderboard::fColumnName( u32 col ) 
	{ 
		switch( col )
		{
		case 0:
			return tGameApp::fInstance( ).fLocString( cMobileRank );
			break;
		case 2:
			return tGameApp::fInstance( ).fLocString( (mBoardID == tXLSP::cMachineID_Totals) ? cMobileTotalScore : cMobileScore );
			break;
		default:
			return tLocalizedString( );
		}
	}

	u32 tXLSPLeaderboard::fColumnUserData( u32 col ) 
	{ 
		return 0; 
	}

	u32 tXLSPLeaderboard::fRowRank( u32 row ) 
	{ 
		return mScores[ row ].rank; 
	}

	u32 tXLSPLeaderboard::fRowUserIdFromScript( u32 row ) 
	{ 
		return 0; //mScores[ row ].xuid; 
	}

	std::string tXLSPLeaderboard::fRowGamerName( u32 row ) 
	{
		return std::string( mScores[ row ].gamertag );
	}

	u32 tXLSPLeaderboard::fColumnCount( ) 
	{ 
		return 3; 
	}

	u32 tXLSPLeaderboard::fRowsAvailable( ) 
	{ 
		return mScores.fCount( );
	}

	u32 tXLSPLeaderboard::fRowsTotal( ) 
	{ 
		return mScores.fCount( );
	}

	tUserData tXLSPLeaderboard::fData( u32 col, u32 row )
	{
		tUserData data;

		if( col == 0 )
			data.fSet( mScores[ row ].rank );
		else if( col == 1 )
		{
			// we need this wide pointer to persist for a while, so store it in the score struct
			mScores[ row ].gamerTagW = tLocalizedString::fFromCString(mScores[ row ].gamertag );
			data.fSet( mScores[ row ].gamerTagW.c_str( ) );
		}
		else
			data.fSet( mScores[ row ].score );

		return data;
	}

#else

	void tXLSP::fInitialize( )
	{
	}

	void tXLSP::fSetFrontEndPlayer( const tUserPtr& user )
	{
	}

	void tXLSP::fUpdate( f32 dt )
	{
	}

	void tXLSP::fWriteTestValues( tUser* user )
	{
	}

	void tXLSP::fWriteTestOwnership( tUser* user )
	{
	}

	// base leaderboard boilerplate


	void tXLSPLeaderboard::fRequestMobileHighScores( u32 leaderboardMachine, u32 numResults ) { }
	b32 tXLSPLeaderboard::fCheckReceivedHighScores( ) { return false; }
	void tXLSPLeaderboard::fReadByFriends( tUser * user, u32 start ) { }
	void tXLSPLeaderboard::fReadByRank( u32 start, u32 amount ) { }
	void tXLSPLeaderboard::fReadByRankAround( tUser * user, u32 toRead ) { }
	b32  tXLSPLeaderboard::fAdvanceRead( ) { return false; }
	void tXLSPLeaderboard::fCancelRead( ) { }
	void tXLSPLeaderboard::fAddBoardToRead( u32 board ) { }
	void tXLSPLeaderboard::fSelectBoard( u32 board ) { }
	tLocalizedString tXLSPLeaderboard::fBoardName( ) { return tLocalizedString( ); }
	u32  tXLSPLeaderboard::fColumnWidth( u32 col ) { return 0; }
	u32  tXLSPLeaderboard::fColumnAlignment( u32 col ) { return 0; }
	tLocalizedString tXLSPLeaderboard::fColumnName( u32 col ) { return tLocalizedString( ); }
	u32 tXLSPLeaderboard::fColumnUserData( u32 col ) { return 0; }
	u32 tXLSPLeaderboard::fRowRank( u32 row ) { return 0; }
	u32 tXLSPLeaderboard::fRowUserIdFromScript( u32 row ) { return 0; }
	std::string tXLSPLeaderboard::fRowGamerName( u32 row ) { return std::string( ); }
	u32 tXLSPLeaderboard::fColumnCount( ) { return 0; }
	u32 tXLSPLeaderboard::fRowsAvailable( ) { return 0; }
	u32 tXLSPLeaderboard::fRowsTotal( ) { return 0; }
	tUserData tXLSPLeaderboard::fData( u32 col, u32 row ) { return tUserData( ); }

	void tXLSPLeaderboard::fRequestMobileOwnership( tUser& user ) { }

#endif



	void tXLSP::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class< tXLSPLeaderboard, Sqrat::DefaultAllocator<tXLSPLeaderboard> > classDesc( vm.fSq( ) );
		classDesc
			.Func(_SC("AddBoardToRead"),	&tXLSPLeaderboard::fAddBoardToRead)
			.Func(_SC("ReadByFriends"),		&tXLSPLeaderboard::fReadByFriends)
			.Func(_SC("ReadByRank"),		&tXLSPLeaderboard::fReadByRank)
			.Func(_SC("ReadByRankAround"),	&tXLSPLeaderboard::fReadByRankAround)
			.Func(_SC("AdvanceRead"),		&tXLSPLeaderboard::fAdvanceRead)
			.Func(_SC("CancelRead"),		&tXLSPLeaderboard::fCancelRead)
			.Func(_SC("SelectBoard"),		&tXLSPLeaderboard::fSelectBoard)
			.Prop(_SC("BoardName"),			&tXLSPLeaderboard::fBoardName)
			.Func(_SC("ColumnWidth"),		&tXLSPLeaderboard::fColumnWidth)
			.Func(_SC("ColumnAlignment"),	&tXLSPLeaderboard::fColumnAlignment)
			.Func(_SC("ColumnName"),		&tXLSPLeaderboard::fColumnName)
			.Func(_SC("ColumnUserData"),	&tXLSPLeaderboard::fColumnUserData)
			.Func(_SC("RowRank"),			&tXLSPLeaderboard::fRowRank)
			.Func(_SC("RowUserId"),			&tXLSPLeaderboard::fRowUserIdFromScript)
			.Func(_SC("RowGamertag"),		&tXLSPLeaderboard::fRowGamerName)
			.Prop(_SC("ColumnCount"),		&tXLSPLeaderboard::fColumnCount)
			.Prop(_SC("RowsAvailable"),		&tXLSPLeaderboard::fRowsAvailable)
			.Prop(_SC("RowsTotal"),			&tXLSPLeaderboard::fRowsTotal)
			.Func(_SC("Data"),				&tXLSPLeaderboard::fData)
			;

		vm.fRootTable( ).Bind( _SC("LSPLeaderboard"), classDesc );
	}
}
