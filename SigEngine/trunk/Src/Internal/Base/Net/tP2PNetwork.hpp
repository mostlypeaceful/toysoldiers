//------------------------------------------------------------------------------
// \file tP2PNetwork.hpp - 12 Oct 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tP2PNetwork__
#define __tP2PNetwork__
#include "tHost.hpp"
#include "tQueue.hpp"

typedef struct _ENetPacket ENetPacket;

namespace Sig { namespace Net
{
	class tP2PNetwork;

	///
	/// \class tP2PNetworkPeer
	/// \brief 
	class tP2PNetworkPeer
	{
	public:
		virtual ~tP2PNetworkPeer( ) { }
		virtual void fOnReceive( 
			tP2PNetwork & network, 
			const tPeer & peer, 
			u32 channelId,
			const byte * data, 
			u32 dataLength ) = 0;

		virtual void fOnConnect( tP2PNetwork & network, const tPeer & peer ) = 0;
		virtual void fOnDisconnect( tP2PNetwork & network, const tPeer & peer, u32 reason ) = 0;
		virtual void fOnJoinFailure( tP2PNetwork & network ) = 0;
		virtual void fOnBeginHosting( tP2PNetwork & network ) = 0;

		virtual u32 fGetAddress( const tPlatformAddr& platformAddr ) = 0;
		virtual u16 fGetPort( const tPlatformAddr& platformAddr ) = 0;
		virtual b32 fGetPlatformAddr( tPlatformAddr& out, u32 address, u32 port ) = 0;
	};

	///
	/// \class tP2PNetworkHost
	/// \brief 
	class tP2PNetworkHost : public tP2PNetworkPeer
	{
	public:

		virtual void fOnConnectAttempt( tP2PNetwork & network, const tPeer & peer ) = 0;
	};


	///
	/// \class tP2PNetwork
	/// \brief 
	class tP2PNetwork
	{
	public:

		enum tState
		{
			cStateInactive = 0,
			cStateHosting,
			cStateConnecting,
			cStateConnected,
			cStateDisconnecting,
			cStateMigratingHost
		};

		enum tDisconnectReason
		{
			cDisconnectReasonLeaving = 0,
			cDisconnectReasonRejected,
			cDisconnectReasonDisconnecting,
			cDisconnectReasonWeakTimeout,
			cDisconnectReasonRejoined,
			cDisconnectReasonServerTimeout,
			cDisconnectReasonServerDisconnect,
			cDisconnectReasonServerInstructed
		};

#if defined( sig_devmenu )
		struct tStats
		{
			u32 mPacketsSent;
			u32 mPacketsReceived;
			u32 mBytesSent;
			u32 mBytesReceived;

			tStats( ) { fReset( ); }

			void fReset( )
			{
				mPacketsSent = 0;
				mPacketsReceived = 0;
				mBytesSent = 0;
				mBytesReceived = 0;
			}
		};

		const tStats& fStats( ) const { return mStats; }
#endif

	public:

		tP2PNetwork( );

		// State helpers
		tState	fState( ) { return mState; }
		b32		fIsActive( ) const { return mState != cStateInactive; }
		b32		fIsConnected( ) const { return mState == cStateConnected || mState == cStateMigratingHost; }
		b32		fIsHosting( ) const { return mState == cStateHosting; }
		b32		fIsConnectedOrHosting( ) const { return fIsConnected( ) || fIsHosting( ); }
		b32		fIsMigratingHost( ) const { return mState == cStateMigratingHost; }
		

		///
		/// \brief Host a network where others can connect
		b32 fHost( 
			tP2PNetworkHost * userHost,
			u32 maxPeerCount,
			u32 channelCount,
			u32 hostAddress = tHost::cDefaultAddress, 
			u32 hostPort = tHost::cDefaultPort );

		///
		/// \brief Join a pre-existing network at the specified addy
		b32 fJoin(
			tP2PNetworkPeer * userPeer,
			u32 maxPeerCount,
			u32 channelCount,
			u32 hostAddress,
			u32 hostPort = tHost::cDefaultPort,
			u32 clientAddress = tHost::cDefaultAddress,
			u32 clientPort = tHost::cDefaultPort );

		/// 
		/// \brief Gently disconnects from all peers and shuts down the host on next service
		void fLeave( b32 afterOutgoing = true );

		///
		/// \brief Disconnect from all peers now and shuts down the host now
		void fLeaveNow( );

		///
		/// \brief Handles any connection events including processing incoming packets
		void fService( u32 timeout = 0 );

		///
		/// \brief Access to the host peer - pointer is not persistant across tP2PNetwork function calls
		tPeer *	fHostPeer( );

		///
		/// \brief Access to our local tPeerId
		tPeerId fLocalPeerId( ) const { return mLocalPeerId; }

		///
		/// \brief Accept a peer that has connected to the host
		void fAcceptPeer( tPeer peer );

		///
		/// \brief Reject a peer that has connected to the host
		void fRejectPeer( const tPeer & peer );

		///
		/// \brief Migrate to the new host
		void fMigrateHost( const tPeer & peer );

		///
		/// \brief Send a message to a single peer
		void fSendMsg( const tPeer & peer, u32 channel, const byte * data, u32 dataLength, b32 reliable );

		///
		/// \brief Broadcast the message to all the currently connected peers, including the host peer
		void fSendMsgToPeers( u32 channel, const byte * data, u32 dataLength, b32 reliable );

