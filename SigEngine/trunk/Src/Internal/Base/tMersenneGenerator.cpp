#include "BasePch.hpp"
#include "tMersenneGenerator.hpp"
#include "tRandom.hpp"
#include "tSync.hpp"

namespace Sig
{

	namespace
	{
		static const u32 cNumMersenneBuffers	=  20;
		static const u32 cMersenneBufferLength	=  624;
		static const u32 cTotalIndices			= cNumMersenneBuffers * cMersenneBufferLength;
		static const u32 MT_IA					=  397;
		static const u32 MT_IB					=  ( cMersenneBufferLength - MT_IA );
		static const u32 UPPER_MASK				=  0x80000000;
		static const u32 LOWER_MASK				=  0x7FFFFFFF;
		static const u32 MATRIX_A				=  0x9908B0DF;

		static tFixedArray<u32,cTotalIndices> gMersenneBuffer;

#define TWIST(b,i,j)    ( ( b ) [ i ] & UPPER_MASK ) | ( ( b ) [ j ] & LOWER_MASK )
#define MAGIC(s)        ( ( ( s ) & 1 ) * MATRIX_A )

		void fGenerateMersenneBuffer( u32 ithBuffer )
		{
			tFixedArray<u32,cTotalIndices>& b = gMersenneBuffer;
			const u32 baseIndex = ithBuffer * cMersenneBufferLength;

			if( ithBuffer > 0 )
			{
				// copy previous mersenne buffer into current
				const u32 dstIndex = ( ithBuffer - 0 ) * cMersenneBufferLength;
				const u32 srcIndex = ( ithBuffer - 1 ) * cMersenneBufferLength;
				sigassert( dstIndex + cMersenneBufferLength <= cTotalIndices );
				fMemCpy( &b[ dstIndex ], &b[ srcIndex ], cMersenneBufferLength * sizeof( b[ 0 ] ) );
			}
			
			u32 s;
			u32 i = 0;

			for ( ; i < MT_IB; ++i )
			{
				s = TWIST( b, i, i+1 );
				b[ baseIndex + i ] = b[ baseIndex + i + MT_IA ] ^ ( s >> 1 ) ^ MAGIC( s );
			}
			for ( ; i < cMersenneBufferLength-1; ++i )
			{
				s = TWIST( b, i, i+1 );
				b[ baseIndex + i ] = b[ baseIndex + i - MT_IB ] ^ ( s >> 1 ) ^ MAGIC( s );
			}
	        
			s = TWIST(b, cMersenneBufferLength-1, 0);
			b[ cMersenneBufferLength-1 ] = b[ MT_IA-1 ] ^ ( s >> 1 ) ^ MAGIC( s );
		}

		define_static_function( fPopulateMersenneBuffer )
		{
			tRandom r;
			for( u32 i = 0; i < cMersenneBufferLength; ++i )
				gMersenneBuffer[ i ] = r.fUInt( );
			for( u32 i = 0; i < cNumMersenneBuffers; ++i )
				fGenerateMersenneBuffer( i );
		}
	}

	const f32 tMersenneGenerator::cLimit = ( f32 )std::numeric_limits<u32>::max( );


	tMersenneGenerator::tMersenneGenerator( )
#ifdef target_game
		: mIndex( 0 )
#else
		: mIndex( tRandom::fSubjectiveRand( ).fUInt( ) % cTotalIndices )
#endif

	{
	}

	tMersenneGenerator::tMersenneGenerator( u32 seed )
		: mIndex( seed % cTotalIndices )
	{
	}

	u32 tMersenneGenerator::fGenerateRandomMersenne( ) const
	{
		if( mIndex == cTotalIndices )
			mIndex = 0;
		return gMersenneBuffer[ mIndex++ ];
	}
}

