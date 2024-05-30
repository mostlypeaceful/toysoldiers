//------------------------------------------------------------------------------
// \file tPeer.cpp - 05 Oct 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tPeer.hpp"
#include "enet/enet.h"


namespace Sig { namespace Net
{

	//------------------------------------------------------------------------------
	tPeer::tPeer( ENetPeer * peer )
		: mPeer( peer )
	{
	}

	//------------------------------------------------------------------------------
	const ENetAddress & tPeer::fAddress( ) const
	{ 
		return mPeer->address; 
	}

	//------------------------------------------------------------------------------
	tAddr tPeer::fAddressHost( ) const
	{
		return mPeer->address.host;
	}

	//------------------------------------------------------------------------------
	u32 tPeer::fAddressPort( ) const
	{
		return mPeer->address.port;
	}

	//------------------------------------------------------------------------------
	void tPeer::fReset( )
	{
		sigassert( mPeer && "Reset called on invalid peer" );
		enet_peer_reset( mPeer );
		mPeer = NULL;
	}

	//------------------------------------------------------------------------------
	void tPeer::fDisconnect( b32 afterOutgoing, u32 data )
	{
		log_line( Log::cFlagNetwork, __FUNCTION__ );
		sigassert( mPeer && "Disconnect called on invalid peer" );
		if( afterOutgoing )
			enet_peer_disconnect_later( mPeer, data );
		else
			enet_peer_disconnect( mPeer, data );

	}

	//------------------------------------------------------------------------------
	void tPeer::fDisconnectNow( u32 data )
	{
		log_line( Log::cFlagNetwork, __FUNCTION__ );
		sigassert( mPeer && "DisconnectNow called on invalid peer" );
		enet_peer_disconnect_now( mPeer, data );
		mPeer = NULL;
	}

	//------------------------------------------------------------------------------
	void * tPeer::fData( ) const
	{
		sigassert( mPeer && "Data requested from invalid peer" );
		return mPeer->data;
	}

	//------------------------------------------------------------------------------
	void tPeer::fSetData( void * data )
	{
		sigassert( mPeer && "Data cannot be set on invalid peer" );
		mPeer->data = data;
	}

	//------------------------------------------------------------------------------
	void tPeer::fSend( 
		const void * buffer, 
		u32 bufferSize, 
		u32 channel, 
		u32 packetFlags, 
		b32 flush ) const
	{
		sigassert( mPeer && "Send called on invalid peer" );

		// Create and queue the packet for sending
		enet_peer_send( 
			mPeer, 
			channel, 
			enet_packet_create( buffer, bufferSize, packetFlags ) );

		if( flush )
			enet_host_flush( mPeer->host );
	}
}}
