#include "BasePch.hpp"
#include "tEncryption.hpp"

namespace Sig
{


	// Data protection Scheme
	/*
		Raw File
		  * Encryption Algo
			* Checksum
			  * True Data
	*/

	namespace
	{
		// was working on CRC here
		//// picking random bits
		//const u32 cDivisorSize = 8;
		//const u32 cDivisor = (1<<7) + (1<<4) + (1<<2) + 1;

		//u8 fExtractInput( u32 shift, const byte* data )
		//{

		//	u32 firstByte = shift / cDivisorSize;
		//	u32 remainShift = shift % cDivisorSize;

		//	u8 value = 0;

		//	// the shift right will truncate data off the front, then shift it into place
		//	value += u8(data[firstByte + 0]) >> remainShift;

		//	if( remainShift != 0 )
		//		value += u8(data[firstByte + 4]) << (cDivisorSize - remainShift);

		//	return value;
		//}

		//// Pass zero to compute sum, pass sum to compute validity (0)
		//u32 fCheckSum( const byte* src, u32 srcBytes, u32 sum )
		//{
		//	u32 dataShift = 0; //in bits;
		//	u32 shifts = srcBytes * 8;

		//	for( u32 i = 0; i < shift; ++i )
		//	{
		//		u8 input = fExtractInput( src, i );



		//	

		//}



		// Basic xor checksum.
		u8 fCheckSum( const byte* src, u32 srcBytes, u8 sum )
		{
			for( u32 i = 0; i < srcBytes; ++i )
				sum = sum ^ src[ i ];			
			return sum;
		}

		u32 fCheckSumLocation( u32 srcBytes )
		{
			return srcBytes / 4;
		}


		const char* cMask[ ] =
		{
			"TestMask",
			"MStIKGnNiAdL",
		};
			
		const u32 cMaskLens[ ] = 
		{ 
			strlen( cMask[ 0 ] ), 
			strlen( cMask[ 1 ] ) 
		};

		void fStoreMask( const tGrowableArray<byte>& src, tGrowableArray<byte>& dst, u32 mask )
		{
			u32 maskOffset = u32(src.fCount( ) * 0.75f) % std::numeric_limits<byte>::max( );

			dst.fSetCount( src.fCount( ) + 2 ); //add in the mask offset and mask index
			dst[ 0 ] = maskOffset;
			dst[ maskOffset ] = mask;

			memcpy( dst.fBegin( ) + 1, src.fBegin( ), maskOffset - 1 );
			memcpy( dst.fBegin( ) + maskOffset + 1, src.fBegin( ) + (maskOffset - 1), src.fCount( ) - (maskOffset - 1) );
		}

		u32 fExtractMask( const tGrowableArray<byte>& src, tGrowableArray<byte>& dst )
		{
			u32 maskOffset = src[ 0 ];
			u32 maskIndex = src[ maskOffset ];
			dst.fSetCount( src.fCount( ) - 2 ); //pull out the mask offset and mask index

			memcpy( dst.fBegin( ), src.fBegin( ) + 1, maskOffset - 1 );
			memcpy( dst.fBegin( ) + maskOffset - 1, src.fBegin( ) + (maskOffset + 1), src.fCount( ) - (maskOffset + 1) );

			return maskIndex;
		}

		// This more complicated version of the maskign will store a variable mask index somwhere in the masked data.
		//
		//// works for masking and unmasking, but masked should always point to the masked data.
		//void fMask( const tGrowableArray<byte>& src, tGrowableArray<byte>& dst, const tGrowableArray<byte>& masked, u32& maskIndex )
		//{
		//	tGrowableArray<byte> op;
		//	tGrowableArray<byte> dstInternal;
		//	b32 storeMask = true;

		//	if( maskIndex == ~0 )
		//	{
		//		storeMask = false;
		//		maskIndex = fExtractMask( src, op );
		//	}
		//	else
		//	{
		//		op.fSetCount( src.fCount( ) );
		//		memcpy( op.fBegin( ), src.fBegin( ), src.fCount( ) );
		//	}

		//	dstInternal.fSetCount( op.fCount( ) );

