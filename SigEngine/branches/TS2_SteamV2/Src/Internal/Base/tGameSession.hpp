#ifndef __tGameSessionBase__
#define __tGameSessionBase__
#include "tGameSessionSearchResult.hpp"
#include "Net/tRemoteConnection.hpp"
#include "Net/tHost.hpp"
#if defined( use_steam )
#include "tLeaderboard.hpp"
#endif

namespace Sig
{
	class tGameInvite;

	struct tGameSessionDetails
	{
		u32 mHostUserIndex;
		u32 mGameType;
		u32 mGameMode;
		u32 mGameVersion;
	};

	struct tGameSessionViewProperties
	{
		u32 mViewId;
		u32 mNumProperties;
		const tUserProperty * mProperties;
#if defined( use_steam )
		tDynamicArray< tUserData > mAggregated;
#endif

		tGameSessionViewProperties( u32 id = ~0, u32 numProps = 0, tUserProperty* props = NULL )
			: mViewId( id ), mNumProperties( numProps ), mProperties( props )
#if defined( use_steam )
			, mAggregated( 0 )
#endif
		{ }
	};

	///
	/// \class tGameSession
	/// \brief 
	class tGameSession : public tRefCounter
	{
	public:

		enum tState
		{
			cStateNull = 0,
			cStateCreating,
			cStateCreated,
			cStateJoiningLocalUsers,
			cStateRemovingLocalUsers,
			cStateJoiningRemoteUsers,
			cStateRemovingRemoteUsers,
			cStateRegistering,
			cStateStarting,
			cStateStarted,
			cStateWritingStats,
			cStateFlushingStats,
			cStateEnding,
			cStateDeleting,
			
			cStateCount
		};

		// Creation flags
		static const u32 cCreateUsesPresence;
		static const u32 cCreateUsesStats;
		static const u32 cCreateUsesMatchmaking;
		static const u32 cCreateUsesArbitration;
		static const u32 cCreateUsesPeerNetwork;
		static const u32 cCreateInvitesDisabled;
		static const u32 cCreateJoinViaPresenceDisabled;
		static const u32 cCreateJoinViaPresenceFriendsOnly;
		static const u32 cCreateJoinInProgressDisabled;

		// Aggregates
		static const u32 cCreateSinglePlayerWithStats;	// UsesPresence | UsesStats | InvitesDisabled | JoinViaPresenceDisabled | JoinInProgressDisabled
		static const u32 cCreateMultiplayerStandard;	// UsesPresence | UsesStats | UsesMatchmaking | UsesPeerNetwork
		static const u32 cCreateMultiplayerRanked;		// MultiplayerStandard | UsesArbitration
		static const u32 cCreateSystemLink;				// UsesPeerNetwork
		static const u32 cCreateGroupLobby;				// UsesPresence | UsesPeerNetwork
		static const u32 cCreateGroupGame;				// UsesStats | UsesMatchmaking | UsesPeerNetwork

		typedef tDelegate<void ( tGameSession& gameSession, u32 oldState, b32 success )> tStateChangedCallback;

#ifdef platform_xbox360
		typedef ULONGLONG tNonce;
#else
		typedef u64 tNonce;
#endif

	public:

		static tAddr fGetAddress( const tGameSessionInfo & info );
		static void fRegisterKey( const tGameSessionInfo & info );
		static void fUnregisterKey( const tGameSessionInfo & info );
#if defined( use_steam )
		static b32 fIsWriteActive( );
#endif

	public:

		// Create a host session
		tGameSession( 
			u32 createFlags,
			u32 maxPublicSlots, 
			u32 maxPrivateSlots,
			tStateChangedCallback callback = tStateChangedCallback( ) );

		// Create a non-host session
		tGameSession( 
			u32 createFlags,
			const tGameSessionInfo & info,
			u32 maxPublicSlots,
			u32 maxPrivateSlots,
			tStateChangedCallback callback = tStateChangedCallback( ) );

		~tGameSession( );

		// State helpers
		tState	fState( ) const { return mState; }
		b32		fIsNull( ) const { return mState == cStateNull; }
		b32		fIsHosting( ) const { return mState == cStateCreated && mIsHost; }
		b32		fIsCreated( ) const { return mState == cStateCreated; }
		b32		fIsCreatedOrJoining( ) const { return mState >= cStateCreated && mState < cStateStarting; }
		b32		fIsStarted( ) const { return mState == cStateStarted; }
		b32		fIsWritingOrFlushingStats( ) const { return mState == cStateWritingStats || mState == cStateFlushingStats; }
		
