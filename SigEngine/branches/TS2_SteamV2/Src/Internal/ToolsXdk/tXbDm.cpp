#include "ToolsXdkPch.hpp"
#include "tXbDm.hpp"
#include "ToolsPaths.hpp"
#include "FileSystem.hpp"
#include "Threads/tProcess.hpp"

namespace Sig
{
	namespace
	{
		tFilePathPtr fComputeXdkBinPath( )
		{
			char* xedkDir = 0;
			size_t xedkDirSize;
			if( _dupenv_s( &xedkDir, &xedkDirSize, "xedk" ) )
				return tFilePathPtr( );
			if( !xedkDir )
				return tFilePathPtr( );

			std::stringstream ss;
			ss << xedkDir << "\\bin\\win32";

			// Free xedkDir
			free( xedkDir );

			return tFilePathPtr( ss.str( ) );
		}
	}

	tXbDm::tXbDm( )
		: mDllHandle( 0 )
		, mXdkBinPath( fComputeXdkBinPath( ) )
	{

		// attempt to load library and connect to devkit
		fLoadLibrary( );
	}

	tXbDm::~tXbDm( )
	{
	}

	b32 tXbDm::fLoadLibrary( )
	{
		if( mDllHandle )
			return true; // already loaded

		if( mXdkBinPath.fLength( ) == 0 )
			return false; // xdk is probably not installed

		char* path = 0;
		size_t pathSize;
		if( _dupenv_s( &path, &pathSize, "path" ) || !path )
			return false; // no path environment variable, computer is fucked
		
		// Build a new path with the %xedk%\bin\win32 directory added.
		// Allocate enough memory for the newPath string.
		std::stringstream ss;
		ss << "path=" << mXdkBinPath.fCStr( ) << ";" << path;

		// Free path
		free( path );

		// store new path string
		const std::string newPathStr = ss.str( );

		// Update the system path with the new value.  This is so our LoadLibrary call can find XBDM.DLL.
		_putenv( newPathStr.c_str( ) );

		// Call LoadLibrary on XBDM.DLL.
		mDllHandle = LoadLibrary( "xbdm.dll" );

		// Print an error message and return zero if XBDM.DLL didn't load.
		if( !mDllHandle )
		{
			log_warning( 0, "Couldn't load xbdm.dll." );
			return false;
		}

		// if we're here, it means we were able to load XBDM.DLL

		// Retrieve the name of the default Xenon devkit.
		DWORD devkitNameSize = MAX_PATH;
		char devkitName[MAX_PATH];
		HRESULT hr = DmGetNameOfXbox( devkitName, &devkitNameSize, FALSE );

		// Print results.
		if( FAILED( hr ) )
		{
			log_warning( 0, "Could not connect to default xbox360 devkit (either it's not connected, or else no devkit name was set)." );
		}
		else
		{
			// store devkit name
			mDevkitName = devkitName;
			log_line( 0, "Connection succeeded to devkit [" << mDevkitName << "]." );
			fEnsureDirExists( tFilePathPtr( std::string( "devkit:\\" ) + ToolsPaths::fGetCurrentProjectName( ) ) );
		}

		// XBDM.DLL loaded.  Return true for success.
		return true;
	}

	void tXbDm::fSetDevkitName( const std::string& name )
	{
		mDevkitName = name;
		log_line( 0, "Changing connection to devkit [" << mDevkitName << "]." );
		fEnsureDirExists( tFilePathPtr( std::string( "devkit:\\" ) + ToolsPaths::fGetCurrentProjectName( ) ) );
	}

	b32 tXbDm::fEnsureDirExists( const tFilePathPtr& xbSubDirDest ) const
	{
		//const tFilePathPtr xbmkdirPath = tFilePathPtr::fConstructPath( mXdkBinPath, tFilePathPtr( "xbmkdir.exe" ) );
		//const std::string systemArg = std::string( "call \"" ) + std::string(xbmkdirPath.fCStr( )) + "\" /x " + mDevkitName + " " + xbSubDirDest.fCStr( );
		//if( system( systemArg.c_str( ) ) != 0 )
		const HRESULT res = DmMkdir( xbSubDirDest.fCStr( ) );
		if( res != XBDM_NOERR && res != XBDM_ALREADYEXISTS )
		{
			log_warning( 0, "Error creating directory: " << xbSubDirDest );
			return false;
		}
		return true;
	}

