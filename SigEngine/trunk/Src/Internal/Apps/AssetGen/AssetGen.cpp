#include "BasePch.hpp"
#include "tAssetGenScanner.hpp"
#include "tAssetPluginDll.hpp"
#include "tConsoleApp.hpp"
#include "tCmdLineOption.hpp"
#include "resource.h"
#include "FileSystem.hpp"
#include "wx/string.h"
#include "wx/regex.h"

using namespace Sig;

namespace
{

	void fParseOptions( tAssetGenOptions& opts, const std::string& cmdLineBuffer )
	{
		const tCmdLineOption log( "log", cmdLineBuffer );
		if( log.fFound( ) )
		{
			opts.mOutputLevel = log.fGetTypedOption<u32>( );
			if( opts.mOutputLevel > 3 )
				opts.mOutputLevel = 2;
		}

		const tCmdLineOption file( "file", cmdLineBuffer );
		if( file.fFound( ) )
			opts.mFile = file.fGetOption( );

		// Extensions
		{
			const tCmdLineOption ext( "ext", cmdLineBuffer );
			if( ext.fFound( ) )
				StringUtil::fSplit( opts.mExt, ext.fGetOption( ).c_str( ), " " );
		}

		const tCmdLineOption recursive0( "r", cmdLineBuffer );
		const tCmdLineOption recursive1( "recursive", cmdLineBuffer );
		opts.mRecursive = ( recursive0.fFound( ) || recursive1.fFound( ) );

		const tCmdLineOption force0( "f", cmdLineBuffer );
		const tCmdLineOption force1( "force", cmdLineBuffer );
		opts.mForce = ( force0.fFound( ) || force1.fFound( ) );

		const tCmdLineOption nopause0( "np", cmdLineBuffer );
		const tCmdLineOption nopause1( "nopause", cmdLineBuffer );
		opts.mPauseOnExit = !( nopause0.fFound( ) || nopause1.fFound( ) );

		const tCmdLineOption dopause0( "dp", cmdLineBuffer );
		const tCmdLineOption dopause1( "dopause", cmdLineBuffer );
		opts.mForcePauseOnExit = dopause0.fFound( ) || dopause1.fFound( );

		const tCmdLineOption disableDepends0( "d", cmdLineBuffer );
		const tCmdLineOption disableDepends1( "disdep", cmdLineBuffer );
		opts.mDisableDependencyFiles = ( disableDepends0.fFound( ) || disableDepends1.fFound( ) );

		const tCmdLineOption genPackFiles( "sacpack", cmdLineBuffer );
		opts.mGenPackFiles = genPackFiles.fFound( );

		const tCmdLineOption sacmlOutput( "osac", cmdLineBuffer );
		if( sacmlOutput.fFound( ) )
			opts.mSacmlOutput = sacmlOutput.fGetOption( );

		const tCmdLineOption game2xbox( "game2xbox", cmdLineBuffer );
		opts.mGame2Xbox = game2xbox.fFound( );

		const tCmdLineOption fullsync( "fullsync", cmdLineBuffer );
		opts.mFullSync = fullsync.fFound( );

		const tCmdLineOption mp0( "m", cmdLineBuffer );
		const tCmdLineOption mp1( "multiproc", cmdLineBuffer );
		opts.mMultiProcMaster = ( mp0.fFound( ) || mp1.fFound( ) );

		const tCmdLineOption wp0( "wp", cmdLineBuffer );
		const tCmdLineOption wp1( "workerproc", cmdLineBuffer );
		opts.mWorkerProc = ( wp0.fFound( ) || wp1.fFound( ) );

		const tCmdLineOption squirrelCheck( "sqrlck", cmdLineBuffer );
		opts.mSquirrelCheck = squirrelCheck.fFound( );

		opts.mPlatforms = 0;

		const tCmdLineOption wii( "wii", cmdLineBuffer );
		opts.mPlatforms |= ( wii.fFound( ) ? fPlatformIdFlag( cPlatformWii ) : 0 );

		const tCmdLineOption pcdx9( "pcdx9", cmdLineBuffer );
		opts.mPlatforms |= ( pcdx9.fFound( ) ? fPlatformIdFlag( cPlatformPcDx9 ) : 0 );

		const tCmdLineOption pcdx10( "pcdx10", cmdLineBuffer );
		opts.mPlatforms |= ( pcdx10.fFound( ) ? fPlatformIdFlag( cPlatformPcDx10 ) : 0 );

		const tCmdLineOption xbox360( "xbox360", cmdLineBuffer );
		opts.mPlatforms |= ( xbox360.fFound( ) ? fPlatformIdFlag( cPlatformXbox360 ) : 0 );

		const tCmdLineOption ps3ppu( "ps3ppu", cmdLineBuffer );
		opts.mPlatforms |= ( ps3ppu.fFound( ) ? fPlatformIdFlag( cPlatformPs3Ppu ) : 0 );

		if( opts.mPlatforms == 0 )
			opts.mPlatforms = ~0; // default to all platforms if none specified
	}

