//------------------------------------------------------------------------------
// \file tHost.hpp - 05 Oct 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tHost__
#define __tHost__
#include "tPeer.hpp"

typedef struct _ENetHost ENetHost;
typedef struct _ENetEvent ENetEvent;

namespace Sig { namespace Net
{
	///
	/// \class tHost
	/// \brief 
	class tHost
	{
	public:

		static const tAddr cDefaultAddress;
#if defined( use_steam )
		static const u32 cDefaultPort = 0; // Port is used as a channel in Steam P2P
#else
		static const u32 cDefaultPort = 1000; // Recommended by XBOX Live
#endif

		tHost( );
		~tHost( );

		inline b32 fValid( ) const { return mHost != NULL; }

		b32 fStartServer( 
			u32 maxPeerCount, 
			u32 channelCount, 
			tAddr address = cDefaultAddress, 
			u32 port = cDefaultPort, 
			u32 inBandwidth = 0, 
			u32 outBandwidth = 0 );

		b32 fStartClient( 
			u32 channelCount, 
			u32 inBandwidth = 0, 
			u32 outBandwidth = 0  );

		tPeer fConnect(
			tAddr address, 
			u32 channelCount = 0, // 0 opens the maximum number of channels
			u32 data = 0, 
			u32 port = cDefaultPort ) const;

		void fDestroy( );
		void fFlush( ) const;

		s32 fService( ENetEvent & e ) const;

		void fBroadcast( 
			const void * buffer, 
			u32 bufferSize, 
			u32 channelId, 
			u32 packetFlags, 
			b32 flush = false ) const;

		void fSend( 
			const tPeer & peer, 
			const void * buffer, 
			u32 bufferSize, 
			u32 channelId, 
			u32 packetFlags, 
			b32 flush = false ) const;

		u32 fAverageRTT( ) const;
		f32 fAveragePacketLoss( ) const;
		
	private:

		ENetHost * mHost;
	};

}}

#endif//__tHost__
