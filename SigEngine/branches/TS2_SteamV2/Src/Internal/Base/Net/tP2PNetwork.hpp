//------------------------------------------------------------------------------
// \file tP2PNetwork.hpp - 12 Oct 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tP2PNetwork__
#define __tP2PNetwork__
#include "tHost.hpp"

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
		virtual void fOnReceive( 
			tP2PNetwork & network, 
			const tPeer & peer, 
			u32 channelId,
			const byte * data, 
			u32 dataLength ) = 0;

		virtual void fOnConnect( tP2PNetwork & network, const tPeer & peer ) = 0;
		virtual void fOnDisconnect( tP2PNetwork & network, const tPeer & peer, u32 reason ) = 0;
		virtual void fOnJoinFailure( tP2PNetwork & network ) = 0;
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
			cStateDisconnecting
		};

		enum tDisconnectReason
		{
			cDisconnectReasonLeaving = 0,
			cDisconnectReasonRejected,
			cDisconnectReasonDisconnecting,
			cDisconnectReasonWeakTimeout,
			cDisconnectReasonServerTimeout,
			cDisconnectReasonServerDisconnect,
			cDisconnectReasonServerInstructed
		};

	public:

		tP2PNetwork( );

		// State helpers
		tState	fState( ) { return mState; }
		b32		fIsActive( ) const { return mState != cStateInactive; }
		b32		fIsConnected( ) const { return mState == cStateConnected; }
		b32		fIsHosting( ) const { return mState == cStateHosting; }
		b32		fIsConnectedOrHosting( ) const { return mState == cStateConnected || mState == cStateHosting; }
		

		///
		/// \brief Host a network where others can connect
		b32 fHost( 
			tP2PNetworkHost * userHost,
			u32 maxPeerCount,
			u32 channelCount,
			tAddr hostAddress = tHost::cDefaultAddress, 
			u32 hostPort = tHost::cDefaultPort );

		///
		/// \brief Join a pre-existing network at the specified addy
		b32 fJoin(
			tP2PNetworkPeer * userPeer,
			u32 maxPeerCount,
			u32 channelCount,
			tAddr hostAddress,
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
		/// \brief Accept a peer that has connected to the host
		void fAcceptPeer( const tPeer & peer );

		///
		/// \brief Reject a peer that has connected to the host
		void fRejectPeer( const tPeer & peer );

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

		inline u32 fAverageRTT( ) const { return mHost.fAverageRTT( ); }
		inline f32 fAveragePacketLoss( ) const { return mHost.fAveragePacketLoss( ); }

	private:

		struct tAddress
		{
			inline tAddress( ) : host( ~0 ), port( ~0 ) { }
			inline tAddress( const ENetAddress & e );
			inline b32 operator == (const ENetAddress & e ) const;
			inline b32 operator != (const ENetAddress & e ) const;

			tAddr host;
			u32 port;
		};

	private:

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
		
		// Peers
		u32		fPeerCount( ) const { return mWeakPeers.fCount( ) + mStrongPeers.fCount( ); }
		tPeer * fFindPeer( ENetPeer * peer );
		tPeer * fFindPeer( const ENetAddress & addy );
		void	fAddUnknownWeakPeer( ENetPeer * peer );
		void	fStrengthenPeer( tPeer peer, b32 sendAcceptance );
		void	fProcessWeakPeers( );
		void	fErasePeer( tPeer * peer );
		b32		fIsWeak( tPeer * peer );
		b32		fIsStrong( tPeer * peer );
		void	fSendMessage( tPeer & peer, const tGrowableArray<byte> & msg );
		void	fDisconnect( b32 afterOutgoing, u32 reason );
		void	fDisconnectNow( u32 reason );
		

		// Packet builders
		void fBuildHandShake( tGrowableArray<byte> & msg );
		void fBuildExpectConnection( tGrowableArray<byte> & msg, const ENetAddress & addy );
		void fBuildConnectionAccepted( tGrowableArray<byte> & msg );
		void fBuildPeerDisconnect( tGrowableArray<byte> & msg, const ENetAddress & addy );
		

	private:

		tState	mState;
		u32		mProtocolChannel;

		tHost mHost;

		tP2PNetworkHost *	mUserHost;
		tP2PNetworkPeer *	mUserPeer;

		tGrowableArray<tPeer> mStrongPeers;
		tGrowableArray<tPeer> mWeakPeers;
		tGrowableArray<tAddress> mExpectedConnections;

#if defined( use_steam )
		STEAM_CALLBACK( tP2PNetwork, fOnSessionRequest, P2PSessionRequest_t, mCallbackSessionRequest );
#endif
	};

}}

#endif//__tP2PNetwork__
