#include "UnitTestsPch.hpp"
#include "tBase64.hpp"
#include "tRandom.hpp"

using namespace Sig;

define_unittest(TestBase64)
{
	const u32 numTests = 11;

	tRandom rand;

	for( u32 itest = 0; itest < numTests; ++itest )
	{
		const u32 toEncodeLength = 297847 + rand.fUInt( ) % 297847;

		tGrowableArray<Sig::byte> toEncode( toEncodeLength );

		for( u32 i = 0; i < toEncodeLength; ++i )
			toEncode[ i ] = fRoundDown<Sig::byte>( rand.fFloatZeroToOne( ) * 255 );

		std::string encoded;
		tBase64::fEncode( toEncode.fBegin( ), toEncode.fCount( ), encoded );

		tGrowableArray<Sig::byte> decoded;
		tBase64::fDecode( &encoded[ 0 ], ( u32 )encoded.length( ), decoded );

		fAssertEqual( decoded.fCount( ), toEncode.fCount( ) );
		fAssertEqual( decoded.fCount( ), toEncodeLength );

		for( u32 i = 0; i < toEncodeLength; ++i )
		{
			fAssertEqual( decoded[ i ], toEncode[ i ] );
		}
	}

}
