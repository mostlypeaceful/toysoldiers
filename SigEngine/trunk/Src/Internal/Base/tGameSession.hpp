#ifndef __tGameSessionBase__
#define __tGameSessionBase__
#include "tGameSessionSearchResult.hpp"
#include "Net/tRemoteConnection.hpp"
#include "Net/tHost.hpp"
#include "XtlUtil.hpp"

namespace Sig
{
	class tGameInvite;

	struct tGameSessionDetails
	{
		u32 mHostUserIndex;
		u32 mGameType;
		u32 mGameMode;
	};

	struct tGameSessionViewProperties
	{
		u32 mViewId;
		u32 mNumProperties;
		const tUserProperty * mProperties;

		tGameSessionViewProperties( u32 id = ~0, u32 numProps = 0, tUserProperty* props = NULL )
			: mViewId( id ), mNumProperties( numProps ), mProperties( props )
		{ }
	};

	/// \class tGameSession
	/// \brief "Wraps" the XDK concept of a session, which is used for leaderboard submission and arbitration.
	class base_export tGameSession : public tRefCounter
	{
	public:

		enum tState
		{
			/// N.B.: External code relies on a specific dance of these states occurring.
			/// For example, tCachedScoreWriterSession checks for:
			///		Creating -> Created
			///		JoininLocalUsers -> Created
			///		{Starting,FlushingStats} -> Started
			/// As such, new implementations shouldn't skip intermediate steps
			/// (e.g. don't skip Creating straight to Created just because creating is immediate, synchronous, and unfailing)

			cStateNull = 0,
			cStateCreating,									///< Waiting for fCreate async operations to complete
			cStateCreated,									///< fCreate async operations completed
			cStateJoiningLocalUsers,						///< Waiting for fJoinLocal async operations to complete
			cStateRemovingLocalUsers,						///< Waiting for fLeaveLocal async operations to complete
			cStateJoiningRemoteUsers,						///< Waiting for fJoinRemote async operations to complete
			cStateRemovingRemoteUsers,						///< Waiting for fLeaveRemote async operations to complete
			cStateRegistering,								///< Waiting for fArbitrationRegister async operations to complete
			cStateStarting,									///< Waiting for fStart async operations to complete
			cStateStarted,									///< fStart async operations completed
			cStateWritingStats,								///< Waiting for fWriteStats async operations to complete
			cStateFlushingStats,							///< Waiting for fFlushStats async operations to complete
			cStateMigratingHost,							///< Waiting for fMigrateHost async operations to complete
			cStateEnding,									///< Waiting for fEnd async operations to complete
			cStateDeleting,									///< Waiting for fDelete async operations to complete
			
			cStateCount
		};

		// Creation flags
		static const u32 cCreateUsesPresence;				///< Session advertises what the user is doing
		static const u32 cCreateUsesStats;					///< Session uses the leaderboards
		static const u32 cCreateUsesMatchmaking;			///< Session uses matchmaking
		static const u32 cCreateUsesArbitration;			///< Session uses leaderboard/stats arbitration
		static const u32 cCreateUsesPeerNetwork;			///< ??? ("The session XNKEY is registered. Parental control settings are enforced.")
		static const u32 cCreateInvitesDisabled;			///< Session is single player
		static const u32 cCreateJoinViaPresenceDisabled;	///< Session can't be joined by anyone via presence
		static const u32 cCreateJoinViaPresenceFriendsOnly; ///< Session can't be joined by anyone but friends via presence
		static const u32 cCreateJoinInProgressDisabled;		///< Session can't be joined once started

		// Aggregates
		static const u32 cCreateSinglePlayerWithStats;		///< UsesPresence | UsesStats | InvitesDisabled | JoinViaPresenceDisabled | JoinInProgressDisabled
		static const u32 cCreateMultiplayerStandard;		///< UsesPresence | UsesStats | UsesMatchmaking | UsesPeerNetwork
		static const u32 cCreateMultiplayerRanked;			///< MultiplayerStandard | UsesArbitration
		static const u32 cCreateSystemLink;					///< UsesPeerNetwork
		static const u32 cCreateGroupLobby;					///< UsesPresence | UsesPeerNetwork
		static const u32 cCreateGroupGame;					///< UsesStats | UsesMatchmaking | UsesPeerNetwork

		typedef tDelegate<void ( tGameSession& gameSession, u32 oldState, b32 success )> tStateChangedCallback;

#ifdef platform_xbox360
		typedef ULONGLONG tNonce;
#else
		typedef u64 tNonce;
#endif

