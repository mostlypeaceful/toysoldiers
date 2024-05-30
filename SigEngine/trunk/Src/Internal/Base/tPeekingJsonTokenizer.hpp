//------------------------------------------------------------------------------
// \file tPeekingJsonTokenizer.hpp - 4 Oct 2013
// \author mrickert
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __tPeekingJsonTokenizer__
#define __tPeekingJsonTokenizer__

#include "tJsonTokenizer.hpp"
#include "tPeekingJsonTokenizerSharedState.hpp"

namespace Sig
{
	/// \class	tPeekingJsonTokenizer
	///
	/// \brief	Basically a copyable version of tJsonTokenizer, allowing inspection of the token stream
	///			multiple times at the cost of increased memory storage (although this is shared between sibling
	///			tPeekingJsonTokenizers.)
	///
	///			NOTE WELL: it is NOT safe to use tPeekingJsonTokenizer s from separate threads if they share state!
	///
	class base_export tPeekingJsonTokenizer
	{
	public:
		tPeekingJsonTokenizer( );
		tPeekingJsonTokenizer( const char* buffer, u32 bufferSize );
		tPeekingJsonTokenizer( const tPeekingJsonTokenizer& other );
		~tPeekingJsonTokenizer( );

		tPeekingJsonTokenizer& operator=( const tPeekingJsonTokenizer& other );

		/// \brief	Advance the current position to read another token.
		/// \return	True if a token was successfully read, false otherwise (e.g. the buffer ran out of JSON)
		/// \param	tokenType		[out, optional]: The type of the token that was read.
		/// \param	tokenLength		[out, optional]: The length of the entire token, including e.g. quotation marks.
		/// \param	newBufferPos	[out, optional]: The index into the buffer after the token and whitespace has been consumed.
		b32 fReadToken( Json::tTokenType* tokenType, u32* tokenLength, u32* newBufferPos );

		/// \brief	Get the value of the token last read with fReadToken as a NUL terminated string.
		///		Strings and field names will have their quotation marks stripped.
		///		Numbers, booleans, and null will still be represented as text.
		///		All other tokens are considered value-less.
		/// \return	True if a token value was successfully retrieved, false otherwise.
		/// \param	buffer			[in, required]: The buffer to write to.  This must be at least maxBufferChars long.  This will be NUL terminated.
		/// \param	maxBufferChars	[in, required]: The length of buffer in bytes/chars.  This size will include the NUL terminator.
		b32 fGetTokenValue( char* buffer, u32 maxBufferChars ) const;

	private:
		friend class tPeekingJsonTokenizerSharedState;

		/// \brief	Get the current token index WRT the entire token stream.
		///			I.E. the index into mState->mTokenizer, if it stored read tokens.
		u32 fCurrentTokenIndex( ) const;

		/// \brief	Get the current token index relative to mState->mHeadToken.
		///			I.E. the index into mState->mPeekedTokens, if it's read far enough ahead.
		u32 fPeekedTokensIndex( ) const;

		/// \brief	Register with mState->mPeekingTokenizers
		void fRegister( );

		/// \brief	Unregister with mState->mPeekingTokenizers, do cleanup
		void fUnregister( );

	private:
		typedef tRefCounterPtr< tPeekingJsonTokenizerSharedState > tSharedStatePtr;
		tSharedStatePtr	mSharedState;
		u32				mCurrentToken;

	}; // tJsonTokenizer
} // namespace Sig

#endif //ndef __tPeekingJsonTokenizer__
