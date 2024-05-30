// InputSimulator.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define DISPLAY_OUTPUT FALSE

static bool gameErrorStarted = false;
static bool gameCrashEncountered = false;
static bool inputErrorEncountered = false;
static bool loadNextLevel = false;
static char lastDmMessage[MAXBUFSIZE]={0};
static char gameErrorMessage[MAXBUFSIZE]={0};

enum CONTROLLER_BUTTONS {	DPADUP,
							DPADDOWN,
							DPADLEFT,
							DPADRIGHT,
							START,
							BACK,
							LEFTTHUMB,
							RIGHTTHUMB,
							LEFTSHOULDER,
							RIGHTSHOULDER,
							A,
							B,
							X,
							Y,
							XBOX360BUTTON,
							RIGHTTRIGGER,
							LEFTTRIGGER,
							THUMBLX,
							THUMBLY,
							THUMBRX,
							THUMBRY	};

//----------------------------------------------------------------------------
// Prototypes
//----------------------------------------------------------------------------
int LoadXBDM();
int DoRandomInput(int seconds, vector<CONTROLLER_BUTTONS> disabledButtons);
int GiveScriptedInput(char* script);
void DisplayError(char* funcName, HRESULT hresult);
void Mode0(MapList* mapList, char* mapPathBuffer, int mapPathBufferSize);
void Mode1(MapList* mapList, char* mapPathBuffer, int mapPathBufferSize);
DWORD DebugEvtHandler(ULONG dwNotification, DWORD dwParam);
int InitializeInput();
int UninitializeInput();
void SelectRandomUnit();
void WaitForControllerIdle();
bool FileExists(const char* filename);

//----------------------------------------------------------------------------
// Name: Main
// Desc: Entry point for program
//----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	// Attempt to load XBDM.DLL.  The project settings specify to delay load
	// it so as to not break the linker.  So long as we load it manually
	// before we try to use it, all will be well.  Exit main on failure.
	if (!LoadXBDM())
		return 0;

	MapList* mapList = new MapList();

	// Return successfully when there's no connection until
	// a dedicated XDK can be set up; otherwise, CCNET will
	// detect this as breaking the build
	if (DISPLAY_OUTPUT)
		printf("Opening a connection to the console...\n");
	PDM_CONNECTION pdmConnection;
	HRESULT hr = DmOpenConnection(&pdmConnection);
	if ( FAILED(hr) )
	{
		DisplayError("DmOpenConnection", hr);
		return 0;
	}

	if (DISPLAY_OUTPUT)
		printf("Opening a notification session on the console...\n");
	PDMN_SESSION pdmnSession;
	hr = DmOpenNotificationSession(DM_DEBUGSESSION, &pdmnSession);
	if ( FAILED(hr) )
	{
		DisplayError("DmOpenNotificationSession", hr);
		return 1;
	}

	if (DISPLAY_OUTPUT)
		printf("Registering notification handlers...\n");
	DmNotify(pdmnSession, DM_BREAK, (PDM_NOTIFY_FUNCTION) DebugEvtHandler);
	DmNotify(pdmnSession, DM_DEBUGSESSION, (PDM_NOTIFY_FUNCTION) DebugEvtHandler);
	DmNotify(pdmnSession, DM_DEBUGSTR, (PDM_NOTIFY_FUNCTION) DebugEvtHandler);
	DmNotify(pdmnSession, DM_SINGLESTEP, (PDM_NOTIFY_FUNCTION) DebugEvtHandler);
	DmNotify(pdmnSession, DM_MODLOAD, (PDM_NOTIFY_FUNCTION) DebugEvtHandler);
	DmNotify(pdmnSession, DM_MODUNLOAD, (PDM_NOTIFY_FUNCTION) DebugEvtHandler);
	DmNotify(pdmnSession, DM_CREATETHREAD, (PDM_NOTIFY_FUNCTION) DebugEvtHandler);
	DmNotify(pdmnSession, DM_DESTROYTHREAD, (PDM_NOTIFY_FUNCTION) DebugEvtHandler);
	DmNotify(pdmnSession, DM_EXCEPTION, (PDM_NOTIFY_FUNCTION) DebugEvtHandler);
	DmNotify(pdmnSession, DM_ASSERT, (PDM_NOTIFY_FUNCTION) DebugEvtHandler);
	DmNotify(pdmnSession, DM_DATABREAK, (PDM_NOTIFY_FUNCTION) DebugEvtHandler);
	DmNotify(pdmnSession, DM_RIP, (PDM_NOTIFY_FUNCTION) DebugEvtHandler);
	DmNotify(pdmnSession, DM_SECTIONLOAD, (PDM_NOTIFY_FUNCTION) DebugEvtHandler);
	DmNotify(pdmnSession, DM_SECTIONUNLOAD, (PDM_NOTIFY_FUNCTION) DebugEvtHandler);
	DmNotify(pdmnSession, DM_ASSERTION_FAILURE, (PDM_NOTIFY_FUNCTION) DebugEvtHandler);

	if ( InitializeInput() )
		return 1;

	char mapPath[MAXBUFSIZE];
	
	if (argc > 2)
	{
		for (int i = 0; i < argc; i++)
		{
			if ( (strcmp(argv[i], "-mode") == 0) && ( (i + 1) < argc ) )
			{
				int mode = atoi(argv[i + 1]);
				switch (mode)
				{
				case 0:
					Mode0(mapList, mapPath, MAXBUFSIZE);
					break;
				case 1:
					Mode1(mapList, mapPath, MAXBUFSIZE);
					break;
				default:
					Mode0(mapList, mapPath, MAXBUFSIZE);
				}
			}
		}
	}
	else
		Mode0(mapList, mapPath, MAXBUFSIZE);

	UninitializeInput();

	if (DISPLAY_OUTPUT)
		printf("Closing notification session on the console...\n");
	hr = DmCloseNotificationSession(pdmnSession);
	if ( FAILED(hr) )
		DisplayError("DmCloseNotificationSession", hr);

	if (DISPLAY_OUTPUT)
		printf("Closing the connection to the console...\n");
	hr = DmCloseConnection(pdmConnection);
	if ( FAILED( hr ) )
		DisplayError("DmCloseConnection", hr);

	if (gameCrashEncountered || inputErrorEncountered)
	{
		// Only some sort of system crash needs a crash dump
		if (gameCrashEncountered)
		{
			printf("!WARNING! Game crashed in map %s.\n", mapPath);
			if( gameErrorMessage[0] )
				printf("!WARNING! Last output from game was:\n%s\n", gameErrorMessage );
			
			DmReboot(DMBOOT_COLD);
		}
		else
			DmReboot(DMBOOT_COLD);

		delete mapList;

		return 1;
	}

	DmReboot(DMBOOT_COLD);

	delete mapList;

	return 0;
}