	b32 tXbDm::fCopyResourcesToDevkit( ) const
	{
		if( mXdkBinPath.fLength( ) == 0 )
			return false; // xdk is probably not installed
		if( !fIsConnectedToDevkit( ) )
			return false;

		const std::string curProjName = ToolsPaths::fGetCurrentProjectName( );

		tFilePathPtrList iniFiles;
		FileSystem::fGetFileNamesInFolder( iniFiles, ToolsPaths::fGetCurrentProjectResFolder( ), true, false, tFilePathPtr( ".ini" ) );

		// always attempt to copy *.ini files
		if( iniFiles.fCount( ) > 0 )
		{
			log_line( 0, "Copying *.ini files to devkit..." );
			fCopyFilesToDevkitFolder( iniFiles, tFilePathPtr( curProjName ) );
		}

		// check for 'clean' and 'force' files
		const tFilePathPtr xboxGameFolder = ToolsPaths::fGetCurrentProjectGamePlatformFolder( cPlatformXbox360 );
		const tFilePathPtr forceFilePath = tFilePathPtr::fConstructPath( xboxGameFolder, tFilePathPtr( "force" ) );
		const b32 forceFileExists = FileSystem::fFileExists( forceFilePath );

//const tFilePathPtr cleanFilePath = tFilePathPtr::fConstructPath( xboxGameFolder, tFilePathPtr( "clean" ) );
//if( !forceFileExists && FileSystem::fFileExists( cleanFilePath ) )
//{
//	log_line( 0, "Copying resources to devkit is up to date, nothing to do." );
//	return true; // copying is up to date, nothing to do
//}

		// copy xbox resources by spawning xbcp process

		const tFilePathPtr xbcpPath = tFilePathPtr::fConstructPath( mXdkBinPath, tFilePathPtr( "xbcp.exe" ) );
		const std::string copyIfMoreRecentOption = ( forceFileExists ? "" : " /d" );
		const std::string  args = "/r /y" + copyIfMoreRecentOption + " /e /t *.* devkit:\\" + curProjName;
		const tFilePathPtr startDir = ToolsPaths::fGetCurrentProjectGamePlatformFolder( cPlatformXbox360 );

		Win32Util::tTemporaryCurrentDirectoryChange changeDir( startDir.fCStr( ) );
		const std::string systemArg = std::string( "call \"" ) + xbcpPath.fCStr( ) + "\" /x " + mDevkitName + " " + args;
		log_line( 0, "Copying resources to devkit..." );
		if( system( systemArg.c_str( ) ) != 0 )
		{
			log_warning( 0, "Error spawning [xbcp.exe] process." );
			return false;
		}

//FileSystem::fWriteBufferToFile( tDynamicBuffer( ), cleanFilePath );
		FileSystem::fDeleteFile( forceFilePath );

		return true;
	}

	b32 tXbDm::fCopyFilesToDevkitFolder( const tFilePathPtrList& src, const tFilePathPtr& xbSubDirDestSimple, b32 forceCopy ) const
	{
		if( mXdkBinPath.fLength( ) == 0 )
			return false; // xdk is probably not installed
		if( !fIsConnectedToDevkit( ) )
			return false;
		if( src.fCount( ) == 0 )
			return true;

		const tFilePathPtr xbSubDirDest = tFilePathPtr::fConstructPath( tFilePathPtr( "devkit:" ), xbSubDirDestSimple );

		// copy xbox resources by spawning xbcp process

		const tFilePathPtr xbcpPath = tFilePathPtr::fConstructPath( mXdkBinPath, tFilePathPtr( "xbcp.exe" ) );
		const std::string copyIfMoreRecentOption = ( forceCopy ? "" : " /d" );

		std::stringstream argsSs;
		argsSs << "/y" << copyIfMoreRecentOption << " /e /t";
		for( u32 i = 0; i < src.fCount( ); ++i )
			argsSs << " \"" << src[ i ].fCStr( ) << "\"";
		argsSs << " " << xbSubDirDest.fCStr( );
		const std::string  args = argsSs.str( );

		const std::string systemArg = std::string( "call \"" ) + xbcpPath.fCStr( ) + "\" /x " + mDevkitName + " " + args;
		if( system( systemArg.c_str( ) ) != 0 )
		{
			log_warning( 0, "Error spawning [xbcp.exe] process." );
			return false;
		}

		return true;
	}

