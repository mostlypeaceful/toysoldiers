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
	devvar( tNatType, Network_NatTypeOverride, cNatTypeInvalid );

	//------------------------------------------------------------------------------
	tPeer::tPeer( ENetPeer * peer )
		: mPeer( peer )
		, mId( cInvalidPeerId )
		, mDesiredHostId( cInvalidPeerId )
	{
		fZeroOut( mPlatformAddr );
	}

	//------------------------------------------------------------------------------
	tPeer::tPeer( ENetPeer * peer, const tPlatformAddr& platformAddr )
		: mPeer( peer ), mPlatformAddr( platformAddr ), mId( cInvalidPeerId ), mDesiredHostId( cInvalidPeerId )
	{
	}

	//------------------------------------------------------------------------------
	const ENetAddress & tPeer::fAddress( ) const
	{ 
		return mPeer->address; 
	}

	//------------------------------------------------------------------------------
	u32 tPeer::fAddressHost( ) const
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
		fZeroOut( mPlatformAddr );
		mId = cInvalidPeerId;
	}

	//------------------------------------------------------------------------------
	void tPeer::fDisconnect( b32 afterOutgoing, u32 data )
	{
		sigassert( mPeer && "Disconnect called on invalid peer" );
		if( afterOutgoing )
			enet_peer_disconnect_later( mPeer, data );
		else
			enet_peer_disconnect( mPeer, data );

	}

	//------------------------------------------------------------------------------
	void tPeer::fDisconnectNow( u32 data )
	{
		sigassert( mPeer && "DisconnectNow called on invalid peer" );
		enet_peer_disconnect_now( mPeer, data );
		mPeer = NULL;
		fZeroOut( mPlatformAddr );
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

	//------------------------------------------------------------------------------
	b32 tPeer::fShouldThrottle( u32 maxRTT ) const
	{
		// Never throttle if our outgoing queue is empty
		if( enet_list_empty( const_cast<ENetList*>( &mPeer->sentReliableCommands ) ) )
			return false;

		return mPeer->packetThrottle == 0 || mPeer->roundTripTime > maxRTT;
	}

	//------------------------------------------------------------------------------
	tNatType tPeer::fGetLocalNatType( )
	{
		if( Network_NatTypeOverride != cNatTypeInvalid )
			return Network_NatTypeOverride;

#ifdef platform_xbox360
		return XOnlineGetNatType( );
#else
		log_warning_unimplemented( );
		return cNatTypeOpen;
#endif
	}
}}