		//	sigassert( maskIndex < array_length( cMask ) );

		//	for( u32 i = 0; i < op.fCount( ); )
		//	{
		//		u32 remaining = fMin( cMaskLens[ maskIndex ], op.fCount( ) - i );

		//		for( u32 r = 0; r < remaining; ++r )
		//		{ 
		//			byte newMask = cMask[ maskIndex ][ r ];

		//			//// incorporate previously encrypted data into mask
		//			//s32 forwardIncorp = i - s32(remaining);
		//			//if( forwardIncorp > 0 )
		//			//	newMask ^= masked[ forwardIncorp ];

		//			dstInternal[ i + r ] = op[ i + r ] ^ newMask;
		//		}

		//		i += remaining;
		//	}

		//	if( storeMask )
		//		fStoreMask( dstInternal, dst, maskIndex );
		//	else
		//	{
		//		dst.fSetCount( dstInternal.fCount( ) );
		//		memcpy( dst.fBegin( ), dstInternal.fBegin( ), dstInternal.fCount( ) );
		//	}
		//}


		// works for masking and unmasking, but masked should always point to the masked data.
		void fMask( const tGrowableArray<byte>& src, tGrowableArray<byte>& dst, const tGrowableArray<byte>& masked, u32& maskIndex )
		{
			maskIndex = 0; //hardcoded for now

			sigassert( maskIndex < array_length( cMask ) );

			for( u32 i = 0; i < src.fCount( ); )
			{
				u32 remaining = fMin( cMaskLens[ maskIndex ], src.fCount( ) - i );

				for( u32 r = 0; r < remaining; ++r )
				{ 
					byte newMask = cMask[ maskIndex ][ r ];

					// incorporate previously encrypted data into mask
					s32 forwardIncorp = i - s32(remaining);
					if( forwardIncorp > 0 )
						newMask ^= masked[ forwardIncorp ];

					dst[ i + r ] = src[ i + r ] ^ newMask;
				}

				i += remaining;
			}
		}
			
	}

	void tEncryption::fEncrypt( const tGrowableArray<byte>& src, tGrowableArray<byte>& output )
	{
		sigassert( src.fCount( ) );

		// compute check sum
		u8 checkSum = fCheckSum( src.fBegin( ), src.fCount( ), 0 );

		// store it somewhere in the data
		u32 checkSumLocation = fCheckSumLocation( src.fCount( ) );

		tGrowableArray<byte> data;
		data.fSetCount( src.fCount( ) + 1 );
		memcpy( data.fBegin( ), src.fBegin( ), checkSumLocation );
		data[ checkSumLocation ] = checkSum;
		memcpy( data.fBegin( ) + checkSumLocation + 1, &src[ checkSumLocation ], src.fCount( ) - checkSumLocation );

		// mask it
		output.fSetCount( data.fCount( ) );
		u32 maskKey = checkSum % array_length( cMask );
		fMask( data, output, output, maskKey );

		log_line( 0, "Encrypt: Sum: " << checkSum << " Loc: " << checkSumLocation << "MaskKey: " << maskKey );
	}

	void tEncryption::fDecrypt( const tGrowableArray<byte>& src, tGrowableArray<byte>& output )
	{
		sigassert( src.fCount( ) );

		tGrowableArray<byte> data;
		data.fSetCount( src.fCount( ) );

		// unmask it
		u32 maskKey = ~0;
		fMask( src, data, src, maskKey );

		// extract sum
		output.fSetCount( src.fCount( ) - 1 ); //one of the bytes is the checksum
		u32 checkSumLocation = fCheckSumLocation( output.fCount( ) );

		u8 checksum = data[ checkSumLocation ];
		memcpy( output.fBegin( ), data.fBegin( ), checkSumLocation );
		memcpy( output.fBegin( ) + checkSumLocation, data.fBegin( ) + checkSumLocation + 1, output.fCount( ) - checkSumLocation );

		// validate zero sum
		u32 validation = fCheckSum( output.fBegin( ), output.fCount( ), checksum );

		log_line( 0, "Decrypt: Sum: " << checksum << " Loc: " << checkSumLocation << " Valid: " << validation << "MaskKey: " << maskKey );

		if( validation != 0 )
			output.fSetCount( 0 );
	}

