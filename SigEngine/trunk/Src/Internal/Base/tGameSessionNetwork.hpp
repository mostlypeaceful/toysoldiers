//------------------------------------------------------------------------------
// \file tGameSessionNetwork.hpp - 15 Oct 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tGameSessionNetwork__
#define __tGameSessionNetwork__
#include "Net/tP2PNetwork.hpp"
#include "Net/tRemoteConnection.hpp"
#include "Net/tVoiceNetwork.hpp"
#include "tGameSession.hpp"
#include "tGameSessionSearch.hpp"
#include "tLanGameSessionSearch.hpp"

namespace Sig
{
	/// 
	/// \brief Wraps tGameSession functionality with a net protocol
	class base_export tGameSessionNetwork : private Net::tP2PNetworkHost
	{
	public:

		enum tState
		{
			cStateInactive = 0,
			cStateConnecting,
			cStateSettingUpUser,
			cStateCreatingSession,
			cStateJoiningLocalUsers,
			cStateRemovingLocalUsers,
			cStateJoiningRemoteUsers,
			cStateRemovingRemoteUsers,
			cStateReady,
			cStateStarting,
			cStateRunning,
			cStateMigratingHost,
			cStateStopping,
			cStateDestroying,
			cStateRestarting,
		};

		class tOwner
		{
		public:
			virtual ~tOwner( ) { }
			virtual void fOnGameMessage( 
				const Net::tPeer & peer, 
				u32 channelId, 
				const byte * data, 
				u32 dataLength ) = 0;

			virtual void fOnReady( ) = 0;
			virtual void fOnRejected( ) = 0;
			virtual void fOnUserJoined( const Net::tPeer & peer, const tUserPtr& user ) = 0;
			virtual void fOnUserLeft( const Net::tPeer & peer, const tUserPtr& user ) = 0;
			virtual void fOnUsersChanged( ) = 0;
			virtual void fOnSessionFull( ) = 0;
			virtual void fOnRunning( ) = 0;
			virtual void fOnStopped( ) = 0;
			virtual void fOnDestroyed( ) = 0;

			virtual void fOnConnect( const Net::tPeer & peer ) = 0;
			virtual void fOnDisconnect( const Net::tPeer & peer, u32 reason ) = 0;
			virtual void fOnError( u32 failedState ) = 0;
			virtual void fOnStatsLost( ) = 0;

			///
			/// \param customDataOut Should contain the same data passed to tGameSessionNetwork::fHost( )
			virtual void fOnHostMigrated( tDynamicBuffer& customDataOut ) = 0;
		};

	public:

		tGameSessionNetwork( u32 gameChannelCount )
			: mChannelId( gameChannelCount )
			, mState( cStateInactive )
			, mOwner( NULL )
			, mInvited( false )
			, mMaxPeers( 15 ) 
			, mIsSystemLink( false )
			, mHostMigrationPending( false )
			, mNatType( Net::cNatTypeInvalid )
		{
		}

		void fSetOwner( tOwner * owner ) { mOwner = owner; }

		u32 fMaxPeers( ) const { return mMaxPeers; }
		void fSetMaxPeers( u32 maxPeers ) { mMaxPeers = maxPeers; }

		tState	fState( ) const { return mState; }
		b32		fIsInactive( ) const { return mState == cStateInactive; }
		b32		fIsRunning( ) const { return mState == cStateRunning; }
		b32		fIsReady( ) const { return	mState == cStateReady; }
		b32		fIsMigratingHost( ) const { return mState == cStateMigratingHost; }
		b32		fIsReadyAndFull( ) const; // fIsReady && Session is full
		b32		fIsReadyAndIdle( ) const; // fIsReady && no pending join/leave operations
		b32		fIsHost( ) const;
		b32		fIsConnected( ) const;
		b32		fIsOnline( ) const;
		b32		fIsSystemLink( ) const { return mIsSystemLink; }
		b32		fIsRunningWithConnections( ) const;

		Net::tNatType fNatType( ) const { return mNatType; }

		b32		fLongStateTime( f32 longTime ) const;

		b32 fHasSession ( ) const { return !mSession.fNull( ); }
		b32 fIsSameSession( const tGameSessionInfo& info ) const;
		std::string fSessionName( ) const { sigassert( mSession ); return mSession->fName( ); }
		u32 fSessionCreateFlags( )  const { sigassert( mSession ); return mSession->fCreateFlags( ); }
		const tGameSessionDetails & fSessionDetails( ) const { sigassert( mSession ); return mSession->fDetails( ); }
		const tUserPtr & fHostUser( ) const { return mHostUser; } // Possibly remote
		const tUserPtr & fOwnerUser( ) const { return mOwnerUser; } // Always local
		
