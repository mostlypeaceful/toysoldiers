#include "BasePch.hpp"
#include "ToolsPaths.hpp"
#ifdef __ToolsPaths__
#include "StringUtil.hpp"
#include "FileSystem.hpp"
#include "Win32Util.hpp"
#include "Threads/tProcess.hpp"
#include "tPlatform.hpp"

namespace Sig { namespace ToolsPaths
{
	///
	/// \section Global private functions and data.
	///

	namespace
	{
#		define signal_registry_key_name							"Software\\SignalStudios"


		const char cUserEnvironmentVarsRegKeyName[]				= "Environment";
		const char cUserSoftwarePerforceRegKeyName[]			= "Software\\Perforce\\Environment";
		const char cSystemEnvironmentVarsRegKeyName[]			= "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment";
		const char cProjectProfileListRegKeyName[]				= signal_registry_key_name "\\ProjectProfiles";
		const char cProjectProfileWorkSpaceOverrideRegKeyName[]	= "WorkSpaceOverride";
		const char cProjectProfileEnginePathRegKeyName[]		= "EnginePath";
		const char cProjectProfileProjectFilePathRegKeyName[]	= "ProjectPath";
        const char cProjectProfileDeployToPathRegKeyName[]      = "DeployTo";
		const char cProjectProfilePerforcePortRegKeyName[]		= "PerforcePort";
		const char cProjectProfileActiveProfileRegKeyName[]		= "ActiveProfile";
		const char cProjectProfileSyncChangelistRegKeyName[]	= "SyncChangelist";

		const char cEngineEnvVarName[]							= "SigEngine";
		const char cCurrentProjectPathEnvVarName[]				= "SigCurrentProject";
		const char cCurrentProjectNameEnvVarName[]				= "SigCurrentProjectName";
        const char cDeployToDirEnvVarName[]                      = "SigDeployTo";
		const char cSigPathEnvVarName[]							= "SigPath";
		const char cSigP4User[]									= "P4USER";
		const char cSigP4Client[]								= "P4CLIENT";
		const char cSigP4Port[]									= "P4PORT";
		const char cPathEnvVarName[]							= "Path";
		const char cMayaPlugInPathEnvVarName[]					= "MAYA_PLUG_IN_PATH";
		const char cMayaScriptPathEnvVarName[]					= "MAYA_SCRIPT_PATH";
		const char cMayaPythonPathEnvVarName[]					= "PYTHONPATH";
		const char cMayaIconPathEnvVarName[]					= "XBMLANGPATH";
		const char cXDKEnvVarName[]								= "XEDK";

		const char cEngineBinFolderName[]						= "Bin";
		const char cEngineTempFolderName[]						= "Tmp";
		const char cEngineDataFolderName[]						= "Data";
		const char cGameFolderName[]							= "Game";
		const char cMayaScriptFolderName[]						= "MayaScripts";
		const char cMayaIconsFolderName[]						= "icons";

		define_static_function( fEnsureFoldersCreated )
		{
			tFilePathPtr rootFolder = fGetEngineRootFolder( );
			if( rootFolder.fNull( ) )
				return; // don't do anything if the engine folder doesn't exist

			tFilePathPtr binFolder = fGetEngineBinFolder( );
			tFilePathPtr tmpFolder = fGetEngineTempFolder( );

			FileSystem::fCreateDirectory( rootFolder );
			FileSystem::fCreateDirectory( binFolder );
			FileSystem::fCreateDirectory( tmpFolder );
		}


		/// 
		/// When set, it will automatically check out any files that try to call fPromptToCheckout.
		static b32 gAutoCheckOut = false;
	}

	///
	/// \section Global public functions.
	///

