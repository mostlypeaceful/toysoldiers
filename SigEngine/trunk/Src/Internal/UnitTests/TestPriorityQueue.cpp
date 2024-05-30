#include "UnitTestsPch.hpp"
#include "tPriorityQueue.hpp"

using namespace Sig;
template tPriorityQueue<u32>;

namespace
{
	struct tPQObject
	{
		u32 mCost;
		tPQObject( u32 c=0 ) : mCost( c ) { }
		inline b32 operator>=( const tPQObject& other ) const
		{
			return mCost >= other.mCost;
		}
	};
	struct tPQObjectCompare
	{
		inline b32 operator( )( const tPQObject* a, const tPQObject* b ) const
		{
			return a->mCost >= b->mCost;
		}
	};
}

define_unittest(TestPriorityQueueInt)
{
	tPriorityQueue<u32> pq;

	const u32 count = 100;
	for( u32 i = 0; i < count; ++i )
		pq.fPut( i );

	u32 last = count;
	while( pq.fCount( ) > 0 )
	{
		const u32 top = pq.fGet( );
		fAssert( top < last );
		last = top;
	}
	fAssert( last == 0 );

	for( u32 i = 0; i < count; ++i )
		pq.fPut( i );

	fAssert( pq.fErase( 25 ) );
	fAssert( pq.fErase( 10 ) );
	fAssert( pq.fErase( 77 ) );
	fAssert( pq.fErase( 13 ) );
	fAssert( pq.fErase( 99 ) );

	//pq.fPut( 25 );
	//pq.fPut( 10 );
	//pq.fPut( 77 );
	//pq.fPut( 13 );
	//pq.fPut( 99 );

	last = count;
	while( pq.fCount( ) > 0 )
	{
		const u32 top = pq.fGet( );
		fAssert( top < last );
		last = top;
	}
	fAssert( last == 0 );

	log_newline( );
}

define_unittest(TestPriorityQueue)
{
	const u32 count = 1000;
	tFixedArray<tPQObject,count> objs;
	for( u32 i = 0; i < objs.fCount( ); ++i )
		objs[ i ] = tPQObject( i );

	tPriorityQueue<tPQObject*, tPQObjectCompare> pq;

	for( u32 i = 0; i < count; ++i )
		pq.fPut( &objs[ i ] );

	u32 last = count;
	while( pq.fCount( ) > 0 )
	{
		const u32 top = pq.fGet( )->mCost;
		fAssert( top < last );
		last = top;
	}
	fAssert( last == 0 );

	for( u32 i = 0; i < count; ++i )
		pq.fPut( &objs[ i ] );

	fAssert( pq.fErase( &objs[ 25 ] ) );
	fAssert( pq.fErase( &objs[ 10 ] ) );
	fAssert( pq.fErase( &objs[ 77 ] ) );
	fAssert( pq.fErase( &objs[ 13 ] ) );
	fAssert( pq.fErase( &objs[ 99 ] ) );
	fAssert( pq.fErase( &objs[ 113 ] ) );
	fAssert( pq.fErase( &objs[ 377 ] ) );

	last = count;
	while( pq.fCount( ) > 0 )
	{
		const u32 top = pq.fGet( )->mCost;
		fAssert( top < last );
		last = top;
	}
	fAssert( last == 0 );

	tRandom rand;
	for( u32 itest = 0; itest < 1000; ++itest )
	{
		for( u32 i = 0; i < count; ++i )
			pq.fPut( &objs[ i ] );

		for( u32 i = 0; i < objs.fCount( ); i += rand.fIntInRange( 1, 10 ) )
		{
			objs[ i ].mCost += rand.fIntInRange( 0, 1000 );
			fAssert( pq.fUpdate( &objs[ i ] ) );
		}

		u32 min = ~0;
		for( u32 i = 0; i < objs.fCount( ); ++i )
			min = fMin( min, objs[ i ].mCost );

		last = ~0;
		while( pq.fCount( ) > 0 )
		{
			const u32 top = pq.fGet( )->mCost;
			fAssert( top <= last );
			last = top;
		}
		fAssert( last == min );
	}

	log_newline( );
}