//----------------------------------------------------------------------------
// Name: LoadXBDM
// Desc: Adds %xedk%\bin\win32 to the path and loads XBDM.DLL.
//----------------------------------------------------------------------------
BOOL LoadXBDM()
{
	char* path;
	char* xedkDir;
	size_t pathSize;
	size_t xedkDirSize;

	errno_t err;

	err = _dupenv_s(&path, &pathSize, "path");
	if (err)
		return 0;

	err = _dupenv_s(&xedkDir, &xedkDirSize, "xedk");
	if (err)
		return 0;

	// Build a new path with the %xedk%\bin\win32 directory added.
	// Allocate enough memory for the newPath string.
	char formatBuffer[] = "path=%s\\bin\\win32;%s";

	// Calculate how much extra space is needed--this will actually
	// allocate a few bytes more than necessary.
	int formatSize = sizeof(formatBuffer);
	size_t newPathSize = pathSize + xedkDirSize + formatSize;
	char* newPath = new char[newPathSize];

	// _snprintf_s will always zero-terminate the buffer.
	_snprintf_s(newPath, newPathSize, newPathSize, formatBuffer, xedkDir ? xedkDir : "", path);

	// Update the system path with the new value.  This is so our LoadLibrary
	// call can find XBDM.DLL.
	_putenv(newPath);

	// Delete newPath
	delete [] newPath;

	// Free path
	if (path)
		free(path);

	// Free xedkDir
	if (xedkDir)
		free(xedkDir);

	// Call LoadLibrary on XBDM.DLL.
	HMODULE hXBDM = LoadLibrary("xbdm.dll");

	// Print an error message and return zero if XBDM.DLL didn't load.
	if (!hXBDM)
	{
		if ( xedkDir )
			printf("\nERROR:\n\nCouldn't load xbdm.dll.\n");
		else
			printf("\nERROR:\n\nCouldn't load xbdm.dll\nXEDK environment variable not set.\n");
		return 0;
	}

	// XBDM.DLL loaded.  Return 1 for success.
	return 1;
}

