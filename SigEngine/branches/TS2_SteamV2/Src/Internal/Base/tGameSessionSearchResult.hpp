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

	struct tGameSessionInfo : XSESSION_INFO { };
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

#if defined( use_steam )
	struct tGameSessionInfo
	{
		explicit tGameSessionInfo( CSteamID id ) { mId = id; }
		tGameSessionInfo( ) { }

		b32 operator==( const tGameSessionInfo& rhs ) const { return fMemCmp( this, &rhs, sizeof( *this ) ) == 0; }
		b32 fIsValid( ) const { return mId.IsValid( ); }

		CSteamID mId;
	};
#else
	struct tGameSessionInfo {};
#endif

	struct tGameSessionSearchResult
	{
		explicit tGameSessionSearchResult( const tGameSessionInfo & info );
		tGameSessionSearchResult( ) : mInfo() { mTotalSlots = 0; mFilledSlots = 0; }

		const tGameSessionInfo & fSessionInfo( ) const { return mInfo; }
		const u32 fOpenPublicSlots( ) const { return mTotalSlots - mFilledSlots; }
		const u32 fOpenPrivateSlots( ) const { return 0; }
		const u32 fFilledPublicSlots( ) const { return mFilledSlots; }
		const u32 fFilledPrivateSlots( ) const { return 0; }
		const u32 fTotalPublicSlots( ) const { return mTotalSlots; }
		const u32 fTotalPrivateSlots( ) const { return 0; }
		const u32 fTotalSlots( ) const { return mTotalSlots; }

		tGameSessionInfo mInfo;
		u32 mTotalSlots;
		u32 mFilledSlots;
	};

#endif
}

#endif//__tGameSessionSearchResult__
