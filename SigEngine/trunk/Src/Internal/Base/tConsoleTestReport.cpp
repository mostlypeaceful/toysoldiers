//------------------------------------------------------------------------------
// \file tConsoleTestReport.cpp - 3 Oct 2013
// \author mrickert
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------

#include "BasePch.hpp"
#include "tConsoleTestReport.hpp"
#include "Debug/tDebugger.hpp"
#include <iostream>

namespace Sig { namespace UnitTest
{
	namespace
	{
		void fIndent( u32 n )
		{
			for( u32 i = 0; i < n; ++i )
				std::cout << "  ";
		}

		void fSetColorNormal( )
		{
#if defined( platform_pcdx )
			if( HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE) )
				SetConsoleTextAttribute( console, FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN );
#endif
		}

		void fSetColorPass( )
		{
#if defined( platform_pcdx )
			if( HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE) )
				SetConsoleTextAttribute( console, FOREGROUND_GREEN );
#endif
		}

		void fSetColorFail( )
		{
#if defined( platform_pcdx )
			if( HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE) )
				SetConsoleTextAttribute( console, FOREGROUND_RED );
#endif
		}
	}

	tConsoleTestReport::tConsoleTestReport( )
	{
		fStartTestScope( __FILE__, __LINE__, "All Tests" );
	}

	tConsoleTestReport::tConsoleTestReport( const tConsoleTestReportConfiguration& configuration )
		: mConfiguration( configuration )
	{
		fStartTestScope( __FILE__, __LINE__, "All Tests" );
	}

	void tConsoleTestReport::fStartTestScope( const char* file, u32 line, const char* name )
	{
		const u32 scopeDepth = mScopeInfo.fCount( );

		tScopeInfo scope = { name, file, line, 0, 0 };
		mScopeInfo.fPushBack( scope );

		if( scopeDepth > mConfiguration.mShowStartingTestsMaxScope )
			return;

		if( scopeDepth == 1 )
			std::cout << "\n\n";
		fSetColorNormal( );
		fIndent( scopeDepth );
		std::cout << "Starting test " << name << " (@ " << file << ":" << line << ")\n";
	}

	void tConsoleTestReport::fEndTestScope( )
	{
		tScopeInfo completed = mScopeInfo.fPopBack( );
		tScopeInfo& parent = mScopeInfo.fBack( );

		parent.mPassed += completed.mPassed;
		parent.mFailed += completed.mFailed;

		if( completed.mFailed == 0 && mScopeInfo.fCount( ) > mConfiguration.mShowPassedTestsMaxScope )
			return;

		fSetColorNormal( );
		fIndent( mScopeInfo.fCount( ) );
		std::cout << "[ ";
		if( completed.mFailed )
		{
			fSetColorFail( );
			std::cout << "FAIL";
		}
		else
		{
			fSetColorPass( );
			std::cout << "PASS";
		}
		fSetColorNormal( );
		std::cout << " ]: " << completed.mName << ": "
			<< completed.mPassed << " of " << completed.fTotal( ) << " test(s) passed"
			<< "\t\t@ " << completed.mFilename << ":" << completed.mLine
			<< "\n";

	}

	void tConsoleTestReport::fReportComplete( )
	{
		while( mScopeInfo.fCount( ) > 1 )
			fEndTestScope( );

		if( mScopeInfo.fCount( ) <= 0 )
			return; // already completed or bad pop

		tScopeInfo& all = mScopeInfo.fBack( );

		std::cout << "\nSUMMARY\n";
		if( all.mFailed )
			fSetColorFail( );
		else
			fSetColorPass( );
		std::cout << all.mPassed << " of " << all.fTotal( ) << " test(s) PASSED.\n";
		std::cout << all.mFailed << " of " << all.fTotal( ) << " test(s) FAILED.\n";
		fSetColorNormal( );
	}

	void tConsoleTestReport::fReportTestResult( const char* file, u32 line, const char* description, b32 passed )
	{
		if( mScopeInfo.fCount( ) )
		{
			if( passed )
				++mScopeInfo.fBack( ).mPassed;
			else
				++mScopeInfo.fBack( ).mFailed;
		}

		if( passed && mScopeInfo.fCount( ) > mConfiguration.mShowPassedTestsMaxScope )
			return;

		fSetColorNormal( );
		fIndent( mScopeInfo.fCount( ) );
		std::cout << "[ ";
		if( passed )
		{
			fSetColorPass( );
			std::cout << "PASS";
		}
		else
		{
			fSetColorFail( );
			std::cout << "FAIL";
		}
		fSetColorNormal( );
		std::cout << " ]: " << description << "\t\t@ " << file << ":" << line << "\n";
		if( !passed )
			break_if_debugger( );
	}
}}
