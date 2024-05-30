#include "BasePch.hpp"
#include "Win32Util.hpp"
#include "tCmdLineOption.hpp"
#include "ToolsPaths.hpp"
#include "Threads/tProcess.hpp"
#include "tStrongPtr.hpp"
#include "tConsoleApp.hpp"
#include "tXbDm.hpp"
using namespace Sig;

namespace
{
	struct tOptions
	{
		std::string mDevkitName;
		b32 mQuiet;
		b32 mMinimal;
		b32 mRelease;
		b32 mProfile;
		b32 mPlaytest;
		b32 mDebug;
		b32 mCopyResourcesOnly;
		b32 mNoWatson;
		tOptions( ) 
			: mQuiet( false )
			, mMinimal( false )
			, mRelease( false )
			, mProfile( false )
			, mDebug( false )
			, mCopyResourcesOnly( false )
			, mNoWatson( false )
		{ }
	};

	void fLaunchPcDx9( const tOptions& options, const std::string& args )
	{
		if( !options.mCopyResourcesOnly )
		{
			tFilePathPtr path;
			if( options.mRelease )		path = tFilePathPtr( "\\bin\\game_pcdx9_release\\game.exe" );
			else						path = tFilePathPtr( "\\bin\\game_pcdx9_internal\\game.exe" );
			const tFilePathPtr exePath = tFilePathPtr::fConstructPath( ToolsPaths::fGetCurrentProjectRootFolder( ), path );
			if( !Threads::tProcess::fSpawnAndForget( exePath.fCStr( ), args.c_str( ) ) )
				log_warning( 0, "Error spawning process" );
		}
	}
	void fLaunchPcDx10( const tOptions& options, const std::string& args )
	{
	}
	void fLaunchWii( const tOptions& options, const std::string& args )
	{
	}
	void fLaunchXbox360( const tOptions& options, const std::string& args )
	{
		tXbDm& xbdm = tXbDm::fInstance( );

		if( options.mDevkitName.length( ) > 0 )
			xbdm.fSetDevkitName( options.mDevkitName );

		if( !xbdm.fIsConnectedToDevkit( ) )
		{
			log_warning( 0, "Can't connect to xbox360 devkit, aborting game launch." );
			return;
		}

		// copy resources to devkit
		xbdm.fCopyResourcesToDevkit( );

		// launch logger output window
		if( !options.mCopyResourcesOnly && !options.mNoWatson )
			xbdm.fLaunchWatson( );

		// launch xbox with game
		tFilePathPtr path;
		if( options.mDebug )		path = tFilePathPtr( "\\bin\\game_xbox360_debug\\default.xex" );
		else if( options.mProfile )	path = tFilePathPtr( "\\bin\\game_xbox360_profile\\default.xex" );
		else if( options.mPlaytest ) path = tFilePathPtr( "\\bin\\game_xbox360_playtest\\default.xex" );
		else if( options.mRelease )	path = tFilePathPtr( "\\bin\\game_xbox360_release\\default.xex" );
		else						path = tFilePathPtr( "\\bin\\game_xbox360_internal\\default.xex" );

		const tFilePathPtr exePath = tFilePathPtr::fConstructPath( ToolsPaths::fGetCurrentProjectRootFolder( ), path );

		xbdm.fLaunchXex( exePath, args, options.mCopyResourcesOnly, options.mMinimal );
	}
	void fLaunchPs3Ppu( const tOptions& options, const std::string& args )
	{
	}

	void fLaunchPlatform( tPlatformId pid, const tOptions& options, const std::string& args )
	{
		switch( pid )
		{
		case cPlatformPcDx9:	fLaunchPcDx9( options, args ); break;
		case cPlatformPcDx10:	fLaunchPcDx10( options, args ); break;
		case cPlatformWii:		fLaunchWii( options, args ); break;
		case cPlatformXbox360:	fLaunchXbox360( options, args ); break;
		case cPlatformPs3Ppu:	fLaunchPs3Ppu( options, args ); break;
		}
	}
}


int main( )
{
	// convert command line to our format
	std::string cmdLineBuffer = GetCommandLine( );

	// TODO maybe we would launch a GUI or something if no command-line options are found

	const tCmdLineOption platform( "platform", cmdLineBuffer );
	const std::string& platformString = platform.fGetOption( );

	const tCmdLineOption programOptions( "options", cmdLineBuffer );
	const std::string& optionsString = programOptions.fGetOption( );

	tOptions options;
	const tCmdLineOption devkit( "x", cmdLineBuffer );
	options.mDevkitName = devkit.fGetTypedOption<std::string>( );
	const tCmdLineOption q0( "q", cmdLineBuffer );
	const tCmdLineOption q1( "quiet", cmdLineBuffer );
	options.mQuiet = ( q0.fFound( ) || q1.fFound( ) );
	const tCmdLineOption min( "min", cmdLineBuffer );
	options.mMinimal = min.fFound( );
	const tCmdLineOption doRelease( "release", cmdLineBuffer );
	options.mRelease = doRelease.fFound( );
	const tCmdLineOption doPlaytest( "playtest", cmdLineBuffer );
	options.mPlaytest = doPlaytest.fFound( );
	const tCmdLineOption doProfile( "profile", cmdLineBuffer );
	options.mProfile = doProfile.fFound( );
	const tCmdLineOption doDebug( "debug", cmdLineBuffer );
	options.mDebug = doDebug.fFound( );
	const tCmdLineOption copyOnly( "copyonly", cmdLineBuffer );
	options.mCopyResourcesOnly = copyOnly.fFound( );
	const tCmdLineOption noWatson( "nowatson", cmdLineBuffer );
	options.mNoWatson = noWatson.fFound( );
	

	// resolve potential ambiguity
	if( options.mDebug )
	{
		options.mRelease = false;
		options.mProfile = false;
		options.mPlaytest = false;
	}
	else if( options.mProfile )
	{
		options.mRelease = false;
		options.mPlaytest = false;
	}
	else if( options.mPlaytest )
	{
		options.mRelease = false;
	}

	// create the console application
	tStrongPtr<tConsoleApp> consoleApp;
	if( !options.mQuiet )
	{
		consoleApp = tStrongPtr<tConsoleApp>( new tConsoleApp( ) );
		consoleApp->fCreateConsole( "-=-=-=-=-=- gAMe lAUNCHEr (c) Signal Studios -=-=-=-=-=-\n", true );
	}

	if( !platform.fFound( ) )
	{
		// launch the default platform, as it wasn't specified on the cmdline
		fLaunchPlatform( cPlatformPcDx9, options, optionsString );
	}
	else
	{
		// launch the specified platform
		for( tPlatformIdIterator pid; !pid.fDone( ); pid.fNext( ) )
		{
			if( !_stricmp( fPlatformIdString( pid ), platformString.c_str( ) ) )
			{
				fLaunchPlatform( pid, options, optionsString );
				break;
			}
		}
	}

	// destroy the console application
	if( consoleApp )
	{
		consoleApp->fDestroyConsole( 
			"-=-=-=-=-=- Finished running gAMe lAUNCHEr -=-=-=-=-=-\n", 
			options.mCopyResourcesOnly ? false : ( consoleApp->fWarningCount( ) > 0 ) );
	}

	return 0;
}