	void fLogOptions( const tAssetGenOptions& opts )
	{
		log_line( 0, "Log output level = " << opts.mOutputLevel );
		if( opts.mFile.length( ) > 0 )
			log_line( 0, "Found option: file=[" << opts.mFile << "]" );
		
		// Extensions
		{
			const u32 cExtCount = opts.mExt.fCount( );
			if( cExtCount )
			{
				std::stringstream ss;
				for( u32 i = 0; i < cExtCount; ++i )
				{
					ss << opts.mExt[ i ];
					if( i < cExtCount - 1 )
						ss << ", ";
				}

				log_line( 0, "Found option: ext=[" << ss.str( ) << "]" );
			}
		}
		
		if( opts.mRecursive )
			log_line( 0, "Found option: recursive" );
		if( opts.mForce )
			log_line( 0, "Found option: force" );
		if( !opts.mPauseOnExit )
			log_line( 0, "Found option: nopause" );
		if( opts.mForcePauseOnExit )
			log_line( 0, "Found option: dopause" );
		if( opts.mDisableDependencyFiles )
			log_line( 0, "Found option: disable dependency file generation" );
		if( opts.mGenPackFiles )
			log_line( 0, "Found option: generate pack files from .sacml files" );
		if( opts.mGame2Xbox )
			log_line( 0, "Found option: game2xbox" );
		if( opts.mFullSync )
			log_line( 0, "Found option: fullsync" );
		if( opts.mMultiProcMaster )
			log_line( 0, "Found option: multi-process master" );
		if( opts.mWorkerProc )
			log_line( 0, "Found option: worker process" );
		if( opts.mSquirrelCheck )
			log_line( 0, "Found option: squirrel dup check" );

		if( opts.mPlatforms & fPlatformIdFlag( cPlatformWii ) )
			log_line( 0, "Found option: Wii" );
		if( opts.mPlatforms & fPlatformIdFlag( cPlatformPcDx9 ) )
			log_line( 0, "Found option: PcDx9" );
		if( opts.mPlatforms & fPlatformIdFlag( cPlatformPcDx10 ) )
			log_line( 0, "Found option: PcDx10" );
		if( opts.mPlatforms & fPlatformIdFlag( cPlatformXbox360 ) )
			log_line( 0, "Found option: Xbox360" );
		if( opts.mPlatforms & fPlatformIdFlag( cPlatformPs3Ppu ) )
			log_line( 0, "Found option: Ps3Ppu" );
	}

	void fRegisterPlugins( const tAssetGenOptions& opts )
	{
		log_newline( );
		log_line( 0, "@ Registering asset plugins..." );
		tAssetPluginDllDepot::fInstance( ).fLoadPluginsBasedOnCurrentProjectFile( );
		log_line( 0, "...finished registering asset plugins." );
	}

	void fGenerateAssets( const tAssetGenOptions& opts )
	{
		log_newline( );
		log_line( 0, "@ Scanning directories for files to convert..." );

		const Time::tStamp timeBegin = Time::fGetStamp( );

		tAssetGenScanner scanner( opts );
		scanner.fHandleCurrentFolder( tAssetPluginDllDepot::fInstance( ) );

		const Time::tStamp timeEnd = Time::fGetStamp( );

		const int seconds = fRound<int>( Time::fGetElapsedS( timeBegin, timeEnd ) );
		log_line( 0, "...finished scanning directories (elapsed " << seconds / 60 << ":" << std::setw( 2 ) << std::setfill( '0' ) << seconds % 60 << ")." );
	}

