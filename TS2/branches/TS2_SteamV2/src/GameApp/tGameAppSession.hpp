#ifndef __tGameAppSession__
#define __tGameAppSession__
#include "GameNetMessages.hpp"
#include "tGameSessionNetwork.hpp"
#include "tReplay.hpp"
#include "tUserProfile.hpp"
#include "Gui/tText.hpp"
#include "tNetUI.hpp"

namespace Sig { namespace Net
{
	class tRemoteConnection;
}}

namespace Sig
{
	class tGameApp;
	class tGameSession;
	class tGameSessionSearch;
	class tGameArchiveLoad;
	class tPlayer;

	///
	/// \class tGameAppSession
	/// \brief 
	class tGameAppSession : private tGameSessionNetwork::tOwner
	{

	public:

		// What should the game app session do on front end load
		// Defaults to cDestroySession and reset to it after every front end load
		enum tFrontEndLoadBehavior
		{
			cFrontEndLoadBehaviorDestroySession = 0,
			cFrontEndLoadBehaviorEndSession = 1,
		};

	public:

		tGameAppSession( );
		~tGameAppSession( );

		void fInitialize( );

		void fOnAppTick( f32 dt );
		void fPollInputDevices( tUserArray & users );
		void fAddToStatText( std::stringstream & statText );

		void fOnLoadLevel( b32 alreadyLoading );
		void fOnLevelUnloadBegin( );
		void fOnLevelLoadBegin( );
		void fOnLevelSpawnBegin( );
		void fOnLevelSpawnEnd( );
		void fOnInGameSimulationState( b32 inSim );

		void fLoadReplay( const tFilePathPtr & path );

		// Game searching
		void fBeginGameSearch( tUser& user, u32 gameType, u32 gameMode );
		void fCancelGameSearch( );
		
		bool fIsHost( ) const;
		bool fGameSessionFailure( ) const;
		void fCancelSession( );
		void fCreateSessionForPreview( );

		// Access to the session users
		u32		fLocalUserCount( ) const;
		tUser * fLocalUser( u32 i ) const;
		u32		fRemoteUserCount( ) const;
		tUser *	fRemoteUser( u32 i ) const;

		void	fAddLocalUserToSession( tUser * user );
		void	fAddLocalZombieToSession( tUser * user );
		void    fRemoveLocalUserFromSession( tUser * user );

		b32		fIsOnline( ) const;
		b32		fIsInactive( ) const { return mGameSessionNetwork.fIsInactive( ); }

		// Signal to mark the sync frame
		void fSyncFrame( );

		u32 fFirstJoinableSession( ) const; // returns ~0 if no valid/joinable session was found
		void fJoinRemoteSessionFromSearchIndex( tUser& user, u32 sessionIndex );
		void fJoinSessionFromInvite( const tGameInvite & invite );

		void fStartQuickMatchMap( u32 mapType, u32 mapIndex, Sqrat::Function& levelLoadInfoCallback );
		void fStartMap( u32 mapType, u32 mapIndex, Sqrat::Function& loadLevelInfoCallback );

		void fAddGameModeFlags( tLevelLoadInfo& info );

		void fRestartMap( );
		void fStartNextMap( );
		void fQueryRemoteUsers( tUserArray & out );

		void fOnUserSignInChange( 
			const tUserSigninInfo oldStates[], 
			const tUserSigninInfo newStates[] );

		void fOnMuteListChanged( );
		b32  fCanAcceptInvite( ) const;
		void fOnGameInviteAccepted( );
		void fOnGameInviteRejected( u32 localHwIndex );
		void fOnGameInviteNeedFullVer( );

		void fQueueErrorDialog( u32 error, tPlatformUserId userId );

		void fWriteStats( tPlayer* player, u32 viewId, u32 propCount, const tUserProperty props[] );

		inline u32 fAverageRTT( ) const { return mGameSessionNetwork.fAverageRTT( ); }
		inline f32 fAveragePacketLoss( ) const { return mGameSessionNetwork.fAveragePacketLoss( ); }

