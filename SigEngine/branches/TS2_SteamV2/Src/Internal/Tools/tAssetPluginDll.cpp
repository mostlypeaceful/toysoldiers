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

	const char*		tAssetPluginDll::fGetPluginExt( )
	{
		return ".apg";
	}

	tAssetPluginDll::tAssetPluginDll( HMODULE dllHandle )
	{
		fZeroOut( this );
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
			return;

		mDllHandle			= dllHandle;
		mGetPluginVersion	= ( tGetPluginVersion )	GetProcAddress( mDllHandle, "fGetPluginVersion" );
		mCreatePlugins		= ( tCreatePlugins )	GetProcAddress( mDllHandle, "fCreatePlugins" );
		mDestroyPlugins		= ( tDestroyPlugins )	GetProcAddress( mDllHandle, "fDestroyPlugins" );

		if( !mGetPluginVersion || !mCreatePlugins || !mDestroyPlugins )
		{
			fFreeDll( );
			return;
		}

		if( mGetPluginVersion( ) != asset_plugin_version )
		{
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
		tProjectFile projectFile;
		projectFile.fLoadXml( projectFilePath );

		// load engine plugins if the project file says so
		if( projectFile.mAssetGenConfig.mUseEnginePlugins )
		{
			const tFilePathPtr engineBinFolder = ToolsPaths::fGetEngineBinFolder( );

			tFilePathPtrList pluginFileNames;
			FileSystem::fGetFileNamesInFolder( 
				pluginFileNames, 
				engineBinFolder, 
				true, 
				true, 
				pluginExtSearchFilter );

			fCullPluginsByConfiguration( pluginFileNames );

			fLoadPlugins( pluginFileNames );
		}

		// switch current directory to the project path, so that any paths that were specified
		// relative to that path will be found...
		const tFilePathPtr projectDirectory( StringUtil::fDirectoryFromPath( projectFilePath.fCStr( ) ).c_str( ) );
		Win32Util::tTemporaryCurrentDirectoryChange setCurrentFolderToProjectPath( projectDirectory.fCStr( ) );

		// loop through additional plugin folders
		for( u32 i = 0; i < projectFile.mAssetGenConfig.mAdditionalPluginPaths.fCount( ); ++i )
		{
			tFilePathPtrList pluginFileNames;
			FileSystem::fGetFileNamesInFolder( 
				pluginFileNames, 
				projectFile.mAssetGenConfig.mAdditionalPluginPaths[i], 
				true, 
				true, 
				pluginExtSearchFilter );

			fCullPluginsByConfiguration( pluginFileNames );

			fLoadPlugins( pluginFileNames );
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
				log_warning( 0, "Invalid AssetPluginDll [" << pluginFileNames[i] << "]." << std::endl <<
					"\tCheck that you have compiled against the most recent version, " << std::endl <<
					"\tand that you have implemented all the required dll export functions." );
				continue; // apparently it was a bogus dll
			}

			// store the successfully loaded plugin dll
			log_line( 0, "Loaded [" << pluginFileNames[i] << "] successfully." );
			fPushBack( pluginDll );
		}
	}

}
