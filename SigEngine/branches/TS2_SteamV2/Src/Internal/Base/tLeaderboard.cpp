#include "BasePch.hpp"
#include "tLeaderboard.hpp"
#include "Scripts/tScript64.hpp"

namespace Sig 
{
	tArraySleeve<const tLeaderboardDesc> tLeaderboard::mLeaderboards;

	//------------------------------------------------------------------------------
	void tLeaderboard::fSetLeaderboards( u32 count, const tLeaderboardDesc * leaderboards )
	{
		mLeaderboards.fSet( leaderboards, count );
	}

	//------------------------------------------------------------------------------
	const tLeaderboardDesc * tLeaderboard::fFindLeaderboard( u32 boardId )
	{
		const u32 boardCount = mLeaderboards.fCount( );
		for( u32 b = 0; b < boardCount; ++b )
		{
			if( mLeaderboards[ b ].fBoardId( ) == boardId )
				return &mLeaderboards[ b ];
		}

		return NULL;
	}

	//------------------------------------------------------------------------------
	const tLeaderboardDesc * tLeaderboard::fFindLeaderboardByViewId( u32 viewId )
	{
		const u32 boardCount = mLeaderboards.fCount( );
		for( u32 b = 0; b < boardCount; ++b )
		{
			if( mLeaderboards[ b ].mSpec.dwViewId == viewId )
				return &mLeaderboards[ b ];
		}

		return NULL;
	}

	//------------------------------------------------------------------------------
	tScript64 tLeaderboard::fRowUserIdFromScript( u32 r ) const
	{
		return tScript64( ( u64 ) fRowUserId( r ) );
	}

	//------------------------------------------------------------------------------
	u32 tLeaderboard::fRowForUserIdFromScript( tScript64 r ) const
	{
		return fRowForUserId( ( tPlatformUserId )r.fGet( ) );
	}

	//------------------------------------------------------------------------------
	// tLeaderboard
	//------------------------------------------------------------------------------
	tLeaderboard::tLeaderboard( )
		: mState( cStateNull )
		, mBoardsToReadCount( 0 )
		, mRetainZeroRanks( false )
	{
		fZeroOut( mSelected );
	}

	//------------------------------------------------------------------------------
	tLeaderboard::~tLeaderboard( )
	{
		fCancelRead( );
	}

