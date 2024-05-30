#ifndef __tDelegate__
#define __tDelegate__

namespace Sig
{

template< class Signature >
class tDelegate;

template< class Signature >
class tEvent;

#define argument_count 0
#define argument_list_template
#define argument_list_template_inst
#define argument_list_declared
#define argument_list_passed
#include "tDelegateTemplate.hpp"
#undef argument_count
#undef argument_list_template
#undef argument_list_template_inst
#undef argument_list_declared
#undef argument_list_passed
// the above defines the type tDelegate0 (a templatized delgate accepting 0 arguments) for use in c++

#define argument_count 1
#define argument_list_template			class t0
#define argument_list_template_inst		t0
#define argument_list_declared			t0 a0
#define argument_list_passed			a0
#include "tDelegateTemplate.hpp"
#undef argument_count
#undef argument_list_template
#undef argument_list_template_inst
#undef argument_list_declared
#undef argument_list_passed
// the above defines the type tDelegate1 (a templatized delgate accepting 1 argument) for use in c++

#define argument_count 2
#define argument_list_template			class t0, class t1
#define argument_list_template_inst		t0, t1
#define argument_list_declared			t0 a0, t1 a1
#define argument_list_passed			a0, a1
#include "tDelegateTemplate.hpp"
#undef argument_count
#undef argument_list_template
#undef argument_list_template_inst
#undef argument_list_declared
#undef argument_list_passed
// the above defines the type tDelegate2 (a templatized delgate accepting 2 arguments) for use in c++

#define argument_count 3
#define argument_list_template			class t0, class t1, class t2
#define argument_list_template_inst		t0, t1, t2
#define argument_list_declared			t0 a0, t1 a1, t2 a2
#define argument_list_passed			a0, a1, a2
#include "tDelegateTemplate.hpp"
#undef argument_count
#undef argument_list_template
#undef argument_list_template_inst
#undef argument_list_declared
#undef argument_list_passed
// the above defines the type tDelegate3 (a templatized delgate accepting 3 arguments) for use in c++

#define argument_count 4
#define argument_list_template			class t0, class t1, class t2, class t3
#define argument_list_template_inst		t0, t1, t2, t3
#define argument_list_declared			t0 a0, t1 a1, t2 a2, t3 a3
#define argument_list_passed			a0, a1, a2, a3
#include "tDelegateTemplate.hpp"
#undef argument_count
#undef argument_list_template
#undef argument_list_template_inst
#undef argument_list_declared
#undef argument_list_passed
// the above defines the type tDelegate4 (a templatized delgate accepting 4 arguments) for use in c++

#define argument_count 5
#define argument_list_template			class t0, class t1, class t2, class t3, class t4
#define argument_list_template_inst		t0, t1, t2, t3, t4
#define argument_list_declared			t0 a0, t1 a1, t2 a2, t3 a3, t4 a4
#define argument_list_passed			a0, a1, a2, a3, a4
#include "tDelegateTemplate.hpp"
#undef argument_count
#undef argument_list_template
#undef argument_list_template_inst
#undef argument_list_declared
#undef argument_list_passed
// the above defines the type tDelegate5 (a templatized delgate accepting 5 arguments) for use in c++

#define argument_count 6
#define argument_list_template			class t0, class t1, class t2, class t3, class t4, class t5
#define argument_list_template_inst		t0, t1, t2, t3, t4, t5
#define argument_list_declared			t0 a0, t1 a1, t2 a2, t3 a3, t4 a4, t5 a5
#define argument_list_passed			a0, a1, a2, a3, a4, a5
#include "tDelegateTemplate.hpp"
#undef argument_count
#undef argument_list_template
#undef argument_list_template_inst
#undef argument_list_declared
#undef argument_list_passed
// the above defines the type tDelegate6 (a templatized delgate accepting 6 arguments) for use in c++

#define argument_count 7
#define argument_list_template			class t0, class t1, class t2, class t3, class t4, class t5, class t6
#define argument_list_template_inst		t0, t1, t2, t3, t4, t5, t6
#define argument_list_declared			t0 a0, t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6
#define argument_list_passed			a0, a1, a2, a3, a4, a5, a6
#include "tDelegateTemplate.hpp"
#undef argument_count
#undef argument_list_template
#undef argument_list_template_inst
#undef argument_list_declared
#undef argument_list_passed
// the above defines the type tDelegate7 (a templatized delgate accepting 7 arguments) for use in c++

#define argument_count 8
#define argument_list_template			class t0, class t1, class t2, class t3, class t4, class t5, class t6, class t7
#define argument_list_template_inst		t0, t1, t2, t3, t4, t5, t6, t7
#define argument_list_declared			t0 a0, t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6, t7 a7
#define argument_list_passed			a0, a1, a2, a3, a4, a5, a6, a7
#include "tDelegateTemplate.hpp"
#undef argument_count
#undef argument_list_template
#undef argument_list_template_inst
#undef argument_list_declared
#undef argument_list_passed
// the above defines the type tDelegate8 (a templatized delgate accepting 8 arguments) for use in c++

#define argument_count 9
#define argument_list_template			class t0, class t1, class t2, class t3, class t4, class t5, class t6, class t7, class t8
#define argument_list_template_inst		t0, t1, t2, t3, t4, t5, t6, t7, t8
#define argument_list_declared			t0 a0, t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6, t7 a7, t8 a8
#define argument_list_passed			a0, a1, a2, a3, a4, a5, a6, a7, a8
#include "tDelegateTemplate.hpp"
#undef argument_count
#undef argument_list_template
#undef argument_list_template_inst
#undef argument_list_declared
#undef argument_list_passed
// the above defines the type tDelegate9 (a templatized delgate accepting 9 arguments) for use in c++

#define argument_count 10
#define argument_list_template			class t0, class t1, class t2, class t3, class t4, class t5, class t6, class t7, class t8, class t9
#define argument_list_template_inst		t0, t1, t2, t3, t4, t5, t6, t7, t8, t9
#define argument_list_declared			t0 a0, t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6, t7 a7, t8 a8, t9 a9
#define argument_list_passed			a0, a1, a2, a3, a4, a5, a6, a7, a8, a9
#include "tDelegateTemplate.hpp"
#undef argument_count
#undef argument_list_template
#undef argument_list_template_inst
#undef argument_list_declared
#undef argument_list_passed
// the above defines the type tDelegate10 (a templatized delgate accepting 10 arguments) for use in c++


// do we really need more? if so, add them here... but c'mon people, more than 10 arguments to a function? ugly...

#define make_delegate_cfn( delType, cFunc ) \
	delType :: fCreateFromRawObjectAndStub(	NULL, & delType :: fInferFunctionStubWrapper( cFunc ).fFunctionStub< &cFunc > )

#define make_delegate_memfn( delType, memType, memFunc ) \
	delType :: fCreateFromRawObjectAndStub(	this, & delType :: fInferMethodStubWrapper( this, & memType :: memFunc ).fMethodStub< & memType :: memFunc > ) \

#define make_delegate_memfn_obj( delType, memType, memFunc, obj ) \
	delType :: fCreateFromRawObjectAndStub(	obj, & delType :: fInferMethodStubWrapper( obj, & memType :: memFunc ).fMethodStub< & memType :: memFunc > ) \

}

#endif//__tDelegate__