		u32 fGetLevelLeaderboardId( u32 mapType, u32 levelIndex, u32 difficulty ) const;
		void fWriteLossForOtherPlayer( ); //this is only used when a player drops out of a net game.

		void fSendClientStateChange( Net::tClientStateChange& clientStatus );
		void fSendLevelSelectStatus( Net::tLevelSelectStatus& levelStatus );

		// Defaults to cDestroySession after every front end load
		void fSetFrontEndLoadBehavior( u32 behavior ) { mFrontEndLoadBehavior = behavior; }

		typedef tFixedGrowingArray< tGrowableArray<tUserProperty>, (GameFlags::cMAP_TYPE_COUNT * GameFlags::cDIFFICULTY_COUNT + 25)> tScoreProps;
		void fCreateScoreWrites( tLevelScores* scores, u32 difficulty, const tPlayer* player, tScoreProps& props, tGrowableArray<tGameSessionViewProperties>& writes, b32 fromSession );
		void fWriteStatsToRightPlace( tPlayer* player, u32 writeCount, const tGameSessionViewProperties writes[] );

		void fWriteLeaderboards( );
		void fApplyProfiles( );

		b32 fLoadingLevel( ) const { return mLoadingLevel; }
		u32 fMiniGameScoreId( );

		// Coop Net Game Rewind
		void fNetRewindBegin( );
		void fRewindMap( tUser* user, u32 waveIndex );

		// Player access
		tPlayer* fGetHostPlayer( ) const;
		tPlayer* fGetClientPlayer( ) const;

	protected:

		void fHostOnlineGame( tUser& user, u32 gameMode, u32 gameType );

	private:

		struct tClientData
		{
			tClientData( ) : mUserId( tUser::cInvalidUserId ) { }
			tClientData( const tPlatformUserId & id ) : mUserId( id ) { mProfile.fReset( NEW tUserProfile( ) ); }
			b32 operator==( const tPlatformUserId & id ) const { return mUserId == id ; }

			tPlatformUserId mUserId;
			tUserProfilePtr mProfile;
			tDynamicArray<byte> mGameControllerConfigData;
		};

		struct tClientInput
		{
			tClientInput( ) : mUserId( tUser::cInvalidUserId ) { }
			
			tPlatformUserId mUserId;
			tRingBuffer< Net::tInputFrame > mFrames;
		};

		struct tSyncData
		{	
			tRingBuffer< Net::tSyncCheck * > mLocalSyncs;
			tRingBuffer< Net::tSyncCheck * > mRemoteSyncs;
		};

		struct tSessionEvent
		{
			u32 mEventId;
			u32 mContext;
		};

	private:

		void fWriteStatFromScript( tPlayer * player, u32 viewId, u32 propId, u32 value );

		void fLoadMapForNetGame( tLevelLoadInfo info );
		void fReloadMapForNetGame( );
		void fLoadNextMapForNetGame( );
		void fCreateSinglePlayerSession( tUser & user );
		void fCreateJoinSession( tUser & user, u32 resultIndex );

		void fCreatePlaycorder( );
		void fRecordInputFrame( );

		u32 fIntersectedAddOns( ) const;
		u32 fRandomQuickMatchLevelIndex( u32 mapType );

	public:
		void fFlushStats( );
		u32 fGameMode( ) const;
		u32 fGameType( ) const;

	private:
		// Lock-step input 
		b32 fReadyForInput( ) const;
		b32 fHasAllInputForThisFrame( ) const;
		void fTryCaptureInputFrame( tUserArray & localUsers );
		void fTryApplyInputFrame( tUserArray & localUsers );
		void fResetInput( );
		void fResetSyncData( );
		void fNullifySessionPads( );
		b32 fHasAllProfiles( ) const;
		void fSendProfiles( );

		bool fAllPeersReadyForLevelStart( );

