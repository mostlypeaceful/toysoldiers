//------------------------------------------------------------------------------
// \file tJsonReader.cpp - 19 July 2012
// \author colins, mrickert
//
// Copyright Signal Studios 2012-2013, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tJsonReader.hpp"
#include "tPeekingJsonTokenizer.hpp"

using namespace Sig::Json; // cToken*

namespace Sig
{
	namespace
	{
		static const u32 cMaxTokenLength = 3000;

		/// \brief	If t's next token is a Json::cToken*Seperator, advance it.
		/// \param	t	The tPeekingJsonTokenizer to advance (if currently referencing a seperator)
		void fSkipSeperators( tPeekingJsonTokenizer& t )
		{
			tPeekingJsonTokenizer c = t; // Work with a temporary copy so we don't modify t except when we really want to.
			Json::tTokenType tt = Json::cTokenInvalid;
			if( c.fReadToken( &tt, NULL, NULL ) )
			{
				switch( tt )
				{
				case Json::cTokenNameSeparator:
				case Json::cTokenObjectSeparator:
				case Json::cTokenValueSeparator:
					t = c; // OK, we read a seperator, actually skip it.
					break;
				default:
					// Not a seperator, discard read token results.
					break;
				}
			}
		}
	}

	//--------------------------------------------------------------------------------------
	// tJsonReader
	//--------------------------------------------------------------------------------------
	tJsonReader::tJsonReader( const char* inBuffer, u32 inBufferSize )
		: mScopeDepth( 0 )
	{
		mReader = NEW tPeekingJsonTokenizer( inBuffer, inBufferSize );
	}

	tJsonReader::~tJsonReader( )
	{
		delete mReader;
		mReader = NULL;
	}

	b32 tJsonReader::fBeginObject( )
	{
		return fParseToToken( cTokenBeginObject, mScopeDepth + 1 );
	}

	b32 tJsonReader::fEndObject( )
	{
		sigassert( mScopeDepth > 0 );
		return fParseToToken( cTokenEndObject, mScopeDepth - 1 );
	}

	b32 tJsonReader::fBeginArray( )
	{
		return fParseToToken( cTokenBeginArray, mScopeDepth + 1 );
	}

	b32 tJsonReader::fEndArray( )
	{
		sigassert( mScopeDepth > 0 );
		return fParseToToken( cTokenEndArray, mScopeDepth - 1 );
	}
	b32 tJsonReader::fGetField( const char* name )
	{
		const u32 nameLen = (u32)strlen(name);

		const u32 initialScopeDepth = mScopeDepth;
		char outbuf[ cMaxTokenLength ];


		while( fParseToToken( Json::cTokenFieldName, initialScopeDepth ) )
		{
			if( !mReader->fGetTokenValue( outbuf, cMaxTokenLength-1 ) )
				return false;

			// If we found the field, we're done
			if( strcmp( name, outbuf ) == 0 )
				return true;
		}

		return false;
	}

	b32 tJsonReader::fGetField( const char* name, std::string& out )
	{
		const u32 initialScopeDepth = mScopeDepth;
		Json::tTokenType tokenType;
		u32 tokenLength;
		u32 parsed;
		char outbuf[ cMaxTokenLength ];

		while( fParseToToken( Json::cTokenFieldName, initialScopeDepth ) )
		{
			if( !mReader->fGetTokenValue( outbuf, cMaxTokenLength ) )
				return false;

			// If we found the field, try to read its value
			if( strcmp(name,outbuf) == 0 )
			{
				if( !mReader->fReadToken( &tokenType, &tokenLength, &parsed ) )
					return false;

				fUpdateParserDepth( tokenType );

				if( mScopeDepth == initialScopeDepth )
				{
					if( !mReader->fGetTokenValue( outbuf, cMaxTokenLength ) )
						return false;

					out = outbuf;
					return true;
				}
			}
		}

		return false; // not found
	}

	b32 tJsonReader::fGetField( const char* name, tStringPtr& out )
	{
		std::string value;
		if( fGetField( name, value ) )
		{
			out = tStringPtr( value );
			return true;
		}
		return false;
	}

	b32 tJsonReader::fGetField( const char* name, unsigned char& out )
	{
		unsigned int i = out;
		const b32 result = fGetField( name, i );
		sigcheckfail( i <= UCHAR_MAX, return false );
		out = (unsigned char)i;
		return result;
	}

	b32 tJsonReader::fGetField( const char* name, signed char& out )
	{
		int i = out;
		const b32 result = fGetField( name, i );
		sigcheckfail( i >= SCHAR_MIN, return false );
		sigcheckfail( i <= SCHAR_MAX, return false );
		out = (signed char)i;
		return result;
	}

	b32 tJsonReader::fGetObject( tVarPropertyBag& out )
	{
		return fBeginObject( ) && fGetFields( out );
	}

	void tJsonReader::fUpdateParserDepth( const u32 parsedTokenType )
	{
		// Update mScopeDepth if we parsed a "begin" or "end" token
		switch( parsedTokenType )
		{
		case cTokenBeginArray:
		case cTokenBeginObject:
			++mScopeDepth;
			break;
		case cTokenEndArray:
		case cTokenEndObject:
			--mScopeDepth;
			break;
		default:
			break;
		}
	}

	Json::tTokenType tJsonReader::fPeekNextValueTokenType( ) const
	{
		tPeekingJsonTokenizer peek = *mReader;
		fSkipSeperators( peek );
		Json::tTokenType tt;
		return peek.fReadToken( &tt, NULL, NULL ) ? tt : Json::cTokenInvalid;
	}

