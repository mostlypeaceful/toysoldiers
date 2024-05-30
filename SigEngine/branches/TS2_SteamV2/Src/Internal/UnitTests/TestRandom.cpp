#include "UnitTestsPch.hpp"
#include "tRandom.hpp"

using namespace Sig;

define_unittest(TestRandom)
{
	const u32 bigNumber = 10000;

	tRandom rand;

	for( u32 i = 0; i < bigNumber; ++i )
	{
		const f32 f = rand.fFloatMinusOneToOne( );
		fAssert( f >= -1.f && f <= 1.f );
	}

	for( u32 i = 0; i < bigNumber; ++i )
	{
		const f32 f = rand.fFloatZeroToOne( );
		fAssert( f >= 0.f && f <= 1.f );
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
