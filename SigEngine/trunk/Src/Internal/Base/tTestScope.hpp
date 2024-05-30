//------------------------------------------------------------------------------
// \file tTestScope.hpp - 3 Oct 2013
// \author mrickert
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __UnitTest_tTestScope__
#define __UnitTest_tTestScope__

#include "tTestReport.hpp"

namespace Sig { namespace UnitTest
{
	/// \class	tTestScope
	/// \brief	Implements the test_scope macro, letting you create nested groups of tests.
	class base_export tTestScope
	{
		declare_uncopyable( tTestScope );
	public:
		tTestScope( tTestReport* report, const char* file, u32 line, const char* name );
		~tTestScope( );

	private:
		tTestReport* mTestReport;
	};
}}

/// Puts everything inside the current scope after this declaration into a unit testing subcategory.
#define test_scope( name ) ::Sig::UnitTest::tTestScope cTestScope ## __LINE__ ( ::Sig::UnitTest::tTestReport::gCurrentTestTarget, __FILE__, __LINE__, #name )

#endif //ndef __UnitTest_tTestScope__
