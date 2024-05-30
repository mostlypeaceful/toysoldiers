#include "UnitTestsPch.hpp"
using namespace Sig;

#include "Scripts/tScriptVm.hpp"


#pragma warning( disable : 4800 )
#pragma warning( disable : 4244 )
#include "sqbind/sqbind.h"

class sqbindVec3
{
public:
	f32 x, y, z;

	static const f32 cPi;

	sqbindVec3( )
		: x( 0.f ), y( 0.f ), z( 0.f )
	{
		log_line( 0, "default constructor" );
	}

	sqbindVec3( f32 _x, f32 _y, f32 _z )
		: x(_x), y(_y), z(_z)
	{
		log_line( 0, "special constructor" );
	}

	f32 fLength( ) const
	{
		return sqrt( x*x + y*y + z*z );
	}

	static sqbindVec3 fAdd( const sqbindVec3& a, const sqbindVec3& b )
	{
		return sqbindVec3( a.x + b.x, a.y + b.y, a.z + b.z );
	}

	static sqbindVec3* vector3_constructor(HSQUIRRELVM v) {

		// regular and copycon handled by sqbind
		
		int params=sq_gettop(v);
		
		if (params!=4)
			return NULL; // need 3 params
					
		SQFloat x;
		SQFloat y;
		SQFloat z;
		
		if (SQ_FAILED( sq_getfloat(v,2,&x) ) )
			return NULL;
		if (SQ_FAILED( sq_getfloat(v,3,&y) ) )
			return NULL;
		if (SQ_FAILED( sq_getfloat(v,4,&z) ) )
			return NULL;
		
		return new sqbindVec3(x,y,z);
	}

};

const f32 sqbindVec3::cPi = 3.14f;

define_unittest(TestSqBind)
{
	tScriptVm svm;
	HSQUIRRELVM vm = svm.fSq( );


	SqBind<sqbindVec3>::init(vm,_SC("Vec3"));
	SqBind<sqbindVec3>::set_custom_constructor( sqbindVec3::vector3_constructor );
	sqbind_method( vm, "Length", &sqbindVec3::fLength );
	sqbind_function( vm, "AddVectors", &sqbindVec3::fAdd, &SqBind<sqbindVec3>::get_id() );
	SqBind<sqbindVec3>::bind_member_variable<float>( vm, _SC("x"), offsetof( sqbindVec3, x ) );
	SqBind<sqbindVec3>::bind_member_variable<float>( vm, _SC("y"), offsetof( sqbindVec3, y ) );
	SqBind<sqbindVec3>::bind_member_variable<float>( vm, _SC("z"), offsetof( sqbindVec3, z ) );
	SqBind<sqbindVec3>::bind_constant( vm, "PI", sqbindVec3::cPi );


	const char* s0="\
				   gA <- Vec3( ) \n\
				   gA.x = 1 \n\
				   print( \"a length = \" + gA.Length( ) ) \n\
				   gB <- Vec3( 1, 0, 1 ) \n\
				   gB.y = 1 \n\
				   print( \"b length = \" + gB.Length( ) ) \n\
				   gC <- Vec3.AddVectors( gA, gB ) \n\
				   gC.z = Vec3.PI \n\
				   print( \"c length = \" + gC.Length( ) ) \n\
				   ";


	const b32 compileSuccess0 = svm.fCompileStringAndRun( s0, "TestScripts.cpp" );
	fAssert( compileSuccess0 );
}


#include "sqrat/sqrat.h"

class OverloadTest
{
public:
	std::string Foo( int a ) { std::stringstream ss; ss << "string Foo( " << a << " )"; return ss.str( ); }
	std::string Foo( float a ) { std::stringstream ss; ss << "string Foo( " << a << " )"; return ss.str( ); }
	std::string Foo( bool a, bool b ) { std::stringstream ss; ss << "string Foo( " << a << ", " << b << " )"; return ss.str( ); }
};


class Animal {
public:
	virtual std::string Speak() { return _SC("[Silent]"); }
};

class Cat : public Animal {
public:
	virtual std::string Speak() { return _SC("Meow!"); }
};

class Dog : public Animal {
public:
	virtual std::string Speak() { return _SC("Woof!"); }
};

std::string MakeSpeak(Animal* a) {
	return a->Speak();
}
define_unittest(TestSqRat)
{
	tScriptVm svm;
	Sqrat::DefaultVM::Set(svm.fSq( ));

	Sqrat::Class<OverloadTest> overloadTest;
	overloadTest
		.Func<std::string (OverloadTest::*)(int)>(_SC("Foo1"), &OverloadTest::Foo)
		.Func<std::string (OverloadTest::*)(float)>(_SC("Foo2"), &OverloadTest::Foo)
		.Func<std::string (OverloadTest::*)(bool,bool)>(_SC("Foo3"), &OverloadTest::Foo)
		;

	Sqrat::RootTable().Bind(_SC("OverloadTest"), overloadTest);

	Sqrat::ConstTable().Const(_SC("SOME_CONSTANT_1234"), 1234);

	{
		Sqrat::Script script;
		script.CompileString(_SC(" \
			ot <- OverloadTest( )\n \
			::print( ot.Foo1( SOME_CONSTANT_1234 ) )\n \
			::print( ot.Foo2( 2.2 ) )\n \
			::print( ot.Foo3( false, false ) )\n \
			"));
		script.Run();
	}


	// Defining class definitions inline
	Sqrat::RootTable().Bind(_SC("Animal"), 
		Sqrat::Class<Animal>()
		.Func(_SC("Speak"), &Animal::Speak)
		);

	Sqrat::RootTable().Bind(_SC("Cat"), 
		Sqrat::DerivedClass<Cat, Animal>()
		.Func(_SC("Speak"), &Cat::Speak)
		);
	
	Sqrat::RootTable().Bind(_SC("Dog"),
		Sqrat::DerivedClass<Dog, Animal>()
		.Func(_SC("Speak"), &Dog::Speak)
		);

	Sqrat::RootTable().Func(_SC("MakeSpeak"), &MakeSpeak);

	{
		Sqrat::Script script;
		script.CompileString(_SC(" \
			class Mouse extends Animal { \
				function Speak() { \
					return \"Squeak!\"; \
				} \
			} \
			\
			c <- Cat(); \
			d <- Dog(); \
			m <- Mouse(); \
			\
			print(c.Speak()); \
			print(d.Speak()); \
			print(m.Speak()); \
			\
			print(MakeSpeak(c)); \
			print(MakeSpeak(d)); \
			print(MakeSpeak(m)); \
			"));
		script.Run();
	}
}



