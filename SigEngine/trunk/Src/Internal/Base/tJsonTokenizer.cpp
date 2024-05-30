//------------------------------------------------------------------------------
// \file tJsonTokenizer.hpp - 19 Oct 2012
// \author mrickert
//
// Copyright Signal Studios 2012-2013, All Rights Reserved
//------------------------------------------------------------------------------

#include "BasePch.hpp"
#if !defined( platform_xbox360 )
#include "tJsonTokenizer.hpp"

namespace Sig
{
	namespace
	{
		b32 fIsWhitespace( char ch ) { return StringUtil::fIsAnyOf(" \t\n\r",ch); }
		b32 fIsDigit( char ch ) { return ('0' <= ch && ch <= '9'); }
		b32 fIsAlpha( char ch ) { return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z'); }
		b32 fIsAlphaNumeric( char ch ) { return fIsDigit(ch) || fIsAlpha(ch); }
		b32 fIsHexdigit( char ch ) { return fIsDigit(ch) || ('a' <= ch && ch <= 'f') || ('A' <= ch && ch <= 'F'); }
	}

	tJsonTokenizer::tJsonTokenizer( const char* buffer, u32 bufferSize )
		: mPos(0)
		, mLastTokenType(Json::cTokenInvalid)
	{
		mBuffer.fInitialize( buffer, bufferSize );
	}

	tJsonTokenizer::~tJsonTokenizer( )
	{
	}

	b32 tJsonTokenizer::fReadToken( Json::tTokenType* tokenType, u32* tokenLength, u32* newBufferPos )
	{
		b32 result = fDoReadToken( );
		if( result && mLastTokenType == Json::cTokenNameSeparator )
			result = fDoReadToken( ); // skip name separator to match XJSON behavior


		if( tokenType )
			*tokenType = mLastTokenType;
		if( tokenLength )
			*tokenLength = mLastToken.fCount( );
		if( newBufferPos )
			*newBufferPos = mPos;

		return result;
	}

	b32 tJsonTokenizer::fGetTokenValue( char* buffer, u32 maxBufferChars ) const
	{
		switch( mLastTokenType )
		{
		case Json::cTokenString:
		case Json::cTokenFieldName:
			{
				// Strip quotes, unescape.
				sigcheckfail( mLastToken.fCount( ) >= 2, return false );
				sigcheckfail( mLastToken.fFront( ) == '\"', return false );
				sigcheckfail( mLastToken.fBack( ) == '\"', return false );

				u32 srcIndex = 1; // skip initial quote mark
				u32 dstIndex = 0;

				const u32 srcLen = mLastToken.fCount( ) - 1; // don't include trailing quote mark
				const u32 dstLen = maxBufferChars - 1; // leave space for trailing NUL

				while( srcIndex < srcLen && dstIndex < dstLen )
				{
					if( mLastToken[ srcIndex ] != '\\' )
					{
						// not an escape
						buffer[ dstIndex++ ] = mLastToken[ srcIndex++ ];
					}
					else
					{
						// an escape
						++srcIndex; // skip backslash
						sigcheckfail( srcIndex < srcLen, return false );
						const char escapeStart = mLastToken[ srcIndex++ ]; // escape character

						switch( escapeStart )
						{
						case 'b':	buffer[ dstIndex++ ] = '\b'; break;
						case 'f':	buffer[ dstIndex++ ] = '\f'; break;
						case 'n':	buffer[ dstIndex++ ] = '\n'; break;
						case 'r':	buffer[ dstIndex++ ] = '\r'; break;
						case 't':	buffer[ dstIndex++ ] = '\t'; break;

						case '\\': case '/': case '"': // character is it's own escape
							buffer[ dstIndex++ ] = escapeStart;
							break;

						case 'u':	// unicode
							{
								u32 chValue = 0;

								for( int i = 0; i < 4; ++i )
								{
									sigcheckfail( srcIndex < srcLen, return false );
									const char srcCh = mLastToken[ srcIndex++ ]; // 0-9 a-f

									chValue *= 0x10;
									if( '0' <= srcCh && srcCh <= '9' )		chValue += srcCh - '0';
									else if( 'a' <= srcCh && srcCh <= 'f' )	chValue += srcCh - 'a' + 10;
									else if( 'A' <= srcCh && srcCh <= 'F' )	chValue += srcCh - 'A' + 10;
									else
									{
										log_warning( "tJsonReader: Invalid JSON string unicode escape" );
										return false;
									}
								}

								if( chValue >= 0xFF )
								{
									log_warning( "tJsonReader: Cannot read UNICODE escape to narrow buffer, substituting with ?" );
									chValue = '?';
								}

								buffer[ dstIndex++ ] = chValue;
							}
							break;

						default:
							log_warning( "tJsonReader: Invalid JSON string escape sequence" );
							return false;
						}
					}
				}

				sigcheckfail( dstIndex < maxBufferChars, return false );
				buffer[ dstIndex++ ] = '\0';
				return true;
			}
		case Json::cTokenFalse:
		case Json::cTokenTrue:
		case Json::cTokenNull:
		case Json::cTokenNumber:
			{
				sigcheckfail( mLastToken.fCount( ) >= 1, return false );
				sigcheckfail( mLastToken.fFront( ) != '\"', return false );
				sigcheckfail( mLastToken.fBack( ) != '\"', return false );

				const u32 len = fMin( maxBufferChars-1, mLastToken.fCount( ) ); // leave space for NUL, no trailing quotes to strip
				fMemCpy( buffer, mLastToken.fBegin(), len ); // no initial quotes to skip
				buffer[len] = '\0';
				return true;
			}
		case Json::cTokenBeginArray:
		case Json::cTokenBeginObject:
		case Json::cTokenEndArray:
		case Json::cTokenEndObject:
		case Json::cTokenNameSeparator:
		case Json::cTokenObjectSeparator:
		case Json::cTokenValueSeparator:
			return false;
		case Json::cTokenInvalid:
			return false;
		}

		sigassert(!"Invalid mLastTokenType" );
		return false;
	}

