//------------------------------------------------------------------------------
// \file tP2PNetwork.cpp - 12 Oct 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tP2PNetwork.hpp"
#include "enet/enet.h"

#include "tGameSessionSearchResult.hpp"

namespace Sig { namespace Net
{
	//------------------------------------------------------------------------------
	// Protocol
	//------------------------------------------------------------------------------

	namespace
	{
		static const u32 cWeakTimeout = 3 * 1000;
		static const u32 cHostVoteTimeout = cWeakTimeout * 2;

		enum tPacketType
		{
			cPacketTypeHandShake = 0,
			cPacketTypeExpectConnection,
			cPacketTypeConnectionAccepted,
			cPacketTypePeerDisconnect,
			cPacketTypeHostVote,

			cPacketTypeCount
		};

		struct tPacketHeader
		{
			u32 mPacketType;
		};

		// HandShake info for a single peer
		struct tPeerHandShakeInfo
		{
			tPlatformAddr mAddr;
			tPeerId mId;
		};

		// A list of the peers connected at the time
		// the peer connected to the host
		struct tHandShakePacket
		{
			tPacketHeader mHeader;
			u32 mPeerCount;
			tPeerId mJoinerId;
			tPeerId mHostId;
		};

		// Informs a peer that a new peer connected to the host
		// and should be trying to connect to it
		struct tExpectConnectionPacket
		{
			tPacketHeader mHeader;
			tPlatformAddr mPlatformAddr;
			tPeerId mPeerId;
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
			tPlatformAddr mPlatformAddr;
		};

		// Informs peers that we'd like to migrate to a new host
		struct tHostVotePacket
		{
			tPacketHeader mHeader;
			tPeerId mDesiredHostId;
		};
	}