		u32 fLocalUserCount( ) const { return mLocalUsers.fCount( ); }
		const tUserPtr & fLocalUser( u32 i ) const { return mLocalUsers[ i ].mUser; }

		u32 fRemoteUserCount( ) const { return mRemoteUsers.fCount( ); }
		const tUserPtr & fRemoteUser( u32 i ) const { return mRemoteUsers[ i ].mUser; }

		u32 fConnectionCount( ) const { return mConnections.fCount( ); }
		const Net::tPeer& fFindPeerByConnectionIndex( u32 connection ) const { return mConnections[ connection ].mPeer; }

		const Net::tPeer* fFindPeer( tPlatformUserId userId );
		b32 fGetUsersByPeer( tGrowableArray< tUserPtr > & out, const Net::tPeer & peer );

		///
		/// \brief Access to our local tPeerId
		Net::tPeerId fLocalPeerId( ) const { return mNetwork.fLocalPeerId( ); }

		// Creates session
		b32 fHost(
			tUser & user,
			u32 sessionCreateFlags,
			u32 publicSlots,
			u32 privateSlots,
			const void* customData = NULL,
			u32 customDataSize = 0 );
		b32 fJoin( tUser & user, const tGameSessionSearchResult & result, b32 usePrivateSlot = false );
		b32 fJoin( tUser & user, const tGameInvite & invite );
		b32 fJoin( tUser & user, u32 hostAddr, u32 hostPort );

		// Zombie users act like regular users, but don't get added to the underlying game session
		b32 fAddLocalUser( tUser & user, b32 addAsZombie ); // Assumes local users are invited
		b32 fRemoveLocalUser( tUser & user );

		u32 fFilledSlots( ) const { sigassert( mSession ); return mSession->fFilledSlots( ); }

		void fWriteStats( tPlatformUserId userId, u32 writeCount, const tGameSessionViewProperties writes[] );
		void fFlushStats( );
	
		void fWait( );
		void fTick( f32 dt );

		b32 fStart( );	// starts session - reports success with fOnRunning
		void fEnd( );	// ends session - reports success with fOnStopped
		void fDestroy( ); // destroys session, - reports success with fOnDestroyed
		void fRestart( ); // ends the session, and then restarts it - reports success with fOnRunning
		void fCancelAndLeave( );

		void fSendMsg( const Net::tPeer & peer, u32 channelId, const byte * data, u32 dataLength, b32 reliable );
		void fSendMsgToPeers( u32 channelId, const byte * data, u32 dataLength, b32 reliable );
		void fSendMsgToHost( u32 channelId, const byte * data, u32 dataLength, b32 reliable );
		void fFlush( );

		// Returns true if the session network can continue, false otherwise
		b32 fOnUserSignInChange(
			const tUserSigninInfo oldStates[], 
			const tUserSigninInfo newStates[] );
		void fOnMuteListChanged( );

		inline u32 fAverageRTT( ) const { return mNetwork.fAverageRTT( ); }
		inline f32 fAveragePacketLoss( ) const { return mNetwork.fAveragePacketLoss( ); }

		void fLogStats( std::wstringstream& statsText );

	private:

		void fSetState( tState state );
		b32	 fJoin( tUser & user, const tGameSessionInfo & info );
		void fClearData( );
		void fKillSession( b32 canDelete );
		b32 fAdvanceDeadSession( tGameSession & gameSession );
		void fProcessAddRemoveLists( );
		void fProcessStatsWriteQueue( );
		void fKillQueuedWritesForUser( tPlatformUserId userId );
		s32 fActiveSlots( ) const;
		b32 fHasFreeSlots( u32 count ) const;
		b32 fHasFreePublicSlots( u32 count ) const;
		void fRejectPeer( const Net::tPeer & peer, u32 reason );
		void fUpdateNatType( );

		void fHandleDeadSessionStateChange( tGameSession & gameSession, u32 oldState, b32 success );
		void fHandleSessionStateChange( tGameSession& gameSession, u32 oldState, b32 success );
		void fHandleSessionCreating( b32 succes );
		void fHandleSessionEnding( b32 success );
		void fHandleSessionJoiningLocalUsers( b32 success );
		void fHandleSessionJoiningRemoteUsers( b32 success );
		void fHandleSessionRemovingLocalUsers( b32 success );
		void fHandleSessionRemovingRemoteUsers( b32 success );
		void fHandleSessionStarting( b32 success );
		void fHandleSessionRegistering( b32 success );
		void fHandleSessionDeleting( b32 success );
		void fHandleSessionWritingStats( b32 success );
		void fHandleSessionFlushingStats( b32 success );
		void fHandleSessionMigratingHost( b32 success );
		

