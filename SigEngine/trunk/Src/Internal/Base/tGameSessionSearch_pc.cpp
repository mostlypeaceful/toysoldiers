//------------------------------------------------------------------------------
// \file tGameSessionSearch_pc.cpp - 19 Oct 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#if defined( platform_pcdx ) || defined( platform_metro )
#include "tGameSessionSearch.hpp"

namespace Sig
{
	void tGameSessionSearch::fCancel( )
	{
		log_warning_unimplemented( );
	}

	void tGameSessionSearch::fTick( )
	{
		log_warning_unimplemented( );
	}

	b32 tGameSessionSearch::fBegin( 
		u32 userIndex,
		u32 gameType,
		u32 gameMode,
		u32 procedure, 
		u32 maxResultCount, 
		u32 numUsers )
	{
		log_warning_unimplemented( );
		return false;
	}

	b32 tGameSessionSearch::fBegin( u32 userIndex, const tGameSessionInfo & info )
	{
		log_warning_unimplemented( );
		return false;
	}

	b32 tGameSessionSearch::fBeginFriendsOnly( 
		u32 userIndex,
		u32 gameType,
		u32 gameMode,
		u32 numUsers )
	{
		log_warning_unimplemented( );
		return false;
	}

	b32 tGameSessionSearch::fBegin(
		u32 userIndex,
		const tGameSessionId* ids,
		u32 numIds,
		u32 numUsers )
	{
		log_warning_unimplemented( );
		return false;
	}

	void tGameSessionSearch::fSetContext( u32 id, u32 value )
	{
		log_warning_unimplemented( );
	}

	void tGameSessionSearch::fSetProperty( u32 id, const tUserData & value )
	{
		log_warning_unimplemented( );
	}

	u32 tGameSessionSearch::fFindOpenResult( u32 slotsNeeded ) const
	{
		log_warning_unimplemented( );
		return ~0;
	}

	u32 tGameSessionSearch::fUnconfirmedResultCount( ) const
	{
		log_warning_unimplemented( );
		return 0;
	}

	const tGameSessionSearchResult & tGameSessionSearch::fUnconfirmedResult( u32 idx ) const
	{
		log_warning_unimplemented( );
		static tGameSessionSearchResult result;
		return result;
	}

	u32 tGameSessionSearch::fResultContext( u32 idx, u32 context ) const
	{
		log_warning_unimplemented( );
		return ~0;
	}

	void tGameSessionSearch::fResultProperty( u32 idx, tUserProperty & prop ) const
	{
		log_warning_unimplemented( );
		prop.mData.fReset( );
	}

	void tGameSessionSearch::fResultCustomData( u32 idx, const byte*& data, u32& dataSize ) const
	{
		log_warning_unimplemented( );
		data = NULL;
		dataSize = 0;
	}
}
#endif // #if defined( platform_pcdx ) || defined( platform_metro )
