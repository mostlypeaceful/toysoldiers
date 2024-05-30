//------------------------------------------------------------------------------
// \file tUrlEncode.hpp - 16 Feb 2012
// \author jwittner
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tUrlEncode__
#define __tUrlEncode__

namespace Sig
{
	class tUrlEncode
	{
	public:

		static void fEncode( const std::string & in, std::string & out );
		static void fEncode( const char * text, std::string & out );
		static void fEncode( const char * text, u32 length, std::string & out );

		static void fDecode( const std::string & in, std::string & out );
		static void fDecode( const char * text, std::string & out );
		static void fDecode( const char * text, u32 length, std::string & out );
	};

} // ::Sig

#endif//__tUrlEncode__
