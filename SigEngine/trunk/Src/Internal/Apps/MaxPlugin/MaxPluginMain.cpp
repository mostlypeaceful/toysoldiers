#include "MaxClassDesc.hpp"
#include "tExporter.hpp"
using namespace Sig;

#define EXPORT_SYMBOL __declspec( dllexport )

HINSTANCE g_Hinst;

// This function is called by Windows when the DLL is loaded.  This 
// function may also be called many times during time critical operations
// like rendering.  Therefore developers need to be careful what they
// do inside this function.  In the code below, note how after the DLL is
// loaded the first time only a few statements are executed.

BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
	Sig::MaxPlugin::tExporter::fMakeLinkerRespectCode( );

	switch(fdwReason)
	{
    case DLL_PROCESS_ATTACH:
        g_Hinst = hinstDLL;
        DisableThreadLibraryCalls( g_Hinst );
        break;
    }

	return TRUE;
}

// This function returns a string that describes the DLL and where the user
// could purchase the DLL if they don't have it.
EXPORT_SYMBOL const TCHAR* LibDescription()
{
	return TEXT( "Sig plugin" );
}

// This function returns the number of plug-in classes this DLL
EXPORT_SYMBOL int LibNumberClasses()
{
	return Sig::MaxPlugin::tBaseClassDesc::fNumberOfClasses( );
}

// This function returns the number of plug-in classes this DLL
EXPORT_SYMBOL ClassDesc* LibClassDesc(int i)
{
	return Sig::MaxPlugin::tBaseClassDesc::fGetClassDesc( i );
}

// This function returns a pre-defined constant indicating the version of 
// the system under which it was compiled.  It is used to allow the system
// to catch obsolete DLLs.
EXPORT_SYMBOL ULONG LibVersion()
{
	return VERSION_3DSMAX;
}

EXPORT_SYMBOL int LibInitialize()
{
	static b32 controlsInit = false;
	if (!controlsInit) 
	{
		controlsInit = TRUE;
		InitCustomControls( g_Hinst );	// Initialize MAX's custom controls
		InitCommonControls( );			// Initialize Win95 controls
	}

	return 1;
}

EXPORT_SYMBOL int LibShutdown()
{
	return 1;
}

