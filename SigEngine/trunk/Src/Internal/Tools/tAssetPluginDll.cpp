#include "ToolsPch.hpp"
#include "tAssetPluginDll.hpp"
#include "FileSystem.hpp"
#include "tProjectFile.hpp"
#include "ToolsPaths.hpp"
#include "Win32Util.hpp"

namespace Sig
{
	///
	/// \section tAssetPluginDll
	///

	const char*		tAssetPluginDll::fGetReleasePluginExt( )
	{
		return ".apgr";
	}

	const char*		tAssetPluginDll::fGetDebugPluginExt( )
	{
		return ".apgd";
	}

	const char*		tAssetPluginDll::fGetOldPluginExt( )
	{
		return ".apg";
	}

	const char*		tAssetPluginDll::fGetPluginExt( )
	{
#if defined( build_release )
		return fGetReleasePluginExt();
#elif defined( build_debug )
		return fGetDebugPluginExt();
#else
#error "tAssetPluginDll::fGetPluginExt( ) doesn't know which extension to use for this build config"
#endif
	}

	tAssetPluginDll::tAssetPluginDll( HMODULE dllHandle )
		: mDllHandle(0)
		, mGetPluginVersion(0)
		, mCreatePlugins(0)
		, mDestroyPlugins(0)
		, mPluginObjects(0)
		, mErrorString("")
	{
		fExtractDll( dllHandle );
	}
	
	tAssetPluginDll::~tAssetPluginDll( )
	{
		fFreeDll( );
	}

	b32 tAssetPluginDll::fValid( ) const
	{
		return mDllHandle != 0;
	}

	void tAssetPluginDll::fFreeDll( )
	{
		// before unloading the dll, free all plugin objects
		if( mPluginObjects.fCount( ) > 0 )
		{
			sigassert( mDestroyPlugins );
			mDestroyPlugins( mPluginObjects );
		}

		// now we can free the dll
		if( mDllHandle )
		{
			FreeLibrary( mDllHandle );
			mDllHandle = 0;
		}
	}

	void tAssetPluginDll::fExtractDll( HMODULE dllHandle )
	{
		if( !dllHandle )
		{
			std::stringstream ss;
			ss << "Null Dll Handle - GetLastError = " << GetLastError( );
			mErrorString = ss.str( );
			return;
		}

		mDllHandle			= dllHandle;
		mGetPluginVersion	= ( tGetPluginVersion )	GetProcAddress( mDllHandle, "fGetPluginVersion" );
		mCreatePlugins		= ( tCreatePlugins )	GetProcAddress( mDllHandle, "fCreatePlugins" );
		mDestroyPlugins		= ( tDestroyPlugins )	GetProcAddress( mDllHandle, "fDestroyPlugins" );

		if( !mGetPluginVersion )
		{
			mErrorString = "No fGetPluginVersion";
			fFreeDll( );
			return;
		}

		if( !mCreatePlugins )
		{
			mErrorString = "No fCreatePlugins";
			fFreeDll( );
			return;
		}

		if( !mDestroyPlugins )
		{
			mErrorString = "No fDestroyPlugins";
			fFreeDll( );
			return;
		}

		if( mGetPluginVersion( ) != asset_plugin_version )
		{
			mErrorString = "Wrong version";
			fFreeDll( );
			return;
		}

		// now extract all the plugin objects
		mCreatePlugins( mPluginObjects );
	}

	///
	/// \section tAssetPluginGroupDepot
	///

	void tAssetPluginDllDepot::fLoadPluginsBasedOnCurrentProjectFile( )
	{
		const std::string pluginExtSearchFilterStr = tAssetPluginDll::fGetPluginExt( );
		const tFilePathPtr pluginExtSearchFilter = tFilePathPtr( pluginExtSearchFilterStr.c_str( ) );

		// load the current project file; if it doesn't exist, we will just get the default settings
		const tFilePathPtr projectFilePath = ToolsPaths::fGetCurrentProjectFilePath( );
		const tProjectFile & projectFile = tProjectFile::fInstance( );

		// load engine plugins if the project file says so
		if( projectFile.mEngineConfig.mAssetGenConfig.mUseEnginePlugins )
		{
			fLoadPluginsFromDirectory( ToolsPaths::fGetEngineBinFolder( ) );
		}

		// HISTORICAL NOTE: Due to a change in location of the project directory, the 
		// additional plugins are based out of the root project directory instead of
		// the folder in which the project file sits.
		const tFilePathPtr projectDirectory = ToolsPaths::fGetCurrentProjectRootFolder();

		// loop through additional plugin folders
		for( u32 i = 0; i < projectFile.mEngineConfig.mAssetGenConfig.mAdditionalPluginPaths.fCount( ); ++i )
		{
			const tFilePathPtr fullDir = tFilePathPtr::fConstructPath( 
				projectDirectory, projectFile.mEngineConfig.mAssetGenConfig.mAdditionalPluginPaths[i] );

			fLoadPluginsFromDirectory( fullDir );
		}
	}
	
	tAssetPluginDllDepot::tAssetPluginDllDepot( )
	{
	}

	tAssetPluginDllDepot::~tAssetPluginDllDepot( )
	{
	}