	b32 tJsonTokenizer::fReachedEnd() const
	{
		return mPos >= mBuffer.fCount( );
	}

	void tJsonTokenizer::fEatWhitespace()
	{
		while( !fReachedEnd() && fIsWhitespace( mBuffer[mPos] ) )
			++mPos;
	}

	char tJsonTokenizer::fCurrentChar() const
	{
		return fReachedEnd() ? '\0' : mBuffer[mPos];
	}

	/// \brief NOT NECESSAIRLY NUL TERMINATED, MAKE SURE YOU USE IN TANDEM WITH fCurrentBufferRemaining
	const char* tJsonTokenizer::fBufferPos() const
	{
		return mBuffer.fBegin() + mPos;
	}

	u32 tJsonTokenizer::fCharsRemaining() const
	{
		return mPos - mBuffer.fCount( );
	}

	template< size_t N >
	b32 tJsonTokenizer::fTryReadToken( const char (&tokenId)[N], Json::tTokenType type )
	{
		if( fCharsRemaining() >= N && strncmp( fBufferPos(), tokenId, N-1 ) == 0 && !fIsAlphaNumeric(mBuffer[mPos+N-1]) )
		{
			// matched tokenId and didn't continue on with more alphanumeric text (e.g. prevent 'null42' or 'nullish' from matching for the token 'null')

			mLastTokenType = type;
			mLastToken = tTokenSlice( fBufferPos(), N-1 );
			mPos += mLastToken.fCount( );
			return true;
		}
		return false;
	}

	void tJsonTokenizer::fAssertReadChar( char ch, Json::tTokenType type )
	{
		sigassert( fCurrentChar( ) == ch );
		mLastTokenType = type;
		mLastToken = tTokenSlice( fBufferPos(), 1 );
		mPos += 1;
	}

	Json::tTokenType tJsonTokenizer::fCurrentScope() const
	{
		if( mScopes.fCount() )
			return mScopes.fBack();
		else
			return Json::cTokenInvalid;
	}

