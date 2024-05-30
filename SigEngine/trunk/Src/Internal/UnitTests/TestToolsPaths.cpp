#include "UnitTestsPch.hpp"

using namespace Sig;

define_unittest( TestToolsPaths )
{
	tFilePathPtr engineRoot = ToolsPaths::fGetEngineRootFolder( );
	ToolsPaths::fConfigureEngineEnvironmentVariables( engineRoot );

	// save existing path
	std::string savedProjectName = ToolsPaths::fGetCurrentProjectName( );
	tFilePathPtr savedProjectPath = ToolsPaths::fGetCurrentProjectRootFolder( );

	// set a new path and verify
	tFilePathPtr newTestPath = tFilePathPtr::fConstructPath( ToolsPaths::fGetEngineRootFolder( ), tFilePathPtr( "..\\Projects\\Tropolis" ) );
	ToolsPaths::fSetCurrentProjectRootFolder( "Test", newTestPath );
	tFilePathPtr curProjPath = ToolsPaths::fGetCurrentProjectRootFolder( );
	fAssertEqual( newTestPath, curProjPath );

	// reset original path and verify
	ToolsPaths::fSetCurrentProjectRootFolder( savedProjectName, savedProjectPath );
	curProjPath = ToolsPaths::fGetCurrentProjectRootFolder( );
	fAssertEqual( savedProjectPath, curProjPath );
}


