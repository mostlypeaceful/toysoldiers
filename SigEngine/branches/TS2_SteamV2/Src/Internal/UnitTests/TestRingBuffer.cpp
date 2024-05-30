#include "UnitTestsPch.hpp"
#include "tRingBuffer.hpp"
#include "Base.rtti.hpp"

using namespace Sig;

template tRingBuffer<u32>;

define_unittest(TestRingBuffer)
{
	// test generating putting and getting

	const u32 maxSize = 5;
	tRingBuffer<u32> rb( maxSize );
	fAssertEqual( rb.fCapacity( ), maxSize );

	rb.fPut( 13 );
	rb.fPut( 14 );
	rb.fPut( 15 );
	rb.fPut( 16 );
	rb.fPut( 17 );

	fAssertEqual( rb.fNumItems( ), 5u );

	fAssertEqual( rb[0], 13u );
	fAssertEqual( rb[1], 14u );
	fAssertEqual( rb[2], 15u );
	fAssertEqual( rb[3], 16u );
	fAssertEqual( rb[4], 17u );

	u32 get;
	b32 success;

	success = rb.fGet( get );
	fAssert( success );
	fAssertEqual( get, 13u );

	success = rb.fGet( get );
	fAssert( success );
	fAssertEqual( get, 14u );

	fAssertEqual( rb.fNumItems( ), 3u );
	fAssertEqual( rb[0], 15u );
	fAssertEqual( rb[1], 16u );
	fAssertEqual( rb[2], 17u );

	// test wrapping

	rb.fPut( 18 );
	rb.fPut( 19 );
	rb.fPut( 20 );

	fAssertEqual( rb.fNumItems( ), 5u );
	fAssertEqual( rb[0], 16u );
	fAssertEqual( rb[1], 17u );
	fAssertEqual( rb[2], 18u );
	fAssertEqual( rb[3], 19u );
	fAssertEqual( rb[4], 20u );

	rb.fPut( 21 );

	fAssertEqual( rb.fNumItems( ), 5u );
	fAssertEqual( rb[0], 17u );
	fAssertEqual( rb[1], 18u );
	fAssertEqual( rb[2], 19u );
	fAssertEqual( rb[3], 20u );
	fAssertEqual( rb[4], 21u );

	success = rb.fGet( get );
	fAssert( success );
	fAssertEqual( get, 17u );
	fAssertEqual( rb.fNumItems( ), 4u );

	success = rb.fGet( get );
	fAssert( success );
	fAssertEqual( get, 18u );
	fAssertEqual( rb.fNumItems( ), 3u );

	fAssertEqual( rb[0], 19u );
	fAssertEqual( rb[1], 20u );
	fAssertEqual( rb[2], 21u );

	// test upsizing

	rb.fResize( 6 );
	fAssertEqual( rb.fNumItems( ), 3u );
	fAssertEqual( rb[0], 19u );
	fAssertEqual( rb[1], 20u );
	fAssertEqual( rb[2], 21u );
	fAssertEqual( rb.fFront( ), 21u );
	fAssertEqual( rb.fBack( ), 19u );

	rb.fPut( 22u );
	rb.fPut( 23u );
	rb.fPut( 24u );
	fAssertEqual( rb.fNumItems( ), 6u );
	fAssertEqual( rb.fFront( ), 24u );
	fAssertEqual( rb.fBack( ), 19u );

	// test downsizing (should retain latest puts)

	rb.fResize( 3 );
	fAssertEqual( rb.fNumItems( ), 3u );
	fAssertEqual( rb.fFront( ), 24u );
	fAssertEqual( rb.fBack( ), 22u );
}
