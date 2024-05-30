#include "UnitTestsPch.hpp"
#include "tRandom.hpp"

using namespace	 Sig;

class TestHashTable : public ::Sig::tUnitTestDerived<TestHashTable>
{
public:

	template<class tValue, int bigNumber>
	void fTest( tFixedArray<tValue,bigNumber>& items );

	virtual void fExecute( );
	virtual const char* fGetName( ) const { return "TestHashTable"; }
} gTestHashTable;

template<class tValue, int bigNumber>
void TestHashTable::fTest( tFixedArray<tValue,bigNumber>& items )
{
	typedef tHashTable<u32,tValue,tHashTableExpandAndShrinkResizePolicy> tMyHashTable;

	tMyHashTable ht;

	// insert all the addresses into table

	for( u32 i = 0; i < items.fCount( ); ++i )
		ht.fInsert( i, items[ i ] );

	// verify all the addresses are in the table

	fAssertEqual( ht.fGetItemCount( ), items.fCount( ) );

	for( u32 i = 0; i < items.fCount( ); ++i )
	{
		tValue* find = ht.fFind( i );
		fAssertNotNull( find );
		fAssertEqual( *find, items[ i ] );
	}

	// now test removal

	for( u32 i = 0; i < items.fCount( ); i += 2 )
	{
		tValue* find = ht.fFind( i );
		fAssertNotNull( find );
		ht.fRemove( find );
	}

	// ensure the addresses we just removed are really gone

	for( u32 i = 0; i < items.fCount( ); i += 2 )
	{
		tValue* find = ht.fFind( i );
		fAssertNull( find );
	}

	// ensure the rest of the addresses are still there

	for( u32 i = 1; i < items.fCount( ); i += 2 )
	{
		tValue* find = ht.fFind( i );
		fAssertNotNull( find );
	}

	// now re-insert them

	for( u32 i = 0; i < items.fCount( ); i += 2 )
		ht.fInsert( i, items[ i ] );


	// re-verify all the addresses are in the table

	fAssertEqual( ht.fGetItemCount( ), items.fCount( ) );

	for( u32 i = 0; i < items.fCount( ); ++i )
	{
		tValue* find = ht.fFind( i );
		fAssertNotNull( find );
		fAssertEqual( *find, items[ i ] );
	}

	// now remove everything

	for( u32 i = 0; i < items.fCount( ); ++i )
	{
		tValue* find = ht.fFind( i );
		fAssertNotNull( find );
		ht.fRemove( find );
	}

	fAssertEqual( ht.fGetItemCount( ), 0 );

}

void TestHashTable::fExecute( )
{
	const u32 bigNumber = 999;


	{
		tRandom rand;

		// generate a bunch of addresses
		tFixedArray<void*,bigNumber> items;
		for( u32 i = 0; i < items.fCount( ); ++i )
		{
			u32 randInt = rand.fUInt( );
			if( randInt == 0 || randInt == 0xffffffff )
				randInt = 13;
			items[i] = ( void* )( size_t )randInt;
		}

		fTest( items );
	}

	{
		tRandom rand;

		// generate a bunch of numbers
		tFixedArray<u64,bigNumber> items;
		for( u32 i = 0; i < items.fCount( ); ++i )
		{
			u32 randInt = rand.fUInt( );
			items[i] = randInt;
		}

		fTest( items );
	}

}