	void fConfigureEngineEnvironmentVariables( const tFilePathPtr& enginePath )
	{
		// get a handle to the user and sysem environment variables registry keys
		HKEY hKeyUser = Win32Util::fGetRegistryKeyCurrentUser( cUserEnvironmentVarsRegKeyName );

		// set the root engine path environment variable
		Win32Util::fSetRegistryKeyValue( hKeyUser, enginePath.fCStr( ), cEngineEnvVarName );

		// set the maya plugin folder path
		const tFilePathPtr binPath = tFilePathPtr::fConstructPath( enginePath, tFilePathPtr( cEngineBinFolderName ) );
		Win32Util::fSetRegistryKeyValue( hKeyUser, binPath.fCStr( ), cMayaPlugInPathEnvVarName );
		const tFilePathPtr mayaScriptPath = tFilePathPtr::fConstructPath( binPath, tFilePathPtr( cMayaScriptFolderName ) );
		Win32Util::fSetRegistryKeyValue( hKeyUser, mayaScriptPath.fCStr( ), cMayaScriptPathEnvVarName );
		Win32Util::fSetRegistryKeyValue( hKeyUser, mayaScriptPath.fCStr( ), cMayaPythonPathEnvVarName );
		const tFilePathPtr mayaIconPath = tFilePathPtr::fConstructPath( mayaScriptPath, tFilePathPtr( cMayaIconsFolderName ) );
		Win32Util::fSetRegistryKeyValue( hKeyUser, mayaIconPath.fCStr( ), cMayaIconPathEnvVarName );

		// set xdk path
		const std::string xedkPath = "%SigEngine%\\Src\\External\\Microsoft Xbox 360 SDK";
		Win32Util::fSetRegistryKeyValue( hKeyUser, xedkPath.c_str( ), cXDKEnvVarName );

		// get current path value
		std::string pathEnvVarValue;
		Win32Util::fGetRegistryKeyValue( hKeyUser, pathEnvVarValue, cPathEnvVarName );

		// insert %SigPath% if its not already there
		if( !StringUtil::fStrStrI( pathEnvVarValue.c_str( ), cSigPathEnvVarName ) )
		{
			// set the new path variable value with windows
			pathEnvVarValue = std::string( "%" ) + cSigPathEnvVarName + "%;" + pathEnvVarValue;
			Win32Util::fSetRegistryKeyValue( hKeyUser, pathEnvVarValue.c_str( ), cPathEnvVarName );
		}

		// now set our custom SigPath environment variable
		std::string sigPathEnvVarVAlue = std::string( binPath.fCStr( ) ) + ";" 
			+ std::string( binPath.fCStr( ) ) + "\\external\\Maya;"
			;
		Win32Util::fSetRegistryKeyValue( hKeyUser, sigPathEnvVarVAlue.c_str( ), cSigPathEnvVarName );

		// we're done setting registry values, close the key
		Win32Util::fCloseRegistryKey( hKeyUser );

		// update the system
		Win32Util::fBroadcastEnvVarsChanged( );
	}

	tFilePathPtr fGetEngineRootFolder( )
	{
		// get a handle to the user environment variables registry key
		HKEY hKey = Win32Util::fGetRegistryKeyCurrentUser( cUserEnvironmentVarsRegKeyName );
		if( !hKey )
			return tFilePathPtr( );

		// get the root engine path environment variable
		std::string engineRoot("");
		Win32Util::fGetRegistryKeyValue( hKey, engineRoot, cEngineEnvVarName );

		// close the registry key
		Win32Util::fCloseRegistryKey( hKey );

		return tFilePathPtr( engineRoot.c_str( ) );
	}

	tFilePathPtr fGetEngineBinFolder( )
	{
		tFilePathPtr engineRoot = fGetEngineRootFolder( );
		if( engineRoot.fExists( ) )
			return tFilePathPtr::fConstructPath( engineRoot, tFilePathPtr( cEngineBinFolderName ) );

		return engineRoot;
	}

	tFilePathPtr fGetEngineTempFolder( )
	{
		tFilePathPtr engineRoot = fGetEngineRootFolder( );
		if( engineRoot.fExists( ) )
			return tFilePathPtr::fConstructPath( engineRoot, tFilePathPtr( cEngineTempFolderName ) );

		return engineRoot;
	}

	tFilePathPtr fGetEngineDataFolder( )
	{
		return tFilePathPtr::fConstructPath( fGetEngineRootFolder( ), tFilePathPtr( cEngineDataFolderName ) );		
	}

	tFilePathPtr fCreateTempEngineFilePath( const char* extension, const tFilePathPtr& subFolder, const char* explicitName )
	{
		tFilePathPtr path = fGetEngineTempFolder( );
		if( !subFolder.fNull( ) )
			path = tFilePathPtr::fConstructPath( path, subFolder );

		std::stringstream s;
		if( explicitName )
			s << explicitName;
		else
			s << Time::fGetStamp( );
		if( extension )
		{
			if( *extension == '.' )
				s << extension;
			else
				s << '.' << extension;
		}

		FileSystem::fCreateDirectory( path );

		return tFilePathPtr::fConstructPath( path, tFilePathPtr( s.str( ).c_str( ) ) );
	}

