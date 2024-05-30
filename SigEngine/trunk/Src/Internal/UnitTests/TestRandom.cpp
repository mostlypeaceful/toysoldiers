#include "UnitTestsPch.hpp"
#include "tRandom.hpp"

using namespace Sig;

define_unittest(TestRandom)
{
	const u32 cBigNumber = 100000;

	tRandom rand;

	for( u32 i = 0; i < cBigNumber; ++i )
	{
		const f32 f = rand.fFloatMinusOneToOne( );
		fAssert( f >= -1.f && f <= 1.f );
	}

	for( u32 i = 0; i < cBigNumber; ++i )
	{
		const f32 f = rand.fFloatZeroToOne( );
		fAssert( f >= 0.f && f <= 1.f );
	}

	const s32 boundaries[] = { -9001, -100, -32, -5, -1, 0, +1, +5, +32, +100, +9001 }; // arbitrary constants

	for( u32 minI = 0; minI < array_length( boundaries ); ++minI )
	{
		for( u32 maxI = 0; maxI < array_length( boundaries ); ++maxI )
		{
			const s32 min = boundaries[ minI ];
			const s32 max = boundaries[ maxI ];
			if( min > max )
				continue;

			for( u32 i = 0; i < cBigNumber; ++i )
			{
				const s32 randValue = rand.fIntInRange( min, max );
				fAssert( min <= randValue );
				fAssert( max >= randValue );
			}
		}
	}
}

//{
//	tRandom gRand( 0 );
//	f32 min = 1.f;
//	f32 max = 0.f;
//	f32 avg = 0.f;
//	f32 dev = 0.f;
//	f32 prev = 0.f;
//	const u32 numIterations = 5000;
//	tDynamicArray<u32> coverage( numIterations );
//	coverage.fFill( 0 );
//	for( u32 i = 0; i < numIterations; ++i )
//	{
//		const f32 v = gRand.fFloatMinusOneToOne( );
//		coverage[ fRound<u32>( (v/2+0.5f) * ( numIterations - 1 ) ) ] += 1;
//		avg += v;
//		max = fMax( max, v );
//		min = fMin( min, v );
//		if( i > 0 )
//			dev += fAbs( v - prev );
//
//		//log_line( 0, v );
//		prev = v;
//	}
//	avg /= numIterations;
//	dev /= numIterations;
//	u32 numEmptyBuckets = 0;
//	u32 maxHits = 0;
//	for( u32 i = 0; i < coverage.fCount( ); ++i )
//	{
//		maxHits = fMax( maxHits, coverage[ i ] );
//		if( coverage[ i ] == 0 )
//			++numEmptyBuckets;
//	}
//	log_line( 0, "avg = " << avg << ", min = " << min << ", max = " << max << ", dev = " << dev << ", emptyBuckets = " << numEmptyBuckets << ", maxHits = " << maxHits << ", iterations = " << numIterations );
//}