	b32 tJsonReader::fReadValue( tStringPtr& value )
	{
		tPeekingJsonTokenizer peek = *mReader;
		fSkipSeperators( peek );
		Json::tTokenType tt;
		u32 len;
		if( !peek.fReadToken( &tt, &len, NULL ) || tt != Json::cTokenString )
			return false;
		malloca_array( char, buffer, len - 2 + 1 ); // minus quotes, plus '\0' terminator.
		sigcheckfail( peek.fGetTokenValue( buffer.fBegin( ), buffer.fCount( ) ), return false );

		// commit read
		*mReader = peek;
		value = tStringPtr( buffer.fBegin( ) );
		return true;
	}

	b32 tJsonReader::fReadValue( std::string& value )
	{
		tPeekingJsonTokenizer peek = *mReader;
		fSkipSeperators( peek );
		Json::tTokenType tt;
		u32 len;
		if( !peek.fReadToken( &tt, &len, NULL ) || tt != Json::cTokenString )
			return false;
		malloca_array( char, buffer, len - 2 + 1 ); // minus quotes, plus '\0' terminator.
		sigcheckfail( peek.fGetTokenValue( buffer.fBegin( ), buffer.fCount( ) ), return false );

		// commit read
		*mReader = peek;
		value = std::string( buffer.fBegin() );
		return true;
	}

	b32 tJsonReader::fReadValue( f64& value )
	{
		tPeekingJsonTokenizer peek = *mReader;
		fSkipSeperators( peek );
		Json::tTokenType tt;
		u32 len;
		if( !peek.fReadToken( &tt, &len, NULL ) || tt != Json::cTokenNumber )
			return false;
		tFixedArray< char, 1001 > buffer; ///< Should be large enough for any number... right?  RIGHT?!?
		sigcheckfail( peek.fGetTokenValue( buffer.fBegin( ), buffer.fCount( ) ), return false );

		// Parse buffer
		std::stringstream ss;
		ss.str( buffer.fBegin( ) );
		f64 tempValue;
		sigcheckfail( (ss >> tempValue), return false );
		char ch;
		sigcheckfail( !(ss >> ch), return false );

		// commit read
		*mReader = peek;
		value = tempValue;
		return true;
	}

	b32 tJsonReader::fReadBooleanValue( b32& value )
	{
		tPeekingJsonTokenizer peek = *mReader;
		fSkipSeperators( peek );
		Json::tTokenType tt;
		if( !peek.fReadToken( &tt, NULL, NULL ) )
			return false;

		switch( tt )
		{
		case Json::cTokenTrue:
			// commit read
			*mReader = peek;
			value = true;
			return true;

		case Json::cTokenFalse:
			// commit read
			*mReader = peek;
			value = false;
			return true;

		default:
			// discard read
			return false;

		}
	}

	b32 tJsonReader::fReadNullValue( )
	{
		tPeekingJsonTokenizer peek = *mReader;
		fSkipSeperators( peek );
		Json::tTokenType tt;
		if( !peek.fReadToken( &tt, NULL, NULL ) || tt != Json::cTokenNull )
			return false;

		// commit read
		*mReader = peek;
		return true;
	}

	b32 tJsonReader::fGetFields( tVarPropertyBag& out )
	{
		const u32 initialScopeDepth = mScopeDepth;
		Json::tTokenType tokenType;
		u32 tokenLength;
		u32 parsed;
		char outbuf[ cMaxTokenLength ];

		while( fParseToToken( Json::cTokenFieldName, initialScopeDepth ) )
		{
			if( !mReader->fGetTokenValue( outbuf, cMaxTokenLength ) )
				return false;

			tStringPtr fieldName( outbuf );

			if( !mReader->fReadToken( &tokenType, &tokenLength, &parsed ) )
				return false;

			fUpdateParserDepth( tokenType );

			if( mScopeDepth == initialScopeDepth )
			{
				if( !mReader->fGetTokenValue( outbuf, cMaxTokenLength ) )
					return false;

				std::stringstream ss;
				ss.str( outbuf );

				// Create the appropriate tVarProperty type
				tVarProperty* newProperty = NULL;
				switch( tokenType )
				{
				case Json::cTokenString:
						newProperty = NEW_TYPED( tVarPropertyTemplate< std::string >( fieldName, ss.str( ) ) );
					break;
				case Json::cTokenNumber:
					{
						s32 value = 0;
						ss >> value;
						newProperty = NEW_TYPED( tVarPropertyTemplate< s32 >( fieldName, value ) );
					}
					break;
				case Json::cTokenBeginObject:
					{
						newProperty = NEW_TYPED( tVarPropertyTemplate< tVarPropertyBag >( fieldName, tVarPropertyBag( ) ) );
						fGetFields( *newProperty->fValueAs< tVarPropertyBag >( ) );
					}
					break;
				default:
					log_warning( "Skipping unhandled Json type: " << tokenType );
					continue;
				}

				out.fAdd( tVarPropertyPtr( newProperty ) );
			}
		}

		return true;
	}

	b32 tJsonReader::fParseToToken( const u32 tokenType, const u32 tokenScopeDepth )
	{
		const u32 initialScopeDepth = mScopeDepth;
		Json::tTokenType parsedTokenType;
		u32 parsedTokenLength;
		u32 parsed;

		// Read until we find the token, or we can't parse anymore
		while( mReader->fReadToken( &parsedTokenType, &parsedTokenLength, &parsed ) )
		{
			fUpdateParserDepth( parsedTokenType );

			// Check if we found the token
			if( parsedTokenType == tokenType && mScopeDepth == tokenScopeDepth )
			{
				return true;
			}

			// If we parsed past the end of the current scope, we're not going to find the token
			if( mScopeDepth < initialScopeDepth )
				return false;
		}

		return false;
	}
}
