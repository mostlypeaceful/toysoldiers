#include "BasePch.hpp"
#if defined( platform_ios )
#include "tGameSession.hpp"
#include "tApplication.hpp"

namespace Sig
{
	const u32 tGameSession::cCreateUsesPresence					= 0;
	const u32 tGameSession::cCreateUsesStats					= 0;
	const u32 tGameSession::cCreateUsesMatchmaking				= 0;
	const u32 tGameSession::cCreateUsesArbitration				= 0;
	const u32 tGameSession::cCreateUsesPeerNetwork				= 0;
	const u32 tGameSession::cCreateInvitesDisabled				= 0;
	const u32 tGameSession::cCreateJoinViaPresenceDisabled		= 0;
	const u32 tGameSession::cCreateJoinViaPresenceFriendsOnly	= 0;
	const u32 tGameSession::cCreateJoinInProgressDisabled		= 0;
	
	const u32 tGameSession::cCreateSinglePlayerWithStats	= 0;
	const u32 tGameSession::cCreateMultiplayerStandard		= 0;
	const u32 tGameSession::cCreateMultiplayerRanked		= 0;
	const u32 tGameSession::cCreateSystemLink				= 0;
	const u32 tGameSession::cCreateGroupLobby				= 0;
	const u32 tGameSession::cCreateGroupGame				= 0;
	
	u32 tGameSession::fGetAddress( const tGameSessionInfo & info )
	{
		log_warning_unimplemented( 0 );
		return 0;
	}
	
	void tGameSession::fRegisterKey( const tGameSessionInfo & info )
	{
		log_warning_unimplemented( 0 );
	}

	void tGameSession::fUnregisterKey( const tGameSessionInfo & info )
	{
		log_warning_unimplemented( 0 );
	}
	
	//------------------------------------------------------------------------------
	tGameSession::tGameSessionData::tGameSessionData( )
	: mNonce( 0 )
	, mFlags( 0 )
	{
	}
	
	//------------------------------------------------------------------------------
	tGameSession::tGameSession( 
	   u32 createFlags,
	   const tGameSessionInfo & info,
	   u32 maxPublicSlots,
	   u32 maxPrivateSlots,
	   tStateChangedCallback callback )
	: mState( cStateNull )
	, mIsHost( false )
	, mCreateFlags( createFlags )
	, mStateChangedCallback( callback )
	{
		log_warning_unimplemented( 0 );
		fResetSlots( maxPublicSlots, maxPrivateSlots );
	}
	
	//------------------------------------------------------------------------------
	tGameSession::~tGameSession( )
	{
		log_warning_unimplemented( 0 );
	}
	
	//------------------------------------------------------------------------------
	void tGameSession::fCloseSessionData( )
	{
		log_warning_unimplemented( 0 );
	}
	
	//------------------------------------------------------------------------------
	b32 tGameSession::fStart( )
	{
		log_warning_unimplemented( 0 );
		return true;
	}
	
	//------------------------------------------------------------------------------
	b32 tGameSession::fJoinLocal( u32 userCount, const u32 users[], const b32 invited[] )
	{
		log_warning_unimplemented( 0 );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fLeaveLocal( u32 userCount, const u32 users[] )
	{
		log_warning_unimplemented( 0 );
		return true;
	}
	
	//------------------------------------------------------------------------------
	b32 tGameSession::fJoinRemote( u32 userCount, const tPlatformUserId users[], const b32 invited[] )
	{
		log_warning_unimplemented( 0 );
		return false;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fLeaveRemote( u32 userCount, const tPlatformUserId users[] )
	{
		log_warning_unimplemented( 0 );
		return false;
	}
	
	//------------------------------------------------------------------------------
	b32 tGameSession::fArbitrationRegister( )
	{
		log_warning_unimplemented( 0 );
		return false;
	}
	
	//------------------------------------------------------------------------------
	b32 tGameSession::fGetArbitrationResults( tGrowableArray< tPlatformUserId > & out ) const
	{
		log_warning_unimplemented( 0 );
		return false;
	}
	
	//------------------------------------------------------------------------------
	b32 tGameSession::fCreate( u32 sessionOwnerId )
	{
		log_warning_unimplemented( 0 );
		return true;
	}
	
	//------------------------------------------------------------------------------
	void tGameSession::fQueryDetails( )
	{
		log_warning_unimplemented( 0 );
		mDetails.mGameType = tUser::cUserContextGameTypeStandard;
		mDetails.mGameMode = ~0;
		mDetails.mHostUserIndex = mData.mCreatorHwIndex;
	}
	
	//------------------------------------------------------------------------------
	u32 tGameSession::fAddress( ) const
	{
		log_warning_unimplemented( 0 );
		return 0;
	}
	
	//------------------------------------------------------------------------------
	std::string tGameSession::fName( ) const
	{
		log_warning_unimplemented( 0 );
		return std::string( );
	}

	//------------------------------------------------------------------------------
	u32 tGameSession::fLastError( ) const
	{
		log_warning_unimplemented( 0 );
		return 0;
	}
	
	//------------------------------------------------------------------------------
	u64 tGameSession::fNonce( ) const
	{
		log_warning_unimplemented( 0 );
		return mData.mNonce;
	}
	
	//------------------------------------------------------------------------------
	b32 tGameSession::fEnd( )
	{
		log_warning_unimplemented( 0 );
		return true;
	}
	
	//------------------------------------------------------------------------------
	b32 tGameSession::fWriteStats( 
		 tPlatformUserId userId, u32 writeCount, const tGameSessionViewProperties writes[] )
	{
		log_warning_unimplemented( 0 );
		return true;
	}
	
	
	//------------------------------------------------------------------------------
	b32 tGameSession::fFlushStats( )
	{
		log_warning_unimplemented( 0 );
		return true;
	}
	
	//------------------------------------------------------------------------------
	b32 tGameSession::fDelete( )
	{
		log_warning_unimplemented( 0 );
		return true;
	}
	
	//------------------------------------------------------------------------------
	b32 tGameSession::fOperationComplete( b32 & success, b32 wait )
	{
		log_warning_unimplemented( 0 );
		success = true;
		return true;
	}

	//------------------------------------------------------------------------------
	void tGameSession::fOnLeaveJoiningLocal( b32 success )
	{
		log_warning_unimplemented( 0 );
	}

	//------------------------------------------------------------------------------
	void tGameSession::fOnLeaveRemovingLocal( b32 success )
	{
		log_warning_unimplemented( 0 );
	}

	//------------------------------------------------------------------------------
	void tGameSession::fOnLeaveJoiningRemote( b32 success )
	{
		log_warning_unimplemented( 0 );
	}

	//------------------------------------------------------------------------------
	void tGameSession::fOnLeaveRemovingRemote( b32 success )
	{
		log_warning_unimplemented( 0 );
	}
}

#endif//#if defined( platform_ios )
