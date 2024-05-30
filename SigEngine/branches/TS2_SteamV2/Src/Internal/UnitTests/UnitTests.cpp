#include "UnitTestsPch.hpp"
#include "tCmdLineOption.hpp"
#include "FileSystem.hpp"
using namespace Sig;

void fRunUnitTests( std::string& failedTestsTextOut, const std::string& cmdLineBuffer )
{
	const tCmdLineOption runSingleTestOnly( "t", cmdLineBuffer );

	std::stringstream failedTests;

	// cleanup temp folder before running
	FileSystem::fDeleteAllFilesInFolder( ToolsPaths::fGetEngineTempFolder( ) );

	for( Rtti::tClassHierarchyMap<tUnitTest>::tIterator test = tUnitTest::tManager::fInstance( ).fBegin( );
		test != tUnitTest::tManager::fInstance( ).fEnd( );
		++test )
	{
		if( test->fNullOrRemoved( ) )
			continue;

		if( runSingleTestOnly.fFound( ) && _stricmp( runSingleTestOnly.fGetOption( ).c_str( ), test->mValue->fGetName( ) ) )
			continue;

		log_line( 0, "-=unittest=- [" << test->mValue->fGetName( ) << "] STARTING" );
		b32 success = false;
		std::stringstream result;

		try
		{
			test->mValue->fExecute( );
			result	<< "-=unittest=- [" << test->mValue->fGetName( ) << "] PASSED";
			success = true;
		}
		catch( tUnitTestException& )
		{
			result	<< "-=unittest=- [" << test->mValue->fGetName( ) << "] "
					<< "FAILED with a known exception.";
		}
		catch(...)
		{
			result	<< "-=unittest=- [" << test->mValue->fGetName( ) << "] "
					<< "FAILED with an unknown exception.";
		}

		std::string resultText = result.str( );

		log_line( 0, resultText );

		if( !success )
			failedTests << resultText << std::endl;
	}

	failedTestsTextOut = failedTests.str( );
}


