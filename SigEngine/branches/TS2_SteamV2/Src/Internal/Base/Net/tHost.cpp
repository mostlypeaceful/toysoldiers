//------------------------------------------------------------------------------
// \file tHost.cpp - 05 Oct 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "enet/enet.h"
#include "tHost.hpp"

namespace Sig { namespace Net
{
	const tAddr tHost::cDefaultAddress = ENET_HOST_ANY;

	//------------------------------------------------------------------------------
	tHost::tHost( )
		: mHost( NULL )
	{

	}

	//------------------------------------------------------------------------------
	tHost::~tHost( )
	{
		fDestroy( );
	}

	//------------------------------------------------------------------------------
	b32 tHost::fStartServer( 
		u32 maxPeerCount, 
		u32 channelCount, 
		tAddr address, 
		u32 port, 
		u32 inBandwidth, 
		u32 outBandwidth )
	{
		log_line( Log::cFlagNetwork, __FUNCTION__ " address " << address );
		fDestroy( );

		ENetAddress addy;
		addy.host = address;
		addy.port = port;

		mHost = enet_host_create( &addy, maxPeerCount, channelCount, inBandwidth, outBandwidth );
		return mHost != NULL;
	}

	//------------------------------------------------------------------------------
	b32 tHost::fStartClient( 
		u32 channelCount, 
		u32 inBandwidth, 
		u32 outBandwidth )
	{
		log_line( Log::cFlagNetwork, __FUNCTION__ );
		fDestroy( );

		mHost = enet_host_create( NULL, 1, channelCount, inBandwidth, outBandwidth );
		return mHost != NULL;
	}

	//------------------------------------------------------------------------------
	tPeer tHost::fConnect( tAddr address, u32 channelCount, u32 data, u32 port ) const
	{
		log_line( Log::cFlagNetwork, __FUNCTION__ " address " << address );
		sigassert( mHost && "Connect called on invalid host" );

		ENetAddress addy;
		addy.host = address;
		addy.port = port;

		ENetPeer * peer = enet_host_connect( 
			mHost, 
			&addy, 
			channelCount ? channelCount : mHost->channelLimit, 
			data );

		return tPeer( peer );
	}

	//------------------------------------------------------------------------------
	void tHost::fDestroy( )
	{
		log_line( Log::cFlagNetwork, __FUNCTION__ );
		if( !mHost )
			return;

		enet_host_destroy( mHost );
		mHost = NULL;
	}

	//------------------------------------------------------------------------------
	void tHost::fFlush( ) const
	{
		sigassert( mHost && "Flush called on invalid host" );
		enet_host_flush( mHost );
	}

	//------------------------------------------------------------------------------
	s32 tHost::fService( ENetEvent & e ) const
	{
		sigassert( mHost && "Service called on invalid host" );

		return enet_host_service( mHost, &e, 0 );
	}

	//------------------------------------------------------------------------------
	void tHost::fBroadcast( 
		const void * buffer, 
		u32 bufferSize, 
		u32 channelId, 
		u32 packetFlags, 
		b32 flush ) const
	{
		sigassert( mHost && "BroadcastMessage called on invalid host" );

		enet_host_broadcast( 
			mHost, 
			channelId,
			enet_packet_create( buffer, bufferSize, packetFlags ) );

		if( flush )
			fFlush( );
	}

	//------------------------------------------------------------------------------
	void tHost::fSend( 
		const tPeer & peer, 
		const void * buffer, 
		u32 bufferSize, 
		u32 channelId, 
		u32 packetFlags, 
		b32 flush ) const
	{
		sigassert( mHost && "Send called on invalid host" );
		sigassert( peer.mPeer && peer.mPeer->host == mHost && "Send called with peer not owned by host" );
		
		peer.fSend( buffer, bufferSize, channelId, packetFlags, flush );
	}

	//------------------------------------------------------------------------------
	u32 tHost::fAverageRTT( ) const
	{
		if( !mHost )
			return 0;

		u32 rtt = 0;
		u32 validPeers = 0;
		for( u32 p = 0; p < mHost->peerCount; ++p )
		{
			if( mHost->peers[ p ].state == ENET_PEER_STATE_CONNECTED )
			{
				rtt += mHost->peers[ p ].roundTripTime;
				++validPeers;
			}
		}

		return validPeers ? rtt /= validPeers : 0;
	}

	//------------------------------------------------------------------------------
	f32 tHost::fAveragePacketLoss( ) const
	{
		if( !mHost )
			return 1.f;

		u32 loss = 0;
		u32 validPeers = 0;
		for( u32 p = 0; p < mHost->peerCount; ++p )
		{
			if( mHost->peers[ p ].state == ENET_PEER_STATE_CONNECTED )
			{
				loss += mHost->peers[ p ].packetLoss;
				++validPeers;
			}
		}

		return validPeers ? ( loss / validPeers ) / (f32)ENET_PEER_PACKET_LOSS_SCALE : 1.f;
	}

}}
