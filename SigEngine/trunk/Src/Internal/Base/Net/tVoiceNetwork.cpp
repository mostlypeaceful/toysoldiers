//------------------------------------------------------------------------------
// \file tVoiceNetwork.cpp - 16 Dec 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tVoiceNetwork.hpp"
#include "enet/enet.h"
#include "tVoice.hpp"

namespace Sig { namespace Net
{
	namespace
	{
		struct tVoiceMsg
		{
			u16 mSizeOfU32;
			u32 mVoiceBytes;
		};
	}

	static_assert( sizeof( u64 ) >= sizeof( ENetSocket ) );

	//------------------------------------------------------------------------------
	tVoiceNetwork::tVoiceNetwork( u32 host, u32 port )
	{
		ENetSocket s = enet_socket_create_vdp( );
		sigassert( s != ENET_SOCKET_NULL && "Failed to create vdp socket" );

		ENetAddress addy = { host, port };
		int error = enet_socket_bind( s, &addy );
		if( error )
		{
			log_warning( "tVoiceNetwork: Failed to bind socket" );
			mSocket = ( u64 )ENET_SOCKET_NULL;
			return;
		}
		
		error = enet_socket_set_option( s, ENET_SOCKOPT_NONBLOCK, 1 );
		sigassert( !error && "Failed to make socket non-blocking" );

		mSocket = ( u64 )s;
	}

	//------------------------------------------------------------------------------
	tVoiceNetwork::~tVoiceNetwork( )
	{
		enet_socket_destroy( ( ENetSocket )mSocket );
	}

	//------------------------------------------------------------------------------
	void tVoiceNetwork::fAddPeer( u32 host, u32 port )
	{
		tAddress addy = { host, port };
		mPeers.fPushBack( addy );
	}

	//------------------------------------------------------------------------------
	void tVoiceNetwork::fRemovePeer( u32 host, u32 port )
	{
		tAddress addy = { host, port };
		mPeers.fFindAndErase( addy );
	}

	//------------------------------------------------------------------------------
	void tVoiceNetwork::fRemoveAllPeers( )
	{
		mPeers.fSetCount( 0 );
	}

	//------------------------------------------------------------------------------
	void tVoiceNetwork::fTick( f32 dt )
	{
		profile( cProfilePerfNetworkVoice );

		if( !tVoice::fInitialized( ) )
			return;

		tVoice & voice = tVoice::fInstance( );
		voice.fTick( dt );

		fSendData( );
		fReceiveData( );
	}

	//------------------------------------------------------------------------------
	void tVoiceNetwork::fSendData( )
	{
		tVoice & voice = tVoice::fInstance( );

		// Do we need to send any data?
		if( !voice.fNeedToSend( ) )
			return;

		const u32 voiceBytes = voice.fBytesToSendAll( );
		const u32 totalSize = sizeof( tVoiceMsg ) + voiceBytes;

		// Allocate our buffer and gather the voice data
		tDynamicBuffer buffer( totalSize );

		byte * ptr = buffer.fBegin( );

		// Set up the header
		tVoiceMsg * msg = ( tVoiceMsg * ) ptr;
		msg->mSizeOfU32 = sizeof( u32 );
		msg->mVoiceBytes = voiceBytes;
		ptr += sizeof( *msg );

		// Get the voice data
		voice.fGetVoiceData( ptr, voiceBytes );

		ENetBuffer netBuffer;
		netBuffer.data = buffer.fBegin( );
		netBuffer.dataLength = buffer.fCount( );
		
		const u32 peerCount = mPeers.fCount( );
		for( u32 p = 0; p < peerCount; ++p )
		{
			ENetAddress address;
			address.host = ( enet_uint32 )mPeers[ p ].mHost;
			address.port = ( enet_uint16 )mPeers[ p ].mPort;

			int sent = enet_socket_send( ( ENetSocket )mSocket, &address, &netBuffer, 1 );
			if( sent != netBuffer.dataLength )
				log_warning( "Failed to send " << netBuffer.dataLength - sent << " voice data bytes" );
		}
	}

	//------------------------------------------------------------------------------
	void tVoiceNetwork::fReceiveData( )
	{	
		// No socket
		if( ( ENetSocket )mSocket == ENET_SOCKET_NULL )
			return;

		tVoice & voice = tVoice::fInstance( );

		tDynamicBuffer buffer;
		buffer.fNewArray( tVoice::fMaxVoiceDataSize( ) );

		ENetBuffer netBuffer;
		netBuffer.dataLength = buffer.fCount( );
		netBuffer.data = buffer.fBegin( );

		// Get the header
		ENetAddress headerAddy;
		int result = enet_socket_receive( ( ENetSocket )mSocket, &headerAddy, &netBuffer, 1 );

		// Socket error
		if( result < 0 )
		{
			log_warning( "Call to receive on voice socket failed" );
			return;
		}

		// No data
		if( result == 0 )
			return;

		// Not a valid packet
		if( result < sizeof( tVoiceMsg ) )
		{
			log_warning( "Msg on voice socket was too small to be correct" );
			return;
		}

		// Unknown address
		if( !fIsAPeer( headerAddy.host, headerAddy.port ) )
		{
			log_warning( "Received data on voice socket from unknown peer" );
			return;
		}

		const tVoiceMsg * msg = ( tVoiceMsg * )buffer.fBegin( );

		// Corrupt message
		if( msg->mSizeOfU32 != sizeof( u32 ) )
		{
			log_warning( "Incoming voice data was corrupt" );
			return;
		}

		// Size of message doesn't match header spec
		if( result != sizeof( *msg ) + msg->mVoiceBytes )
		{
			log_warning( "Incoming voice data didn't match header specified length" );
			return;
		}

		voice.fSubmitVoiceData( ( const byte * )( msg + 1 ), msg->mVoiceBytes );
	}

	//------------------------------------------------------------------------------
	b32 tVoiceNetwork::fIsAPeer( u32 host, u32 port ) const
	{
		const u32 peerCount = mPeers.fCount( );
		for( u32 p = 0; p < peerCount; ++p )
		{
			if( mPeers[ p ].mHost == host && mPeers[ p ].mPort == port )
				return true;
		}

		return false;
	}
}}