//----------------------------------------------------------------------------
// Name: InitializeInput
// Desc: Initializes the XSim API and acquires the controller.
//----------------------------------------------------------------------------
int InitializeInput()
{
	HRESULT hr = XSimInitialize(0);
	if ( FAILED( hr ) && hr != XSIM_E_ALREADY_INITIALIZED)
	{
		DisplayError("XSimInitialize", hr);
		return 1;
	}

	// Get control of the first controller port
	hr = XSimAcquireControl(XSIM_USERINDEXMASK_0);
	if ( FAILED( hr ) && hr != XSIM_E_ALREADY_ACQUIRED )
	{
		DisplayError("XSimAcquireControl", hr);
		return 1;
	}

	return 0;
}

//----------------------------------------------------------------------------
// Name: UninitializeInput
// Desc: Uninitializes the XSim API and releases the controller.
//----------------------------------------------------------------------------
int UninitializeInput()
{
	HRESULT hr = XSimReturnControl(XSIM_USERINDEXMASK_0);
	if ( FAILED( hr ) )
		DisplayError("XSimReturnControl", hr);

	hr = XSimUninitialize();
	if ( FAILED( hr ) )
	{
		DisplayError("XSimUninitialize", hr);
		return 1;
	}

	return 0;
}

//----------------------------------------------------------------------------
// Name: GiveScriptedInput
// Desc: Sends scripted input to the console.
//----------------------------------------------------------------------------
int GiveScriptedInput(char* script)
{
	if (DISPLAY_OUTPUT)
		printf("Playing back textual inputs.\n");

	XSIMHANDLE hPlayer;
	HRESULT hr = XSimCreateTextSequencePlayer(script, XSIM_SYNCHMODE_TIME, 20, &hPlayer);
	if ( FAILED( hr ) )
	{
		DisplayError("XSimCreateTextSequencePlayer", hr);
		return 1;
	}

	hr = XSimStartPlayer(hPlayer, 0);
	if ( FAILED( hr ) )
	{
		DisplayError("XSimStartPlayer", hr);
		return 1;
	}

	WaitForControllerIdle();

	hr = XSimStopPlayer(0);
	if ( FAILED( hr ) )
	{
		DisplayError("XSimStopPlayer", hr);
		return 1;
	}

	return 0;
}

