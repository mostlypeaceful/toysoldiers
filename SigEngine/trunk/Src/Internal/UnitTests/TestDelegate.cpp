#include "UnitTestsPch.hpp"
#include "tDelegate.hpp"

using namespace Sig;

struct tComplex
{
	tComplex( int a, int b ) : mA(a), mB(b) { }
	int mA, mB;
};

class tSomeObject0
{
public:
	void foo0( int a, const tComplex& b ) { log_line( 0, "tSomeObject0::foo0 !!! " << a << " " << b.mA << " " << b.mB ); }
	void foo1( int a, const tComplex& b ) { log_line( 0, "tSomeObject0::foo1 !!! " << a << " " << b.mA << " " << b.mB ); }
};

class tSomeObject1
{
public:
	void foo0( int a, const tComplex& b ) { log_line( 0, "tSomeObject1::foo0 !!! " << a << " " << b.mA << " " << b.mB ); }
	void foo1( int a, const tComplex& b ) { log_line( 0, "tSomeObject1::foo1 !!! " << a << " " << b.mA << " " << b.mB ); }
};

void global_foo0( int a, const tComplex& b ) { log_line( 0, "::global_foo0 !!! " << a << " " << b.mA << " " << b.mB ); }
void global_foo1( int a, const tComplex& b ) { log_line( 0, "::global_foo1 !!! " << a << " " << b.mA << " " << b.mB ); }


void global_void_func( )
{
	log_line( 0, "global_void_func !!!" );
}

define_unittest(TestDelegate)
{
	tSomeObject0 so0;
	tSomeObject1 so1;

	typedef tDelegate<void (int, const tComplex& )> tMyDelegate;

	tMyDelegate d00 = tMyDelegate::fCreateFromMethod<tSomeObject0, &tSomeObject0::foo0>( &so0 );
	tMyDelegate d01 = tMyDelegate::fCreateFromMethod<tSomeObject0, &tSomeObject0::foo1>( &so0 );
	d00( 0, tComplex( 1, 2 ) );
	d01( 1, tComplex( 2, 3 ) );

	tMyDelegate d10 = tMyDelegate::fCreateFromMethod<tSomeObject1, &tSomeObject1::foo0>( &so1 );
	tMyDelegate d11 = tMyDelegate::fCreateFromMethod<tSomeObject1, &tSomeObject1::foo1>( &so1 );
	d10( 2, tComplex( 3, 4 ) );
	d11( 3, tComplex( 4, 5 ) );

	tMyDelegate d20 = tMyDelegate::fCreateFromFunction<&global_foo0>( );
	tMyDelegate d21 = tMyDelegate::fCreateFromFunction<&global_foo1>( );
	d20( 4, tComplex( 5, 6 ) );
	d21( 5, tComplex( 6, 7 ) );

	typedef tDelegate<void ( )> tVoidDelegate;

	tVoidDelegate dvoid = tVoidDelegate::fCreateFromFunction<&global_void_func>( );
	dvoid( );
}