		void fHandleJoinRequestPacket( const byte * data, u32 dataLength, const Net::tPeer & peer );
		void fHandleJoinAcceptedPacket( const byte * data, u32 dataLength, const Net::tPeer & peer );
		void fHandleJoinRejectedPacket( const byte * data, u32 dataLength, const Net::tPeer & peer );
		void fHandlePeerInfoPacket( const byte * data, u32 dataLength, const Net::tPeer & peer );
		void fHandleArbitratedPacket( const byte * data, u32 dataLength, const Net::tPeer & peer );
		void fHandleVoiceMutePacket( const byte * data, u32 dataLength, const Net::tPeer & peer );
		void fHandleHostMigratedPacket( const byte * data, u32 dataLength, const Net::tPeer & peer );

		// tP2PNetworkHost
		virtual void fOnReceive( 
			Net::tP2PNetwork & network, 
			const Net::tPeer & peer, 
			u32 channelId,
			const byte * data, 
			u32 dataLength );
		virtual void fOnConnect( Net::tP2PNetwork & network, const Net::tPeer & peer );
		virtual void fOnDisconnect( Net::tP2PNetwork & network, const Net::tPeer & peer, u32 reason );
		virtual void fOnJoinFailure( Net::tP2PNetwork & network );
		virtual void fOnConnectAttempt( Net::tP2PNetwork & network, const Net::tPeer & peer );
		virtual void fOnBeginHosting( Net::tP2PNetwork & network );
		virtual u32 fGetAddress( const Net::tPlatformAddr& platformAddr );
		virtual u16 fGetPort( const Net::tPlatformAddr& platformAddr );
		virtual b32 fGetPlatformAddr( Net::tPlatformAddr& out, u32 address, u32 port );

	private:
		
		struct tSessionUser
		{
			tSessionUser( ) : mInvited( false ), mIsZombie( false ) { }
			tSessionUser( tUser & user, b32 invited, b32 zombie = false ) 
				: mUser( &user ), mInvited( invited ), mIsZombie( zombie ) { }

			b32 operator ==( tUser * user ) const
			{
				return mUser == user;
			}

			tUserPtr mUser;
			b16 mInvited;
			b16 mIsZombie;
		};

		///
		/// \brief tSessionUserArray
		class tSessionUserArray : public tGrowableArray< tSessionUser >
		{
		public:
			b32 fJoinLocal( tGameSession & session );
			b32 fLeaveLocal( tGameSession & session );
			b32 fJoinRemote( tGameSession & session );
			b32 fLeaveRemote( tGameSession & session );

			tSessionUser* fFindByUserId( const tPlatformUserId& userId );
		};

		struct tConnection
		{
			tConnection( ) : mArbitrated( false ), mCanHear( false ), mNatType( Net::cNatTypeInvalid )
			{ }

			b32 operator ==( tUser * user ) const
			{
				return mUsers.fFind( user ) != NULL;
			}

			b32 operator ==( const Net::tPeer & peer ) const
			{
				return peer == mPeer;
			}

			b32 mArbitrated;
			b32 mCanHear;
			Net::tPeer mPeer;
			tSessionUserArray mUsers;
			tDynamicBuffer mMuteMsg;
			Net::tNatType mNatType;
		};

		class tConnectionArray : public tGrowableArray< tConnection >
		{
		public:

			b32 fArbitrated( ) const;
		};

	private:

		u32							mMaxPeers;
		u32							mChannelId;
		b32							mInvited;
		tState						mState;
		tGameSessionPtr				mSession;
		tGameSessionInfo			mJoinInfo;
		Net::tP2PNetwork 			mNetwork;
		Net::tVoiceNetworkPtr		mVoiceNetwork;
		tOwner *					mOwner;
		b32							mIsSystemLink;
		b32							mHostMigrationPending;
		tGameSessionInfo			mMigrationJoinInfo;
		tLanGameSessionAdvertiser	mLanAdvertiser;
		Net::tNatType				mNatType;
		
		tUserPtr	mHostUser; // Host of the session
		tUserPtr	mOwnerUser; // Owner of the session
		
		tSessionUserArray	mLocalToAdd;
		tSessionUserArray	mRemoteToAdd;
		tSessionUserArray	mLocalToRemove;
		tSessionUserArray	mRemoteToRemove;

		tSessionUserArray	mPendingAddOrRemove;

		tSessionUserArray	mLocalUsers;
		tSessionUserArray   mRemoteUsers;
		tConnectionArray	mConnections;

		tGrowableArray< tGameSessionPtr > mDeadSessions;
	};
}

#endif//__tGameSessionNetwork__