	tFilePathPtr fMakeEngineDataAbsolute( const char * relativePath )
	{
		return tFilePathPtr::fConstructPath( fGetEngineDataFolder( ), tFilePathPtr( relativePath ) );
	}

    void fSetCurrentProjectDeployTo( const std::string& name )
    {
        HKEY hKey = Win32Util::fGetRegistryKeyCurrentUser( cUserEnvironmentVarsRegKeyName );
        if( hKey )
        {
            Win32Util::fSetRegistryKeyValue( hKey, name.c_str( ), cDeployToDirEnvVarName );
        }

        // close the registry key
        Win32Util::fCloseRegistryKey( hKey );

        // update the system
        Win32Util::fBroadcastEnvVarsChanged( );
    }

	void fSetCurrentProjectRootFolder( const std::string& name, const tFilePathPtr& path )
	{
		HKEY hKey = Win32Util::fGetRegistryKeyCurrentUser( cUserEnvironmentVarsRegKeyName );
		if( hKey )
		{
			Win32Util::fSetRegistryKeyValue( hKey, name.c_str( ), cCurrentProjectNameEnvVarName );
			Win32Util::fSetRegistryKeyValue( hKey, path.fCStr( ), cCurrentProjectPathEnvVarName );
		}

		// close the registry key
		Win32Util::fCloseRegistryKey( hKey );

		// update the system
		Win32Util::fBroadcastEnvVarsChanged( );
	}
	std::string	fGetCurrentProjectName( )
	{
		std::string value("");

		HKEY hKey = Win32Util::fGetRegistryKeyCurrentUser( cUserEnvironmentVarsRegKeyName );
		if( !hKey )
			return value;

		Win32Util::fGetRegistryKeyValue( hKey, value, cCurrentProjectNameEnvVarName );

		// close the registry key
		Win32Util::fCloseRegistryKey( hKey );

		return value;
	}

    std::string fGetCurrentProjectDeployTo( )
    {
        std::string value("");

        HKEY hKey = Win32Util::fGetRegistryKeyCurrentUser( cUserEnvironmentVarsRegKeyName );
        if( !hKey )
            return fGetCurrentProjectName( );

        Win32Util::fGetRegistryKeyValue( hKey, value, cDeployToDirEnvVarName );

        // close the registry key
        Win32Util::fCloseRegistryKey( hKey );

        return value;
    }

	tFilePathPtr fGetCurrentProjectRootFolder( )
	{
		std::string value("");

		HKEY hKey = Win32Util::fGetRegistryKeyCurrentUser( cUserEnvironmentVarsRegKeyName );
		if( !hKey )
			return tFilePathPtr( value.c_str( ) );

		Win32Util::fGetRegistryKeyValue( hKey, value, cCurrentProjectPathEnvVarName );

		// close the registry key
		Win32Util::fCloseRegistryKey( hKey );

		return tFilePathPtr( value.c_str( ) );
	}

	tFilePathPtr fGetCurrentProjectResFolder( )
	{
		return tFilePathPtr::fConstructPath( fGetCurrentProjectRootFolder( ), tFilePathPtr( "Res" ) );
	}

	tFilePathPtr fGetCurrentProjectSrcFolder( )
	{
		return tFilePathPtr::fConstructPath( fGetCurrentProjectRootFolder( ), tFilePathPtr( "Src" ) );
	}

	tFilePathPtr fGetCurrentProjectGameFolder( )
	{
		return tFilePathPtr::fConstructPath( fGetCurrentProjectRootFolder( ), tFilePathPtr( "Game" ) );
	}

	tFilePathPtr fGetCurrentProjectGamePlatformFolder( tPlatformId pid )
	{
		return tFilePathPtr::fConstructPath( fGetCurrentProjectGameFolder( ), tFilePathPtr( fPlatformIdString( pid ) ) );
	}

	tFilePathPtr fGetCurrentProjectFilePath( )
	{
		return tFilePathPtr::fConstructPath( fGetCurrentProjectResFolder( ), tFilePathPtr( "Project.xml" ) );
	}

	tFilePathPtr fGetGameRootPathFromProjectPath( const tFilePathPtr& path )
	{
		const char* platformFolder = fPlatformIdString( cCurrentPlatform );
		sigassert( platformFolder );

		tFixedArray<tFilePathPtr,3> pathFragments;
		pathFragments[0] = path;
		pathFragments[1] = tFilePathPtr( cGameFolderName );
		pathFragments[2] = tFilePathPtr( platformFolder );

		return tFilePathPtr::fConstructPath( pathFragments.fBegin( ), pathFragments.fCount( ) );
	}

