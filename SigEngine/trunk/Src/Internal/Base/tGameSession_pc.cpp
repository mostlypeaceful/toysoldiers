#include "BasePch.hpp"
#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
#include "tGameSession.hpp"
#include "tApplication.hpp"

namespace Sig
{
	const u32 tGameSession::cCreateUsesPresence					= 0x00000001;
	const u32 tGameSession::cCreateUsesStats					= 0x00000002;
	const u32 tGameSession::cCreateUsesMatchmaking				= 0x00000004;
	const u32 tGameSession::cCreateUsesArbitration				= 0x00000008;
	const u32 tGameSession::cCreateUsesPeerNetwork				= 0x00000010;
	const u32 tGameSession::cCreateInvitesDisabled				= 0x00000020;
	const u32 tGameSession::cCreateJoinViaPresenceDisabled		= 0x00000040;
	const u32 tGameSession::cCreateJoinViaPresenceFriendsOnly	= 0x00000080;
	const u32 tGameSession::cCreateJoinInProgressDisabled		= 0x00000100;

	const u32 tGameSession::cCreateSinglePlayerWithStats	= tGameSession::cCreateUsesPresence |
		tGameSession::cCreateUsesStats |
		tGameSession::cCreateInvitesDisabled |
		tGameSession::cCreateJoinViaPresenceDisabled |
		tGameSession::cCreateJoinInProgressDisabled;
	const u32 tGameSession::cCreateMultiplayerStandard		= tGameSession::cCreateUsesPresence |
		tGameSession::cCreateUsesStats |
		tGameSession::cCreateUsesMatchmaking |
		tGameSession::cCreateUsesPeerNetwork;
	const u32 tGameSession::cCreateMultiplayerRanked		= tGameSession::cCreateMultiplayerStandard |
		tGameSession::cCreateUsesArbitration;
	const u32 tGameSession::cCreateSystemLink				= tGameSession::cCreateUsesPeerNetwork;
	const u32 tGameSession::cCreateGroupLobby				= tGameSession::cCreateUsesPresence |
		tGameSession::cCreateUsesPeerNetwork;
	const u32 tGameSession::cCreateGroupGame				= tGameSession::cCreateUsesStats |
		tGameSession::cCreateUsesMatchmaking |
		tGameSession::cCreateUsesPeerNetwork;

	u32 tGameSession::fGetAddress( const tGameSessionInfo & info )
	{
		log_warning_unimplemented( );
		return 0;
	}

	b32 tGameSession::fIsHostedByLocal( const tGameSessionInfo& sessionInfo )
	{
		log_warning_unimplemented( );
		return false;
	}

	void tGameSession::fRegisterKey( const tGameSessionInfo & info )
	{
		log_warning_unimplemented( );
	}

	void tGameSession::fUnregisterKey( const tGameSessionInfo & info )
	{
		log_warning_unimplemented( );
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
		log_warning_unimplemented( );
		fResetSlots( maxPublicSlots, maxPrivateSlots );
	}

	//------------------------------------------------------------------------------
	tGameSession::~tGameSession( )
	{
		log_warning_unimplemented( );
	}

	b32 tGameSession::fIsSameSession( const tGameSessionInfo & info ) const
	{
		log_warning_unimplemented( );
		return false;
	}

	//------------------------------------------------------------------------------
	void tGameSession::fCloseSessionData( )
	{
		log_warning_unimplemented( );
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fStart( )
	{
		log_warning_unimplemented( );
		fPushState( cStateStarting );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fJoinLocal( u32 userCount, const u32 users[], const b32 invited[] )
	{
		log_warning_unimplemented( );
		fPushState( cStateJoiningLocalUsers );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fLeaveLocal( u32 userCount, const u32 users[] )
	{
		log_warning_unimplemented( );
		fPushState( cStateRemovingLocalUsers );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fJoinRemote( u32 userCount, const tPlatformUserId users[], const b32 invited[] )
	{
		log_warning_unimplemented( );
		fPushState( cStateJoiningRemoteUsers );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fLeaveRemote( u32 userCount, const tPlatformUserId users[] )
	{
		log_warning_unimplemented( );
		fPushState( cStateRemovingRemoteUsers );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fArbitrationRegister( )
	{
		log_warning_unimplemented( );
		return false;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fGetArbitrationResults( tGrowableArray< tPlatformUserId > & out ) const
	{
		log_warning_unimplemented( );
		return false;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fCreate( u32 sessionOwnerId )
	{
		log_warning_unimplemented( );
		fPushState( cStateCreating );
		return true;
	}

	//------------------------------------------------------------------------------
	void tGameSession::fQueryDetails( )
	{
		log_warning_unimplemented( );
		mDetails.mGameType = tUser::cUserContextGameTypeStandard;
		mDetails.mGameMode = ~0;
		mDetails.mHostUserIndex = mData.mCreatorHwIndex;
	}

	//------------------------------------------------------------------------------
	void tGameSession::fEnableQosListen( )
	{
		log_warning_unimplemented( );
	}

	//------------------------------------------------------------------------------
	void tGameSession::fDisableQosListen( )
	{
		log_warning_unimplemented( );
	}

	//------------------------------------------------------------------------------
	void tGameSession::fSetQosListenData( const void* data, u32 dataSize )
	{
		log_warning_unimplemented( );
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fMigrateHost( u32 index, tGameSessionInfo& sessionInfo )
	{
		log_warning_unimplemented( );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fCheckDetailState( u32 targetState )
	{
		log_warning_unimplemented( );
		return true;
	}

	//------------------------------------------------------------------------------
	u32 tGameSession::fAddress( ) const
	{
		log_warning_unimplemented( );
		return 0;
	}

	//------------------------------------------------------------------------------
	std::string tGameSession::fName( ) const
	{
		log_warning_unimplemented( );
		return std::string( );
	}

	//------------------------------------------------------------------------------
	u32 tGameSession::fLastError( ) const
	{
		log_warning_unimplemented( );
		return 0;
	}

	//------------------------------------------------------------------------------
	u64 tGameSession::fNonce( ) const
	{
		log_warning_unimplemented( );
		return mData.mNonce;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fEnd( )
	{
		log_warning_unimplemented( );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fWriteStats( 
		tPlatformUserId userId, u32 writeCount, const tGameSessionViewProperties writes[] )
	{
		log_warning_unimplemented( );
		fPushState( cStateWritingStats );
		return true;
	}


	//------------------------------------------------------------------------------
	b32 tGameSession::fFlushStats( )
	{
		log_warning_unimplemented( );
		fPushState( cStateFlushingStats );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fDelete( )
	{
		log_warning_unimplemented( );
		fPushState( cStateDeleting );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fOperationComplete( b32 & success, b32 wait )
	{
		log_warning_unimplemented( );
		success = true;
		return true;
	}

	//------------------------------------------------------------------------------
	void tGameSession::fOnLeaveJoiningLocal( b32 success )
	{
		log_warning_unimplemented( );
	}

	//------------------------------------------------------------------------------
	void tGameSession::fOnLeaveRemovingLocal( b32 success )
	{
		log_warning_unimplemented( );
	}

	//------------------------------------------------------------------------------
	void tGameSession::fOnLeaveJoiningRemote( b32 success )
	{
		log_warning_unimplemented( );
	}

	//------------------------------------------------------------------------------
	void tGameSession::fOnLeaveRemovingRemote( b32 success )
	{
		log_warning_unimplemented( );
	}
}
#endif//#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
