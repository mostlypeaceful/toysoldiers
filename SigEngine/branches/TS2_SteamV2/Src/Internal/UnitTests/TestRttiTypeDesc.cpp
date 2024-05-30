#include "UnitTestsPch.hpp"
#include "Base.rtti.hpp"

using namespace Sig;
using namespace Sig::Math;

//
//using namespace Sig;
//
//struct A {};
//struct B {};
//struct C {};
//
//// @begin_class@
//class desc_test_base_0
//{
//public:
//
//	declare_member(A,_a0);
//	declare_member(B,_b1);
//
//#include "TestRttiTypeDesc_desc_test_base_0.Sig.rtti"
//};
//// @end_class@
//
//// @begin_class@
//class desc_test_base_1
//{
//public:
//
//	declare_member(C,_c0);
//
//#include "TestRttiTypeDesc_desc_test_base_1.Sig.rtti"
//};
//// @end_class@
//
//
//// @begin_class@
//class desc_test : public desc_test_base_0, public desc_test_base_1
//{
//public:
//
//	// @begin_class@
//	class internal_class
//	{
//	public:
//		declare_member(int, i);
//#include "TestRttiTypeDesc_internal_class.Sig.rtti"
//	};
//	// @end_class@
//
//	declare_member( Sig::u32,			_i0);
//	declare_member( int,				_i1);
//	declare_member( A,					_a0);
//	declare_member( internal_class,		_intern);
//
//#include "TestRttiTypeDesc_desc_test.Sig.rtti"
//};
//// @end_class@

define_unittest(TestRttiTypeDesc)
{

	const Rtti::tReflector& r = tVec4f::fGetReflector( );
	const Rtti::tBaseClassDesc* baseClasses = r.fBaseClasses( );
	const Rtti::tClassMemberDesc* members = r.fClassMembers( );

	const size_t size = sizeof(tVec4f);
	const size_t size2 = sizeof(tBaseVector<float,4,tVector4<float> >);

	while( baseClasses && !baseClasses->fNull( )) 
	{
		++baseClasses;
	}

	while( members && !members->fNull( )) 
	{
		++members;
	}

}
