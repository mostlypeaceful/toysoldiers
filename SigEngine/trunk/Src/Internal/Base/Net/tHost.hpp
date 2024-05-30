//------------------------------------------------------------------------------
// \file tHost.hpp - 05 Oct 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tHost__
#define __tHost__
#include "tPeer.hpp"

#include "enet/enet.h" //For the ENetSocket typedef

typedef struct _ENetHost ENetHost;
typedef struct _ENetEvent ENetEvent;

namespace Sig { namespace Net
{
	///
	/// \class tHost
	/// \brief 
	class base_export tHost
	{
	public:

		static const u32 cDefaultAddress;
		static const u32 cDefaultPort;

		tHost( );
		~tHost( );

		inline b32 fValid( ) const { return mHost != NULL; }
		const ENetSocket& fSocket( ) const;

		b32 fStartServer( 
			u32 maxPeerCount, 
			u32 channelCount, 
			u32 address = cDefaultAddress, 
			u32 port = cDefaultPort, 
			u32 inBandwidth = 0, 
			u32 outBandwidth = 0 );

		b32 fStartClient( 
			u32 channelCount, 
			u32 inBandwidth = 0, 
			u32 outBandwidth = 0  );

		tPeer fConnect(
			u32 address, 
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

		u32 fPacketsSent( ) const;
		u32 fPacketsReceived( ) const;
		u32 fBytesSent( ) const;
		u32 fBytesReceived( ) const;
		void fResetStats( );
		
	private:

		ENetHost * mHost;
	};

}}

#endif//__tHost__
