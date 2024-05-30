//------------------------------------------------------------------------------
// \file tUrlEncode.cpp - 16 Feb 2012
// \author jwittner
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tUrlEncode.hpp"

namespace Sig
{
	namespace
	{
		// NOTE: If every character in cEncodes is not switched on in fEncodeChar
		// then you'll hit the no default assert
		const char * cEncodes = " /+=:.-"; 
		static const char * fEncodeChar( char c )
		{
			switch( c )
			{
			case ' ': return "%20";
			case '/': return "%2F";
			case '+': return "%2B";
			case '=': return "%3D";
			case ':': return "%3A";
			case '.': return "%2E";
			case '-': return "%2D";
			default: sig_nodefault( );
			}

			return NULL;
		}
	}

	//------------------------------------------------------------------------------
	// tUrlEncode
	//------------------------------------------------------------------------------
	void tUrlEncode::fEncode( const std::string & in, std::string & out )
	{
		fEncode( in.c_str( ), in.length( ), out );
	}

	//------------------------------------------------------------------------------
	void tUrlEncode::fEncode( const char * text, std::string & out )
	{
		fEncode( text, ~0, out );
	}

	//------------------------------------------------------------------------------
	void tUrlEncode::fEncode( const char * text, u32 length, std::string & out )
	{
		// Allow for not knowing the length
		if( length == ~0 ) 
			length = strlen( text );

		// URL Encode
		std::stringstream ss;
		const char * writeStart = text;
		const char * writeEnd = strpbrk( writeStart, cEncodes );
		while( writeEnd != NULL )
		{
			ss.write( writeStart, writeEnd - writeStart );
			ss << fEncodeChar( *writeEnd );

			writeStart = writeEnd + 1;
			writeEnd = strpbrk( writeStart, cEncodes );
		}

		// Write what's left
		ss.write( writeStart, ( text + length ) - writeStart );
		out = ss.str( );
	}

	//------------------------------------------------------------------------------
	void tUrlEncode::fDecode( const std::string & in, std::string & out )
	{
		fDecode( in.c_str( ), in.length( ), out );
	}

	//------------------------------------------------------------------------------
	void tUrlEncode::fDecode( const char * text, std::string & out )
	{
		fDecode( text, strlen( text ), out );
	}

	//------------------------------------------------------------------------------
	void tUrlEncode::fDecode( const char * text, u32 length, std::string & out )
	{
		log_warning_unimplemented( );
	}

} // ::Sig
