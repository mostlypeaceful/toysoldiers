//------------------------------------------------------------------------------
// \file tPeekingJsonTokenizerSharedState.cpp - 4 Oct 2013
// \author mrickert
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------

#include "BasePch.hpp"
#include "tPeekingJsonTokenizerSharedState.hpp"
#include "tJsonCommon.hpp"
#include "tPeekingJsonTokenizer.hpp"

namespace Sig
{
	tPeekingJsonTokenizerSharedState::tPeekingJsonTokenizerSharedState( const char* buffer, u32 bufferSize )
		: mTokenizer( buffer, bufferSize )
		, mHeadToken( 0 )
	{
	}

	tPeekingJsonTokenizerSharedState::~tPeekingJsonTokenizerSharedState( )
	{
		log_assert( mPeekingTokenizers.fCount( ) == 0, "All peeking tokenizers should've unsubscribed from this shared state if it's being destroyed..." );
	}

	b32 tPeekingJsonTokenizerSharedState::fTryPushMorePeek( )
	{
		Json::tTokenType tt;
		u32 tokenLength;
		u32 newBufferPos;
		if( !mTokenizer.fReadToken( &tt, &tokenLength, &newBufferPos ) )
			return false;

		tStoredToken& st = mPeekedTokens.fPushBack( );
		st.mTokenType = tt;
		st.mTokenLength = tokenLength;
		st.mNewBufferPos = newBufferPos;
		if( !Json::fHasValue( tt ) )
			return true; // already done!

		const u32 valueLength = tokenLength + 1 - ( ( tt == Json::cTokenFieldName || tt == Json::cTokenString ) ? 2 : 0 );
		st.mTokenData.fResize( valueLength );
		sigcheckfail( mTokenizer.fGetTokenValue( (char*)st.mTokenData.fBegin( ), st.mTokenData.fCount( ) ), return true );
		return true;
	}

	void tPeekingJsonTokenizerSharedState::fPopAnyUnnecessaryHead( )
	{
		u32 earliestDesiredHead = ~0u;
		for( u32 i = 0; i < mPeekingTokenizers.fCount( ); ++i )
		{
			u32 headRequestedToken = mPeekingTokenizers[ i ]->fCurrentTokenIndex( );
			if( headRequestedToken == ~0u ) // haven't gotten any tokens yet
				headRequestedToken = 0;

			earliestDesiredHead = fMin( headRequestedToken, earliestDesiredHead );
		}

		while( mHeadToken < earliestDesiredHead && mPeekedTokens.fCount( ) )
		{
			mPeekedTokens.fPopFront( );
			++mHeadToken;
		}
	}
}
