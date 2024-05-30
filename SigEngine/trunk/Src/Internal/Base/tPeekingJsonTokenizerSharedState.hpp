//------------------------------------------------------------------------------
// \file tPeekingJsonTokenizerSharedState.hpp - 4 Oct 2013
// \author mrickert
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __tPeekingJsonTokenizerSharedState__
#define __tPeekingJsonTokenizerSharedState__

#include "tJsonTokenizer.hpp"

namespace Sig
{
	class tPeekingJsonTokenizer;

	/// \brief	Implementation detail of tPeekingJsonTokenizer.  Contains state shared between multiple related
	///			tPeekingJsonTokenizer s -- namely, the underlying tJsonTokenizer, and tokens already ready from
	///			said tokenizer but still required by one or more tPeekingJsonTokenizer s.
	class base_export tPeekingJsonTokenizerSharedState : public tRefCounter
	{
	public:
		/// \brief	Used to create a new family of tPeekingJsonTokenizer s unrelated to any existing family,
		///			given a buffer of JSON to start tokenizing.
		/// \param	buffer		Low-ASCII/UTF8 JSON to decode.  Doesn't have to be '\0' terminated.
		/// \param	bufferSize	The length of the buffer up to but not including any final terminating '\0'.
		tPeekingJsonTokenizerSharedState( const char* buffer, u32 bufferSize );

		~tPeekingJsonTokenizerSharedState( );

		/// \brief	Try to read more tokens from mTokenizer into mPeekedTokens.
		/// \return	True if mPeekedTokens was successfully grown, false otherwise.
		b32 fTryPushMorePeek( );

		/// \brief	Pop tokens off the front of mPeekedTokens by seeing what tokens are still required by all
		///			the subscribed mPeekingTokenizers.
		void fPopAnyUnnecessaryHead( );

		/// \brief	A stored token read from mTokenizer.
		struct tStoredToken
		{
			Json::tTokenType	mTokenType;		///< The type of the token.
			u32					mTokenLength;	///< The fReadToken reported token length (e.g. INCLUDING quotes)
			u32					mNewBufferPos;	///< The fReadToken reported new buffer position
			tDynamicBuffer		mTokenData;		///< The fGetTokenValue buffer (e.g. NOT including quotes)
		};

		tJsonTokenizer								mTokenizer;			///< The underlying tokenizer.
		tGrowableArray< tPeekingJsonTokenizer* >	mPeekingTokenizers;	///< Subscribed tokenizers.
		u32											mHeadToken;			///< The effective index of mPeekedTokens.fFront( ) if we never popped from it.
		tGrowableArray< tStoredToken >				mPeekedTokens;		///< Tokens read from mTokenizer but still desired by one or more tPeekingJsonTokenizer s
	};
}

#endif //ndef __tPeekingJsonTokenizerSharedState__