//----------------------------------------------------------------------------
// Name: DoRandomInput
// Desc: Sends random input to the console for a given number of seconds.
//----------------------------------------------------------------------------
int DoRandomInput(int seconds, vector<CONTROLLER_BUTTONS> disabledButtons)
{
	if (DISPLAY_OUTPUT)
		printf("Random input is being given for the next %d seconds.\n", seconds);

	XSIMHANDLE hPlayer;
	HRESULT hr = XSimCreateRandomInputPlayer(&hPlayer);
	if ( FAILED( hr ) )
	{
		DisplayError("XSimCreateRandomInputPlayer", hr);
		return 1;
	}

	XSIM_CONTROLLERPRESSATTRIBUTES pressAttr;
	hr = XSimGetRandomInputPlayerPressAttributes(hPlayer, &pressAttr);
	if ( FAILED( hr ) )
	{
		DisplayError("XSimGetRandomInputPlayerPressAttributes", hr);
		return 1;
	}

	for (unsigned int i = 0; i < disabledButtons.size(); i++)
	{
		switch (disabledButtons[i])
		{
		case DPADUP:
			if (DISPLAY_OUTPUT)
				printf("Disabled DPADUP input.\n");
			memset( &pressAttr.DpadUp, 0, sizeof( XSIM_BUTTONPRESSATTRIBUTES ) );
			break;
		case DPADDOWN:
			if (DISPLAY_OUTPUT)
				printf("Disabled DPADDOWN input.\n");
			memset( &pressAttr.DpadDown, 0, sizeof( XSIM_BUTTONPRESSATTRIBUTES ) );
			break;
		case DPADLEFT:
			if (DISPLAY_OUTPUT)
				printf("Disabled DPADLEFT input.\n");
			memset( &pressAttr.DpadLeft, 0, sizeof( XSIM_BUTTONPRESSATTRIBUTES ) );
			break;
		case DPADRIGHT:
			if (DISPLAY_OUTPUT)
				printf("Disabled DPADRIGHT input.\n");
			memset( &pressAttr.DpadRight, 0, sizeof( XSIM_BUTTONPRESSATTRIBUTES ) );
			break;
		case START:
			if (DISPLAY_OUTPUT)
				printf("Disabled START input.\n");
			memset( &pressAttr.Start, 0, sizeof( XSIM_BUTTONPRESSATTRIBUTES ) );
			break;
		case BACK:
			if (DISPLAY_OUTPUT)
				printf("Disabled BACK input.\n");
			memset( &pressAttr.Back, 0, sizeof( XSIM_BUTTONPRESSATTRIBUTES ) );
			break;
		case LEFTTHUMB:
			if (DISPLAY_OUTPUT)
				printf("Disabled LEFTTHUMB input.\n");
			memset( &pressAttr.LeftThumb, 0, sizeof( XSIM_BUTTONPRESSATTRIBUTES ) );
			break;
		case RIGHTTHUMB:
			if (DISPLAY_OUTPUT)
				printf("Disabled RIGHTTHUMB input.\n");
			memset( &pressAttr.RightThumb, 0, sizeof( XSIM_BUTTONPRESSATTRIBUTES ) );
			break;
		case LEFTSHOULDER:
			if (DISPLAY_OUTPUT)
				printf("Disabled LEFTSHOULDER input.\n");
			memset( &pressAttr.LeftShoulder, 0, sizeof( XSIM_BUTTONPRESSATTRIBUTES ) );
			break;
		case RIGHTSHOULDER:
			if (DISPLAY_OUTPUT)
				printf("Disabled RIGHTSHOULDER input.\n");
			memset( &pressAttr.RightShoulder, 0, sizeof( XSIM_BUTTONPRESSATTRIBUTES ) );
			break;
		case A:
			if (DISPLAY_OUTPUT)
				printf("Disabled A input.\n");
			memset( &pressAttr.A, 0, sizeof( XSIM_BUTTONPRESSATTRIBUTES ) );
			break;
		case B:
			if (DISPLAY_OUTPUT)
				printf("Disabled B input.\n");
			memset( &pressAttr.B, 0, sizeof( XSIM_BUTTONPRESSATTRIBUTES ) );
			break;
		case X:
			if (DISPLAY_OUTPUT)
				printf("Disabled X input.\n");
			memset( &pressAttr.X, 0, sizeof( XSIM_BUTTONPRESSATTRIBUTES ) );
			break;
		case Y:
			if (DISPLAY_OUTPUT)
				printf("Disabled Y input.\n");
			memset( &pressAttr.Y, 0, sizeof( XSIM_BUTTONPRESSATTRIBUTES ) );
			break;
		case XBOX360BUTTON:
			if (DISPLAY_OUTPUT)
				printf("Disabled XBOX360BUTTON input.\n");
			memset( &pressAttr.XBox360Button, 0, sizeof( XSIM_BUTTONPRESSATTRIBUTES ) );
			break;
		case RIGHTTRIGGER:
			if (DISPLAY_OUTPUT)
				printf("Disabled RIGHTTRIGGER input.\n");
			memset( &pressAttr.RightTrigger, 0, sizeof( XSIM_BUTTONPRESSATTRIBUTES ) );
			break;
		case LEFTTRIGGER:
			if (DISPLAY_OUTPUT)
				printf("Disabled LEFTTRIGGER input.\n");
			memset( &pressAttr.LeftTrigger, 0, sizeof( XSIM_BUTTONPRESSATTRIBUTES ) );
			break;
		case THUMBLX:
			if (DISPLAY_OUTPUT)
				printf("Disabled THUMBLX input.\n");
			memset( &pressAttr.ThumbLX, 0, sizeof( XSIM_BUTTONPRESSATTRIBUTES ) );
			break;
		case THUMBLY:
			if (DISPLAY_OUTPUT)
				printf("Disabled THUMBLY input.\n");
			memset( &pressAttr.ThumbLY, 0, sizeof( XSIM_BUTTONPRESSATTRIBUTES ) );
			break;
		case THUMBRX:
			if (DISPLAY_OUTPUT)
				printf("Disabled THUMBRX input.\n");
			memset( &pressAttr.ThumbRX, 0, sizeof( XSIM_BUTTONPRESSATTRIBUTES ) );
			break;
		case THUMBRY:
			if (DISPLAY_OUTPUT)
				printf("Disabled THUMBRY input.\n");
			memset( &pressAttr.ThumbRY, 0, sizeof( XSIM_BUTTONPRESSATTRIBUTES ) );
			break;
		}
	}

	hr = XSimSetRandomInputPlayerPressAttributes(hPlayer, &pressAttr);
	if ( FAILED( hr ) )
	{
		DisplayError("XSimGetRandomInputPlayerPressAttributes", hr);
		return 1;
	}

	hr = XSimStartPlayer(hPlayer, 0);
	if ( FAILED( hr ) )
	{
		DisplayError("XSimStartPlayer", hr);
		return 1;
	}

	Sleep( (DWORD) (seconds * 1000) );

	hr = XSimStopPlayer(0);
	if ( FAILED( hr ) )
	{
		DisplayError("XSimStopPlayer", hr);
		return 1;
	}

	if (DISPLAY_OUTPUT)
		printf("Random input is stopped.\n");

	return 0;
}

