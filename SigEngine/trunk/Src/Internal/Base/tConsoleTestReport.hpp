//------------------------------------------------------------------------------
// \file tConsoleTestReport.cpp - 3 Oct 2013
// \author mrickert
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __UnitTest_tConsoleTestReport__
#define __UnitTest_tConsoleTestReport__

#include "tTestReport.hpp"

namespace Sig { namespace UnitTest
{
	/// \class	tConsoleTestReportConfiguration
	/// \brief	Configuration options for a tConsoleTestReport
	struct tConsoleTestReportConfiguration
	{
		u32		mShowStartingTestsMaxScope;	///< The maximum scope to show "Starting test "..."" for.  0 = none, 1 = root test methods only, 2 = root & direct children, etc.
		u32		mShowPassedTestsMaxScope;	///< The maximum scope to show "[ PASS ] ..." for.  0 = none, 1 = root test methods only, 2 = root & direct children, etc.

		tConsoleTestReportConfiguration( )
			: mShowStartingTestsMaxScope	( ~0u )
			, mShowPassedTestsMaxScope		( ~0u )
		{
		}
	};

	/// \class	tConsoleTestReport
	/// \brief	Report test results straight to the console / standard output.  Colorized on windows!
	class base_export tConsoleTestReport : public tTestReport
	{
	public:
		/// Implement tTestReport

		tConsoleTestReport( );
		explicit tConsoleTestReport( const tConsoleTestReportConfiguration& configuration );

		virtual void fStartTestScope( const char* file, u32 line, const char* name ) OVERRIDE;
		virtual void fEndTestScope( ) OVERRIDE;

		virtual void fReportComplete( ) OVERRIDE;

		virtual void fReportTestResult( const char* file, u32 line, const char* description, b32 passed ) OVERRIDE;

	private:
		struct tScopeInfo
		{
			const char*		mName;
			const char*		mFilename;
			u32				mLine;
			u32				mPassed;
			u32				mFailed;
			u32				fTotal( ) const { return mPassed + mFailed; }
		};

		tConsoleTestReportConfiguration	mConfiguration;
		tGrowableArray< tScopeInfo >	mScopeInfo;
	};
}}

#endif //ndef __UnitTest_tConsoleTestReport__
