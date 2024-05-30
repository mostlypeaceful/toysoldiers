//------------------------------------------------------------------------------
// \file tGameSessionSearchResult.hpp - 13 Oct 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tGameSessionSearchResult__
#define __tGameSessionSearchResult__

namespace Sig
{
#ifdef platform_xbox360

	typedef XNKID tGameSessionId;
	struct tGameSessionInfo : XSESSION_INFO
	{
		b32 operator==( const tGameSessionInfo& rhs ) const { return fMemCmp( this, &rhs, sizeof( *this ) ) == 0; }
	};
	struct tGameSessionSearchResult : XSESSION_SEARCHRESULT 
	{
		const tGameSessionInfo & fSessionInfo( ) const { return ( const tGameSessionInfo & )info; }
		const u32 fOpenPublicSlots( ) const { return dwOpenPublicSlots; }
		const u32 fOpenPrivateSlots( ) const { return dwOpenPrivateSlots; }
		const u32 fFilledPublicSlots( ) const { return dwFilledPublicSlots; }
		const u32 fFilledPrivateSlots( ) const { return dwFilledPrivateSlots; }
		const u32 fTotalPublicSlots( ) const { return dwOpenPublicSlots + dwFilledPublicSlots; }
		const u32 fTotalPrivateSlots( ) const { return dwOpenPrivateSlots + dwFilledPrivateSlots; }
		const u32 fTotalSlots( ) const { return fTotalPublicSlots( ) + fTotalPrivateSlots( ); }
	};

#else

	typedef u32 tGameSessionId;
	struct tGameSessionInfo
	{
		b32 operator==( const tGameSessionInfo& rhs ) const { return fMemCmp( this, &rhs, sizeof( *this ) ) == 0; }
	};
	struct tGameSessionSearchResult
	{
		const tGameSessionInfo & fSessionInfo( ) const { static tGameSessionInfo info; return info; }
		const u32 fOpenPublicSlots( ) const { return 0; }
		const u32 fOpenPrivateSlots( ) const { return 0; }
		const u32 fFilledPublicSlots( ) const { return 0; }
		const u32 fFilledPrivateSlots( ) const { return 0; }
		const u32 fTotalPublicSlots( ) const { return 0; }
		const u32 fTotalPrivateSlots( ) const { return 0; }
		const u32 fTotalSlots( ) const { return 0; }
	};

#endif
}

#endif//__tGameSessionSearchResult__