	b32 tXbDm::fLaunchXex( const tFilePathPtr& xexFullPath, const std::string& userArgs, b32 copyOnly, b32 minimal ) const
	{
		if( mXdkBinPath.fLength( ) == 0 )
			return false; // xdk is probably not installed
		if( !fIsConnectedToDevkit( ) )
			return false;

		const tFilePathPtr pdbFullPath = tFilePathPtr::fSwapExtension( xexFullPath, ".pdb" );
		const tFilePathPtr xdbFullPath = tFilePathPtr::fSwapExtension( xexFullPath, ".xdb" );
		const tFilePathPtr exeFullPath = tFilePathPtr::fSwapExtension( xexFullPath, ".exe" );

		const std::string curProjName = ToolsPaths::fGetCurrentProjectName( );
		const tFilePathPtr xexSimplePath = tFilePathPtr( StringUtil::fNameFromPath( xexFullPath.fCStr( ) ) );
		const tFilePathPtr xexOnXboxPath = tFilePathPtr::fConstructPath( tFilePathPtr( "devkit:" ), tFilePathPtr( curProjName ), xexSimplePath );

		// copy xex to dest folder
		tFilePathPtrList neededFiles;
		neededFiles.fPushBack( xexFullPath );
		if( !minimal )
		{
			neededFiles.fPushBack( pdbFullPath );
			neededFiles.fPushBack( xdbFullPath );
			neededFiles.fPushBack( exeFullPath );
		}
		log_line( 0, "Copying xex to devkit..." );
		fCopyFilesToDevkitFolder( neededFiles, tFilePathPtr( curProjName ), !copyOnly );

		// call xbreboot on specified xex
		if( !copyOnly )
		{
			const tFilePathPtr xbrebootPath = tFilePathPtr::fConstructPath( mXdkBinPath, tFilePathPtr( "xbreboot.exe" ) );
			std::string  args = "\"" + std::string( xexOnXboxPath.fCStr( ) ) + "\"";
			if( userArgs.length( ) > 0 )
				args += " \"" + userArgs + "\"";
			const tFilePathPtr startDir = ToolsPaths::fGetCurrentProjectGamePlatformFolder( cPlatformXbox360 );

			Win32Util::tTemporaryCurrentDirectoryChange changeDir( startDir.fCStr( ) );
			const std::string systemArg = std::string( "call \"" ) + xbrebootPath.fCStr( ) + "\" /x " + mDevkitName + " " + args;
			log_line( 0, "Launching Game..." );
			if( system( systemArg.c_str( ) ) != 0 )
			{
				log_warning( 0, "Error spawning [xbreboot.exe] process." );
				return false;
			}
		}

		return true;
	}

	b32 tXbDm::fLaunchWatson( ) const
	{
		if( mXdkBinPath.fLength( ) == 0 )
			return false; // xdk is probably not installed
		if( !fIsConnectedToDevkit( ) )
			return false;

		if( Win32Util::fIsProcessRunning( "xbwatson.exe" ) )
			return true;

		const tFilePathPtr watsonPath = tFilePathPtr::fConstructPath( /*mXdkBinPath*/ToolsPaths::fGetEngineBinFolder( ), tFilePathPtr( "xbwatson.exe" ) );
		const tFilePathPtr startDir = ToolsPaths::fGetCurrentProjectGamePlatformFolder( cPlatformXbox360 );

		log_line( 0, "Launching xbWatson..." );
		if( !Threads::tProcess::fSpawnAndForget( watsonPath.fCStr( ), 0, startDir.fCStr( ) ) )
		{
			log_warning( 0, "Error spawning [xbWatson.exe] process." );
			return false;
		}

		return true;
	}

}

