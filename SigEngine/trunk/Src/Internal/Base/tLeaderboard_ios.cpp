#include "BasePch.hpp"
#if defined( platform_ios )
#include "tLeaderboard.hpp"

namespace Sig 
{

	//------------------------------------------------------------------------------
	// tLeaderboardDesc
	//------------------------------------------------------------------------------
	u32 tLeaderboardDesc::fBoardId( ) const
	{
		log_warning_unimplemented( );
		return ~0;
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::fResetLeaderBoards( )
	{
		log_warning_unimplemented( );
	}

		//------------------------------------------------------------------------------
	b32 tLeaderboard::fFindResult( u32 col, u32 row, tUserData & data ) const
	{
		log_warning_unimplemented( );
		return false;
	}

	//------------------------------------------------------------------------------
	b32 tLeaderboard::fFindResultById( u32 columnId, u32 row, tUserData & data ) const
	{
		log_warning_unimplemented( );
		return false;
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::fReadByRank( u32 rankStart, u32 toRead )
	{
		log_warning_unimplemented( );
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::fReadByFriends( tUser * user, u32 friendStart, u32 toRead )
	{
		log_warning_unimplemented( );
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::fReadByPlatformId( const tPlatformUserId userIds[], u32 userIdCount )
	{
		log_warning_unimplemented( );
	}

	//------------------------------------------------------------------------------
	b32	tLeaderboard::fAdvanceRead( )
	{
		log_warning_unimplemented( );
		return true;
	}

	//------------------------------------------------------------------------------
	void tLeaderboard::fCancelRead( )
	{
		log_warning_unimplemented( );
	}

	//------------------------------------------------------------------------------
	tLocalizedString tLeaderboard::fBoardName( ) const
	{
		log_warning_unimplemented( );
		return tLocalizedString( );
	}

	//------------------------------------------------------------------------------
	u32 tLeaderboard::fColumnCount( ) const
	{
		log_warning_unimplemented( );
		return 0;
	}

	//------------------------------------------------------------------------------
	u32	tLeaderboard::fColumnWidth( u32 col ) const
	{
		log_warning_unimplemented( );
		return 0;
	}

	//------------------------------------------------------------------------------
	u32	tLeaderboard::fColumnAlignment( u32 col ) const
	{
		log_warning_unimplemented( );
		return Gui::tText::cAlignMax;
	}

	//------------------------------------------------------------------------------
	tLocalizedString tLeaderboard::fColumnName( u32 col ) const
	{
		log_warning_unimplemented( );
		return tLocalizedString( );
	}

	//------------------------------------------------------------------------------
	u32 tLeaderboard::fRowsAvailable( ) const
	{
		log_warning_unimplemented( );
		return 0;
	}

	//------------------------------------------------------------------------------
	u32 tLeaderboard::fRowsTotal( ) const
	{
		log_warning_unimplemented( );
		return 0;
	}

	//------------------------------------------------------------------------------
	u32 tLeaderboard::fRowRank( u32 r ) const
	{
		log_warning_unimplemented( );
		return 0;
	}

	//------------------------------------------------------------------------------
	tPlatformUserId tLeaderboard::fRowUserId( u32 r ) const
	{
		log_warning_unimplemented( );
		return tUser::cInvalidUserId;
	}

	//------------------------------------------------------------------------------
	std::string tLeaderboard::fRowGamerName( u32 r ) const
	{
		log_warning_unimplemented( );
		return "NotImplemented";
	}
}
#endif//#if defined( platform_ios )