	public:

		static u32	fGetAddress( const tGameSessionInfo & info );		///< Converts info.hostAddress to IP address?
		static b32	fIsHostedByLocal( const tGameSessionInfo & info );
		static void fRegisterKey( const tGameSessionInfo & info );		///< Register session info key
		static void fUnregisterKey( const tGameSessionInfo & info );	///< Unregister session info key

	public:

		// Create a host session
		tGameSession( 
			u32 createFlags, // see cCreate*
			u32 maxPublicSlots,
			u32 maxPrivateSlots,
			tStateChangedCallback callback = tStateChangedCallback( ) );

		// Create a non-host session
		tGameSession( 
			u32 createFlags, // see cCreate*
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
		b32		fIsMigratingHost( ) const { return mState == cStateMigratingHost; }
		
		b32 fIsSameSession( const tGameSessionInfo & info ) const;	///< Just memcmps mData.mInfo vs info
		b32 fIsHost( ) const { return mIsHost; }					///< The local machine is hosting this session
		b32	fIsOnline( ) const;										///< Can other people join this game?
		b32 fIsArbitrated( ) const { return ( mCreateFlags & cCreateUsesArbitration ) != 0; }
		b32 fJoinInProgressEnabled( ) const { return ( mCreateFlags & ( cCreateJoinInProgressDisabled | cCreateUsesArbitration ) ) == 0; }
		u32 fCreateFlags( ) const { return mCreateFlags; }			///< Construction creation flags.

		// See tStateChangedCallback above for callback parameters.  Invoked on f{Push,Pop}State.
		void fSetStateChangedCallback( tStateChangedCallback callback ) { mStateChangedCallback = callback; }
		b32 fHasStateChangedCallback( ) { return !mStateChangedCallback.fNull( ); }

		/// Number used once. Host sends it to joining peers for secure arbitration
		tNonce fNonce( ) const; // Host only
		void fSetNonce( tNonce nonce ); // Non-host and state < cStateStarting

		u32 fAddress( ) const;										///< Get our host address
		std::string fName( ) const;									///< Hexadecimal display of the session ID

		u32 fLastError( ) const;

		// Slots
		u32 fTotalPublicSlots( ) const { return mSlots[ cSlotTotalPublic ]; }
		u32 fTotalPrivateSlots( ) const { return mSlots[ cSlotTotalPrivate ] ; }
		u32 fTotalSlots( ) const { return fTotalPublicSlots( ) + fTotalPrivateSlots( ); }
		u32 fFreePublicSlots( ) const { return mSlots[ cSlotTotalPublic ] - mSlots[ cSlotFilledPublic ]; }
		u32 fFreePrivateSlots( ) const { return mSlots[ cSlotTotalPrivate ] - mSlots[ cSlotFilledPrivate ]; }
		u32 fFreeSlots( ) const { return fFreePublicSlots( ) + fFreePrivateSlots( ); }
		u32 fFilledPublicSlots( ) const { return mSlots[ cSlotTotalPublic ]; }
		u32 fFilledPrivateSlots( ) const { return mSlots[ cSlotTotalPrivate ]; }
		u32 fFilledSlots( ) const { return fFilledPublicSlots( ) + fFilledPrivateSlots( ); }

		const tGameSessionDetails & fDetails( );

		const tGameSessionInfo & fSessionInfo( );

		f32 fStateTimer( ) const { return mStateTimer; }			///< Returns how long we've been in our current mState

		// Advance any state
		void fTick( f32 dt );

		// Wait till any pending operation completes
		void fWait( );
		
		// Creation
		b32 fCreate( u32 sessionOwnerId );														///< Creates the session at the XDK level.  sessionOwnerId == local hardware index of owner in XDK
		b32 fDelete( );																			///< Deletes the session at the XDK level.

		// Local Join / Leave
		b32 fJoinLocal( u32 index, b32 invited = false );										///< mState cState{Created,Started} -> pushes cStateJoiningLocalUsers
		b32 fJoinLocal( u32 userCount, const u32 indices[], b32 invited = false );				///< mState cState{Created,Started} -> pushes cStateJoiningLocalUsers
		b32 fJoinLocal( u32 userCount, const u32 indices[], const b32 invited[] );				///< mState cState{Created,Started} -> pushes cStateJoiningLocalUsers

		b32 fLeaveLocal( u32 index );															///< mState cState{Created,Started} -> pushes cStateRemovingLocalUsers
		b32 fLeaveLocal( u32 userCount, const u32 indices[] );									///< mState cState{Created,Started} -> pushes cStateRemovingLocalUsers
		b32 fLeavelAllLocal( u32 & localUserCount );											///< mState cState{Created,Started} -> pushes cStateRemovingLocalUsers

		// Zombies, to adjust slot counts without adding/removing to session
		b32 fJoinZombie( u32 count );	// Sync operation										///< mState cState{Created,Started} -> no change
		b32 fLeaveZombie( u32 count );	// Sync operation										///< mState cState{Created,Started} -> no change

		// Remote Join / Leave
		b32 fJoinRemote( const tPlatformUserId & user, b32 invited = false );					///< mState cState{Created,Started} -> pushes cStateJoiningRemoteUsers
		b32 fJoinRemote( u32 userCount, const tPlatformUserId users[], b32 invited = false );	///< mState cState{Created,Started} -> pushes cStateJoiningRemoteUsers
		b32 fJoinRemote( u32 userCount, const tPlatformUserId users[], const b32 invited[] );	///< mState cState{Created,Started} -> pushes cStateJoiningRemoteUsers

		b32 fLeaveRemote( const tPlatformUserId & user );										///< mState cState{Created,Started} -> pushes cStateRemovingRemoteUsers
		b32 fLeaveRemote( u32 userCount, const tPlatformUserId users[] );						///< mState cState{Created,Started} -> pushes cStateRemovingRemoteUsers

		// Arbitration - must be called after all players are joined, before starting, for each session
		// with the host making the last call
		// See XDK documentation.  This has to do with ensuring leaderboard accuracy by making sure players agree what everyone's stats were.
		b32 fArbitrationRegister( );
		b32 fGetArbitrationResults( tGrowableArray< tPlatformUserId > & out ) const;

		// Run controls
		b32 fStart( );
		b32 fStuffToFlush( ) { return fIsStarted( ) && mViewsWrittenTo[ 0 ] != ~0; }
		b32 fNoQueuedWrites( ) { return fIsStarted( ) && !mQueuedWrites.fNumItems( ); }
		b32 fEnd( );

		// Stats
		void fClearQueuedWrites( ) { mQueuedWrites.fReset( ); }
		void fClearViewsWrittenTo( ) { mViewsWrittenTo.fFill( ~0 ); }
		void fProcessStatsWriteQueue( );
		void fKillQueuedWritesForUser( tPlatformUserId userId );
		void fQueueWriteStats( tPlatformUserId userId, u32 writeCount, const tGameSessionViewProperties writes[] );
	private:
		//b32 fWriteStats( tPlatformUserId userId, u32 leaderBoardId, u32 propCount, const tUserProperty props[] );
		b32 fWriteStats( tPlatformUserId userId, u32 writeCount, const tGameSessionViewProperties writes[] );	///< Writes XDK leaderboard stats (cached until XSessionEnd or flushed)
	public:
		void fFlushQueuedStats( );
	private:
		b32 fFlushStats( );																						///< Flushes XDK leaderboard stats before session end.
	public:

		// Quality of Service Listener
		void fSetQosListenData( const void* data, u32 dataSize );
		void fEnableQosListen( );
		void fDisableQosListen( );

		// Host migration
		b32 fMigrateHostLocal( u32 index );							///< mState cStateCreated -> cStateMigratingHost.  Migrates the host to the specified local user.
		b32 fMigrateHostRemote( tGameSessionInfo& sessionInfo );	///< mState cStateCreated -> cStateMigratingHost.  Migrates the host to a remote user.

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
			void fCancelOverlap( );
			

			HANDLE			mHandle;
			XOVERLAPPED		mOverlapped;

			tGameSessionInfo	mInfo;
			DWORD				mFlags;
			tNonce				mNonce;
			DWORD				mLastError;
			tDynamicBuffer		mRegistrationResults;
			tDynamicBuffer		mQosListenData;
		};