	tGrowableArray<std::string> GetClassNamesFromFile(tFilePathPtr& path)
	{
		wxRegEx classRegex("class\\s+(\\S+)\\s*", wxRE_ADVANCED );
		wxRegEx commentRegex("^\\s*//", wxRE_ADVANCED );

		tGrowableArray<std::string> classNames;

		std::string fileData;
		FileSystem::fReadFileToString( fileData, path );
		
		tGrowableArray<std::string> lines;
		StringUtil::fSplit( lines, fileData.c_str( ), "\n" );

		for( u32 i = 0; i < lines.fCount( ); ++i )
		{
			std::string line = lines[ i ];

			if( commentRegex.Matches( line ) )
				continue;

			if( !classRegex.Matches( line ) )
				continue;

			// Start after the first match because that's always
			// the "global" match and we want sub-matches
			for( u32 j = 1; j < classRegex.GetMatchCount( ); ++j )
			{
				std::string match = classRegex.GetMatch( line, j );
				classNames.fPushBack( match );
			}
		}

		return classNames;
	}

	void fDumpDuplicateSquirrelClasses( const tAssetGenOptions& opts )
	{
		if ( !opts.mSquirrelCheck )
			return;

		log_newline( );
		log_line( 0, "@ Scanning .nut files for duplicate class names..." );

		// Verify that this folder is a sub-folder of res
		std::string curDir;
		Win32Util::fGetCurrentDirectory( curDir );
		tFilePathPtr curDirRelativeToRes = ToolsPaths::fMakeResRelative( tFilePathPtr( curDir.c_str( ) ), true );
		if( curDirRelativeToRes.fNull( ) )
		{
			log_warning( "The current directory [" << curDir << "] is not a subfolder of the current project's Res folder; aborting." );
			return;
		}

		const Time::tStamp timeBegin = Time::fGetStamp( );

		const tFilePathPtr resPath = ToolsPaths::fGetCurrentProjectResFolder( );

		tGrowableArray<std::string> classNames;
		tGrowableArray<tGrowableArray<tFilePathPtr>> filePaths;

		// Get the path to all the .nut files in the res directory
		tFilePathPtrList dirFiles;
		tFilePathPtr extFilter( "nut" );
		FileSystem::fGetFileNamesInFolder( dirFiles, resPath, true, true, extFilter );

		// Find the duplicates across files
		for( u32 i = 0; i < dirFiles.fCount( ); ++i )
		{
			tFilePathPtr file = dirFiles[ i ];

			// Ignore any .nut file underneath a tilde directory
			tGrowableArray<std::string> pathComponents;
			StringUtil::fSplit( pathComponents, file.fCStr( ), "\\" );
			bool tildeFound = false;
			for( u32 j = 0; j < pathComponents.fCount( ); ++j )
			{
				char firstChar = pathComponents[ j ].c_str( )[0];
				if( firstChar == '~' )
				{
					tildeFound = true;
					break;
				}
			}
			if( tildeFound )
				continue;

			tGrowableArray<std::string> fileClassNames = GetClassNamesFromFile( file );
			for( u32 j = 0; j < fileClassNames.fCount( ); ++j )
			{
				std::string className = fileClassNames[ j ];

				bool found = false;
				for( u32 k = 0; k < classNames.fCount( ); ++k )
				{
					if( classNames[ k ] == className )
					{
						filePaths[ k ].fPushBack( file );

						found = true;

						break;
					}
				}
				if( !found )
				{
					classNames.fPushBack( className );
					
					tGrowableArray<tFilePathPtr> paths;
					paths.fPushBack( file );
					filePaths.fPushBack( paths );
				}
			}
		}

		// Print warnings for any duplicate
		for( u32 i = 0; i < classNames.fCount( ); ++i )
		{
			u32 fileCount = filePaths[ i ].fCount( );
			if( fileCount > 1 )
			{
				std::stringstream output;

				output << "Duplicate class: " << classNames[ i ];
				for( u32 j = 0; j < fileCount; ++j )
					output << "\n\t" << filePaths[ i ][ j ];

				output << "\n";
				log_warning( output.str( ) );
			}
		}

		const Time::tStamp timeEnd = Time::fGetStamp( );

		const int seconds = fRound<int>( Time::fGetElapsedS( timeBegin, timeEnd ) );
		log_line( 0, "...finished scanning .nut files (elapsed " << seconds / 60 << ":" << std::setw( 2 ) << std::setfill( '0' ) << seconds % 60 << ")." );
	}

