//------------------------------------------------------------------------------
// \file tP2PNetwork.cpp - 12 Oct 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tP2PNetwork.hpp"
#include "enet/enet.h"

namespace Sig { namespace Net
{
	//------------------------------------------------------------------------------
	// Protocol
	//------------------------------------------------------------------------------

	namespace
	{
		static const u32 cWeakTimeout = 3 * 1000;

		enum tPacketType
		{
			cPacketTypeHandShake = 0,
			cPacketTypeExpectConnection,
			cPacketTypeConnectionAccepted,
			cPacketTypePeerDisconnect,

			cPacketTypeCount
		};

		struct tPacketHeader
		{
			u32 mPacketType;
		};

		// A list of the peers connected at the time
		// the peer connected to the host
		struct tHandShakePacket
		{
			tPacketHeader mHeader;
			u32 mPeerCount;
		};

		// Informs a peer that a new peer connected to the host
		// and should be trying to connect to it
		struct tExpectConnectionPacket
		{
			tPacketHeader mHeader;
			ENetAddress mAddress;
		};

		// Informs a non-host peer that their connection
		// was accepted
		struct tConnectionAcceptedPacket
		{
			tPacketHeader mHeader;
		};

		// Informs peers that one of their peers has 
		// disconnected from the server
		struct tPeerDisconnectPacket
		{
			tPacketHeader mHeader;
			ENetAddress mAddress;
		};
	}

	//------------------------------------------------------------------------------
	void fLogPeerWarning( const char * msg, ENetPeer * peer )
	{
		char hostName[256];
		enet_address_get_host_ip( &peer->address, hostName, array_length(hostName ) );
		log_warning( Log::cFlagNetwork, "tP2PNetwork: " << msg << " from " << hostName );
	}

	//------------------------------------------------------------------------------
	template< class PType >
	static PType * fDecodePacket( ENetPacket * packet, ENetPeer * peer )
	{
		// It's assumed that the packetType has been used to determine PType
		if( packet->dataLength < sizeof( PType ) )
		{
			fLogPeerWarning( "Unrecognized ( Too small for packetType ) packet", peer );
			return NULL;
		}

		return (PType *)packet->data;
	}

	//------------------------------------------------------------------------------
	static tPacketHeader * fDecodePacketHeader( ENetPacket * packet, ENetPeer * peer )
	{
		if( packet->dataLength < sizeof( tPacketHeader ) )
		{
			fLogPeerWarning( "Unrecognized ( Too small ) packet", peer );
			return NULL;
		}
		
		tPacketHeader * header = (tPacketHeader *)packet->data;
		if( header->mPacketType >= cPacketTypeCount )
		{
			fLogPeerWarning( "Unrecognized ( Invalid packetType ) packet", peer );
			return NULL;
		}

		return header;
	}

	//------------------------------------------------------------------------------
	// tAddress
	//------------------------------------------------------------------------------
	inline tP2PNetwork::tAddress::tAddress( const ENetAddress & e )
		: host( e.host ) , port( e.port ) { }

	//------------------------------------------------------------------------------
	inline b32 tP2PNetwork::tAddress::operator == (const ENetAddress & e ) const
	{
		return host == e.host && port == e.port;
	}

	//------------------------------------------------------------------------------
	inline b32 tP2PNetwork::tAddress::operator != (const ENetAddress & e ) const
	{
		return host != e.host || port != e.port;
	}

	//------------------------------------------------------------------------------
	// tP2PNetwork
	//------------------------------------------------------------------------------
	tP2PNetwork::tP2PNetwork( )
		: mState( cStateInactive )
		, mUserHost( NULL )
		, mUserPeer( NULL )
		, mProtocolChannel( ~0 )
#if defined( use_steam )
		, mCallbackSessionRequest( this, &tP2PNetwork::fOnSessionRequest )
#endif
	{
	}

