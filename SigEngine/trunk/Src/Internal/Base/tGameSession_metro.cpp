#include "BasePch.hpp"
#if defined( platform_metro )
#include "tGameSession.hpp"
#include "tApplication.hpp"
#include "tLeaderboard.hpp"
#include "MetroUtil.hpp"
#include "Debug/CrashDump.hpp"
#include "Debug/DumpExceptions.hpp"
#include "Debug/tDebugger.hpp"
#include <collection.h>

namespace Sig
{
	// N.B.  We don't currently do any actual sessions/arbitration.
	// If implementing it for multiplayer, use tGameSessions_xbox360.cpp as a reference.

	using namespace Microsoft::Xbox;
	using namespace Microsoft::Xbox::Foundation;
	using namespace Microsoft::Xbox::Leaderboards;
	using namespace Platform;
	using namespace Platform::Collections;
	using namespace Windows::Foundation;

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

	namespace
	{
		LeaderboardService^ fCreateService()
		{
			try
			{
				return ref new LeaderboardService();
			}
			catch( Exception^ ex )
			{
				Debug::fDumpException("Exception creating leaderboard service",ex);
				return nullptr;
			}
		}

		LeaderboardService^ fService()
		{
			static LeaderboardService^ service = fCreateService();
			return service;
		}
	}

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
		mViewsWrittenTo.fFill( ~0 );
		fResetSlots( maxPublicSlots, maxPrivateSlots );
	}

	//------------------------------------------------------------------------------
	tGameSession::~tGameSession( )
	{
		log_warning_unimplemented( 0 );
	}

	b32 tGameSession::fIsSameSession( const tGameSessionInfo & info ) const
	{
		log_warning_unimplemented( 0 );
		return false;
	}

	//------------------------------------------------------------------------------
	void tGameSession::fCloseSessionData( )
	{
		sigassert( mState == cStateNull && "Sanity: ClearSessionData called with non-Null state" );
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fStart( )
	{
		sigassert( mState == cStateCreated && "Sessions can only be started from the Created state" );
		fPushState(cStateStarting);
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fJoinLocal( u32 userCount, const u32 users[], const b32 invited[] )
	{
		sigassert( ( mState == cStateCreated || mState == cStateStarted ) && "Sessions can only be joined from Created/Started state" );
		// N.B.: If doing true multiplayer sessions, we probably need to do something for our one local user here.
		fPushState(cStateJoiningLocalUsers);
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fLeaveLocal( u32 userCount, const u32 users[] )
	{
		sigassert( ( mState == cStateCreated || mState == cStateStarted ) && "Session can only be left from Created/Started state" );
		// N.B.: If doing true multiplayer sessions, we probably need to do something for our one local user here.
		fPushState(cStateRemovingLocalUsers);
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fJoinRemote( u32 userCount, const tPlatformUserId users[], const b32 invited[] )
	{
		sigassert( ( mState == cStateCreated || mState == cStateStarted ) && "Sessions can only be joined from Created/Started state" );
		log_warning_unimplemented( 0 );
		fPushState(cStateJoiningRemoteUsers);
		return false;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fLeaveRemote( u32 userCount, const tPlatformUserId users[] )
	{
		sigassert( ( mState == cStateCreated || mState == cStateStarted ) && "Session can only be left from Created/Started state" );
		log_warning_unimplemented( 0 );
		fPushState(cStateRemovingRemoteUsers);
		return false;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fArbitrationRegister( )
	{
		sigassert( mState == cStateCreated && "Sessions can only be registered from Created state" );
		log_warning_unimplemented( 0 );
		return false;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fGetArbitrationResults( tGrowableArray< tPlatformUserId > & out ) const
	{
		sigassert( mState != cStateRegistering );
		log_warning_unimplemented( 0 );
		return false;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fCreate( u32 sessionOwnerId )
	{
		fPushState( cStateCreating );
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
		return false;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fCheckDetailState( u32 targetState )
	{
		log_warning_unimplemented( 0 );
		return true;
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
		if( mState != cStateStarted )
		{
			log_warning( 0, __FUNCTION__ << ": Called with state != Started" );
			return false;
		}

		fPushState(cStateEnding);
		return true;
	}

	//------------------------------------------------------------------------------
	void fPostedResult( IAsyncAction^ action, AsyncStatus status )
	{
		try
		{
			log_line( Log::cFlagSession, "post results status==" << MetroUtil::fAsyncStatusToString(status) );
			action->GetResults();
			log_line( Log::cFlagSession, "Wrote stats successfully" );
		}
		catch( Exception^ ex )
		{
			log_line( Log::cFlagSession, "Failed to write stats:" );
			Debug::fDumpException( "Exception posting results", ex );
		}
	}

	//------------------------------------------------------------------------------
	u32 fLeaderboardStatsColumnId( u32 viewId, const tUserProperty& prop )
	{
		const tLeaderboardDesc* desc = tLeaderboard::fFindLeaderboard(viewId);
		if( !desc )
		{
			log_warning( 0, "No such leaderboard " << viewId );
			break_if_debugger();
			return ~0u;
		}

		for( u32 i=0 ; i<desc->mSpec.dwNumColumnIds ; ++i )
			if( desc->mColumnPropertyIds[i] == prop.mId )
				return desc->mSpec.rgwColumnIds[i];

		log_warning( 0, "No such leaderboard column 0x" << std::hex << prop.mId << " in leaderboard " << std::dec << viewId );
		break_if_debugger();
		return ~0u;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fWriteStats( tPlatformUserId userId, u32 writeCount, const tGameSessionViewProperties writes[] )
	{
		sigassert( mState == cStateStarted && "Can only write stats on a running session" );

		if( !fService() )
			return false;

		for( u32 w = 0; w < writeCount; ++w )
		{
			auto out = ref new Vector<LeaderboardAttribute^>();
			const tGameSessionViewProperties & in = writes[ w ];

			log_line( Log::cFlagSession, "Preparing view: 0x" << std::hex << in.mViewId );

			// Cast is validated by sig_static_assert in tUser_xbox360.cpp
			for( u32 p = 0; p < in.mNumProperties; ++p )
			{
				const tUserProperty & inProp = in.mProperties[ p ];
				auto prop = ref new LeaderboardAttribute();
				prop->AttributeId = fLeaderboardStatsColumnId(in.mViewId,inProp);
				inProp.mData.fGetPlatformSpecific( reinterpret_cast<void*>(prop) );

				if( prop->AttributeId==~0u )
				{
					log_warning( Log::cFlagSession, "\t[0x" << std::hex << inProp.mId << " (N/A)] = " << prop->Value << " (NOT SENT)" );
				}
				else
				{
					log_line( Log::cFlagSession, "\t[0x" << std::hex << inProp.mId << " (" << std::dec << prop->AttributeId << ")] = " << prop->Value );
					out->Append(prop);
				}
			}

			try
			{
				auto postOp = fService()->PostResultsAsync( in.mViewId, out->GetView() );
				log_line( Log::cFlagSession, "PostResultsAsync started OK" );
				postOp->Completed = Debug::fInferCallbackTypeAndDumpCrashSEH( &fPostedResult );
			}
			catch( Exception^ ex )
			{
				log_line( Log::cFlagSession, "PostResultsAsync failed" );
				Debug::fDumpException("Exception beginning to post results",ex);
				return false;
			}
		}

		// Make the call
		fPushState( cStateWritingStats );
		return true;
	}


	//------------------------------------------------------------------------------
	b32 tGameSession::fFlushStats( )
	{
		sigassert( mState == cStateStarted && "Can only flush stats on a running session" );
		fPushState( cStateFlushingStats ); // All MoLIVE stat pushes are immediate, do the state dance anyways
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fDelete( )
	{
		sigassert( ( mState == cStateStarted || mState == cStateCreated ) && "Delete can only be called from Created or Started state" );
		fPushState( cStateDeleting );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fOperationComplete( b32 & success, b32 wait )
	{
		success = true;
		return true;
	}

	//------------------------------------------------------------------------------
	void tGameSession::fOnLeaveJoiningLocal( b32 success )
	{
	}

	//------------------------------------------------------------------------------
	void tGameSession::fOnLeaveRemovingLocal( b32 success )
	{
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
#endif // #if defined( platform_metro )
