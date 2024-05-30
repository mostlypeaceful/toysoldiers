#include "BasePch.hpp"
#include "tAssetPluginDll.hpp"
#include "tAssetGenScanner.hpp"
#include "iMaterialGenPlugin.hpp"
#include "tConsoleApp.hpp"
#include "tCmdLineOption.hpp"
#include "ToolsPaths.hpp"
#include "FileSystem.hpp"
#include "resource.h"

using namespace Sig;

namespace
{
	struct tMaterialGenOpts
	{
		b32 mForce;
		b32 mQuiet;
		b32 mPauseOnExit;
		std::string mFilter;

		tMaterialGenOpts( ) : mForce( false ), mQuiet( false ), mPauseOnExit( true ) { }
	};

	///
	/// \brief Aid in iterating over all registered plugins.
	struct tForEachPlugin
	{
		const tMaterialGenOpts& mOpts;
		explicit tForEachPlugin( const tMaterialGenOpts& opts )
			: mOpts( opts ) { }
		b32 operator()( const tAssetPluginDllPtr& dllPtr, iAssetPlugin& assetPlugin ) const;
	};

	b32 tForEachPlugin::operator()( const tAssetPluginDllPtr& dllPtr, iAssetPlugin& assetPlugin ) const
	{
		// query for the material gen interface
		iMaterialGenPlugin* mtlGenPlugin = assetPlugin.fGetMaterialGenPluginInterface( );
		if( !mtlGenPlugin )
			return true;


		// generate the game binaries
#ifdef build_debug
		const tFilePathPtr exePath = tFilePathPtr::fConstructPath( ToolsPaths::fGetEngineBinFolder( ), tFilePathPtr( "MaterialGend.exe" ) );
#else
		const tFilePathPtr exePath = tFilePathPtr::fConstructPath( ToolsPaths::fGetEngineBinFolder( ), tFilePathPtr( "MaterialGen.exe" ) );
#endif

		if( mOpts.mFilter.length( ) == 0 || _stricmp( mtlGenPlugin->fGetMaterialName( ), mOpts.mFilter.c_str( ) ) == 0 )
		{
			log_line( 0, "-> Generating material [" << mtlGenPlugin->fGetMaterialName( ) << "]..." );
			mtlGenPlugin->fGenerateMaterialFile( exePath, mOpts.mForce );
		}

		return true;
	}

	void fParseOptions( tMaterialGenOpts& opts, const std::string& cmdLineBuffer )
	{
		const tCmdLineOption f0( "f", cmdLineBuffer );
		const tCmdLineOption f1( "force", cmdLineBuffer );
		opts.mForce = ( f0.fFound( ) || f1.fFound( ) );

		const tCmdLineOption q0( "q", cmdLineBuffer );
		const tCmdLineOption q1( "quiet", cmdLineBuffer );
		opts.mQuiet = ( q0.fFound( ) || q1.fFound( ) );

		const tCmdLineOption nopause0( "np", cmdLineBuffer );
		const tCmdLineOption nopause1( "nopause", cmdLineBuffer );
		opts.mPauseOnExit = !( nopause0.fFound( ) || nopause1.fFound( ) );

		const tCmdLineOption filter( "filter", cmdLineBuffer );
		if( filter.fFound( ) )
			opts.mFilter = filter.fGetTypedOption<std::string>( );
	}

	void fRegisterPlugins( )
	{
		log_newline( );
		log_line( 0, "@ Registering asset plugins..." );
		tAssetPluginDllDepot::fInstance( ).fLoadPluginsBasedOnCurrentProjectFile( );
		log_line( 0, "...finished registering asset plugins." );
	}

	void fGenerateMaterials( const tMaterialGenOpts& opts )
	{
		log_newline( );
		log_line( 0, "@ Generating materials..." );

		tForEachPlugin forEach( opts );
		tAssetPluginDllDepot::fInstance( ).fForEachPlugin( forEach );

		const tFilePathPtr xboxGameFolder = ToolsPaths::fGetCurrentProjectGamePlatformFolder( cPlatformXbox360 );
		const tFilePathPtr cleanFilePath = tFilePathPtr::fConstructPath( xboxGameFolder, tFilePathPtr( "clean" ) );
		FileSystem::fDeleteFile( cleanFilePath );

		log_line( 0, "...finished generating materials." );
	}

}

int main( )
{
	// convert command line to our format
	std::string cmdLineBuffer = GetCommandLine( );

	// parse command-line options
	tMaterialGenOpts opts;
	fParseOptions( opts, cmdLineBuffer );

	// create the console application
	tStrongPtr<tConsoleApp> consoleApp;
	if( !opts.mQuiet )
	{
		consoleApp = tStrongPtr<tConsoleApp>( new tConsoleApp( ) );
		consoleApp->fCreateConsole( "-=-=-=-=-=- mATERIAl gEn (c) Signal Studios -=-=-=-=-=-\n", true );
	}

	fRegisterPlugins( );

	fGenerateMaterials( opts );

	// destroy the console application
	if( consoleApp )
		consoleApp->fDestroyConsole( "-=-=-=-=-=- Finished running mATERIAl gEn -=-=-=-=-=-\n", opts.mPauseOnExit );

	return 0;
}