//----------------------------------------------------------------------------
// Name: DisplayError
// Desc: Takes a function and error code and displays them.
//----------------------------------------------------------------------------
void DisplayError(char* funcName, HRESULT hresult)
{
	char errorString[MAXBUFSIZE];
	DmTranslateError(hresult, errorString, MAXBUFSIZE);

	printf("\n%s Error 0x%X:\n%s\n", funcName, hresult, errorString);

	return;
}

//----------------------------------------------------------------------------
// Name: WaitForControllerIdle
// Desc: Waits for the controller to become idle again.
//----------------------------------------------------------------------------
void WaitForControllerIdle()
{
	if (DISPLAY_OUTPUT)
		printf("Waiting for controller to become idle.\n");

	XSIM_COMPONENTSTATUS playerStatus = XSIM_COMPONENTSTATUS_RUNNING;
	while (playerStatus != XSIM_COMPONENTSTATUS_IDLE)
	{
		XSimGetPortPlayerStatus(0, &playerStatus);
		Sleep(100);
	}
}

//----------------------------------------------------------------------------
// Name: ShortTest
// Desc: Runs a short test.
//----------------------------------------------------------------------------
void Mode0(MapList* mapList, char* mapPathBuffer, int mapPathBufferSize)
{
	char launchCommand[MAXBUFSIZE];
	const int mapCount = mapList->GetMapCount();
	const char* mapPath = NULL;
	for (int i = 0; i < mapCount; i++)
	{
		loadNextLevel = false;

		// Send the command to load another level
		strcpy_s(launchCommand, "GameLauncher.exe -nowatson -platform xbox360 -options \"-preview ");
		mapPath = mapList->GetMapPath(i);

		char filePath[MAXBUFSIZE];
		size_t requiredSize;
		getenv_s(&requiredSize, filePath, MAXBUFSIZE, "SigCurrentProject");
		strcat_s(filePath, "\\res");
		strcat_s(filePath, mapPath);
		if ( !FileExists(filePath) )
		{
			printf("!WARNING! File not found: %s\n", filePath);
			continue;
		}

		strcpy_s(mapPathBuffer, mapPathBufferSize, mapPath);
		strcat_s(launchCommand, mapPath);
		strcat_s(launchCommand, "\"");

		if (DISPLAY_OUTPUT)
			printf( "Launching new map: %s...\n",  mapPath);

		if ( system(launchCommand) )
			break;

		// Wait until the console tells us it's OK to proceed
		int sleepSeconds = 180;
		while (!loadNextLevel && !gameCrashEncountered)
		{
			Sleep(1000);

			sleepSeconds--;
			if (sleepSeconds <= 0)
			{
				printf("!WARNING! The message to continue to the next level was never received while testing %s.\n", mapPath);
				gameCrashEncountered = true;
			}
		}

		if (gameCrashEncountered)
			break;

		int inputResult = NULL;
		if (i == 0)
		{
			// Get the first level started and wait for it to load
			// and possibly sign into Xbox LIVE
			inputResult = GiveScriptedInput("S,A,A,A,A");
		}

		// If there was an issue with the input simulator,
		// just exit with an error
		if (inputResult != 0)
		{
			inputErrorEncountered = TRUE;
			break;
		}
	}

	return;
}