	void fDumpWarnings( tConsoleApp& app, const tAssetGenOptions& opts )
	{
		const b32 wasBufferingWarnings = app.fBufferWarnings( );
		app.fSetBufferWarnings( false ); // stop buffering warnings
		const tGrowableArray<std::string>& warnings = app.fBufferedWarnings( );

		log_newline( );
		log_line( 0, "@ Conversion is complete, " << warnings.fCount( ) << " warnings..." );

		for( u32 i = 0; i < warnings.fCount( ); ++i )
			log_output( 0, i << ": " << warnings[ i ] );

		if( warnings.fCount( ) > 0 )
			log_line( 0, "...finished dumping all warnings." );
		app.fSetBufferWarnings( wasBufferingWarnings ); // reset buffering of warnings
	}

	void fUpdateToolsResources( const tAssetGenOptions& opts )
	{
		const tFilePathPtr engineToolsResourcesPath = tFilePathPtr::fConstructPath( ToolsPaths::fGetEngineRootFolder( ), tFilePathPtr( "Src\\Internal\\Tools\\Resources" ) );
		const tFilePathPtr projectToolsResourcesPath = tFilePathPtr::fConstructPath( ToolsPaths::fGetCurrentProjectResFolder( ), tFilePathPtr( "_tools" ) );

		// Make sure the engine tools directory exists
		if( FileSystem::fFolderExists( engineToolsResourcesPath ) )
		{		
			// Create the project tools directory if required
			if( !FileSystem::fFolderExists( projectToolsResourcesPath ) )
			{
				log_line( 0, projectToolsResourcesPath << " does not exist. Creating folder.." );
				sig_assert( Win32Util::fCreateDirectory( projectToolsResourcesPath.fCStr( ) ) );
			}

			std::string roboCommand = std::string( "robocopy \"" ) + engineToolsResourcesPath.fCStr( ) + "\" \"" + projectToolsResourcesPath.fCStr( ) + "\" /mir";
			s32 result = system( roboCommand.c_str( ) );
		}
		else
			log_line( 0, "Engine resource path does not exist: " << engineToolsResourcesPath );
	}
}

class tAssetGenConsoleApp : public tConsoleApp
{
public:
	tAssetGenConsoleApp( u32 logLevel )
		: mLogLevel( logLevel )
	{
	}
	virtual b32 fLogFilter( const char* text )
	{
		switch( mLogLevel )
		{
		case 0: // warnings only
			if( StringUtil::fStrStrI( text, "Output from file" ) )
				return true;
			if( StringUtil::fStrStrI( text, "!WARNING!" ) )
				return true;
			return false;
		case 1: // cull innocuous text
			if( StringUtil::fStrStrI( text, ">> Scanning" ) )
				return false;
			if( StringUtil::fStrStrI( text, "-> Skipping" ) )
				return false;
			return true;
		case 2: // default, for now this accepts everything
			return true;
		case 3: // verbose, currently same as default
			return true;
		}
		return true;
	}
private:
	u32 mLogLevel;
};

int main( )
{
	// convert command line to our format
	const std::string cmdLineBuffer = GetCommandLine( );

	// first things first, get the options from the command-line
	tAssetGenOptions opts;
	fParseOptions( opts, cmdLineBuffer );

	// create the console application
	tStrongPtr<tConsoleApp> consoleApp;
	if( !opts.mWorkerProc )
	{
		consoleApp = tStrongPtr<tConsoleApp>( new tAssetGenConsoleApp( opts.mOutputLevel ) );
		consoleApp->fSetBufferWarnings( true );
		consoleApp->fCreateConsole( "-=-=-=-=-=- aSSEt gEn (c) Signal Studios -=-=-=-=-=-\n", true );
	}

	fLogOptions( opts );

	if( !opts.mWorkerProc )
		fUpdateToolsResources( opts );

	fRegisterPlugins( opts );

	fGenerateAssets( opts );

	u32 warningCount = 0;

	if( consoleApp )
	{
		fDumpDuplicateSquirrelClasses( opts );

		fDumpWarnings( *consoleApp, opts );

		// destroy the console application
		warningCount = consoleApp->fWarningCount( );
		consoleApp->fDestroyConsole( "-=-=-=-=-=- Finished running aSSEt gEn -=-=-=-=-=-\n", 
			opts.mForcePauseOnExit || ( opts.mPauseOnExit && warningCount > 0 ) );
	}

	return warningCount;
}