	std::string fGetCurrentP4User( )
	{
		std::string value("");

		HKEY hKey = Win32Util::fGetRegistryKeyCurrentUser( cUserSoftwarePerforceRegKeyName );
		if( !hKey )
			return value;

		Win32Util::fGetRegistryKeyValue( hKey, value, cSigP4User );

		// close the registry key
		Win32Util::fCloseRegistryKey( hKey );

		return value;
	}
	void fSetCurrentP4ClientWorkspaceFromProject( const std::string& projectName )
	{
		const std::string userName = fGetCurrentP4User( );
		if( userName.length( ) == 0 )
		{
			log_warning( "Current p4 user not specified - aborting ToolsPaths::fSetCurrentP4ClientWorkspace" );
			return;
		}
		if( projectName.length( ) == 0 )
		{
			log_warning( "Current project not specified - aborting ToolsPaths::fSetCurrentP4ClientWorkspace" );
			return;
		}

		std::string clientWorkspaceName = userName + "_" + projectName;
		std::transform( clientWorkspaceName.begin( ), clientWorkspaceName.end( ), clientWorkspaceName.begin( ), tolower );
		fSetCurrentP4ClientWorkspaceName( clientWorkspaceName );
	}
	void fSetCurrentP4ClientWorkspaceName( const std::string& workspaceName )
	{
		HKEY hKey = Win32Util::fGetRegistryKeyCurrentUser( cUserSoftwarePerforceRegKeyName );
		if( !hKey )
			return;

		Win32Util::fSetRegistryKeyValue( hKey, workspaceName, cSigP4Client );

		// close the registry key
		Win32Util::fCloseRegistryKey( hKey );
	}

	void fSetCurrentP4Port( const std::string& port )
	{
		HKEY hKey = Win32Util::fGetRegistryKeyCurrentUser( cUserSoftwarePerforceRegKeyName );
		if( !hKey )
			return;

		Win32Util::fSetRegistryKeyValue( hKey, port, cSigP4Port );

		Win32Util::fCloseRegistryKey( hKey );
	}

	namespace
	{
		const char* fFindInPath( const char* path, u32& oLen, const char* v0, const char* v1 )
		{
			oLen = 0;
			const char* find = 0;
			if( !find ) // try first variant
			{
				find = StringUtil::fStrStrI( path, v0 );
				if( find )
				{
					oLen = ( u32 )strlen( v0 );
				}
			}
			if( !find ) // try second variant
			{
				find = StringUtil::fStrStrI( path, v1 );
				if( find )
				{
					oLen = ( u32 )strlen( v1 );
					if( find[ oLen ] != '\0' )
						find = 0;
				}
			}
			return find;
		}
		const char* fFindResInPath( const char* path, u32& resDirLen )
		{
			const char platformSlash = fPlatformFilePathSlash( cCurrentPlatform );
			const char resDir0[] = { platformSlash, 'r', 'e', 's', platformSlash, '\0' };
			const char resDir1[] = { platformSlash, 'r', 'e', 's', '\0' };
			return fFindInPath( path, resDirLen, resDir0, resDir1 );
		}
		const char* fFindGameInPath( const char* path, tPlatformId pid, u32& gameDirLen )
		{
			const char platformSlash = fPlatformFilePathSlash( cCurrentPlatform );

			std::string gameDir1;
			gameDir1 += platformSlash;
			gameDir1 += "game";
			gameDir1 += platformSlash;
			gameDir1 += fPlatformIdString( pid );

			std::string gameDir0 = gameDir1;
			gameDir0 += platformSlash;

			return fFindInPath( path, gameDirLen, gameDir0.c_str( ), gameDir1.c_str( ) );
		}
	}

	b32	fIsUnderSourceFolder( const tFilePathPtr& path )
	{
		const char pathSlash = fPlatformFilePathSlash( cCurrentPlatform );
		const char ignoreString0[] = { pathSlash, '~', '\0' };
		const char ignoreString1[] = { pathSlash, '.', '\0' };
		if( StringUtil::fStrStrI( path.fCStr( ), ignoreString0 ) ||
			StringUtil::fStrStrI( path.fCStr( ), ignoreString1 ) )
		{
			return true;
		}

		return false;
	}