		class tGetDetailsOp : public XtlUtil::tOverlappedOp
		{
		public:
			b32 fCreate( HANDLE sessionHandle );
			b32 fGetDetails( tGameSessionDetails& detailsOut, b32 wait = false );

			b32 fValid( ) const;

		private:
			XSESSION_LOCAL_DETAILS mDetails;
			DWORD mSize;
		};

#else

		struct base_export tGameSessionData
		{
			tGameSessionData( );

			tGameSessionInfo	mInfo;

			u32					mCreatorHwIndex;

			tNonce				mNonce;
			u32					mFlags;
		};

		class tGetDetailsOp
		{
		public:
			b32 fGetDetails( tGameSessionDetails& detailsOut, b32 wait = false ) { return false; }
			void fReset( ) { }

			b32 fValid( ) const { return false; }
		};

#endif

		struct tSessionUser
		{
			union
			{
				tPlatformUserId mUserId;
				u32 mLocalHwIndex;
			};

			b16 mInPrivateSlot;
			b16 mIsLocal;
		};

		struct tStatsWrite : public tRefCounter
		{
			tPlatformUserId mUserId;
			u32 mViewId;
			tDynamicArray< tUserProperty > mProperties;
		};
		typedef tRefCounterPtr< tStatsWrite > tStatsWritePtr;

