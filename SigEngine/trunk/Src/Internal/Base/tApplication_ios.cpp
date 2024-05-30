#include "BasePch.hpp"
#if defined( platform_ios )
#include "tApplication.hpp"
#include "tGameSessionSearchResult.hpp"
#include "enet/enet.h"
#include <sys/time.h>

namespace Sig
{
	tGameInvite::tGameInvite( u32 localHwIndex )
		: mLocalHwIndex( localHwIndex )
	{
	}

	const tGameSessionInfo & tGameInvite::fSessionInfo( ) const
	{
		static tGameSessionInfo info;
		return info;
	}

	tApplicationPlatformBase::tApplicationPlatformBase( )
	{
	}

	void tApplicationPlatformBase::fShowWindow( u32 width, u32 height, s32 x, s32 y, b32 maximize )
	{
	}

	void tApplicationPlatformBase::fGetLocalSystemTime( u32& hour, u32& minute, u32& second )
	{
		const time_t t = time( NULL );
		struct tm lt = *localtime( &t );
		
		hour = lt.tm_hour;
		minute = lt.tm_min;
		second = lt.tm_sec;
	}

	//HACK: hack to help artists debug stuff fading out
	const Math::tVec3f& tApplication::fFadePos( const Gfx::tCamera& camera ) const
	{
		return camera.fGetTripod( ).mEye;
	}

	void tApplication::fQuitAsync( b32 reboot )
	{
		mKeepRunning = false;

		enet_deinitialize( );
		
		log_warning_unimplemented( );
	}

	b32 tApplication::fPreRun( )
	{
		enet_initialize( );

		return true;
	}

	void tApplication::fPostRun( )
	{
	}

	void tApplication::fSetNetworkTimeouts( u32 peerMin, u32 peerMax )
	{
		enet_set_peer_timeouts( peerMin, peerMax );
	}

	void tApplication::fPreOnTick( )
	{
	}

	void tApplication::fPostOnTick( )
	{
	}

	b32 tApplication::fCacheCurrentGameInvite( u32 localHwIndex )
	{
		if( mGameInvite && mGameInvite->mLocalHwIndex != localHwIndex )
		{
			log_warning( 
					"User [" << localHwIndex << "] has attempted to accept a game invite while user [" 
				<< mGameInvite->mLocalHwIndex << "] is already in the process of trying to join a game - ignoring new User [" 
				<< localHwIndex << "]'s invite." );

			return false;
		}

		// consider buffering these? i.e., the invite has been accepted, but might be awhile before we get into lobby - more
		// invites could conceivably be accepted before we get there - if so, always prefer the latest one? check with MSFT on acceptable behavior in this case
		mGameInvite.fReset( NEW tGameInvite( localHwIndex ) );
		return true;
	}

	void tApplication::fSetGameRoot( )
	{
		log_warning_unimplemented( );
		mGameRoot = tFilePathPtr( "game:\\" );
	}
}
#endif//#if defined( platform_ios )