	//------------------------------------------------------------------------------
	b32 tP2PNetwork::fHost( 
		tP2PNetworkHost * userHost,
		u32 maxPeerCount,
		u32 channelCount,
		tAddr hostAddress, 
		u32 hostPort )
	{
		// If we're disconnecting and trying to move on then just force the disconnect
		if( mState == cStateDisconnecting )
			fLeaveNow( );

		sigassert( mState == cStateInactive && "HostNetwork called with invalid state" );
		sigassert( userHost && "HostNetwork called with invalid user host" );
		sigassert( !fPeerCount( ) && "Inactive networks should not already have peers" );

		if( !mHost.fStartServer( maxPeerCount, channelCount + 1, hostAddress, hostPort ) )
			return false;

		mProtocolChannel = channelCount;
		mUserHost = userHost;
		mUserPeer = userHost;
		mState = cStateHosting;
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tP2PNetwork::fJoin( 
		tP2PNetworkPeer * userPeer,
		u32 maxPeerCount,
		u32 channelCount,
		tAddr hostAddress,
		u32 hostPort,
		u32 clientAddress,
		u32 clientPort )
	{
		// If we're disconnecting and trying to move on then just force the disconnect
		if( mState == cStateDisconnecting )
			fLeaveNow( );

		sigassert( mState == cStateInactive && "JoinNetwork called with invalid state" );
		sigassert( userPeer && "JoinNetwork called with invalid user peer" );
		sigassert( !fPeerCount( ) && "Inactive networks should not already have peers" );

		if( !mHost.fStartServer( maxPeerCount, channelCount + 1, clientAddress, clientPort ) )
			return false;

		tPeer peer = mHost.fConnect( hostAddress, channelCount + 1, 0, hostPort );
		if( !peer.fValid( ) )
		{
			mHost.fDestroy( );
			return false;
		}

		mProtocolChannel = channelCount;
		mUserPeer = userPeer;
		mWeakPeers.fPushBack( peer );
		mState = cStateConnecting;
		return true;
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fLeave( b32 afterOutgoing )
	{
		if( mState == cStateInactive )
			return;

		fDisconnect( afterOutgoing, cDisconnectReasonLeaving );
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fLeaveNow( )
	{
		if( mState == cStateInactive )
			return;

		fDisconnectNow( cDisconnectReasonLeaving );

		// Destroy
		fDestroy( );
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fService( u32 timeout )
	{
		if( mState == cStateInactive )
			return;

		ENetEvent netEvent;
		while( mHost.fValid( ) && mHost.fService( netEvent ) > 0 )
		{
			switch( netEvent.type )
			{
			case ENET_EVENT_TYPE_CONNECT:
				fOnConnect( netEvent.peer );
				break;
			case ENET_EVENT_TYPE_DISCONNECT:
				fOnDisconnect( netEvent.peer, netEvent.data );
				break;
			case ENET_EVENT_TYPE_RECEIVE:
				fOnReceive( netEvent );
				break;
			}
		}

		fProcessWeakPeers( );

		// We're disconnecting and all peers have been disconnected so destroy and go inactive
		if( mState == cStateDisconnecting && !mWeakPeers.fCount( ) && !mStrongPeers.fCount( ) )
			fDestroy( );
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fAcceptPeer( const tPeer & peer )
	{
		sigassert( mState == cStateHosting && "Peers can only be accepted by network hosts" );
		fStrengthenPeer( peer, false /*unsued for hosts*/ );
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fRejectPeer( const tPeer & _peer )
	{
		sigassert( mState == cStateHosting && "Peers can only be rejected by network hosts" );

		tPeer * peer = mWeakPeers.fFind( _peer );

		// We no long have this peer so nothing to do
		if( !peer )
			return;

		peer->fDisconnect( false, cDisconnectReasonRejected );
	}

	//------------------------------------------------------------------------------
	tPeer *	tP2PNetwork::fHostPeer( )
	{
		if( mState == cStateConnecting )
			return mWeakPeers.fBegin( );
		else if( mState == cStateConnected )
			return mStrongPeers.fBegin( );

		return NULL;
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fSendMsgToPeers( u32 channel, const byte * data, u32 dataLength, b32 reliable )
	{
		sigassert( channel != mProtocolChannel && "Invalid attempt to send msg on P2PNetwork protocol channel" );
		sigassert( ( mState == cStateConnected || mState == cStateHosting ) && "Cannot send messages from current state" );
		
		const u32 count =  mStrongPeers.fCount( );
		for( u32 p = 0; p < count; ++p )
		{
			mStrongPeers[ p ].fSend( 
				data, dataLength, channel, 
				reliable ? ENET_PACKET_FLAG_RELIABLE : 0 );
		}
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fSendMsgToHost( u32 channel, const byte * data, u32 dataLength, b32 reliable )
	{
		sigassert( channel != mProtocolChannel && "Invalid attempt to send msg on P2PNetwork protocol channel" );
		sigassert( mState == cStateConnected && "Can only send msg to host if in StateConnected" );

		mStrongPeers[ 0 ].fSend( 
			data, dataLength, channel,
			reliable ? ENET_PACKET_FLAG_RELIABLE: 0 );
	}

	//------------------------------------------------------------------------------
	b32 tP2PNetwork::fFlush( )
	{
		if( !mHost.fValid( ) )
			return false;

		mHost.fFlush( );
		return true;
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fDestroy( )
	{
		sigassert( mState == cStateDisconnecting );

		// Destroy and clear
		mHost.fDestroy( );
		mUserHost = NULL;
		mUserPeer = NULL;
		mWeakPeers.fDeleteArray( );
		mStrongPeers.fDeleteArray( );
		mExpectedConnections.fDeleteArray( );

		// Go Inactive
		mState = cStateInactive;
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fOnConnect( ENetPeer * _peer )
	{
		sigassert( mState != cStateInactive && "Invalid state for OnConnect" );
		sigassert( _peer && "Invalid peer passed to OnConnect" );

		switch( mState )
		{
		// New connections to the host
		case cStateHosting:
			fAddUnknownWeakPeer( _peer );
			mUserHost->fOnConnectAttempt( *this, tPeer( _peer ) );
			return;

		// We're waiting for our server connection
		case cStateConnecting:
			
			// This may be the server message or this may be a peer who has connected to the
			// server after us, but received his handshake before we got our connection 
			// success message
			if( mWeakPeers[ 0 ] == _peer )
			{
				mState = cStateConnected;
				fStrengthenPeer( mWeakPeers[ 0 ], false );
			}
			else
				fAddUnknownWeakPeer( _peer );
			return;

		// New peers are connecting via handshakes from the server
		case cStateConnected:
			{
				sigassert( !fFindPeer( _peer )&& "Newly connected peer already existed" );

				// Add the peer
				fAddUnknownWeakPeer( _peer );

				// Then check if we've already been informed about this peer
				// and strengthen the connection if we have
				const u32 expectedCount = mExpectedConnections.fCount( );
				for( u32 e = 0; e < expectedCount; ++e )
				{
					if( mExpectedConnections[ e ] != _peer->address )
						continue;

					fStrengthenPeer( mWeakPeers.fBack( ), true );
					mExpectedConnections.fErase( e );
					break;
				}
				
			} return;

		case cStateDisconnecting:

			// We don't accept new connections while disconnecting
			enet_peer_disconnect_now( _peer, cDisconnectReasonDisconnecting );
			return;
		}
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fOnDisconnect( ENetPeer * _peer, u32 reason )
	{
		sigassert( mState != cStateInactive && "Invalid state for OnDisconnect" );
		sigassert( _peer && "Invalid peer passed to OnDisconnect" );
	
		b32 acknowledge = true;

		switch( mState )
		{
		case cStateHosting:
			if( tPeer * peer = fFindPeer( _peer ) )
			{
				b32 isStrong = fIsStrong( peer );
				fErasePeer( peer );
				
				// Send our remaining peers that this peer has disconnected
				// from the server and should be rejected by them as well
				if( isStrong )
				{
					tGrowableArray<byte> msg;
					fBuildPeerDisconnect( msg, _peer->address );

					const u32 peerCount = mStrongPeers.fCount( );
					for( u32 p = 0; p < peerCount; ++p )
						fSendMessage( mStrongPeers[ p ], msg );
				}
			} 
			else
			{
				// We don't know who this peer is
				acknowledge = false;

			}break;
		case cStateConnecting:
			if( tPeer * peer = fFindPeer( _peer ) )
			{
				sigassert( fIsWeak( peer ) && "Sanity: Connecting peers should only have weak peers" );

				b32 isHost = ( peer == mWeakPeers.fBegin( ) );
				fErasePeer( peer );

				acknowledge = false; // Don't acknowledge weak peer disconnects

				// Host disconnect means this was a timeout and so a join failure.
				// Any other disconnects in this state were from unimportant peers
				if( isHost )
				{
					fDisconnect( false, cDisconnectReasonServerTimeout );
					mUserPeer->fOnJoinFailure( *this );
				}
			} 
			else
			{
				acknowledge = false;
			}break;
		case cStateConnected:
			if( tPeer * peer = fFindPeer( _peer ) )
			{
				b32 isWeak = fIsWeak( peer );
				b32 isHost = ( peer == mStrongPeers.fBegin( ) );
				fErasePeer( peer );

				if( isWeak )
				{
					acknowledge = false;
				}
				else // We can assume its strong because we found it
				{
					if( isHost )
						fDisconnect( false, cDisconnectReasonServerDisconnect );
					else
					{
						// TODO: analyze the reason for the disconnect and handle appropriately
						sigassert( 0 && "Sanity: Network is broken without more codes" );
					}
				}
			}
			else
			{
				acknowledge = false;
			} break;
		case cStateDisconnecting:
			if( tPeer * peer = fFindPeer( _peer ) )
			{
				acknowledge = fIsStrong( peer );
				fErasePeer( peer );
			}
			else
			{
				acknowledge = false;
			} break;
		}

		if( acknowledge )
			mUserPeer->fOnDisconnect( *this, tPeer( _peer ), reason );
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fOnReceive( const ENetEvent & e )
	{
		// Our message
		if( e.channelID == mProtocolChannel )
			fHandleProtocolPacket( e.peer, e.packet );

		// User message
		else
		{
			// If we have this peer process the message
			if( tPeer * peer = fFindPeer( e.peer ) )
			{
				mUserPeer->fOnReceive( 
					*this, 
					*peer, 
					e.channelID,
					e.packet->data, 
					e.packet->dataLength );
			}
			
			// Otherwise if we're not disconnecting print a warning
			else if( mState != cStateDisconnecting )
			{
				fLogPeerWarning( "Unrecognized packet ( Unknown peer )", e.peer );
			}
		}

		enet_packet_destroy( e.packet );
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fHandleProtocolPacket( ENetPeer * peer, ENetPacket * packet )
	{
		tPacketHeader * header = fDecodePacketHeader( packet, peer );
		if( !header )
			return;

		switch( header->mPacketType )
		{
		case cPacketTypeHandShake: // Server
			fHandleHandShakePacket( peer, packet );
			break;
		case cPacketTypeExpectConnection: // Server
			fHandleExpectConnectionPacket( peer, packet );
			break;
		case cPacketTypeConnectionAccepted: // Peer
			fHandleConnectionAcceptedPacket( peer, packet );
			break;
		case cPacketTypePeerDisconnect: // Server
			fHandlePeerDisconnectPacket( peer, packet );
			break;
		default:
			sigassert( 0 && "Sanity: Decode functions are designed to avoid this code executing" );
			break;
		}
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fHandleHandShakePacket( ENetPeer * peer, ENetPacket * packet )
	{
		if( mState != cStateConnected )
		{
			fLogPeerWarning( "Invalid state for handshake", peer );
			return;
		}

		sigassert( mStrongPeers.fCount( ) == 1 && "Connecting network should have 1 strong peer - the host" );

		if( !( mStrongPeers[ 0 ] == peer ) )
		{
			fLogPeerWarning( "Hand shake from non-host", peer );
			return;
		}

		// Decode the packet if possible
		tHandShakePacket * handShake = fDecodePacket<tHandShakePacket>( packet, peer );
		if( !handShake )
			return;

		// Not big enough for how many peers it specifies
		if( packet->dataLength < sizeof( tHandShakePacket ) + sizeof( ENetAddress ) * handShake->mPeerCount )
		{
			fLogPeerWarning( "Corrupt handshake", peer );
			return;
		}
		
		// Attempt to connect to all my new peers
		ENetAddress * address = (ENetAddress *)( handShake + 1 );
		for( u32 p = 0; p < handShake->mPeerCount; ++p )
			mWeakPeers.fPushBack( mHost.fConnect( address[ p ].host, 0, 0, address[ p ].port ) );
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fHandleExpectConnectionPacket( ENetPeer * peer, ENetPacket * packet )
	{
		// Decode the packet if possible
		tExpectConnectionPacket * expect = fDecodePacket<tExpectConnectionPacket>( packet, peer );
		if( !expect )
			return;

		tAddress addy( expect->mAddress );
		
		// First test if we have any weak connections we can resolve with this expectation
		b32 resolved = false;
		if( mState == cStateConnected )
		{
			const u32 weakPeerCount = mWeakPeers.fCount( );
			for( u32 p = 0; p < weakPeerCount; ++p )
			{
				if( addy != mWeakPeers[ p ].fAddress( ) )
					continue;

				fStrengthenPeer( mWeakPeers[ p ], true );
				resolved = true;
				break;
			}
		}

		// Add the expected connection to our reserve list
		if( !resolved )
		{
			sigassert( !mExpectedConnections.fFind( expect->mAddress ) && "Connection was already expected" );
			mExpectedConnections.fPushBack( addy );
		}
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fHandleConnectionAcceptedPacket( ENetPeer * _peer, ENetPacket * packet )
	{
		// Decode the packet if possible
		tConnectionAcceptedPacket * accepted = fDecodePacket<tConnectionAcceptedPacket>( packet, _peer );
		if( !accepted )
			return;

		tPeer * peer = mWeakPeers.fFind( _peer );

		// We don't have this peer available anymore so we can't strengthen it
		if( !peer )
			return;

		fStrengthenPeer( *peer, false );
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fHandlePeerDisconnectPacket( ENetPeer * _peer, ENetPacket * packet )
	{
		tPeerDisconnectPacket * disconnect = fDecodePacket<tPeerDisconnectPacket>( packet, _peer );
		if( !disconnect )
			return;

		// Some peer sent us this message, ignore for now, but we may
		// want to whip it into shape for doing such things
		if( mState == cStateHosting )
		{
			fLogPeerWarning( "Receieved host only packet as host", _peer );
			return;
		}

		// This message should only come from the host
		if( !( *fHostPeer( ) == _peer ) )
		{
			fLogPeerWarning( "Received host only packet from non-host", _peer );
			return;
		}

		tPeer * msgPeer = fFindPeer( disconnect->mAddress );
		
		// We don't have this peer so look for an 
		// expected connection and remove it
		if( !msgPeer )
		{
			const u32 ecCount = mExpectedConnections.fCount( );
			for( u32 ec = 0; ec < ecCount; ++ec )
			{
				if( mExpectedConnections[ ec ] == disconnect->mAddress )
				{
					mExpectedConnections.fErase( ec );
					break;
				}
			}
			return;
		}

		// If it's weak we disconnect the peer nicely, waiting 
		// for outgoing messages if its a strong peer.
		msgPeer->fDisconnect( fIsStrong( msgPeer ), cDisconnectReasonServerInstructed );
	}

	//------------------------------------------------------------------------------
	tPeer * tP2PNetwork::fFindPeer( ENetPeer * _peer )
	{
		tPeer * peer = mStrongPeers.fFind( _peer );
		if( !peer )
			peer = mWeakPeers.fFind( _peer );

		return peer;
	}

	//------------------------------------------------------------------------------
	tPeer * tP2PNetwork::fFindPeer( const ENetAddress & _addy )
	{
		tAddress addy( _addy );

		const u32 weakPeerCount = mWeakPeers.fCount( );
		for( u32 p = 0; p < weakPeerCount; ++p )
		{
			if( addy == mWeakPeers[ p ].fAddress( ) )
				return &mWeakPeers[ p ];
		}

		const u32 strongPeerCount = mStrongPeers.fCount( );
		for( u32 p = 0; p < strongPeerCount; ++p )
		{
			if( addy == mStrongPeers[ p ].fAddress( ) )
				return &mStrongPeers[ p ];
		}

		return NULL;
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fAddUnknownWeakPeer( ENetPeer * peer )
	{
		// Give weak peers 3 seconds for their expect message to arrive
		peer->data = (void*)( enet_time_get( ) + cWeakTimeout );
		mWeakPeers.fPushBack( tPeer( peer ) );
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fStrengthenPeer( tPeer peer, b32 sendAcceptance )
	{	
		// Remove from the weak peers
		b32 found = mWeakPeers.fFindAndErase( peer );
		sigassert( found && "Peer passed to StrengthenPeer a non-weak peer" );

		// If we're the host introduce our peers to their new friend
		if( mState == cStateHosting )
		{
			// Send a hand shake with peer info to the new client
			tGrowableArray< byte > msg;
			fBuildHandShake( msg );
			fSendMessage( peer, msg );

			// Send messages to the clients to expect and accept connections their new peer
			fBuildExpectConnection( msg, peer.fAddress( ) );
			const u32 strongPeerCount = mStrongPeers.fCount( );
			for( u32 p = 0; p < strongPeerCount; ++p )
				fSendMessage( mStrongPeers[ p ], msg );
		}

		// Otherwise report that we accepted the connection
		else if( sendAcceptance )
		{
			sigassert( mState == cStateConnected );

			tGrowableArray<byte> msg;
			fBuildConnectionAccepted( msg );
			fSendMessage( peer, msg );
		}

		mStrongPeers.fPushBack( peer );
		mUserPeer->fOnConnect( *this, peer );
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fProcessWeakPeers( )
	{
		const u32 time = enet_time_get( );

		const u32 peerCount = mWeakPeers.fCount( );
		for( s32 p = peerCount - 1; p >= 0; --p )
		{
			tPeer & peer = mWeakPeers[ p ];

			// If the peer has a timeout set and we're past it
			// then disconnect that peer
			if( mState == cStateDisconnecting || ( peer.fData( ) && time > (u32)peer.fData( ) ) )
			{
				peer.fDisconnect( true, cDisconnectReasonWeakTimeout );
				mWeakPeers.fErase( fPtrDiff( &peer, mWeakPeers.fBegin( ) ) );
			}
		}
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fErasePeer( tPeer * peer )
	{
		if( fIsWeak( peer ) )
		{
			mWeakPeers.fErase( fPtrDiff( peer, mWeakPeers.fBegin( ) ) );
			return;
		}

		if( fIsStrong( peer ) )
		{
			mStrongPeers.fErase( fPtrDiff( peer, mStrongPeers.fBegin( ) ) );
			return;
		}

		sigassert( 0 && "Sanity: Peer passed to ErasePeer not found" );
	}

	//------------------------------------------------------------------------------
	b32 tP2PNetwork::fIsWeak( tPeer * peer )
	{
		return peer >= mWeakPeers.fBegin( ) && peer < mWeakPeers.fEnd( );
	}

	//------------------------------------------------------------------------------
	b32	tP2PNetwork::fIsStrong( tPeer * peer )
	{
		return peer >= mStrongPeers.fBegin( ) && peer < mStrongPeers.fEnd( );
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fSendMessage( tPeer & peer, const tGrowableArray<byte> & msg )
	{
		peer.fSend( 
			msg.fBegin( ), 
			msg.fTotalSizeOf( ), 
			mProtocolChannel, 
			ENET_PACKET_FLAG_RELIABLE );
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fDisconnect( b32 afterOutgoing, u32 reason )
	{
		const u32 weakPeerCount = mWeakPeers.fCount( );
		for( u32 p = 0; p < weakPeerCount; ++p )
			mWeakPeers[ p ].fDisconnect( afterOutgoing, reason );

		const u32 strongPeerCount = mStrongPeers.fCount( );
		for( u32 p = 0; p < strongPeerCount; ++p )
			mStrongPeers[ p ].fDisconnect( afterOutgoing, reason );

		mState = cStateDisconnecting;
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fDisconnectNow( u32 reason )
	{
		const u32 weakPeerCount = mWeakPeers.fCount( );
		for( u32 p = 0; p < weakPeerCount; ++p )
			mWeakPeers[ p ].fDisconnectNow( reason );
		mWeakPeers.fSetCount( 0 );

		const u32 strongPeerCount = mStrongPeers.fCount( );
		for( u32 p = 0; p < strongPeerCount; ++p )
			mStrongPeers[ p ].fDisconnectNow( reason );
		mStrongPeers.fSetCount( 0 );

		mState = cStateDisconnecting;
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fBuildHandShake( tGrowableArray<byte> & msg )
	{
		const u32 strongPeerCount = mStrongPeers.fCount( );
		const u32 totalSize = sizeof(tHandShakePacket) + sizeof( ENetAddress ) * strongPeerCount;
		msg.fSetCount( totalSize );

		tHandShakePacket * shake = (tHandShakePacket *)msg.fBegin( );
		ENetAddress * address = (ENetAddress *)( shake + 1 );

		shake->mHeader.mPacketType = cPacketTypeHandShake;
		shake->mPeerCount = strongPeerCount;

		for( u32 p = 0; p < strongPeerCount; ++p )
			address[ p ] = mStrongPeers[ p ].fAddress( );
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fBuildExpectConnection( tGrowableArray<byte> & msg, const ENetAddress & addy )
	{
		msg.fSetCount( sizeof( tExpectConnectionPacket ) );

		tExpectConnectionPacket * expect = (tExpectConnectionPacket *)msg.fBegin( );
		expect->mHeader.mPacketType = cPacketTypeExpectConnection;
		expect->mAddress = addy;
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fBuildConnectionAccepted( tGrowableArray<byte> & msg )
	{
		msg.fSetCount( sizeof( tConnectionAcceptedPacket ) );

		tConnectionAcceptedPacket * accepted = (tConnectionAcceptedPacket *)msg.fBegin( );
		accepted->mHeader.mPacketType = cPacketTypeConnectionAccepted;
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fBuildPeerDisconnect( tGrowableArray<byte> & msg, const ENetAddress & addy )
	{
		msg.fSetCount( sizeof( tPeerDisconnectPacket ) );
		
		tPeerDisconnectPacket * disconnect = (tPeerDisconnectPacket *)msg.fBegin( );
		disconnect->mHeader.mPacketType = cPacketTypePeerDisconnect;
		disconnect->mAddress = addy;
	}

#if defined( use_steam )
	//------------------------------------------------------------------------------
	void tP2PNetwork::fOnSessionRequest(P2PSessionRequest_t *param)
	{
		log_line( Log::cFlagSession, __FUNCTION__ << " remoteID " << param->m_steamIDRemote.ConvertToUint64( ) );
		SteamNetworking( )->AcceptP2PSessionWithUser( param->m_steamIDRemote );
	}
#endif
}}
