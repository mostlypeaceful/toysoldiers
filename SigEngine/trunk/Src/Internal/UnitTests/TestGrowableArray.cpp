#include "UnitTestsPch.hpp"

using namespace Sig;

namespace PushBackIntegrityTest
{
	//Integrity test added because of a naive implementation of fPushBack() discovered in tGrowableArray
	// The implementation was:
	//   fGrow( 1 )
	//   fBack() = object;
	// This can cause memory corruption if object happened to be an element contained within that array.
	//  A const& to that object is passed in, fGrow() copies and destroys the old array so now object
	//  is a dangling reference, then we set the last element of our new array to the value pointed
	//  to by the dangling reference.
	const u32 cOkayData = 555;
	struct test
	{
		u32 data;
		test()
			: data(cOkayData)
		{
		}
		~test()
		{
			data = ~0;
		}
		void check() const
		{
			if(data != cOkayData)
				throw tUnitTestException();
		}
	};
	void fTest( )
	{
		tGrowableArray<test> tests;

		tests.fPushBack();
		while(tests.fCount()!=tests.fCapacity())
			tests.fPushBack();

		tests.fPushBack(tests[0]);

		for(u32 i = 0; i < tests.fCount( ); ++i)
			tests[i].check();
	}
}

namespace PushEvensInsertOdds
{
	void fTestCase( b32	testNoResizeBranchOfInsert )
	{
		const u32 cSize = 1000;

		// Setup
		tGrowableArray< u32 > test;
		if( testNoResizeBranchOfInsert )
			test.fReserve( cSize ); // reserve so the bellow insertions never have to reallocate

		// Test pattern
		for( u32 i = 0; i < cSize; i += 2 )
			test.fPushBack( i ); // push back even numbers
		for( u32 i = 1; i < cSize; i += 2 )
			test.fInsert( i, i ); // insert odd numbers

		// Verify results
		sigassert( test.fCount( ) == cSize );
		sigassert( test.fCapacity( ) >= cSize );
		for( u32 i = 0; i < cSize; ++i )
			sigassert( test[ i ] == i );
	}

	void fTest( )
	{
		// Copy and paste courtesy of cbramwell
		fTestCase( false );
		fTestCase( true );
	}
}

namespace VerifyNoPointlessReallocs
{
	void fTestInsert( )
	{
		const u32 cSize = 1000;
		tGrowableArray< u32 > test;
		test.fReserve( cSize );

		const u32* originalBuffer = test.fBegin( );
		const u32 originalBufferSize = test.fCapacity( );

		for( u32 i = 0; i < cSize; ++i )
		{
			sigassert( test.fCount( ) < test.fCapacity( ) ); // test precondition, not what we're actually testing
			test.fInsert( i/2, i );
			sigassert( test.fBegin( ) == originalBuffer ); // memory buffer shouldn't have been reallocated, we had capacity!
			sigassert( test.fCapacity( ) == originalBufferSize ); // capacity shouldn't have changed either, we had no reason to reallocate yet!
		}
	}

	void fTest( )
	{
		fTestInsert( );
	}
}

define_unittest(TestGrowableArray)
{
	PushBackIntegrityTest::fTest();
	PushEvensInsertOdds::fTest( );
	VerifyNoPointlessReallocs::fTest( );
}