		///
		/// \brief Send a message to the host peer only - only valid if not the host
		void fSendMsgToHost( u32 channel, const byte * data, u32 dataLength, b32 reliable );

		///
		/// \brief Flush any queued messages
		/// \return Returns false if host is invalid
		b32 fFlush( );

		///
		/// \brief Get the socket used for sending
		inline const ENetSocket& fSocket( ) const { return mHost.fSocket( ); }

		inline u32 fAverageRTT( ) const { return mHost.fAverageRTT( ); }
		inline f32 fAveragePacketLoss( ) const { return mHost.fAveragePacketLoss( ); }

	private:

		struct tAddress
		{
			inline tAddress( ) : host( ~0 ), port( ~0 ) { }
			inline tAddress( const ENetAddress & e );
			inline b32 operator == (const ENetAddress & e ) const;
			inline b32 operator != (const ENetAddress & e ) const;

			u32 host;
			u16 port;
		};

		struct tExpectedConnection
		{
			inline tExpectedConnection( ) : mPeerId( cInvalidPeerId ) { }
			inline tExpectedConnection( const tPlatformAddr& addr, tPeerId peerId );
			inline b32 operator == ( const tPlatformAddr& addr ) const;
			inline b32 operator != ( const tPlatformAddr& addr ) const;
			inline b32 operator == ( const tExpectedConnection& e ) const;
			inline b32 operator != ( const tExpectedConnection& e ) const;

			tPlatformAddr mPlatformAddr;
			tPeerId mPeerId;
		};

		///
		/// \brief Used by the host for generating network-unique peer Ids
		class tPeerIdGenerator
		{
		public:
			tPeerIdGenerator( ) : mNextId( 0 ) { }

			tPeerId fNext( );
			void fRelease( tPeerId id );
			void fReset( );
			void fReset( const tGrowableArray<tPeerId>& reserved );

		private:
			tPeerId mNextId;
			tQueue< tPeerId > mReleasedIds;
		};

		///
		/// \brief Used during host migration to determine which peer becomes the new host
		class tHostVote
		{
			enum tState
			{
				cStateNull,
				cStateStarted,
				cStateFinished
			};

		public:
			tHostVote( );
			void fStart( tPeerId localPeerId, const tGrowableArray<tPeer>& peers );
			void fFinish( );
			void fReset( );

			b32 fStarted( ) const { return mState > cStateNull; }
			b32 fFinished( ) const { return mState == cStateFinished; }
			u32 fTimeElapsedMs( ) const;
			tPeerId fDesiredHostId( ) const { return mDesiredHostId; }

		private:
			u32 mStartTime;
			tPeerId mDesiredHostId;
			tState mState;
		};

	private:

		void fSetState( tState state );

		void fDestroy( );

		// Direct net event responses
		void fOnConnect( ENetPeer * peer );
		void fOnDisconnect( ENetPeer * peer, u32 reason );
		void fOnReceive( const ENetEvent & e );

		// Packet handlers
		void fHandleProtocolPacket( ENetPeer * peer, ENetPacket * packet );
		void fHandleHandShakePacket( ENetPeer * peer, ENetPacket * packet );
		void fHandleExpectConnectionPacket( ENetPeer * peer, ENetPacket * packet );
		void fHandleConnectionAcceptedPacket( ENetPeer * peer, ENetPacket * packet );
		void fHandlePeerDisconnectPacket( ENetPeer * peer, ENetPacket * packet );
		void fHandleHostVotePacket( ENetPeer * peer, ENetPacket * packet );
		
		// Peers
		u32		fPeerCount( ) const { return mWeakPeers.fCount( ) + mStrongPeers.fCount( ); }
		tPeer * fFindPeer( ENetPeer * peer );
		tPeer * fFindPeer( const tPlatformAddr & platformAddr );
		tPeer * fFindPeer( tPeerId peerId );
		void	fAddUnknownWeakPeer( ENetPeer * peer );
		void	fStrengthenPeer( tPeer peer, b32 sendAcceptance );
		void	fProcessWeakPeers( );
		void	fErasePeer( tPeer * peer );
		b32		fIsWeak( tPeer * peer );
		b32		fIsStrong( tPeer * peer );
		void	fSendProtocolMsg( tPeer & peer, const tGrowableArray<byte> & msg );
		void	fDisconnect( b32 afterOutgoing, u32 reason );
		void	fDisconnectNow( u32 reason );
		void	fStartHostVote( );
		void	fFinishHostVote( );
		

		// Packet builders
		void fBuildHandShake( tGrowableArray<byte> & msg, const tPeer & peer );
		void fBuildExpectConnection( tGrowableArray<byte> & msg, const tPeer & peer );
		void fBuildConnectionAccepted( tGrowableArray<byte> & msg );
		void fBuildPeerDisconnect( tGrowableArray<byte> & msg, const tPlatformAddr & platformAddr );
		void fBuildHostVote( tGrowableArray<byte> & msg );
		

	private:

		tState	mState;
		u32		mProtocolChannel;

		tHost mHost;

		tP2PNetworkHost *	mUserHost;
		tP2PNetworkPeer *	mUserPeer;

		tGrowableArray<tPeer> mStrongPeers;
		tGrowableArray<tPeer> mWeakPeers;
		tGrowableArray<tExpectedConnection> mExpectedConnections;

		tPeerId mLocalPeerId;
		tPeerIdGenerator mPeerIdGenerator;

		tHostVote mHostVote;

#if defined( sig_devmenu )
		tStats mStats;
#endif
	};

}}

#endif//__tP2PNetwork__