		b32 fIsSameSession( const tGameSessionInfo & info ) const;
		b32 fIsHost( ) const { return mIsHost; }
		b32	fIsOnline( ) const;
		u32 fCreateFlags( ) const { return mCreateFlags; }

		void fSetStateChangedCallback( tStateChangedCallback callback ) { mStateChangedCallback = callback; }
		b32 fHasStateChangedCallback( ) { return !mStateChangedCallback.fNull( ); }

		tNonce fNonce( ) const; // Host only
		void fSetNonce( tNonce nonce ); // Non-host and state < cStateStarting

		tAddr fAddress( ) const;
		std::string fName( ) const;

		u32 fLastError( ) const;

		// Slots
		u32 fTotalPublicSlots( ) const { return mSlots[ cSlotTotalPublic ]; }
		u32 fTotalPrivateSlots( ) const { return mSlots[ cSlotTotalPrivate ] ; }
		u32 fTotalSlots( ) const { return fTotalPublicSlots( ) + fTotalPrivateSlots( ); }
		u32 fFreePublicSlots( ) const { return mSlots[ cSlotTotalPublic ] - mSlots[ cSlotFilledPublic ]; }
		u32 fFreePrivateSlots( ) const { return mSlots[ cSlotTotalPrivate ] - mSlots[ cSlotFilledPrivate ]; }
		u32 fFreeSlots( ) const { return fFreePublicSlots( ) + fFreePrivateSlots( ); }
		u32 fFilledPublicSlots( ) const { return mSlots[ cSlotFilledPublic ]; }
		u32 fFilledPrivateSlots( ) const { return mSlots[ cSlotFilledPrivate ]; }
		u32 fFilledSlots( ) const { return fFilledPublicSlots( ) + fFilledPrivateSlots( ); }
		const tGameSessionDetails & fDetails( ) { return mDetails; }

		f32 fStateTimer( ) const { return mStateTimer; }

		// Advance any state
		void fTick( f32 dt );

		// Wait till any pending operation completes
		void fWait( );
		
		// Creation
		b32 fCreate( u32 sessionOwnerId );
		b32 fDelete( );

		// Local Join / Leave
		b32 fJoinLocal( u32 index, b32 invited = false );
		b32 fJoinLocal( u32 userCount, const u32 indices[], b32 invited = false );
		b32 fJoinLocal( u32 userCount, const u32 indices[], const b32 invited[] );

		b32 fLeaveLocal( u32 index );
		b32 fLeaveLocal( u32 userCount, const u32 indices[] );
		b32 fLeavelAllLocal( u32 & localUserCount ); // Leave all local users

		// Zombies, to adjust slot counts without adding/removing to session
		b32 fJoinZombie( u32 count );	// Sync operation
		b32 fLeaveZombie( u32 count );	// Sync operation

		// Remote Join / Leave
		b32 fJoinRemote( const tPlatformUserId & user, b32 invited = false );
		b32 fJoinRemote( u32 userCount, const tPlatformUserId users[], b32 invited = false );
		b32 fJoinRemote( u32 userCount, const tPlatformUserId users[], const b32 invited[] );

		b32 fLeaveRemote( const tPlatformUserId & user );
		b32 fLeaveRemote( u32 userCount, const tPlatformUserId users[] );

		// Arbitration - must be called after all players are joined, before starting, for each session
		// with the host making the last call
		b32 fArbitrationRegister( );
		b32 fGetArbitrationResults( tGrowableArray< tPlatformUserId > & out ) const;

		// Run controls
		b32 fStart( );
		b32 fEnd( );

		// Stats
		//b32 fWriteStats( tPlatformUserId userId, u32 leaderBoardId, u32 propCount, const tUserProperty props[] );
		b32 fWriteStats( tPlatformUserId userId, u32 writeCount, const tGameSessionViewProperties writes[] );
		b32 fFlushStats( );

	private:

		enum tSlot
		{
			cSlotTotalPublic = 0,
			cSlotTotalPrivate,
			cSlotFilledPublic,
			cSlotFilledPrivate,
			cSlotZombiePublic,
			cSlotZombiePrivate,
			cSlotCount
		};

#ifdef platform_xbox360

		struct tGameSessionData
		{
			tGameSessionData( );
			~tGameSessionData( );

