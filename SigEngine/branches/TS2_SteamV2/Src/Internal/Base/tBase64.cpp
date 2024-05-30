#include "BasePch.hpp"
#include "tBase64.hpp"

namespace Sig
{
	void tBase64::fEncode( const Sig::byte* toEncode, u32 toEncodeLength, std::string& encodedOutput )
	{
		std::stringstream ss;

		for( u32 i = 0; i < toEncodeLength; i += 3 )
		{
			Sig::byte by1 = 0, by2 = 0, by3 = 0;

			by1 = toEncode[ i ];
			if( i + 1 < toEncodeLength )
				by2 = toEncode[ i + 1 ];
			if( i + 2 < toEncodeLength )
				by3 = toEncode[i+2];

			Sig::byte by4 = 0, by5 = 0, by6 = 0, by7 = 0;
			by4 = by1>>2;
			by5 = ((by1&0x3)<<4)|(by2>>4);
			by6 = ((by2&0xf)<<2)|(by3>>6);
			by7 = by3&0x3f;
			ss << fEncode(by4);
			ss << fEncode(by5);

			if( i + 1 < toEncodeLength )
				ss << fEncode(by6);
			else
				ss << "=";

			if( i + 2 < toEncodeLength )
				ss << fEncode(by7);
			else
				ss << "=";

			if( i % ( 76 / 4 * 3 ) == 0 )
				ss << "\r\n";
		}

		encodedOutput = ss.str( );
	}

	void tBase64::fDecode( const char* toDecode, u32 toDecodeLength, tGrowableArray<Sig::byte>& decodedOutput )
	{
		decodedOutput.fSetCount( 0 );

		std::stringstream ss;
		for( u32 i = 0; i < toDecodeLength; ++i )
			if( fIsBase64( toDecode[ i ] ) )
				ss << toDecode[ i ];
		std::string toDecodeCopy = ss.str( );
		if( toDecodeCopy.length( ) == 0 )
			return;

		toDecode = &toDecodeCopy[ 0 ];
		toDecodeLength = ( u32 )toDecodeCopy.length( );

		for( u32 i = 0; i < toDecodeLength; i += 4 )
		{
			char c1 = 'A', c2 = 'A', c3 = 'A', c4 = 'A';

			c1 = toDecode[ i ];

			if( i+1<toDecodeLength )
				c2 = toDecode[ i + 1 ];
			if( i+2<toDecodeLength )
				c3 = toDecode[ i + 2 ];
			if( i+3<toDecodeLength )
				c4 = toDecode[ i + 3 ];

			Sig::byte by1 = 0, by2 = 0, by3 = 0, by4 = 0;
			by1 = fDecode( c1 );
			by2 = fDecode( c2 );
			by3 = fDecode( c3 );
			by4 = fDecode( c4 );
			decodedOutput.fPushBack( (by1<<2)|(by2>>4) );
			if( c3 != '=' )
				decodedOutput.fPushBack( ((by2&0xf)<<4)|(by3>>2) );
			if( c4 != '=' )
				decodedOutput.fPushBack( ((by3&0x3)<<6)|by4 );
		}
	}

	char tBase64::fEncode( Sig::byte uc )
	{
		if( uc <  26 ) return 'A' + uc;
		if( uc <  52 ) return 'a' + ( uc - 26 );
		if( uc <  62 ) return '0' + ( uc - 52 );
		if( uc == 62 ) return '+';

		return '/';
	}

	Sig::byte tBase64::fDecode( char c )
	{
		if( c >= 'A' && c <= 'Z' )	return c - 'A';
		if( c >= 'a' && c <= 'z' )	return c - 'a' + 26;
		if( c >= '0' && c <= '9' )	return c - '0' + 52;
		if( c == '+' )				return 62;

		return 63;
	}

	b32 tBase64::fIsBase64( char c )
	{
		if( c >= 'A' && c <= 'Z' )	return true;
		if( c >= 'a' && c <= 'z' )	return true;
		if( c >= '0' && c <= '9' )	return true;
		if( c == '+' )				return true;
		if( c == '/' )				return true;
		if( c == '=' )				return true;

		return false;
	}

}

