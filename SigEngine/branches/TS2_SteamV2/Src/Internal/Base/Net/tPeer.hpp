//------------------------------------------------------------------------------
// \file tPeer.hpp - 05 Oct 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tPeer__
#define __tPeer__

typedef struct _ENetPeer ENetPeer;
typedef struct _ENetAddress ENetAddress;

namespace Sig { namespace Net
{
	///
	/// \class tPeer
	/// \brief 
	class tPeer
	{
	public:

		explicit tPeer( ENetPeer * peer = NULL );

		inline b32 fValid( ) const { return mPeer != NULL; }
		inline b32 operator== (const tPeer & other ) const { return other.mPeer == mPeer; }
		inline b32 operator== (const ENetPeer * other ) const { return other == mPeer; }

		const ENetAddress & fAddress( ) const;
		tAddr fAddressHost( ) const;
		u32 fAddressPort( ) const;

		void fReset( );
		void fDisconnect( b32 afterOutgoing = true, u32 data = 0 );
		void fDisconnectNow( u32 data = 0 );

		void * fData( ) const;
		void fSetData( void * data );
		
		void fSend( 
			const void * buffer, 
			u32 bufferSize, 
			u32 channel, 
			u32 packetFlags, 
			b32 flush = false ) const;

	private:
		friend class tHost;

		ENetPeer * mPeer;
	};
}}

#endif//__tPeer__
