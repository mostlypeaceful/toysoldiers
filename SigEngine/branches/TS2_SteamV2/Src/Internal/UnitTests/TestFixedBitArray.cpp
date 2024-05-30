#include "UnitTestsPch.hpp"
#include "tFixedBitArray.hpp"

using namespace Sig;
using namespace Sig::Math;

template tFixedBitArray<32>;

define_unittest(TestFixedBitArray)
{
	tFixedBitArray<256> bits;

	bits.fSetAll( true );
	bits.fSetAll( false );

	tGrowableArray<u32> bitsOn;
	bitsOn.fPushBack( 2 );
	bitsOn.fPushBack( 31 );
	bitsOn.fPushBack( 32 );
	bitsOn.fPushBack( 67 );
	bitsOn.fPushBack( 121 );
	bitsOn.fPushBack( 128 );
	bitsOn.fPushBack( 129 );
	for( u32 i = 0; i < bitsOn.fCount( ); ++i )
		bits.fTurnBitOn( bitsOn[i] );

	for( u32 i = 0; i < bits.cNumBits; ++i )
	{
		b32 found = false;
		for( u32 j = 0; j < bitsOn.fCount( ); ++j )
		{
			if( bitsOn[j] == i )
			{
				found = true;
				break;
			}
		}

		fAssertEqual( bits.fGetBit( i ), found );
	}

	tFixedBitArray<256> copy = bits;

	bits ^= copy;
	fAssertNotEqual( bits, copy );
	bits |= copy;
	fAssertEqual( bits, copy );
	copy = ~bits;
	tFixedBitArray<256> allOff = (bits & copy);
	fAssertEqual( allOff, tFixedBitArray<256>() );

	bits = tFixedBitArray<256>( true );

	tGrowableArray<u32> bitsOff;
	bitsOff.fPushBack( 4 );
	bitsOff.fPushBack( 11 );
	bitsOff.fPushBack( 37 );
	bitsOff.fPushBack( 69 );
	bitsOff.fPushBack( 111 );
	bitsOff.fPushBack( 213 );
	bitsOff.fPushBack( 245 );
	for( u32 i = 0; i < bitsOff.fCount( ); ++i )
		bits.fTurnBitOff( bitsOff[i] );

	for( u32 i = 0; i < bits.cNumBits; ++i )
	{
		b32 on = true;
		for( u32 j = 0; j < bitsOff.fCount( ); ++j )
		{
			if( bitsOff[j] == i )
			{
				on = false;
				break;
			}
		}

		fAssertEqual( bits.fGetBit( i ), on );
	}
}
