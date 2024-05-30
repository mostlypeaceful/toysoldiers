//------------------------------------------------------------------------------
// \file tTestScope.cpp - 3 Oct 2013
// \author mrickert
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------

#include "BasePch.hpp"
#include "tTestScope.hpp"

namespace Sig { namespace UnitTest
{
	tTestScope::tTestScope( tTestReport* report, const char* file, u32 line, const char* name )
		: mTestReport( report )
	{
		if( mTestReport )
			mTestReport->fStartTestScope( file, line, name );
	}

	tTestScope::~tTestScope( )
	{
		if( mTestReport )
			mTestReport->fEndTestScope( );
	}
}}