//----------------------------------------------------------------------------
// Name: LongTest
// Desc: Runs a longer test.
//----------------------------------------------------------------------------
void Mode1(MapList* mapList, char* mapPathBuffer, int mapPathBufferSize)
{
	char launchCommand[MAXBUFSIZE];
	const int mapCount = mapList->GetMapCount();
	const char* mapPath = NULL;
	for (int i = 0; i < mapCount; i++)
	{
		loadNextLevel = false;

		// Send the command to load another level
		strcpy_s(launchCommand, "GameLauncher.exe -nowatson -platform xbox360 -options \"-preview ");
		mapPath = mapList->GetMapPath(i);

		char filePath[MAXBUFSIZE];
		size_t requiredSize;
		getenv_s(&requiredSize, filePath, MAXBUFSIZE, "SigCurrentProject");
		strcat_s(filePath, "\\res");
		strcat_s(filePath, mapPath);
		if ( !FileExists(filePath) )
		{
			printf("!WARNING! File not found: %s\n", filePath);
			continue;
		}

		strcpy_s(mapPathBuffer, mapPathBufferSize, mapPath);
		strcat_s(launchCommand, mapPath);
		strcat_s(launchCommand, "\"");

		if (DISPLAY_OUTPUT)
			printf( "Launching new map: %s...\n",  mapPath);

		if ( system(launchCommand) )
			break;

		// Wait until the console tells us it's OK to proceed
		int sleepSeconds = 120;
		while (!loadNextLevel && !gameCrashEncountered)
		{
			Sleep(1000);

			sleepSeconds--;
			if (sleepSeconds <= 0)
			{
				printf("!WARNING! The message to continue to the next level was never received while testing %s.\n", mapPath);
				gameCrashEncountered = true;
			}
		}

		if (gameCrashEncountered)
			break;

		int inputResult = NULL;
		if (i == 0)
		{
			// Get the first level started and wait for it to load
			// and possibly sign into Xbox LIVE
			inputResult = GiveScriptedInput("S,A,A,A,A");
		}
		else
		{
			// Try to maximize the chance of deploying a unit
			// by disabling almost everything but the left
			// joystick, A and B buttons
			vector<CONTROLLER_BUTTONS> disabledButtons;
			disabledButtons.push_back(DPADUP);
			disabledButtons.push_back(DPADDOWN);
			disabledButtons.push_back(DPADLEFT);
			disabledButtons.push_back(DPADRIGHT);
			disabledButtons.push_back(START);
			disabledButtons.push_back(BACK);
			disabledButtons.push_back(RIGHTTHUMB);
			disabledButtons.push_back(LEFTSHOULDER);
			disabledButtons.push_back(RIGHTSHOULDER);
			disabledButtons.push_back(X);
			disabledButtons.push_back(Y);
			disabledButtons.push_back(XBOX360BUTTON);
			disabledButtons.push_back(LEFTTRIGGER);
			inputResult = DoRandomInput(180, disabledButtons);
		}

		// If there was an issue with the input simulator,
		// just exit with an error
		if (inputResult != 0)
		{
			inputErrorEncountered = TRUE;
			break;
		}
	}

	return;
}

//----------------------------------------------------------------------------
// Name: SelectRandomUnit
// Desc: Picks a unit at random.
//----------------------------------------------------------------------------
void SelectRandomUnit()
{
	srand ( (unsigned int) time(NULL) );

	char command[MAXBUFSIZE];
	strcpy_s(command, "RT100");

	for (int j = 0; j < 2; j++)
	{
		char* stickDir;
		int dirNum = rand() % 4;
		switch (dirNum)
		{
		case 0:
			stickDir = "U";
			break;
		case 1:
			stickDir = "D";
			break;
		case 2:
			stickDir = "L";
			break;
		case 3:
			stickDir = "R";
			break;
		default:
			printf("!WARNING! Error generating stick directional.\n");
			break;
		}
		strcat_s(command, "+J");
		strcat_s(command, stickDir);
		strcat_s(command, "65535");
	}
	
	int inputResult = GiveScriptedInput(command);
}