	b32 tJsonTokenizer::fDoReadToken( )
	{
		// See http://www.json.org/ for grammar

		// Invalidate mLastToken{,Type}
		mLastTokenType = Json::cTokenInvalid;
		mLastToken = tTokenSlice();

		// Eat leading whitespace
		fEatWhitespace( );
		if( fReachedEnd() )
			return false;

		const char* tokenStart = fBufferPos( );

		switch( fCurrentChar() )
		{
		case '\"': // must be a string (either a string string or a field name string)
			if( fCurrentScope() == Json::cTokenInvalid )
			{
				log_warning( "tJsonReader: Encountered a string with no scope" );
				return false;
			}
			++mPos; // "

			while( !fReachedEnd() && fCurrentChar() != '\"' )
			{
				switch( fCurrentChar() )
				{
				case '\\': // escape sequence
					++mPos;

					if( StringUtil::fIsAnyOf( "\"\\/bfnrt", fCurrentChar() ) )
					{
						++mPos; // simple escape character
					}
					else if( fCurrentChar() == 'u' )
					{
						// unicode escape
						++mPos; // u
						for( int i=0 ; i<4 ; ++i )
						{
							if( !fIsHexdigit(fCurrentChar()) )
							{
								log_warning( "tJsonReader: Invalid JSON string unicode escape" );
								return false;
							}
							++mPos; // 0-9 a-f
						}
					}
					else
					{
						log_warning( "tJsonReader: Invalid JSON string escape sequence" );
						return false;
					}
					break;
				default:
					++mPos;
					break;
				}
			}
			if( fReachedEnd() )
			{
				log_warning( "tJsonReader: End of buffer reached while reading string" );
				return false;
			}
			sigassert( fCurrentChar() == '\"' );
			++mPos; // "

			mLastTokenType = (fCurrentScope() == Json::cTokenBeginObject) ? Json::cTokenFieldName : Json::cTokenString;
			mLastToken = tTokenSlice( tokenStart, (u32)(fBufferPos()-tokenStart) );
			return true;
		case '{': // must be an object start
			if( fCurrentScope() == Json::cTokenBeginObject )
			{
				log_warning( "tJsonReader: Cannot use objects as object keys" );
				return false;
			}

			fAssertReadChar( '{', Json::cTokenBeginObject );
			mScopes.fPushBack( Json::cTokenBeginObject );
			return true;
		case '}': // must be an object end
			if( fCurrentScope() == Json::cTokenNameSeparator )
			{
				mScopes.fPopBack();
			}

			if( fCurrentScope() == Json::cTokenBeginObject )
			{
				fAssertReadChar( '}', Json::cTokenEndObject );
				mScopes.fPopBack( );
				return true;
			}
			else
			{
				log_warning( "tJsonReader: Closing an object, but not in object scope" );
				return false;
			}
			break;
		case '[': // must be an array start
			if( fCurrentScope() == Json::cTokenBeginObject )
			{
				log_warning( "tJsonReader: Cannot use arrays as object keys" );
				return false;
			}

			fAssertReadChar( '[', Json::cTokenBeginArray );
			mScopes.fPushBack( Json::cTokenBeginArray );
			return true;
		case ']': // must be an array end
			if( fCurrentScope() != Json::cTokenBeginArray )
			{
				log_warning( "tJsonReader: Closing an array, but not in array scope" );
				return false;
			}

			fAssertReadChar( ']', Json::cTokenEndArray );
			mScopes.fPopBack( );
			return true;
		case ':': // must be a name seperator
			if( fCurrentScope() != Json::cTokenBeginObject )
			{
				log_warning( "tJsonReader: Field seperator, but not in object scope" );
				return false;
			}

			fAssertReadChar( ':', Json::cTokenNameSeparator );
			mScopes.fPushBack(Json::cTokenNameSeparator);
			return true;
		case ',': // array or object seperator
			switch( fCurrentScope() )
			{
			case Json::cTokenBeginArray:
				fAssertReadChar( ',', Json::cTokenValueSeparator );
				return true;
			case Json::cTokenNameSeparator:
				mScopes.fPopBack( ); // pop field scope
				fAssertReadChar( ',', Json::cTokenObjectSeparator );
				return true;
			case Json::cTokenBeginObject:
			default:
				log_warning( "tJsonReader: Unexpected scope for seperator" );
				return false;
			}
		default:
			if( (fCurrentChar() == '-') || fIsDigit( fCurrentChar() ) )
			{
				// Must be a number.

				++mPos; // '-' or digit
				// strictly speaking JSON doesn't allow e.g. "00124" as a number but we're not bothering to special case leading zeros.

				while( fIsDigit(fCurrentChar()) )
					++mPos;

				if( fCurrentChar() == '.' )
				{
					// decimal notation
					++mPos; // '.'

					if( !fIsDigit(fCurrentChar()) )
					{
						log_warning( "tJsonReader: Expected digits after decimal indicator" );
						return false;
					}

					while( fIsDigit(fCurrentChar()) )
						++mPos;
				}

				if( fCurrentChar() == 'e' || fCurrentChar() == 'E' )
				{
					// exponent notation
					++mPos; // 'e' or 'E'
					if( fCurrentChar() == '+' || fCurrentChar() == '-' )
						++mPos;
					if( !fIsDigit(fCurrentChar()) )
					{
						log_warning( "tJsonReader: Expected digits after exponent indicator" );
						return false;
					}
					while( fIsDigit(fCurrentChar()) )
						++mPos;
				}

				if( fIsAlphaNumeric( fCurrentChar() ) ) // trailed
				{
					log_warning( "tJsonReader: Invalid number" );
					return false;
				}

				mLastTokenType = Json::cTokenNumber;
				mLastToken = tTokenSlice( tokenStart, (u32)(fBufferPos()-tokenStart) );
				return true;
			}
			else if( fTryReadToken( "true", Json::cTokenTrue ) || fTryReadToken( "false", Json::cTokenFalse ) || fTryReadToken( "null", Json::cTokenNull ) )
			{
				return true;
			}
			else
			{
				log_warning( "tJsonReader: Unrecognized token start" );
				return false;
			}
		}

		sigassert(!"shouldn't be reached");
		return false;
	}
} // namespace Sig

#else

void fIgnoreNoPublicSymbolsWarningtJsonTokenizer( ) { }

#endif // !platform_xbox360