	//------------------------------------------------------------------------------
	// tLeaderboard Copy Constructor
	//------------------------------------------------------------------------------
	tLeaderboard::tLeaderboard(const tLeaderboard& leaderboard)
		: mState(leaderboard.mState)
		, mBoardsToReadCount(leaderboard.mBoardsToReadCount)
		, mRetainZeroRanks(leaderboard.mRetainZeroRanks)
	{
		fZeroOut( mSelected );
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::fSelectBoard( u32 boardId )
	{
		fZeroOut( mSelected );

		for( u32 b = 0; b < mBoardsToReadCount; ++b )
		{
			if( mBoardsToRead[ b ]->fBoardId( ) == boardId )
			{
				mSelected.mBoard = mBoardsToRead[ b ];
				return;
			}
		}

		log_warning( 0, "Leaderboard " << boardId << " not accessible because it wasn't added for read" );
	}

		//------------------------------------------------------------------------------
	void tLeaderboard::fAddBoardToRead( u32 boardId )
	{
		sigassert( mState != cStateReading && "Cannot switch leaderboards in a read state" );
		
		// Check to see if it's already added
		for( u32 b = 0; b < mBoardsToReadCount; ++b )
		{
			if( mBoardsToRead[ b ]->fBoardId( ) == boardId )
				return;
		}

		// We're full of boards
		if( mBoardsToReadCount >= mBoardsToRead.fCount( ) )
		{
			log_warning( 0, "Leaderboard already has max of " << mBoardsToRead.fCount( ) << " boards to read set." );
			return;
		}

		// Reset the state so that users fail if they attempt to access boards
		fZeroOut( mSelected );
		mState = cStateNull;

		const u32 count = mLeaderboards.fCount( );
		for( u32 b = 0; b < count; ++b )
		{
			if( mLeaderboards[ b ].fBoardId( ) == boardId )
			{
				mBoardsToRead[ mBoardsToReadCount++ ] = &mLeaderboards[ b ];
				return;
			}
		}

		log_warning( 0, "Leaderboard " << boardId << " could not be found for reading" );
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::fRemoveBoardToRead( u32 boardId )
	{
		sigassert( mState != cStateReading && "Cannot switch leaderboards in a read state" );

		// Reset the state so that users fail if they attempt to access boards
		fZeroOut( mSelected );
		mState = cStateNull;

		for( u32 b = 0; b < mBoardsToReadCount; ++b )
		{
			if( mBoardsToRead[ b ]->fBoardId( ) == boardId )
			{
				mBoardsToRead[ b ] = mBoardsToRead[ --mBoardsToReadCount ];
				return;
			}
		}

		log_warning( 0, "Leaderboard " << boardId << " was not added for reading" );
	}

	//------------------------------------------------------------------------------
	b32 tLeaderboard::fDataIsValid( u32 col, u32 row ) const
	{
		tUserData result;
		if( !fFindResult( col, row, result ) )
			return false;

		return result.fIsSet( );
	}

	//------------------------------------------------------------------------------
	tUserData	tLeaderboard::fData( u32 col, u32 row ) const
	{
		tUserData result;
		fFindResult( col, row, result );
		return result;
	}

	//------------------------------------------------------------------------------
	b32 tLeaderboard::fDataIsValidById( u32 columnId, u32 row ) const
	{
		tUserData result;
		if( !fFindResultById( columnId, row, result ) )
			return false;

		return result.fIsSet( );
	}

	//------------------------------------------------------------------------------
	tUserData tLeaderboard::fDataById( u32 columnId, u32 row ) const
	{
		tUserData result;
		fFindResultById( columnId, row, result );
		return result;
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class< tLeaderboard, Sqrat::RefCountedAllocator<tLeaderboard> > classDesc( vm.fSq( ) );
		classDesc
			.Func(_SC("AddBoardToRead"),	&tLeaderboard::fAddBoardToRead)
			.Func(_SC("RemoveBoardToRead"), &tLeaderboard::fRemoveBoardToRead)
			.Func(_SC("ReadByRank"),		&tLeaderboard::fReadByRank)
			.Func(_SC("ReadByRankAround"),	&tLeaderboard::fReadByRankAround)
			.Func(_SC("ReadByFriends"),		&tLeaderboard::fReadByFriends)
			.Func(_SC("AdvanceRead"),		&tLeaderboard::fAdvanceRead)
			.Func(_SC("CancelRead"),		&tLeaderboard::fCancelRead)
			.Func(_SC("SelectBoard"),		&tLeaderboard::fSelectBoard)
			.Func(_SC("ColumnWidth"),		&tLeaderboard::fColumnWidth)
			.Func(_SC("ColumnAlignment"),	&tLeaderboard::fColumnAlignment)
			.Func(_SC("ColumnName"),		&tLeaderboard::fColumnName)
			.Func(_SC("ColumnUserData"),	&tLeaderboard::fColumnUserData)
			.Func(_SC("RowRank"),			&tLeaderboard::fRowRank)
			.Func(_SC("RowUserId"),			&tLeaderboard::fRowUserIdFromScript)
			.Func(_SC("RowGamertag"),		&tLeaderboard::fRowGamerName)
			.Func(_SC("RowForUserId"),		&tLeaderboard::fRowForUserIdFromScript)
			.Func(_SC("Data"),				&tLeaderboard::fData)
			.Func(_SC("DataById"),			&tLeaderboard::fDataById)
			.Prop(_SC("BoardName"),			&tLeaderboard::fBoardName)
			.Prop(_SC("ColumnCount"),		&tLeaderboard::fColumnCount)
			.Prop(_SC("RowsAvailable"),		&tLeaderboard::fRowsAvailable)
			.Prop(_SC("RowsTotal"),			&tLeaderboard::fRowsTotal)
			;

		vm.fRootTable( ).Bind( _SC("Leaderboard"), classDesc );

		vm.fConstTable( ).Const( _SC("LEADER_BOARD_COLUMN_RANK"), (s32)tLeaderboard::cColumnIndexRank );
		vm.fConstTable( ).Const( _SC("LEADER_BOARD_COLUMN_GAMER_NAME"), (s32)tLeaderboard::cColumnIndexGamerName );
		vm.fConstTable( ).Const( _SC("LEADER_BOARD_COLUMN_USER_DATA"), (s32)tLeaderboard::cColumnIndexUserData );
	}

} //namespace Sig