	b32 fCompare( tGrowableArray<byte>& left, tGrowableArray<byte>& right )
	{
		if( left.fCount( ) != right.fCount( ) )
			return false;

		for( u32 i = 0; i < left.fCount( ); ++i )
		{
			if( left[ i ] != right[ i ] )
				return false;
		}

		return true;
	}

	void tEncryption::fTest( )
	{
#ifdef target_release
		return;
#endif
		tGrowableArray<byte> data;
		tGrowableArray<byte> dataSlightChange;

		const u32 cTestSize = 100;
		data.fSetCount( cTestSize );
		dataSlightChange.fSetCount( cTestSize );

		//fill data
		for( u32 i = 0; i < cTestSize; ++i )
		{
			data[ i ] = tRandom::fSubjectiveRand( ).fIntInRange( 0, std::numeric_limits<byte>::max( ) );
			dataSlightChange[ i ] = ( i % 5 == 0 ) ? data[ i ] + 1 : data[ i ];

			//{
			//	u32 alphaRange = 'Z' - 'A';
			//	data[ i ] = 'A' + (i % alphaRange);

			//	// inject an abc over klm, to simulate changing a single score in the data.
			//	u32 scLetter = 'A' + (i % alphaRange);
			//	if( scLetter == 'K' )
			//		scLetter = 'A';
			//	else if( scLetter == 'L' )
			//		scLetter = 'B';
			//	else if( scLetter == 'M' )
			//		scLetter = 'C';

			//	dataSlightChange[ i ] = scLetter;
			//}
		}

		//key store test
		tGrowableArray<byte> stored;
		tGrowableArray<byte> storedTest;
		u32 mask = tRandom::fSubjectiveRand( ).fIntInRange( 0, std::numeric_limits<byte>::max( ) );
		fStoreMask( data, stored, mask );
		u32 retrieved = fExtractMask( stored, storedTest );
		sigassert( mask == retrieved );
		sigassert( fCompare( data, storedTest ) );
		log_line( 0, "key storage works" );

		tGrowableArray<byte> scrambled;
		tGrowableArray<byte> scrambledSC;
		tGrowableArray<byte> scrambledModified;

		fEncrypt( data, scrambled );
		fEncrypt( dataSlightChange, scrambledSC );

		// make a copy and furks with it
		scrambledModified.fInsert( 0, scrambled.fBegin( ), scrambled.fCount( ) );

		u32 cErrors = 2;
		for( u32 i = 0; i < cErrors; ++i )
			scrambledModified[ tRandom::fSubjectiveRand( ).fIntInRange( 0, scrambledModified.fCount( ) - 1 ) ] += tRandom::fSubjectiveRand( ).fIntInRange( 0, std::numeric_limits<byte>::max( ) );
		
		// uncommenting this should simulate a person seeing the "slight change" in data and trying to adjust it themselves.
		// consides with first KLM of alpha test data
		//scrambledSC[ 11 ] += 1;

		//for( u32 i = 0; i < data.fCount( ); ++i )
		//	log_line( 0, "D: " << data[ i ] << scrambled[ i ] << " " << dataSlightChange[ i ] << scrambledSC[ i ] );

		//decript data:
		tGrowableArray<byte> decrypted;
		tGrowableArray<byte> decryptedSC;
		tGrowableArray<byte> decryptedModified;

		fDecrypt( scrambled, decrypted );
		fDecrypt( scrambledSC, decryptedSC );
		fDecrypt( scrambledModified, decryptedModified );

		sigassert( !fCompare( data, scrambled ) );
		sigassert( !fCompare( data, scrambledModified ) );
		sigassert( !fCompare( scrambled, scrambledModified ) );
		log_line( 0, "Encryption succeeded" );

		sigassert( fCompare( data, decrypted ) );
		sigassert( !fCompare( data, decryptedModified ) );
		sigassert( fCompare( dataSlightChange, decryptedSC ) );
		log_line( 0, "Descryption succeeded" );
	}
}