	b32	fIsUnderDevFolder( const tFilePathPtr& path )
	{
		const char pathSlash = fPlatformFilePathSlash( cCurrentPlatform );
		const char ignoreString0[] = { pathSlash, '_', '\0' };
		if( StringUtil::fStrStrI( path.fCStr( ), ignoreString0 ) )
		{
			return true;
		}

		return false;
	}

	tFilePathPtr fMakeResRelative( const tFilePathPtr& path, b32 returnNullIfNotRelative, tPlatformId pid )
	{
		u32 resDirLen = 0;
		const char* res = fFindResInPath( path.fCStr( ), resDirLen );
		if( !res ) // failure, might already by relative
			return returnNullIfNotRelative ? tFilePathPtr( ) : path;

		std::string pathRelativeToRes = res + resDirLen;
		return tFilePathPtr( pathRelativeToRes.c_str( ), pid );
	}

	tFilePathPtr fMakeResAbsolute( const tFilePathPtr& path, tPlatformId pid )
	{
		const tFilePathPtr pathToRes = fGetCurrentProjectResFolder( );

		// check if the path is already absolute
		const char* pathToResFound = StringUtil::fStrStrI( path.fCStr( ), pathToRes.fCStr( ) );
		if( pathToResFound == path.fCStr( ) )
			return path;

		return tFilePathPtr::fConstructPath( pathToRes, fMakeResRelative( path ), pid );
	}

	//------------------------------------------------------------------------------
	tFilePathPtr fValidateResPath ( 
		const tFilePathPtr & path,
		std::string * errorString,
		b32 returnRelativePath,
		tPlatformId pid )
	{
		if( path.fNull( ) || !path.fLength( ) )
		{
			if( errorString )
				*errorString = "File not specified";
			return tFilePathPtr( );
		}

		tFilePathPtr resRelative = fMakeResRelative( path, true, pid );
		if( resRelative.fNull( ) )
		{
			if( errorString )
				*errorString = "File is not relative to res folder";
			return tFilePathPtr( );
		}

		if( !FileSystem::fFileExists( fMakeResAbsolute( resRelative, pid ) ) )
		{
			if( errorString )
				*errorString = "File does not exist or isn't under current project";
			return tFilePathPtr( );
		}

		return returnRelativePath ? resRelative : path;
	}


	tFilePathPtr fMakeGameRelative( const tFilePathPtr& path, tPlatformId gamePid, b32 returnNullIfNotRelative, tPlatformId pid )
	{
		u32 gameDirLen = 0;
		const char* game = fFindGameInPath( path.fCStr( ), gamePid, gameDirLen );
		if( !game ) // failure, might already by relative
			return returnNullIfNotRelative ? tFilePathPtr( ) : path;

		std::string pathRelativeToGame = game + gameDirLen;
		return tFilePathPtr( pathRelativeToGame.c_str( ), pid );
	}

	tFilePathPtr fMakeGameAbsolute( const tFilePathPtr& path, tPlatformId pid )
	{
		const tFilePathPtr game = fGetCurrentProjectGamePlatformFolder( pid );
		const tFilePathPtr absolute = tFilePathPtr::fConstructPath( game, path );
		return absolute;
	}

	//------------------------------------------------------------------------------
	tFilePathPtr fValidateGamePath ( 
		const tFilePathPtr & path,
		tPlatformId gamePid,
		std::string * errorString,
		b32 returnRelativePath,
		tPlatformId pid )
	{
		if( path.fNull( ) || !path.fLength( ) )
		{
			if( errorString )
				*errorString = "File not specified";
			return tFilePathPtr( );
		}

		tFilePathPtr gameRelative = fMakeGameRelative( path, gamePid, true, pid  );
		if( gameRelative.fNull( ) )
		{
			if( errorString )
				*errorString = "File is not relative to game folder";
			return tFilePathPtr( );
		}

		if( !FileSystem::fFileExists( fMakeGameAbsolute( gameRelative, pid ) ) )
		{
			if( errorString )
				*errorString = "File does not exist or isn't under current project";
			return tFilePathPtr( );
		}

		return returnRelativePath ? gameRelative : path;
	}

	void fDeleteGameFileForAllPlatforms( const tFilePathPtr& gameRelativePath )
	{
		for( tPlatformIdIterator pid; !pid.fDone( ); pid.fNext( ) )
		{
			const tFilePathPtr gameAbsPath = fMakeGameAbsolute( gameRelativePath, pid );
			//log_line( Log::cFlagResource, "Deleting game file " << gameAbsPath );
			FileSystem::fDeleteFile( gameAbsPath );
		}
	}

