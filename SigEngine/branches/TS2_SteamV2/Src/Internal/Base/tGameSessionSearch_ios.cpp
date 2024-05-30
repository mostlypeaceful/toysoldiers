//------------------------------------------------------------------------------
// \file tGameSessionSearch_ios.cpp - 19 Oct 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#if defined( platform_ios )
#include "tGameSessionSearch.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	tGameSessionSearch::tGameSessionSearch( )
		: mState( cStateNull )
	{
	}

	void tGameSessionSearch::fCancel( )
	{
		log_warning_unimplemented( 0 );
	}
	
	void tGameSessionSearch::fTick( )
	{
		log_warning_unimplemented( 0 );
	}
	
	b32 tGameSessionSearch::fBegin( 
								   u32 userIndex,
								   u32 gameType,
								   u32 gameMode,
								   u32 procedure, 
								   u32 maxResultCount, 
								   u32 numUsers )
	{
		log_warning_unimplemented( 0 );
		return false;
	}
	
	b32 tGameSessionSearch::fBegin( u32 userIndex, const tGameSessionInfo & info )
	{
		log_warning_unimplemented( 0 );
		return false;
	}
	
	void tGameSessionSearch::fSetContext( u32 id, u32 value )
	{
		log_warning_unimplemented( 0 );
	}
	
	void tGameSessionSearch::fSetProperty( u32 id, const tUserData & value )
	{
		log_warning_unimplemented( 0 );
	}
	
	u32 tGameSessionSearch::fFindOpenResult( u32 slotsNeeded ) const
	{
		log_warning_unimplemented( 0 );
		return ~0;
	}
	
	u32 tGameSessionSearch::fResultCount( ) const
	{
		log_warning_unimplemented( 0 );
		return 0;
	}
	
	const tGameSessionSearchResult & tGameSessionSearch::fResult( u32 idx ) const
	{
		log_warning_unimplemented( 0 );
		static tGameSessionSearchResult result;
		return result;
	}
	
	u32 tGameSessionSearch::fResultContext( u32 idx, u32 context ) const
	{
		log_warning_unimplemented( 0 );
		return ~0;
	}
	
	void tGameSessionSearch::fResultProperty( u32 idx, tUserProperty & prop ) const
	{
		log_warning_unimplemented( 0 );
		prop.mData.fReset( );
	}
}
#endif//#if defined( platform_ios )