		tClientInput * fFindClientInput( tPlatformUserId userId );
		const tClientInput * fFindClientInput( tPlatformUserId userId ) const;
		void fPushInputFrame( const Net::tInputFrame & frame );
		void fPopInputFrame( Net::tInputFrame & frame );
		void fMarkReadyForInput( tPlatformUserId userId );
		void fPushSyncFrame( Net::tSyncCheck * syncFrame, b32 local );
		void fLoadFromSave( const byte * data, u32 dataLength );

		// tGameSessionNetwork::tOwner
		virtual void fOnGameMessage( const Net::tPeer & peer, u32 channelId, const byte * data, u32 dataLength );
		virtual void fOnReady( );
		virtual void fOnRejected( );
		virtual void fOnUsersChanged( );
		virtual void fOnSessionFull( );
		virtual void fOnRunning( );
		virtual void fOnStopped( );
		virtual void fOnDestroyed( );
		virtual void fOnDisconnect( );
		virtual void fOnError( u32 failedState );
		virtual void fOnStatsLost( );
		virtual b32  fIsCompatible( );

		void fReplayGameSessionStateChanged( tGameSession & gameSession, u32 oldState, b32 success );

		// Message handlers
		void fHandleStartMapMsg( const Net::tPeer& peer, u32 channelId, const byte * data, u32 dataLength );
		void fHandleLoadFromSaveMsg( const Net::tPeer& peer, u32 channelId, const byte * data, u32 dataLength );
		void fHandleReadyForInputMsg( const Net::tPeer & peer, u32 channelId, const byte * data, u32 dataLength );
		void fHandlePlayerInfo( const Net::tPeer & peer, u32 channelId, const byte * data, u32 dataLength );
		void fHandleInputMsg( const Net::tPeer & peer, u32 channelId, const byte * data, u32 dataLength );
		void fHandleSyncCheckMsg( const Net::tPeer & peer, u32 channelId, const byte * data, u32 dataLength );
		void fHandleDesyncMsg( const Net::tPeer & peer, u32 channelId, const byte * data, u32 dataLength );
		void fHandleLobbyMenuStateChange( const Net::tPeer & peer, u32 channelId, const byte * data, u32 dataLength );
		void fHandleLobbyPlayerStateChange( const Net::tPeer & peer, u32 channelId, const byte * data, u32 dataLength );

		void fDrawStats( ) const;

	public: //shhh, this is a secret public function
		void fSendSessionEvent( u32 eventId, u32 context );

	private:

		b32									mWaitingForProfiles;
		b32									mStartingReplaySession;
		tRefCounterPtr<tGameSessionSearch>  mGameSessionSearch;
		tGameSessionNetwork					mGameSessionNetwork;

		tFixedArray<tRandom, GameFlags::cMAP_TYPE_COUNT> mQuickMatchMapRand;

		u32 mObjectiveSeed;		
		u32 mApplyInputFrame;
		u32 mCaptureInputFrame;
		u32 mFrontEndLoadBehavior;
		b32	mLoadingLevel;
		b32 mLevelUnloaded;
		b32 mHasAllInput;
		
		tPlaycorderPtr	mPlaycorder;
		tReplayerPtr	mReplayer;

		Gui::tNetUIPtr	mNetUI;

		tGrowableArray< tClientData > mClientData;
		tGrowableArray< tClientInput > mInput;
		tGrowableArray< tSessionEvent > mQueuedSessionEvents;
		tSyncData mSyncData;
		b32 mIsQuickMatch;

		struct tErrorDialogInfo
		{
			u32 mErrorCode;
			tPlatformUserId mUserId;
		};

		tGrowableArray<tErrorDialogInfo> mErrorPrequeue;
		tGrowableArray<tErrorDialogInfo> mErrorQueue;


	public:
		static void fExportScriptInterface( tScriptVm& vm );
	};
}

#endif//__tGameAppSession__
