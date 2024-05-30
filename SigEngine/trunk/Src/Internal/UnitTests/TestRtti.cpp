#include "UnitTestsPch.hpp"

using namespace Sig;

class a
{
public:
	a( ) { }
	a( tNoOpTag ) { }
};

class b : public a
{
public:
	b( ) { }
	b( tNoOpTag ) { }
};

class c : public Rtti::tSerializableBaseClass
{
	declare_null_reflector( );
	implement_rtti_serializable_base_class(c,0x41CE46F8);
public:
	c( ) : blah(0xbeef) { }
	c( tNoOpTag ) { }
	int blah;
};
register_rtti_factory( c, true );

class d
{
public:
	d( ) : blah(0xbeef) { }
	d( tNoOpTag ) { }
	int blah;
};


define_unittest(TestRtti)
{
	const b32 a_is_a = can_convert( a, a );
	const b32 b_is_a = can_convert( b, a );
	const b32 c_is_a = can_convert( c, a );

	fAssertEqual( a_is_a, true );
	fAssertEqual( b_is_a, true );
	fAssertEqual( c_is_a, false );

	c* newc = ( c* )Rtti::fNewClass( Rtti::fGetClassId<c>( ) );
	fAssertNotNull( newc );
	fAssertEqual( newc->blah, 0xbeef );

	const b32 dontFindD = Rtti::fIsFactoryRegistered( Rtti::fGetClassId<d>( ) );
	fAssertEqual( dontFindD, false );

	// now register class d
	Rtti::tFactoryRegistrar<d,true>( );

	d* newd = ( d* )Rtti::fNewClass( Rtti::fGetClassId<d>( ) );
	fAssertNotNull( newd );
	fAssertEqual( newd->blah, 0xbeef );

	fAssertNotEqual( ( size_t )newc, ( size_t )newd );

	delete newc;
	delete newd;
}
