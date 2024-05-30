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
#ifdef platform_xbox360
	struct tPlatformAddr
	{
		inline tPlatformAddr( ) { fZeroOut(xnAddr); }

		inline b32 fValid( ) const { return xnAddr.ina.S_un.S_addr > 0 || xnAddr.inaOnline.S_un.S_addr > 0; }
		inline b32 operator== (const tPlatformAddr & other ) const { return fMemCmp( this, &other, sizeof( *this ) ) == 0; }
		inline b32 operator!= (const tPlatformAddr & other ) const { return !operator==( other ); }

		XNADDR xnAddr;
	};

	typedef u32 tNatType;
	const tNatType cNatTypeInvalid = 0;
	const tNatType cNatTypeOpen = XONLINE_NAT_OPEN;
	const tNatType cNatTypeModerate = XONLINE_NAT_MODERATE;
	const tNatType cNatTypeStrict = XONLINE_NAT_STRICT;
#else
	struct tPlatformAddr
	{
		inline tPlatformAddr( ) : inAddr( 0 ), port( 0 ) { }

		inline b32 fValid( ) const { return inAddr > 0; }
		inline b32 operator== (const tPlatformAddr & other ) const { return fMemCmp( this, &other, sizeof( *this ) ) == 0; }
		inline b32 operator!= (const tPlatformAddr & other ) const { return !operator==( other ); }

		u32 inAddr;
		u32 port;
	};

	typedef u32 tNatType;
	const tNatType cNatTypeInvalid = 0;
	const tNatType cNatTypeOpen = 1;
	const tNatType cNatTypeModerate = 2;
	const tNatType cNatTypeStrict = 3;
#endif

	typedef u32 tPeerId;
	static const tPeerId cInvalidPeerId = ~0;

	///
	/// \class tPeer
	/// \brief 
	class base_export tPeer
	{
	public:

		explicit tPeer( ENetPeer * peer = NULL );
		tPeer( ENetPeer * peer, const tPlatformAddr& platformAddr );

		inline b32 fValid( ) const { return mPeer != NULL; }
		inline b32 operator== (const tPeer & other ) const { return other.mPeer == mPeer; }
		inline b32 operator== (const ENetPeer * other ) const { return other == mPeer; }

		const ENetAddress & fAddress( ) const;
		u32 fAddressHost( ) const;
		u32 fAddressPort( ) const;

		const tPlatformAddr & fPlatformAddr( ) const { return mPlatformAddr; }
		void fSetPlatformAddr( const tPlatformAddr& platformAddr ) { mPlatformAddr = platformAddr; }

		void fReset( );
		void fDisconnect( b32 afterOutgoing = true, u32 data = 0 );
		void fDisconnectNow( u32 data = 0 );

		void * fData( ) const;
		void fSetData( void * data );

		tPeerId fId( ) const { return mId; }
		void fSetId( tPeerId id ) { mId = id; }

		tPeerId fDesiredHostId( ) const { return mDesiredHostId; }
		void fSetDesiredHostId( tPeerId id ) { mDesiredHostId = id; }
		
		void fSend( 
			const void * buffer, 
			u32 bufferSize, 
			u32 channel, 
			u32 packetFlags, 
			b32 flush = false ) const;

		b32 fShouldThrottle( u32 maxRTT ) const;

	public:

		static tNatType fGetLocalNatType( );

	private:
		friend class tHost;

		ENetPeer * mPeer;
		tPlatformAddr mPlatformAddr;
		tPeerId mId;
		tPeerId mDesiredHostId; //For host migration
	};
}}

#endif//__tPeer__