	private:
		tState fSuccessNextState( ) const;										///< Returns the next value mState will transition to, if the operation completes successfully.
		void fAdvanceState( b32 wait );											///< mState cState{*} -> cState{*}.  Checks overlapped IO, pushes a successful state depending 
		b32 fOperationPending( ) const;											///< Returns true if the current mState implies there may be incomplete operations.  Combine with fOperationComplete?
		b32 fOperationComplete( b32 & success, b32 wait );						///< Checks the overlapped IO for completion, success.  Will block until completion if "wait" is true.

		void fResetSlots( u32 maxPublic, u32 maxPrivate );						///< mState cStateNull -> no change.  Clears mUsers, sets slot availability.
		b32  fTryFillSlots( u32 & additionalPublic, u32 & additionalPrivate );	///< Increment mSlots[*] if possible.  Will try to use more public slots if said private slots aren't available, but not vicea versa.
		
		void fPushState( tState newState );										///< mState cState{*} -> newState.    Invokes fOnLeave*( true  ), indicating a success (but usually in the simple sense of 'we've successfully begun to try xyz' sense), and mStateChangedCallback.
		void fPopState( );														///< mState cState{*} -> mPrevState.  Invokes fOnLeave*( false ), indicating a failure, and mStateChangedCallback.  Will pop at most one state.

		/// Self explanitory -- invoked exclusively by f{Push,Pop}State.
		void fOnLeaveCreating( b32 success );
		void fOnLeaveJoiningLocal( b32 success );
		void fOnLeaveRemovingLocal( b32 success );
		void fOnLeaveJoiningRemote( b32 success );
		void fOnLeaveRemovingRemote( b32 success );
		void fOnLeaveDeleting( b32 success );

		b32 fMigrateHost( u32 index, tGameSessionInfo& sessionInfo );			///< Migrates the host. tUser::cUserIndexNone indicates a remote user.

		void fCloseSessionData( );												///< mState cStateNull -> no change.  Closes the session handle.

		void fQueryDetails( );													///< mState cState{Created,Started} -> no change.  Updates mDetails with XDK session details.

		b32 fCheckDetailState( u32 targetState );								///< Used by fAdvanceState to determine success.  returns (mState==cStateCreated && eState==XSESSION_STATE_LOBBY) || (mState==cStateNull && eState==XSESSION_STATE_DELETED);

	private:
		
		tState mState;															///< Current state.
		tState mPrevState;														///< The previous state as used by f{Push,Pop}State.
		f32 mStateTimer;														///< Time spent in the current state.

		b32 mIsHost;															///< Is this machine hosting this session
		u32 mCreateFlags;														///< Creation flags
		tFixedArray<u32, cSlotCount> mSlots;									///< The number of slots available for each slot category.

		tGameSessionData mData;													///< Platform-specific data.
		tGrowableArray<byte> mOperationData;									///< Temporary buffer used in join/leave/stats operations.
		tGrowableArray<tSessionUser> mUsers;									///< List of local and remote users for this session

		tStateChangedCallback mStateChangedCallback;							///< mState changed callback invoked by f{Push,Pop}State
		tGameSessionDetails mDetails;											///< Game session metadata
		tGetDetailsOp mDetailsOp;

		tFixedArray< u32, 5 > mViewsWrittenTo; // Five is from the XDK docs
		tRingBuffer< tStatsWritePtr > mQueuedWrites;
	};

	typedef tRefCounterPtr<tGameSession> tGameSessionPtr;

}

#endif//__tGameSessionBase__
