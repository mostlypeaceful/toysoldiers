//------------------------------------------------------------------------------
// \file tJsonReader.hpp - 18 July 2012
// \author colins, mrickert
//
// Copyright Signal Studios 2012-2013, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tJsonReader__
#define __tJsonReader__

#include "tJsonCommon.hpp"
#include "tVarProperty.hpp"

namespace Sig
{
	class tPeekingJsonTokenizer;

	///
	/// \class tJsonReader
	/// \brief Used for parsing JSON objects from a memory buffer
	class tJsonReader : public tUncopyable, public tRefCounter
	{
	public:

		tJsonReader( const char* inBuffer, u32 inBufferSize );
		~tJsonReader( );

		b32 fBeginObject( );
		b32 fEndObject( );
		b32 fBeginArray( );
		b32 fEndArray( );
		b32 fGetField( const char* name ); // Parses a field name without parsing the value
		b32 fGetField( const char* name, std::string& out );
		b32 fGetField( const char* name, tStringPtr& out );

		b32 fGetField( const char* name, unsigned char& out );
		b32 fGetField( const char* name, signed char& out );
	private:
		b32 fGetField( const char* name, char& out ); /// Intentionally not implemented -- ambiguous, suppress usage of template version

	public:
		template<class t>
		b32 fGetField( const char* name, t& out )
		{
			std::string value;
			if( fGetField( name, value ) )
			{
				std::stringstream ss; ss.str( value );
				ss >> out;

				return !ss.fail( );
			}

			return false;
		}

		///
		/// \brief Reads an entire object into the specified tVarPropertyBag.
		///	\note  Supports reading child objects but not arrays.
		///TODO: Add support for arrays
		b32 fGetObject( tVarPropertyBag& out );

		/// \brief Ignoring separators, what is the next token type?
		Json::tTokenType fPeekNextValueTokenType( ) const;

		/// Read e.g. array values
		b32 fReadValue( tStringPtr& value );			///< Read a JSON String
		b32 fReadValue( std::string& value );			///< Read a JSON String
		b32 fReadValue( f64& value );					///< Read a JSON Number
		b32 fReadBooleanValue( b32& value );			///< Read a JSON Boolean
		b32 fReadNullValue( );							///< Read a JSON null

	private:
		b32 fGetFields( tVarPropertyBag& out );
  
		void fUpdateParserDepth( const u32 parsedTokenType );
		b32 fParseToToken( const u32 tokenType, const u32 tokenDepth );

	private:

		u32 mScopeDepth; // Increments on a begin* tag, and decrements on an end* tag
		tPeekingJsonTokenizer* mReader;

	private:
		typedef Json::tTokenType tTokenType; /// Moved to tJsonCommon.hpp
	};

	typedef tRefCounterPtr< tJsonReader > tJsonReaderPtr;
}
#endif//__tJsonReader__