	std::string	fGetSignalRegistryKeyName( )
	{
		return signal_registry_key_name;
	}

	
	void fLaunchGame( tPlatformId platform, const std::string& gameOptions )
	{
		std::stringstream ss;
		ss << "-min -platform " << fPlatformIdString( platform );
		if( gameOptions.length( ) > 0 )
			ss << " -options \"" << gameOptions << "\"";
		const std::string cmdLine = ss.str( );

		const tFilePathPtr exePath = tFilePathPtr::fConstructPath( fGetEngineBinFolder( ), tFilePathPtr( "GameLauncher.exe" ) );

		if( !Threads::tProcess::fSpawnAndForget( exePath.fCStr( ), cmdLine.c_str( ) ) )
			log_warning( "Error spawning game launcher process" );
	}

	void fLaunchSigScript( const std::string& commandLine )
	{
		const char* app =
#ifdef _DEBUG
			"\\SigScriptd.exe";
#else
			"\\SigScript.exe";
#endif

		const std::string appPath = std::string( ToolsPaths::fGetEngineBinFolder( ).fCStr( ) ) + app;

		// wxWidgets expects the first element of the command line be the executable path for some reason.
		std::stringstream cmdLine;
		cmdLine << "\"" << appPath << "\" " << commandLine.c_str( );

		Threads::tProcess::fSpawnAndForget( appPath.c_str( ), cmdLine.str( ).c_str( ) );
	}

	void fLaunchSigEd( const std::string& commandLine )
	{
		const char* app =
#ifdef _DEBUG
			"\\SigEdd.exe";
#else
			"\\SigEd.exe";
#endif

		const std::string appPath = std::string( ToolsPaths::fGetEngineBinFolder( ).fCStr( ) ) + app;

		// not a widgets app, dont prepend app path.

		Threads::tProcess::fSpawnAndForget( appPath.c_str( ), commandLine.c_str( ) );
	}

#if !defined( platform_metro ) // MessageBox is banned.
	b32 fPromptToCheckout( const tFilePathPtr& path )
	{
		// For batch check out of multiple files.
		if( gAutoCheckOut )
		{
			fCheckout( path );
			return true;
		}

		std::stringstream msgTextSS; msgTextSS << "The file [" << path.fCStr( ) << "] is read-only and cannot be saved.\nOpen the file for edits?";
		const std::string msgText = msgTextSS.str( );

		const int result = MessageBox( NULL, msgText.c_str( ), "Open for edit?", MB_ICONQUESTION | MB_OKCANCEL | MB_APPLMODAL );
		if( result == IDOK )
		{
			fCheckout( path );
			return true;
		}

		return false;
	}

	b32 fCheckout( const tFilePathPtr& path, b32 msgBoxOnError )
	{
		const tFilePathPtr resRelativePath = fMakeResRelative( path );
		log_line( 0, "Opening file " << resRelativePath << " for edit." );
		const tFilePathPtr openForEditPath = tFilePathPtr::fConstructPath( fGetEngineBinFolder( ), tFilePathPtr( "OpenForEdit.cmd" ) );
		std::stringstream systemStringSS; systemStringSS << "\"" << openForEditPath.fCStr( ) << "\" " << path.fCStr( );
		const std::string systemString = systemStringSS.str( );
		const int result = system( systemString.c_str( ) );

		if( Win32Util::fIsFileReadOnly( path ) )
		{
			if( msgBoxOnError )
			{
				std::stringstream ss; ss << "Couldn't check out file: " << resRelativePath << ". Ask for help.";
				std::string s = ss.str( );
				MessageBox( NULL, s.c_str( ), "Source Control Error", MB_ICONEXCLAMATION | MB_OK | MB_APPLMODAL );
			}
			return false;
		}

		return true;
	}
#endif