	void tAssetPluginDllDepot::fCullPluginsByConfiguration( tFilePathPtrList& pluginFileNames )
	{
		// look for release/debug dll pairs; we want to load only one from each pair,
		// and we want to load the right one depending on our build configurtion...

		tFilePathPtrList debugDlls;
		tFilePathPtrList releaseDlls;

		const std::string dPlusExt = std::string("d") + tAssetPluginDll::fGetPluginExt( );

		for( u32 i = 0; i < pluginFileNames.fCount( ); ++i )
		{
			std::string f = pluginFileNames[i].fCStr( );

			StringUtil::fReplaceAllOf( f, tAssetPluginDll::fGetPluginExt( ), dPlusExt.c_str( ) );

			tFilePathPtr lookFor( f.c_str( ) );

			const u32 badIndex = ~0;
			u32 matchIndex = badIndex;
			for( u32 j = 0; j < pluginFileNames.fCount( ); ++j )
			{
				if( j == i )
					continue;
				if( pluginFileNames[j] == lookFor )
				{
					matchIndex = j;
					break;
				}
			}

			if( matchIndex != badIndex )
			{
				// by turning the current file name into a debug file name,
				// we turned up a match; this means the current file name
				// is a release plugin, and the match we found is the debug version
				releaseDlls.fPushBack( pluginFileNames[i] );
				debugDlls.fPushBack( pluginFileNames[matchIndex] );

				// now remove both of these from the main list... have to be careful
				// bcz removing is indexed base, so removing one could invalidate
				// the other index if we're not careful
				pluginFileNames.fEraseOrdered( fMax( i, matchIndex ) );
				pluginFileNames.fEraseOrdered( fMin( i, matchIndex ) );
			}
		}

		// alright, we've removed the release/debug pairs and put them in their own lists;
		// the rest of the file names in the passed-in list were unmatched, so we'll keep those
#ifdef build_debug
		pluginFileNames.fJoin( debugDlls );
#else// build_release
		pluginFileNames.fJoin( releaseDlls );
#endif// build_debug and build_release
	}

	void tAssetPluginDllDepot::fLoadPlugins( const tFilePathPtrList& pluginFileNames )
	{
		for( u32 i = 0; i < pluginFileNames.fCount( ); ++i )
		{
			tFilePathPtr pluginPath = pluginFileNames[i];

			// attempt to load the plugin dll
			tAssetPluginDllPtr pluginDll( new tAssetPluginDll( LoadLibrary( pluginPath.fCStr( ) ) ) );
			if( !pluginDll->fValid( ) )
			{
				log_warning( "Invalid AssetPluginDll [" << pluginFileNames[i] << "]." << std::endl <<
					"\tError reported: " << pluginDll->fErrorString( ) << std::endl <<
					"\tCheck that you have compiled against the most recent version, " << std::endl <<
					"\tand that you have implemented all the required dll export functions." );
				continue; // apparently it was a bogus dll
			}

			// store the successfully loaded plugin dll
			log_line( 0, "Loaded [" << pluginFileNames[i] << "] successfully." );
			fPushBack( pluginDll );
		}
	}

	namespace
	{
		void fCollect( std::string& baseName, tFilePathPtr ptr, const std::string& trim )
		{
			baseName = ptr.fCStr( );
			sigassert( StringUtil::fStricmp(baseName.c_str()+baseName.size()-trim.size(),trim.c_str()) == 0 );
		}
	}

	void tAssetPluginDllDepot::fLoadPluginsFromDirectory( const tFilePathPtr& directory )
	{
		/// First, find all the existing plugins.

		tFilePathPtrList plugins;
		tGrowableArray<std::string> basePluginNames;

		FileSystem::fGetFileNamesInFolder( plugins, directory, true, true, tFilePathPtr(tAssetPluginDll::fGetReleasePluginExt()) );
		for( u32 i=0 ; i<plugins.fCount( ) ; ++i )
		{
			std::string f = plugins[i].fCStr();
			StringUtil::fRemoveAllOf(f,".apgr");
			if( !basePluginNames.fFind(f) )
				basePluginNames.fPushBack(f);
		}
		plugins.fSetCount(0);

		FileSystem::fGetFileNamesInFolder( plugins, directory, true, true, tFilePathPtr(tAssetPluginDll::fGetDebugPluginExt()) );
		for( u32 i=0 ; i<plugins.fCount( ) ; ++i )
		{
			std::string f = plugins[i].fCStr();
			StringUtil::fRemoveAllOf(f,".apgd");
			if( !basePluginNames.fFind(f) )
				basePluginNames.fPushBack(f);
		}
		plugins.fSetCount(0);

		FileSystem::fGetFileNamesInFolder( plugins, directory, true, true, tFilePathPtr(tAssetPluginDll::fGetOldPluginExt()) );
		for( u32 i=0 ; i<plugins.fCount( ) ; ++i )
		{
			std::string f = plugins[i].fCStr();
			StringUtil::fRemoveAllOf(f,"d.apg");
			StringUtil::fRemoveAllOf(f,".apg");
			if( !basePluginNames.fFind(f) )
				basePluginNames.fPushBack(f);
		}
		plugins.fSetCount(0);

		/// Now check to make sure our current build isn't missing any of those plugins.

		FileSystem::fGetFileNamesInFolder( plugins, directory, true, true, tFilePathPtr(tAssetPluginDll::fGetPluginExt()) );
		for( u32 base=0 ; base<basePluginNames.fCount( ) ; ++base )
		{
			b32 found = false;
			for( u32 i=0 ; i<plugins.fCount( ) ; ++i )
			{
				if( StringUtil::fStrStrI(plugins[i].fCStr(),basePluginNames[base].c_str()) )
				{
					found=true;
					break;
				}
			}

			if( !found )
			{
				std::stringstream error;
				error << "The current build is missing the following plugin:\n" << basePluginNames[base];
				Log::fFatalError(error.str().c_str());
			}
		}

		/// Now load the plugins specifically for our build.
		fLoadPlugins( plugins );
	}
}