			void fCloseHandle( );
			b32 fOverlapComplete( b32 & success, b32 wait );
			

			HANDLE			mHandle;
			XOVERLAPPED		mOverlapped;

			tGameSessionInfo	mInfo;
			DWORD				mFlags;
			tNonce				mNonce;
			DWORD				mLastError;
			tDynamicBuffer		mRegistrationResults;
		};

#elif defined( use_steam )

		enum tSubState
		{
			cSubStateNone,
			cSubStateReading,
			cSubStateWritePrepared,
			cSubStateWriting,
		};

		struct tGameSessionData
		{
			tGameSessionData( );
			~tGameSessionData();

			void fCancel( );
			void fClear( );
			b32 fOverlapComplete( b32 & success, b32 wait );

			SteamAPICall_t		mApiCall;

			tGameSessionInfo	mInfo;
			u32					mCreatorHwIndex;
			u32					mFlags;
			tNonce				mNonce;
			ESteamAPICallFailure	mLastError;
			b32 mSuccess;
			b32 mComplete;

			tDynamicArray< tGameSessionViewProperties > mWrites;
			u32 mCurrentWrite;
			tDynamicArray< s32 > mWriteBuffer;
			tDynamicArray< tUserProperty > mProperties;
			tSubState mSubState;
		};

		tLeaderboardPtr mLeaderboard;

		STEAM_CALLBACK( tGameSession, fOnLobbyCreated, LobbyCreated_t, mCallbackLobbyCreated );
		STEAM_CALLBACK( tGameSession, fOnLobbyJoined, LobbyEnter_t, mCallbackLobbyJoined );
		STEAM_CALLBACK( tGameSession, fOnLobbyChatUpdate, LobbyChatUpdate_t, mCallbackLobbyChatUpdate );
		STEAM_CALLBACK( tGameSession, fOnLobbyGameCreated, LobbyGameCreated_t, mCallbackLobbyGameCreated );

		b32 fAdvanceLeaderboardRead( );

		void fOnFindLeaderboard( LeaderboardFindResult_t *pResult, bool bIOFailure );
		CCallResult< tGameSession, LeaderboardFindResult_t > mCallbackFindLeaderboard;

		void fOnLeaderboardUploaded( LeaderboardScoreUploaded_t *pResult, bool bIOFailure );
		CCallResult< tGameSession, LeaderboardScoreUploaded_t > mCallbackLeaderboardUploaded;

		static u32 mNumWritingInstances;
#else

		struct tGameSessionData
		{
			tGameSessionData( );

			u32					mCreatorHwIndex;
			u32					mFlags;
			tNonce				mNonce;
			DWORD				mLastError;
		};

#endif

		struct tSessionUser
		{
			union
			{
#if defined( use_enet )
				tPlatformUserId mUserId;
#elif defined( use_steam )
				u64 mUserId;
#endif
				u32 mLocalHwIndex;
			};

			b16 mInPrivateSlot;
			b16 mIsLocal;
		};

	private:

		void fAdvanceState( b32 wait );
		b32 fOperationComplete( b32 & success, b32 wait );

		void fResetSlots( u32 maxPublic, u32 maxPrivate );
		b32  fTryFillSlots( u32 & additionalPublic, u32 & additionalPrivate );
		
		void fPushState( tState newState );
		void fPopState( );

		void fOnLeaveCreating( b32 success );
		void fOnLeaveJoiningLocal( b32 success );
		void fOnLeaveRemovingLocal( b32 success );
		void fOnLeaveJoiningRemote( b32 success );
		void fOnLeaveRemovingRemote( b32 success );
		void fOnLeaveDeleting( b32 success );

		void fCloseSessionData( );

		void fQueryDetails( );

		b32 fCheckDetailState( u32 targetState );

	private:
		
		tState mState;
		tState mPrevState;
		f32 mStateTimer;

		b32 mIsHost;
		u32 mCreateFlags;
		tFixedArray<u32, cSlotCount> mSlots;

		tGameSessionData mData;
		tGrowableArray<byte> mOperationData;
		tGrowableArray<tSessionUser> mUsers; 

		tStateChangedCallback mStateChangedCallback;
		tGameSessionDetails mDetails;
		b32 mLobbyCreated;
	};

	typedef tRefCounterPtr<tGameSession> tGameSessionPtr;

}

#endif//__tGameSessionBase__