	b32 fCheckoutAdd( const tFilePathPtr& path, b32 msgBoxOnError )
	{
		//NOTE: both checkout and AddToSourceControl will ALWAYS return 0 (aka success)
		//  so: "p4 add someFileAlreadyInP4"   will return 0
		// and: "p4 edit someFileNotInP4"      will return 0
		// WTF Perforce!

		//try edit
		fCheckout( path, false );

		//try add
		const tFilePathPtr openForEditPath = tFilePathPtr::fConstructPath( fGetEngineBinFolder( ), tFilePathPtr( "AddToSourceControl.cmd" ) );
		std::stringstream systemStringSS; systemStringSS << "\"" << openForEditPath.fCStr( ) << "\" " << path.fCStr( );
		const std::string cmdLine = systemStringSS.str( );
		std::string output;
		Win32Util::fLaunchProcessAndCaptureOutput( cmdLine, output );

		//if it is still read only then someone probably has a lock on the file
		if( Win32Util::fIsFileReadOnly( path ) )
		{
			if( msgBoxOnError )
			{
				const tFilePathPtr resRelativePath = fMakeResRelative( path );
				std::stringstream ss; ss << "Couldn't check out or add file: " << resRelativePath << ". Ask for help.";
				std::string s = ss.str( );
				MessageBox( NULL, s.c_str( ), "Source Control Error", MB_ICONEXCLAMATION | MB_OK | MB_APPLMODAL );
			}
			return false;
		}

		return true;
	}

	b32 fIsOutOfDate ( const tFilePathPtr& path )
	{
		const tFilePathPtr resRelativePath = fMakeResRelative( path );
		log_line( 0, "Checking file " << resRelativePath << " for newer revisions." );

		const tFilePathPtr checkForNewRevisionsPath = tFilePathPtr::fConstructPath( fGetEngineBinFolder( ), tFilePathPtr( "CheckForNewRevisions.vbs" ) );
		std::stringstream systemStringSS; systemStringSS << "\"" << checkForNewRevisionsPath.fCStr( ) << "\" " << path.fCStr( );
		const std::string systemString = systemStringSS.str( );
		const int result = system( systemString.c_str( ) );
		if (result)
			return true;

		return false;
	}

	b32 fIsCheckedOut ( const tFilePathPtr& path, std::string& returnedUserList )
	{
		const tFilePathPtr resRelativePath = fMakeResRelative( path );
		log_line( 0, "Checking file " << resRelativePath << " to see if it is checked out." );

		// Construct the path to the script
		const tFilePathPtr checkForNewRevisionsPath = tFilePathPtr::fConstructPath( fGetEngineBinFolder( ), tFilePathPtr( "CheckForCheckouts.vbs" ) );
		std::stringstream commandLine; commandLine << "cscript.exe /Nologo \"" << checkForNewRevisionsPath.fCStr( ) << "\" \"" << path.fCStr( ) << "\"";
		std::string cmdLine = commandLine.str( );

		return Win32Util::fLaunchProcessAndCaptureOutput( cmdLine, returnedUserList );
	}

	void fCheckOutAllSubsequent( b32 autoCheckout )
	{
		gAutoCheckOut = autoCheckout;
	}

	///
	/// \section tProjectProfile
	///

	void tProjectProfile::fLoadProfilesFromRegistry( tGrowableArray<tProjectProfile>& profiles )
	{
		// get the string that contains all the profile names
		std::string allProfilesString;
		HKEY hKey = Win32Util::fGetRegistryKeyCurrentUser( cProjectProfileListRegKeyName );
		Win32Util::fGetRegistryKeyValue( hKey, allProfilesString );

		// parse the string
		tGrowableArray<std::string> profileNames;
		StringUtil::fSplit( profileNames, allProfilesString.c_str( ), ";" );

		// now load all the profiles
		profiles.fSetCount( profileNames.fCount( ) );
		for( u32 i = 0; i < profiles.fCount( ); ++i )
		{
			profiles[i].mProfileName = profileNames[i];
			profiles[i].fLoadFromRegistry( );
		}

		// close the registry key
		Win32Util::fCloseRegistryKey( hKey );
	}

	void tProjectProfile::fSaveProfilesToRegistry( const tGrowableArray<tProjectProfile>& profiles )
	{
		// get the registry key that contains all the profile names
		HKEY hKey = Win32Util::fGetRegistryKeyCurrentUser( cProjectProfileListRegKeyName );

		// generate new key value
		std::string allProfilesString;
		for( u32 i = 0; i < profiles.fCount( ); ++i )
			allProfilesString += profiles[i].mProfileName + ";";

		// set new value back in registry
		Win32Util::fSetRegistryKeyValue( hKey, allProfilesString.c_str( ) );

		// now save each individual profile
		for( u32 i = 0; i < profiles.fCount( ); ++i )
			profiles[i].fSaveToRegistry( );

		// close the registry key
		Win32Util::fCloseRegistryKey( hKey );
	}

	tProjectProfile::tProjectProfile( )
		: mActiveProfile( false )
	{
	}

	tProjectProfile::~tProjectProfile( )
	{
	}

