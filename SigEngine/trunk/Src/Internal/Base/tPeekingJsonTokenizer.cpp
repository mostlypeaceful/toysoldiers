//------------------------------------------------------------------------------
// \file tPeekingJsonTokenizer.hpp - 4 Oct 2013
// \author mrickert
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------

#include "BasePch.hpp"
#include "tPeekingJsonTokenizer.hpp"
#include "tPeekingJsonTokenizerSharedState.hpp"

namespace Sig
{
	tPeekingJsonTokenizer::tPeekingJsonTokenizer( )
		: mSharedState( )
		, mCurrentToken( ~0u )
	{
	}

	tPeekingJsonTokenizer::tPeekingJsonTokenizer( const char* buffer, u32 bufferSize )
		: mSharedState( NEW_TYPED( tPeekingJsonTokenizerSharedState )( buffer, bufferSize ) )
		, mCurrentToken( ~0u )
	{
		fRegister( );
	}

	tPeekingJsonTokenizer::tPeekingJsonTokenizer( const tPeekingJsonTokenizer& other )
		: mSharedState( other.mSharedState )
		, mCurrentToken( other.mCurrentToken )
	{
		fRegister( );
	}

	tPeekingJsonTokenizer::~tPeekingJsonTokenizer( )
	{
		fUnregister( );
	}

	tPeekingJsonTokenizer& tPeekingJsonTokenizer::operator=( const tPeekingJsonTokenizer& other )
	{
		if( &other == this )
			return *this;

		fUnregister( );
		mSharedState = other.mSharedState;
		mCurrentToken = other.mCurrentToken;
		fRegister( );

		return *this;
	}

	b32 tPeekingJsonTokenizer::fReadToken( Json::tTokenType* tokenType, u32* tokenLength, u32* newBufferPos )
	{
		log_sigcheckfail( mSharedState.fGetRawPtr( ), "Haven't initialized the tPeekingJsonTokenizer to a buffer yet!", return false );

		const u32 originalToken = mCurrentToken;

		if( fPeekedTokensIndex( ) + 1 >= mSharedState->mPeekedTokens.fCount( ) && !mSharedState->fTryPushMorePeek( ) )
			return false;

		++mCurrentToken;
		if( mSharedState->mHeadToken == originalToken )
			mSharedState->fPopAnyUnnecessaryHead( );

		tPeekingJsonTokenizerSharedState::tStoredToken& st = mSharedState->mPeekedTokens[ fPeekedTokensIndex( ) ];
		if( tokenType )
			*tokenType = st.mTokenType;
		if( tokenLength )
			*tokenLength = st.mTokenLength;
		if( newBufferPos )
			*newBufferPos = st.mNewBufferPos;

		return true;
	}

	b32 tPeekingJsonTokenizer::fGetTokenValue( char* buffer, u32 maxBufferChars ) const
	{
		log_sigcheckfail( mSharedState.fGetRawPtr( ), "Haven't initialized the tPeekingJsonTokenizer to a buffer yet!", return false );
		log_sigcheckfail( mCurrentToken != ~0u, "Haven't yet read an initial token, call fReadToken first!", return false );
		sigcheckfail( fPeekedTokensIndex( ) < mSharedState->mPeekedTokens.fCount( ), return false );

		tPeekingJsonTokenizerSharedState::tStoredToken& st = mSharedState->mPeekedTokens[ fPeekedTokensIndex( ) ];
		if( !st.mTokenData.fCount( ) )
			return false;

		const u32 n = fMin( maxBufferChars-1, st.mTokenData.fCount( ) );
		fMemCpy( buffer, st.mTokenData.fBegin( ), n );
		buffer[ n ] = '\0';
		return true;
	}

	u32 tPeekingJsonTokenizer::fCurrentTokenIndex( ) const
	{
		return mCurrentToken;
	}

	u32 tPeekingJsonTokenizer::fPeekedTokensIndex( ) const
	{
		return mCurrentToken - mSharedState->mHeadToken;
	}

	void tPeekingJsonTokenizer::fRegister( )
	{
		sigcheckfail( mSharedState.fGetRawPtr( ), return );
		mSharedState->mPeekingTokenizers.fPushBack( this );
	}

	void tPeekingJsonTokenizer::fUnregister( )
	{
		if( mSharedState )
			mSharedState->mPeekingTokenizers.fFindAndErase( this );
	}
}