//----------------------------------------------------------------------------
// Name: DebugStrEvtHandler
// Desc: Handles events from the console.
//----------------------------------------------------------------------------
DWORD DebugEvtHandler(ULONG dwNotification, DWORD dwParam)
{
	if( gameCrashEncountered )
		return 0;

	DMN_BREAK breakInfo;
	DMN_DEBUGSTR debugStr;
	DMN_MODLOAD modLoad;
	DMN_EXCEPTION exceptionInfo;

	HRESULT hr;
	PDM_THREADSTOP pdm_threadStop = NULL;

	switch(dwNotification)
	{
	case DM_BREAK:
		breakInfo = *((DMN_BREAK*) dwParam);
		printf("Thread %s caused a break at address %s.\n", breakInfo.ThreadId, breakInfo.Address);

		break;
	case DM_DEBUGSTR:

		debugStr = *((DMN_DEBUGSTR*) dwParam);

		// The strings from the console aren't null terminated!
		if(debugStr.Length > 0)
		{
			memcpy_s(lastDmMessage, MAXBUFSIZE, debugStr.String, debugStr.Length);
			lastDmMessage[debugStr.Length] = '\0';
		}

		// Filter messages without @ prepended
		if(debugStr.Length > 0 && debugStr.String[0] == '@')
		{
			if( strstr(lastDmMessage, "@ERROR@") )
				gameErrorStarted = true;
			if( gameErrorStarted && strstr(lastDmMessage, "@END@") )
				gameCrashEncountered = true;
			else if ( strcmp(lastDmMessage, "@LEVEL_START_SUCCESS@\n") == 0 )
				loadNextLevel = true;
		}

		if( gameErrorStarted )
		{
			if( !strstr( gameErrorMessage, lastDmMessage ) )
			{
				if( !strstr( lastDmMessage, "!WARNING!" ) )
					strcat_s( gameErrorMessage, "!WARNING! " );
				strcat_s( gameErrorMessage, lastDmMessage );
			}
		}

		// When bad things start happening, we can find ourselves
		// with a stopped thread - if we don't do this, we'll just
		// be stuck doing nothing forever
		hr = DmIsThreadStopped(debugStr.ThreadId, pdm_threadStop);
		if (hr != XBDM_NOTSTOPPED)
		{
			DmContinueThread(debugStr.ThreadId, TRUE);
			DmGo();
		}

		break;
	case DM_EXEC:
		break;
	case DM_SINGLESTEP:
		break;
	case DM_MODLOAD:
		modLoad = *((DMN_MODLOAD*) dwParam);

		break;
	case DM_MODUNLOAD:
		modLoad = *((DMN_MODLOAD*) dwParam);

		break;
	case DM_CREATETHREAD:
		break;
	case DM_DESTROYTHREAD:
		break;
	case DM_EXCEPTION:
		exceptionInfo = *((DMN_EXCEPTION*) dwParam);

		gameCrashEncountered = TRUE;

		if (DISPLAY_OUTPUT)
			printf("DM_EXCEPTION\n");

		break;
	case DM_ASSERT:
		gameCrashEncountered = TRUE;

		if (DISPLAY_OUTPUT)
			printf("DM_ASSERT\n");
		break;
	case DM_DATABREAK:
		if (DISPLAY_OUTPUT)
			printf("DM_DATABREAK\n");

		break;
	case DM_RIP:
		break;
	case DM_SECTIONLOAD:
		break;
	case DM_SECTIONUNLOAD:
		break;
	case DM_ASSERTION_FAILURE:
		if (DISPLAY_OUTPUT)
			printf("DM_ASSERTION_FAILURE\n");

		break;
	}

	return 0;
}

//----------------------------------------------------------------------------
// Name: FileExists
// Desc: Checks whether the given file exists or not.
//----------------------------------------------------------------------------
bool FileExists(const char* filename)
{
	struct stat fileInfo;
	return ( stat(filename, &fileInfo) ?  false : true);
}