	std::string tProjectProfile::fToString( ) const
	{
		return	std::string( mActiveProfile ? "*" : "" )
			+	mProfileName
			+	std::string("   [ Engine: ") + mEnginePath.fCStr( )
			+	std::string(" ] [ Project: ") + mProjectPath.fCStr( ) + std::string(" ]");
	}

	void tProjectProfile::fLoadFromRegistry( )
	{
		// find my registry key
		HKEY hKey = Win32Util::fGetRegistryKeyCurrentUser( mProfileName.c_str( ), cProjectProfileListRegKeyName );

		// get the values in my registry key
		std::string enginePathStr, projectPathStr;
		Win32Util::fGetRegistryKeyValue( hKey, mWorkSpaceOverride, cProjectProfileWorkSpaceOverrideRegKeyName );
		Win32Util::fGetRegistryKeyValue( hKey, enginePathStr, cProjectProfileEnginePathRegKeyName );
		Win32Util::fGetRegistryKeyValue( hKey, projectPathStr, cProjectProfileProjectFilePathRegKeyName );
        Win32Util::fGetRegistryKeyValue( hKey, mDeployToDir, cProjectProfileDeployToPathRegKeyName );
		Win32Util::fGetRegistryKeyValue( hKey, mPerforcePort, cProjectProfilePerforcePortRegKeyName );

		mEnginePath = tFilePathPtr( enginePathStr.c_str( ) );
		mProjectPath = tFilePathPtr( projectPathStr.c_str( ) );

		if( mDeployToDir.empty( ) || mDeployToDir[0] == 0 )
			mDeployToDir = mProfileName;

		if( mPerforcePort.empty( ) || mPerforcePort[ 0 ] == 0 )
			mPerforcePort = "";

		std::string activeString;
		Win32Util::fGetRegistryKeyValue( hKey, activeString, cProjectProfileActiveProfileRegKeyName );
		if( activeString=="0" )
			mActiveProfile = false;
		else
			mActiveProfile = true;

		std::string syncString;
		Win32Util::fGetRegistryKeyValue( hKey, syncString, cProjectProfileSyncChangelistRegKeyName );
		if( syncString=="1" )
			mSyncChangelist = true;
		else
			mSyncChangelist = false;

		// close the registry key
		Win32Util::fCloseRegistryKey( hKey );
	}

	void tProjectProfile::fSaveToRegistry( ) const
	{
		// find my registry key
		HKEY hKey = Win32Util::fGetRegistryKeyCurrentUser( mProfileName.c_str( ), cProjectProfileListRegKeyName );

		// set the values in my registry key
		Win32Util::fSetRegistryKeyValue( hKey, mWorkSpaceOverride, cProjectProfileWorkSpaceOverrideRegKeyName );
		Win32Util::fSetRegistryKeyValue( hKey, mEnginePath.fCStr( ), cProjectProfileEnginePathRegKeyName );
		Win32Util::fSetRegistryKeyValue( hKey, mProjectPath.fCStr( ), cProjectProfileProjectFilePathRegKeyName );
        Win32Util::fSetRegistryKeyValue( hKey, mDeployToDir, cProjectProfileDeployToPathRegKeyName );
		Win32Util::fSetRegistryKeyValue( hKey, mPerforcePort, cProjectProfilePerforcePortRegKeyName );

		std::string activeString = mActiveProfile ? "1" : "0";
		Win32Util::fSetRegistryKeyValue( hKey, activeString.c_str( ), cProjectProfileActiveProfileRegKeyName );

		std::string flashString = mSyncChangelist ? "1" : "0";
		Win32Util::fSetRegistryKeyValue( hKey, flashString.c_str( ), cProjectProfileSyncChangelistRegKeyName );

		Win32Util::fCloseRegistryKey( hKey );
	}

	void tProjectProfile::fConfigureEnvironment( )
	{
		fConfigureEngineEnvironmentVariables( mEnginePath );
		fSetCurrentProjectRootFolder( mProfileName, mProjectPath );
        fSetCurrentProjectDeployTo( mDeployToDir );

		if( mWorkSpaceOverride.length( ) > 0 )
			fSetCurrentP4ClientWorkspaceName( mWorkSpaceOverride );
		else
			fSetCurrentP4ClientWorkspaceFromProject( mProfileName );

		if( mPerforcePort.length( ) > 0 )
			fSetCurrentP4Port( mPerforcePort );
	}

}}

#endif//__ToolsPaths__

