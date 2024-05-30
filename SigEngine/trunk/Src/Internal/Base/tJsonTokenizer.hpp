//------------------------------------------------------------------------------
// \file tJsonTokenizer.hpp - 19 Oct 2012
// \author mrickert
//
// Copyright Signal Studios 2012-2013, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __tJsonTokenizer__
#define __tJsonTokenizer__

#include "tJsonCommon.hpp"

#if defined( platform_xbox360 )
#include <XJSON.h>
#endif

namespace Sig
{
	/// \class tJsonTokenizer
	///
	/// \brief Implements an interface similar to and modeled after XJSON.  Unlike XJSON, it is platform
	/// independent.  It handles the low level validation of JSON and it's transformation into a series of
	/// logical tokens.
	///
	///		An example.  The following JSON:
	/// {"foo":[{"bar":1},{"baz":2},{"a":"b","c":"d"}]}
	///
	///		Results in the following tokens:
	/// tTokenType							Raw		Value
	/// ----------------------------------------------------
	/// [1]: cTokenBeginObject				{
	///   [2]: cTokenFieldName				"foo"	foo
	///   [3]: cTokenBeginArray				[
	///     [4]: cTokenBeginObject			{
	///       [5]: cTokenFieldName			"bar"	bar
	///       [6]: cTokenNumber				1		1
	///     [7]: cTokenEndObject			}
	///     [8]: cTokenValueSeparator		,
	///     [9]: cTokenBeginObject			{
	///       [10]: cTokenFieldName			"baz"	baz
	///       [11]: cTokenNumber			2		2
	///     [12]: cTokenEndObject			}
	///     [13]: cTokenValueSeparator		,
	///     [14]: cTokenBeginObject			{
	///       [15]: cTokenFieldName			"a"		a
	///       [16]: cTokenString			"b"		b
	///       [17]: cTokenObjectSeparator	,
	///       [18]: cTokenFieldName			"c"		c
	///       [19]: cTokenString			"d"		d
	///     [20]: cTokenEndObject			}
	///   [21]: cTokenEndArray				]
	/// [22]: cTokenEndObject				}
	class tJsonTokenizer
	{
		declare_uncopyable( tJsonTokenizer );
	public:
		/// \brief Initialize the tokenizer with some JSON.
		tJsonTokenizer( const char* buffer, u32 bufferSize );

		~tJsonTokenizer( );

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


#if defined( platform_xbox360 )
	public:
		/// \brief For potential future interop with e.g. native smartglass APIs.
		/// \param nativeHandle	The JSON reader to own.
		/// \param ownHandle	If true, this tJsonTokenizer will "own" the handle and close it once destroyed.
		tJsonTokenizer( HJSONREADER nativeHandle, b32 ownHandle );

	private:
		/// \brief Release all claim to mReader (double-release safe and respects mOwnsReader WRT ownership)
		void fReleaseReader( );

		HJSONREADER	mReader;
		b32			mOwnsReader;

#else
	private:
		typedef tArraySleeve<const char> tTokenSlice;			///< Generally reference a section of mBuffer.

		tDynamicArray<char>					mBuffer;			///< The original JSON passed in via constructor for tokenization.
		u32									mPos;				///< Index into mBuffer of the next read position.

		Json::tTokenType					mLastTokenType;		///< Set on fReadToken on successful read.  cTokenInvalid = no token set.
		tTokenSlice							mLastToken;			///< Set on fReadToken on successful read.  0 length = no token set.  Subset/slice of mBuffer.  Generally includes entire token e.g. for strings this includes surrounding quotes.

		tGrowableArray<Json::tTokenType>	mScopes;			///< list of TokenBeginArray, TokenBeginObject, and TokenNameSeperators indicating what kind of scope we're in.

	private:
		b32 fReachedEnd() const;								///< We ran out of buffer to read.
		void fEatWhitespace();									///< Advance the read position until it's no longer pointing at whitespace.
		char fCurrentChar() const;								///< The character under the read position, or NUL if we're out of buffer.

		const char* fBufferPos() const;							///< N.B. NOT NECESSAIRLY NUL TERMINATED!!!  Make sure you use fCharsRemaining in tandem with this function.
		u32 fCharsRemaining() const;							///< How many characters remain in the buffer after & including the current read position?

		/// \brief	Try to consume a specific known fixed length token such as true, false, or null.
		/// \return	True if we successfully
		/// \param	tokenId	The fixed length identifier (e.g. true, false, or null)
		/// \param	type	The type that a successful match of that token will be considered to be.
		template< size_t N >
		b32 fTryReadToken( const char (&tokenId)[N], Json::tTokenType type );

		void fAssertReadChar( char ch, Json::tTokenType type );	///< Demand that well formed JSON will contain this character representing this token next, and tokenize it as such.

		Json::tTokenType fCurrentScope() const;					///< The deepest mScopes we're in, or Json::cTokenInvalid if we're in no scopes.
		b32 fDoReadToken();										///< The implementation of fReadToken: Handle the bulk work of advancing mPos and appropriately initializing mLastToken & mLastTokenType.

#endif


	}; // tJsonTokenizer
} // namespace Sig

#endif //ndef __tJsonTokenizer__
