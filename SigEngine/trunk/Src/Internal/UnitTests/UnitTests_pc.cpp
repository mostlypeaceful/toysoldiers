#include "UnitTestsPch.hpp"
#include "tWin32Window.hpp"
#include "tCmdLineOption.hpp"
#include "resource.h"

using namespace Sig;

extern void fRunUnitTests( std::string& failedTestsTextOut, const std::string& cmdLineBuffer );

int WINAPI WinMain( HINSTANCE h1, HINSTANCE h2, LPSTR cmdLine, int showCmd )
{
	std::string failedTestsText;

	fRunUnitTests( failedTestsText, cmdLine );

	if( failedTestsText.length( ) > 0 )
		MessageBox( 0, failedTestsText.c_str( ), "Sig UnitTest Errors", MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_TOPMOST );
	else
		MessageBox( 0, "All unit tests passed", "Sig UnitTest Success", MB_OK | MB_ICONINFORMATION | MB_APPLMODAL | MB_TOPMOST );

	return 0;
}

