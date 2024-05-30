//------------------------------------------------------------------------------
// \file tTestReport.hpp - 3 Oct 2013
// \author mrickert
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __UnitTest_tTestReport__
#define __UnitTest_tTestReport__

#include <sstream>

namespace Sig { namespace UnitTest
{
	/// \class	tTestReport
	/// \brief	A test reporting interface, talked to by the various test_* macros.
	class base_export tTestReport
	{
		declare_uncopyable( tTestReport );
	public:
		static tTestReport* gCurrentTestTarget;		///< The current tTestReport to use for test_* macros.

		tTestReport( ) { }
		virtual ~tTestReport( ) { }

		/// \brief	Call when you've completed all your tests for final reporting, stream closure, etc.
		virtual void fReportComplete( ) { }

		/// \brief	Start a new grouping / test "scope".
		///			Consider using the test_scope macro instead of this directly.
		virtual void fStartTestScope( const char* file, u32 line, const char* name ) = 0;

		/// \brief	Close a test scope you previously opened with fStartTestScope.
		///			Consider using the test_scope macro instead of this directly.
		virtual void fEndTestScope( ) = 0;

		/// \brief	Report the results of an individual test.
		///			Consider using test, test_cmp, or one of the many other condition testing macros instead of this directly.
		virtual void fReportTestResult( const char* file, u32 line, const char* description, b32 passed ) = 0;
	};


/// \brief		Report a specific unit test result.
///
/// \example	test_result( "Verify fFoo(123) returns true", fFoo(123) );
///
/// \param	description		A description of the test.  Example: "Verify fFoo(123) returns true"
/// \param	result			True if the test passed, false if the test failed.  Example: fFoo(123)
#define test_result( description, result ) \
	if( ::Sig::UnitTest::tTestReport::gCurrentTestTarget ) { \
		::std::stringstream ss; \
		ss << description; \
		::Sig::UnitTest::tTestReport::gCurrentTestTarget->fReportTestResult( __FILE__, __LINE__, ss.str( ).c_str( ), result ); \
	} else ((void)0)


/// \brief		Report that a specific unit test passed.
///
/// \example	if( fFoo(123) ) test_pass( "Verified that fFoo(123) returns true" );
///
/// \param	description		A description of the test.
#define test_pass( description ) test_result( description, true )


/// \brief		Report that a specific unit test failed.
///
/// \example	if( !fFoo(123) ) test_fail( "Failed to verify that fFoo(123) returns true" );
///
/// \param	description		A description of the test that failed.
#define test_fail( description ) test_result( description, false )


/// \brief		Unit test a condition.  Takes a user-supplied description to describe the test.
///				Additionally takes some statement(s) to execute if the test fails.
///
/// \example	test_desc_else( 1 == 1, "Verify basic mathematical identity", fPanic( ); return );
///
/// \param	condition		The test condition.  True indicates success, False indicates failure.  Example: 1 == 1
/// \param	description		A description of the test condition. Example: "Verify basic mathematical identity"
 /// \param	ontestfail		Additional statement(s) to execute if condition evaluated to false.  Example: fPanic( ); return
#define test_desc_else( condition, description, ontestfail ) \
	if( true ) \
	{ \
		if(( condition )) \
		{ \
			test_pass( description ); \
		} \
		else \
		{ \
			test_fail( description ); \
			ontestfail; \
		} \
	} else ((void)0)

/// \brief		Unit test a condition.  Uses the conditional expression as the description of the test.
///
/// \example	test( 1 == 1 );
///
/// \param	cond				The test condition.  True indicates success, False indicates failure.  Example: 1 == 1
#define test( cond )								test_desc_else( (cond), #cond, )

/// \brief		Unit test a condition.  Uses the conditional expression as the description of the test.
///				Additionally takes some statement(s) to execute if the test fails.
///
/// \example	test_else( 1 == 1, fPanic( ); return );
///
/// \param		condition		The test condition.  True indicates success, False indicates failure.  Example: 1 == 1
/// \param		ontestfail		Additional statement(s) to execute if condition evaluated to false.  Example: fPanic( ); return
#define test_else( cond, ontestfail )				test_desc_else( (cond), #cond, ontestfail )

/// \brief		Unit test a condition.  Compares two expressions and displays their values as part of the test
///				description.  The test is assumed to pass if the conditional holds, or fail if it does not.
///
/// \example	test_cmp( 1, <=, 2 );
///
/// \param		lhs				The left hand side of the comparison.  Example: 1
/// \param		op				The operator to compare.  Example: ==
/// \param		rhs				The right hand side of the comparison.  Example: 2
#define test_cmp( lhs, op, rhs )					test_desc_else( (lhs) op (rhs), #lhs << " (" << (lhs) << ")  " #op "  " #rhs << " (" << (rhs) << ")", )

/// \brief		Unit test a condition.  Compares two expressions and displays their values as part of the test
///				description.  The test is assumed to pass if the conditional holds, or fail if it does not.
///				Additionally takes some statement(s) to execute if the test fails.
///
/// \example	test_cmp_else( 1, <=, 2, fPanic( ); return );
///
/// \param		lhs				The left hand side of the comparison.  Example: 1
/// \param		op				The operator to compare.  Example: ==
/// \param		rhs				The right hand side of the comparison.  Example: 2
/// \param		ontestfail		The statement(s) to execute if the conditional evaluates to false.
#define test_cmp_else( lhs, op, rhs, ontestfail )	test_desc_else( (lhs) op (rhs), #lhs << " (" << (lhs) << ")  " #op "  " #rhs << " (" << (rhs) << ")", ontestfail )
}}

#endif //ndef __UnitTest_tTestReport__
