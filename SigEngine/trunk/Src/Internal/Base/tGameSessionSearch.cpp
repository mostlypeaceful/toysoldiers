//------------------------------------------------------------------------------
// \file tGameSessionSearch.cpp - 13 Oct 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tGameSessionSearch.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	tGameSessionSearch::tGameSessionSearch( )
		: mState( cStateNull )
	{
	}

	//------------------------------------------------------------------------------
	tGameSessionSearch::~tGameSessionSearch( )
	{
		fCancel( );
	}

	//------------------------------------------------------------------------------
	u32 tGameSessionSearch::fResultCount( ) const
	{
		if( mState != cStateSuccess )
			return 0;

		return mConfirmedResults.fCount( );
	}

	//------------------------------------------------------------------------------
	const tGameSessionSearchResult & tGameSessionSearch::fResult( u32 idx ) const
	{
		sigassert( mState == cStateSuccess && "Can only access confirmed search results on successful searches" );

		return *mConfirmedResults[ idx ].mResult;
	}

	//------------------------------------------------------------------------------
	void tGameSessionSearch::fEraseResult( u32 idx )
	{
		sigassert( mState == cStateSuccess && "Can only access confirmed search results on successful searches" );

		mConfirmedResults.fErase( idx );
	}

	//------------------------------------------------------------------------------
	// tConfirmedResult
	//------------------------------------------------------------------------------
	tGameSessionSearch::tConfirmedResult::tConfirmedResult( )
		: mResult( NULL )
		, mCustomData( NULL )
		, mCustomDataSize( 0 )
	{ }
}