	//------------------------------------------------------------------------------
	void fLogPeerWarning( const char * msg, ENetPeer * peer )
	{
		char hostName[256];
		enet_address_get_host_ip( &peer->address, hostName, array_length(hostName ) );
		log_warning( "tP2PNetwork: " << msg << " from " << hostName );
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
	// tExpectedConnection
	//------------------------------------------------------------------------------
	inline tP2PNetwork::tExpectedConnection::tExpectedConnection( const tPlatformAddr& addr, tPeerId peerId )
		: mPlatformAddr( addr ), mPeerId( peerId ) { }

	//------------------------------------------------------------------------------
	inline b32 tP2PNetwork::tExpectedConnection::operator == ( const tPlatformAddr& addr ) const
	{
		return mPlatformAddr == addr;
	}

	//------------------------------------------------------------------------------
	inline b32 tP2PNetwork::tExpectedConnection::operator != ( const tPlatformAddr& addr ) const
	{
		return !( *this == addr );
	}

	//------------------------------------------------------------------------------
	inline b32 tP2PNetwork::tExpectedConnection::operator == ( const tExpectedConnection& e ) const
	{
		return mPlatformAddr == e.mPlatformAddr;
	}

	//------------------------------------------------------------------------------
	inline b32 tP2PNetwork::tExpectedConnection::operator != ( const tExpectedConnection& e ) const
	{
		return !( *this == e );
	}

	//------------------------------------------------------------------------------
	// tPeerIdGenerator
	//------------------------------------------------------------------------------
	tPeerId tP2PNetwork::tPeerIdGenerator::fNext( )
	{
		tPeerId id = mNextId;
		if( mReleasedIds.fCount( ) )
			mReleasedIds.fPop( id );
		else
			++mNextId;
		return id;
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::tPeerIdGenerator::fRelease( tPeerId id )
	{
		mReleasedIds.fPush( id );
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::tPeerIdGenerator::fReset( )
	{
		mNextId = 0;
		mReleasedIds.fClear( );
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::tPeerIdGenerator::fReset( const tGrowableArray<tPeerId>& reserved )
	{
		fReset( );

		// Set mNextId to 1 past the largest reserved id
		for( u32 i = 0; i < reserved.fCount( ); ++i )
			mNextId = fMax( mNextId, reserved[ i ] + 1 );

		// Release any ids that are not in our reserved list
		for( tPeerId id = 0; id < mNextId; ++id )
		{
			if( !reserved.fFind( id ) )
				fRelease( id );
		}
	}

	//------------------------------------------------------------------------------
	// tHostVote
	//------------------------------------------------------------------------------
	tP2PNetwork::tHostVote::tHostVote( )
	{
		fReset( );
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::tHostVote::fStart( tPeerId localPeerId, const tGrowableArray<tPeer>& peers )
	{
		mStartTime = enet_time_get( );

		// Set mDesiredHostId to the smallest id
		mDesiredHostId = localPeerId;
		for( u32 i = 0; i < peers.fCount( ); ++i )
		{
			if( peers[ i ].fId( ) < mDesiredHostId )
				mDesiredHostId = peers[ i ].fId( );
		}

		mState = cStateStarted;

		log_line( Log::cFlagNetwork, "HostVote::fStart: Voting for peer: " << mDesiredHostId );
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::tHostVote::fFinish( )
	{
		sigassert( mState == cStateStarted && "Can only finish from the Started state" );
		mState = cStateFinished;
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::tHostVote::fReset( )
	{
		mStartTime = 0;
		mDesiredHostId = cInvalidPeerId;
		mState = cStateNull;
	}

	//------------------------------------------------------------------------------
	u32 tP2PNetwork::tHostVote::fTimeElapsedMs( ) const
	{
		sigassert( fStarted( ) && "Host vote has not started" );
		return enet_time_get( ) - mStartTime;
	}

	//------------------------------------------------------------------------------
	// Utility functions
	//------------------------------------------------------------------------------
	namespace
	{
		const char* fStateToString( tP2PNetwork::tState state )
		{
#ifdef sig_devmenu
			switch( state )
			{
			case tP2PNetwork::cStateInactive: return "inactive";
			case tP2PNetwork::cStateHosting: return "hosting";
			case tP2PNetwork::cStateConnecting: return "connecting";
			case tP2PNetwork::cStateConnected: return "connected";
			case tP2PNetwork::cStateDisconnecting: return "disconnecting";
			case tP2PNetwork::cStateMigratingHost: return "migrating host";
			}
#endif //sig_devmenu

			return "";
		}
	}

	//------------------------------------------------------------------------------
	// tP2PNetwork
	//------------------------------------------------------------------------------
	tP2PNetwork::tP2PNetwork( )
		: mState( cStateInactive )
		, mUserHost( NULL )
		, mUserPeer( NULL )
		, mProtocolChannel( ~0 )
		, mLocalPeerId( cInvalidPeerId )
	{
	}

	//------------------------------------------------------------------------------
	b32 tP2PNetwork::fHost( 
		tP2PNetworkHost * userHost,
		u32 maxPeerCount,
		u32 channelCount,
		u32 hostAddress, 
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
		mLocalPeerId = mPeerIdGenerator.fNext( );
		fSetState( cStateHosting );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tP2PNetwork::fJoin( 
		tP2PNetworkPeer * userPeer,
		u32 maxPeerCount,
		u32 channelCount,
		u32 hostAddress,
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
		sigassert( !mExpectedConnections.fCount( ) && "Inactive networks should not expect connections" );

		if( hostAddress == 0 )
			return false;

		if( !mHost.fStartServer( maxPeerCount, channelCount + 1, clientAddress, clientPort ) )
			return false;

		tPeer peer = mHost.fConnect( hostAddress, channelCount + 1, 0, hostPort );
		if( !peer.fValid( ) )
		{
			mHost.fDestroy( );
			return false;
		}
		
		tPlatformAddr platformAddr;
		userPeer->fGetPlatformAddr( platformAddr, hostAddress, hostPort );
		peer.fSetPlatformAddr( platformAddr );

		// TODO: Either get the Id from the host, or do something special for host migration and session merging
		peer.fSetId( 0 );

		mProtocolChannel = channelCount;
		mUserPeer = userPeer;
		mWeakPeers.fPushBack( peer );
		fSetState( cStateConnecting );
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
		profile( cProfilePerfNetworkService );

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
			default:
				break;
			}
		}

#if defined( sig_devmenu )
		// Update stats
		if( mHost.fValid( ) )
		{
			mStats.mPacketsSent = mHost.fPacketsSent( );
			mStats.mPacketsReceived = mHost.fPacketsReceived( );
			mStats.mBytesSent = mHost.fBytesSent( );
			mStats.mBytesReceived = mHost.fBytesReceived( );
			mHost.fResetStats( );
		}
		else
		{
			mStats.fReset( );
		}
#endif

		fProcessWeakPeers( );

		// Update host migration
		if( mState == cStateMigratingHost && !mWeakPeers.fCount( ) )
		{
			if( !mHostVote.fStarted( ) )
			{
				mHostVote.fStart( mLocalPeerId, mStrongPeers );

				// Send a message to our peers informing them of our vote
				tGrowableArray< byte > msg;
				fBuildHostVote( msg );
				const u32 strongPeerCount = mStrongPeers.fCount( );
				for( u32 p = 0; p < strongPeerCount; ++p )
					fSendProtocolMsg( mStrongPeers[ p ], msg );
			}
			else if( !mHostVote.fFinished( ) )
			{
				// Check if all peers have voted
				b32 allPeersVoted = true;
				const u32 strongPeerCount = mStrongPeers.fCount( );
				for( u32 p = 0; p < strongPeerCount; ++p )
				{
					if( mStrongPeers[ p ].fDesiredHostId( ) == cInvalidPeerId )
					{
						allPeersVoted = false;
						break;
					}
				}

				if( allPeersVoted || mHostVote.fTimeElapsedMs( ) > cHostVoteTimeout )
					fFinishHostVote( );
			}
		}

		// We're disconnecting and all peers have been disconnected so destroy and go inactive
		if( mState == cStateDisconnecting && !mWeakPeers.fCount( ) && !mStrongPeers.fCount( ) )
			fDestroy( );
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fAcceptPeer( tPeer peer )
	{
		sigassert( mState == cStateHosting && "Peers can only be accepted by network hosts" );
		peer.fSetId( mPeerIdGenerator.fNext( ) );
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
	void tP2PNetwork::fMigrateHost( const tPeer & peer )
	{
		sigassert( mState == cStateMigratingHost && "Cannot migrate host from current state" );
		sigassert( mHostVote.fDesiredHostId( ) == peer.fId( ) && "Cannot migrate to a host we didn't vote for" );

		// Set desiredHost as the new host
		tPeer peerCopy = peer;
		mStrongPeers.fFindAndErase( peerCopy );
		mStrongPeers.fPushFront( peerCopy );
		fSetState( cStateConnected );
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
	void tP2PNetwork::fSendMsg( const tPeer & peer, u32 channel, const byte * data, u32 dataLength, b32 reliable )
	{
		sigassert( channel != mProtocolChannel && "Invalid attempt to send msg on P2PNetwork protocol channel" );
		sigassert( fIsConnectedOrHosting( ) && "Cannot send messages from current state" );

		peer.fSend( 
			data, dataLength, channel,
			reliable ? ENET_PACKET_FLAG_RELIABLE: 0 );
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fSendMsgToPeers( u32 channel, const byte * data, u32 dataLength, b32 reliable )
	{
		sigassert( channel != mProtocolChannel && "Invalid attempt to send msg on P2PNetwork protocol channel" );
		sigassert( fIsConnectedOrHosting( ) && "Cannot send messages from current state" );
		
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
	void tP2PNetwork::fSetState( tState state )
	{
		//log_line( Log::cFlagNetwork, "tP2PNetwork::fSetState " << fStateToString( mState ) << " -> " << fStateToString( state ) );
		mState = state;
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
		mLocalPeerId = cInvalidPeerId;
		mPeerIdGenerator.fReset( );

		// Go Inactive
		fSetState( cStateInactive );
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
				fSetState( cStateConnected );
				fStrengthenPeer( mWeakPeers[ 0 ], false );
			}
			else
				fAddUnknownWeakPeer( _peer );
			return;

		// New peers are connecting via handshakes from the server
		case cStateConnected:
			{
				// Add the peer if it doesn't already exist
				tPeer * peer = fFindPeer( _peer );
				if( !peer )
				{
					fAddUnknownWeakPeer( _peer );
					peer = &mWeakPeers.fBack( );
				}

				tPlatformAddr platformAddr;
				mUserPeer->fGetPlatformAddr( platformAddr, _peer->address.host, _peer->address.port );

				// Check if we've already been informed about this peer
				// and strengthen the connection if we have
				const u32 expectedCount = mExpectedConnections.fCount( );
				for( u32 e = 0; e < expectedCount; ++e )
				{
					if( mExpectedConnections[ e ] != platformAddr )
						continue;

					peer->fSetId( mExpectedConnections[ e ].mPeerId );
					fStrengthenPeer( *peer, true );
					mExpectedConnections.fErase( e );
					break;
				}
				
			} return;

		case cStateDisconnecting:

			// We don't accept new connections while disconnecting
			enet_peer_disconnect_now( _peer, cDisconnectReasonDisconnecting );
			return;
				
		default:
			break;
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
				tPlatformAddr platformAddr = peer->fPlatformAddr( );
				tPeerId peerId = peer->fId( );
				b32 isStrong = fIsStrong( peer );
				fErasePeer( peer );

				if( peerId != cInvalidPeerId )
					mPeerIdGenerator.fRelease( peerId );
				
				// Send our remaining peers that this peer has disconnected
				// from the server and should be rejected by them as well
				if( isStrong )
				{
					tGrowableArray<byte> msg;

					fBuildPeerDisconnect( msg, platformAddr );
					const u32 peerCount = mStrongPeers.fCount( );
					for( u32 p = 0; p < peerCount; ++p )
						fSendProtocolMsg( mStrongPeers[ p ], msg );
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
					{
						if( mLocalPeerId != cInvalidPeerId && tPeer::fGetLocalNatType( ) != cNatTypeStrict )
							fStartHostVote( );
						else
							fDisconnect( false, cDisconnectReasonServerDisconnect );
					}
					else
					{
						sigassert( acknowledge );
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
		case cStateInactive:
			break;
		}

		if( acknowledge )
			mUserPeer->fOnDisconnect( *this, tPeer( _peer ), reason );
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fOnReceive( const ENetEvent & e )
	{
		profile( cProfilePerfNetworkOnReceive );

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
		case cPacketTypeHostVote: // Peer
			fHandleHostVotePacket( peer, packet );
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
		if( packet->dataLength < sizeof( tHandShakePacket ) + sizeof( tPeerHandShakeInfo ) * handShake->mPeerCount )
		{
			fLogPeerWarning( "Corrupt handshake", peer );
			return;
		}

		sigassert( mLocalPeerId == cInvalidPeerId && "Local PeerId should not be valid yet" );
		mLocalPeerId = handShake->mJoinerId;

		mStrongPeers[ 0 ].fSetId( handShake->mHostId );
		
		// Attempt to connect to all my new peers
		tPeerHandShakeInfo * handShakeInfo = (tPeerHandShakeInfo *)( handShake + 1 );
		for( u32 p = 0; p < handShake->mPeerCount; ++p )
		{
			u32 address = mUserPeer->fGetAddress( handShakeInfo[ p ].mAddr );
			u16 port = mUserPeer->fGetPort( handShakeInfo[ p ].mAddr );

			mWeakPeers.fPushBack( mHost.fConnect( address, 0, 0, port ) );
			mWeakPeers.fBack( ).fSetPlatformAddr( handShakeInfo[ p ].mAddr );
			mWeakPeers.fBack( ).fSetId( handShakeInfo[ p ].mId );
		}
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fHandleExpectConnectionPacket( ENetPeer * peer, ENetPacket * packet )
	{
		// Decode the packet if possible
		tExpectConnectionPacket * expect = fDecodePacket<tExpectConnectionPacket>( packet, peer );
		if( !expect )
			return;

		sigassert( expect->mPlatformAddr.fValid( ) && "Received ExpectConnection packet for invalid address" );

		ENetAddress enetAddress;
		enetAddress.host = mUserPeer->fGetAddress( expect->mPlatformAddr );
		enetAddress.port = mUserPeer->fGetPort( expect->mPlatformAddr );

		sigassert( enetAddress.host != ENET_HOST_ANY && "Could not get peer address from platform address." );
		sigassert( enetAddress.port != ENET_PORT_ANY && "Could not get peer port from platform address." );

		tAddress addy( enetAddress );

		// First test if we have any weak connections we can resolve with this expectation
		b32 resolved = false;
		if( mState == cStateConnected )
		{
			const u32 weakPeerCount = mWeakPeers.fCount( );
			for( u32 p = 0; p < weakPeerCount; ++p )
			{
				if( addy != mWeakPeers[ p ].fAddress( ) )
					continue;

				mWeakPeers[ p ].fSetId( expect->mPeerId );
				fStrengthenPeer( mWeakPeers[ p ], true );
				resolved = true;
				break;
			}
		}

		// Add the expected connection to our reserve list
		if( !resolved )
		{
			sigassert( !mExpectedConnections.fFind( expect->mPlatformAddr ) && "Connection was already expected" );
			mExpectedConnections.fPushBack( tExpectedConnection( expect->mPlatformAddr, expect->mPeerId ) );
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
		if( !fHostPeer( ) || !( *fHostPeer( ) == _peer ) )
		{
			fLogPeerWarning( "Received host only packet from non-host", _peer );
			return;
		}

		tPeer * msgPeer = fFindPeer( disconnect->mPlatformAddr );
		
		// We don't have this peer so look for an 
		// expected connection and remove it
		if( !msgPeer )
		{
			const u32 ecCount = mExpectedConnections.fCount( );
			for( u32 ec = 0; ec < ecCount; ++ec )
			{
				if( mExpectedConnections[ ec ] == disconnect->mPlatformAddr )
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
	void tP2PNetwork::fHandleHostVotePacket( ENetPeer * _peer, ENetPacket * packet )
	{
		tHostVotePacket * vote = fDecodePacket<tHostVotePacket>( packet, _peer );
		if( !vote )
			return;

		tPeer * msgPeer = fFindPeer( _peer );
		msgPeer->fSetDesiredHostId( vote->mDesiredHostId );

		log_line( Log::cFlagNetwork, "Peer " << msgPeer->fId( ) << " voted for " << msgPeer->fDesiredHostId( ) );
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
	tPeer * tP2PNetwork::fFindPeer( const tPlatformAddr & platformAddr )
	{
		const u32 weakPeerCount = mWeakPeers.fCount( );
		for( u32 p = 0; p < weakPeerCount; ++p )
		{
			if( platformAddr == mWeakPeers[ p ].fPlatformAddr( ) )
				return &mWeakPeers[ p ];
		}

		const u32 strongPeerCount = mStrongPeers.fCount( );
		for( u32 p = 0; p < strongPeerCount; ++p )
		{
			if( platformAddr == mStrongPeers[ p ].fPlatformAddr( ) )
				return &mStrongPeers[ p ];
		}

		return NULL;
	}

	//------------------------------------------------------------------------------
	tPeer * tP2PNetwork::fFindPeer( tPeerId peerId )
	{
		const u32 weakPeerCount = mWeakPeers.fCount( );
		for( u32 p = 0; p < weakPeerCount; ++p )
		{
			if( peerId == mWeakPeers[ p ].fId( ) )
				return &mWeakPeers[ p ];
		}

		const u32 strongPeerCount = mStrongPeers.fCount( );
		for( u32 p = 0; p < strongPeerCount; ++p )
		{
			if( peerId == mStrongPeers[ p ].fId( ) )
				return &mStrongPeers[ p ];
		}

		return NULL;
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fAddUnknownWeakPeer( ENetPeer * peer )
	{
		tPlatformAddr platformAddr;
		mUserPeer->fGetPlatformAddr( platformAddr, peer->address.host, peer->address.port );

		// Give weak peers 3 seconds for their expect message to arrive
		peer->data = (void*)( enet_time_get( ) + cWeakTimeout );
		mWeakPeers.fPushBack( tPeer( peer, platformAddr ) );
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fStrengthenPeer( tPeer peer, b32 sendAcceptance )
	{	
		// Remove from the weak peers
		b32 found = mWeakPeers.fFindAndErase( peer );
		sigassert( found && "Peer passed to StrengthenPeer is non-weak" );
		sigassert( peer.fPlatformAddr( ).fValid( ) && "Peer passed to StrengthenPeer has an invalid PlatformAddr" );
		sigassert( peer.fId( ) != cInvalidPeerId && "Peer passed to StrengthenPeer has an invalid PeerId" );

#if defined( sig_assert )
		for( u32 p = 0; p < mStrongPeers.fCount( ); ++p )
			sigassert( mStrongPeers[ p ].fId( ) != peer.fId( ) && "Peer passed to StrengthenPeer has a duplicate PeerId" );
#endif

		// If we're the host introduce our peers to their new friend
		if( mState == cStateHosting )
		{
			// Send a hand shake with peer info to the new client
			tGrowableArray< byte > msg;
			fBuildHandShake( msg, peer );
			fSendProtocolMsg( peer, msg );

			// Send messages to the clients to expect and accept connections their new peer
			fBuildExpectConnection( msg, peer );
			const u32 strongPeerCount = mStrongPeers.fCount( );
			for( u32 p = 0; p < strongPeerCount; ++p )
				fSendProtocolMsg( mStrongPeers[ p ], msg );
		}

		// Otherwise report that we accepted the connection
		else if( sendAcceptance )
		{
			sigassert( mState == cStateConnected );

			tGrowableArray<byte> msg;
			fBuildConnectionAccepted( msg );
			fSendProtocolMsg( peer, msg );
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
	void tP2PNetwork::fSendProtocolMsg( tPeer & peer, const tGrowableArray<byte> & msg )
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

		fSetState( cStateDisconnecting );
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

		fSetState( cStateDisconnecting );
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fStartHostVote( )
	{
		sigassert( mState == cStateConnected && "StartHostVote called with invalid state" );
		sigassert( mLocalPeerId != cInvalidPeerId && "StartHostVote called with invalid local id" );

		mHostVote.fReset( );

		fSetState( cStateMigratingHost );
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fFinishHostVote( )
	{
		sigassert( mState == cStateMigratingHost && "FinishHostVote called with invalid state" );

		mHostVote.fFinish( );

		// Disconnect any peers that aren't going to migrate with us
		for( u32 i = 0; i < mStrongPeers.fCount( ); ++i )
		{
			tPeer& curPeer = mStrongPeers[ i ];
			if( curPeer.fDesiredHostId( ) != mHostVote.fDesiredHostId( ) )
			{
				log_line( Log::cFlagNetwork, "FinishHostVote: Disconnecting peer: " << curPeer.fId( ) );
				curPeer.fDisconnect( );
			}
		}

		if( mHostVote.fDesiredHostId( ) == mLocalPeerId )
		{
			// Become the host
			mUserHost = (tP2PNetworkHost *)mUserPeer;
			fSetState( cStateHosting );

			// Reset mPeerIdGenerator, reserving the current ids
			tGrowableArray<tPeerId> mReservedIds;
			mReservedIds.fPushBack( mLocalPeerId );
			for( u32 i = 0; i < mStrongPeers.fCount( ); ++i )
				mReservedIds.fPushBack( mStrongPeers[ i ].fId( ) );
			mPeerIdGenerator.fReset( mReservedIds );
		}
		else
		{
			// If our desired host is not going to become the host, disconnect
			tPeer* desiredHost = fFindPeer( mHostVote.fDesiredHostId( ) );
			if( !desiredHost || desiredHost->fDesiredHostId( ) != mHostVote.fDesiredHostId( ) )
				fDisconnect( false, cDisconnectReasonServerDisconnect );
		}

		if( !fIsMigratingHost( ) && fIsConnectedOrHosting( ) )
			mUserPeer->fOnBeginHosting( *this );
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fBuildHandShake( tGrowableArray<byte> & msg, const tPeer & peer )
	{
		const u32 strongPeerCount = mStrongPeers.fCount( );
		const u32 totalSize = sizeof(tHandShakePacket) + sizeof( tPeerHandShakeInfo ) * strongPeerCount;
		msg.fSetCount( totalSize );

		tHandShakePacket * shake = (tHandShakePacket *)msg.fBegin( );
		tPeerHandShakeInfo * peerInfo = (tPeerHandShakeInfo *)( shake + 1 );

		shake->mHeader.mPacketType = cPacketTypeHandShake;
		shake->mPeerCount = strongPeerCount;
		shake->mJoinerId = peer.fId( );
		shake->mHostId = mLocalPeerId;

		for( u32 p = 0; p < strongPeerCount; ++p )
		{
			const ENetAddress& peerAddress = mStrongPeers[ p ].fAddress( );
			mUserPeer->fGetPlatformAddr( peerInfo[ p ].mAddr, peerAddress.host, peerAddress.port );
			peerInfo[ p ].mId = mStrongPeers[ p ].fId( );
		}
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fBuildExpectConnection( tGrowableArray<byte> & msg, const tPeer & peer )
	{
		msg.fSetCount( sizeof( tExpectConnectionPacket ) );

		tExpectConnectionPacket * expect = (tExpectConnectionPacket *)msg.fBegin( );
		expect->mHeader.mPacketType = cPacketTypeExpectConnection;
		b32 gotPlatformAddr = mUserPeer->fGetPlatformAddr( expect->mPlatformAddr, peer.fAddress( ).host, peer.fAddress( ).port );
		sigassert( expect->mPlatformAddr.fValid( ) && "Built ExpectConnection packet with an invalid address" );
		sigassert( gotPlatformAddr && "Failed to get platform addr for ExpectConnection packet" );
		expect->mPeerId = peer.fId( );
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fBuildConnectionAccepted( tGrowableArray<byte> & msg )
	{
		msg.fSetCount( sizeof( tConnectionAcceptedPacket ) );

		tConnectionAcceptedPacket * accepted = (tConnectionAcceptedPacket *)msg.fBegin( );
		accepted->mHeader.mPacketType = cPacketTypeConnectionAccepted;
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fBuildPeerDisconnect( tGrowableArray<byte> & msg, const tPlatformAddr & platformAddr )
	{
		msg.fSetCount( sizeof( tPeerDisconnectPacket ) );
		
		tPeerDisconnectPacket * disconnect = (tPeerDisconnectPacket *)msg.fBegin( );
		disconnect->mHeader.mPacketType = cPacketTypePeerDisconnect;
		disconnect->mPlatformAddr = platformAddr;
	}

	//------------------------------------------------------------------------------
	void tP2PNetwork::fBuildHostVote( tGrowableArray<byte> & msg )
	{
		msg.fSetCount( sizeof( tHostVotePacket ) );

		tHostVotePacket * packet = (tHostVotePacket *)msg.fBegin( );
		packet->mHeader.mPacketType = cPacketTypeHostVote;
		packet->mDesiredHostId = mHostVote.fDesiredHostId( );
	}
}}
