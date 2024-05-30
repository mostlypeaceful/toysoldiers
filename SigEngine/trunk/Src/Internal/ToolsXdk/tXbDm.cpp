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
			log_warning( "Couldn't load xbdm.dll." );
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
			log_warning( "Could not connect to default xbox360 devkit (either it's not connected, or else no devkit name was set)." );
		}
		else
		{
			// store devkit name
			mDevkitName = devkitName;
			log_line( 0, "Connection succeeded to devkit [" << mDevkitName << "]." );
			fEnsureDirExists( tFilePathPtr( std::string( "devkit:\\" ) + ToolsPaths::fGetCurrentProjectDeployTo( ) ) );
		}

		// XBDM.DLL loaded.  Return true for success.
		return true;
	}

	void tXbDm::fSetDevkitName( const std::string& name )
	{
		mDevkitName = name;
		log_line( 0, "Changing connection to devkit [" << mDevkitName << "]." );
		fEnsureDirExists( tFilePathPtr( std::string( "devkit:\\" ) + ToolsPaths::fGetCurrentProjectDeployTo( ) ) );
	}

	b32 tXbDm::fEnsureDirExists( const tFilePathPtr& xbSubDirDest ) const
	{
		//const tFilePathPtr xbmkdirPath = tFilePathPtr::fConstructPath( mXdkBinPath, tFilePathPtr( "xbmkdir.exe" ) );
		//const std::string systemArg = std::string( "call \"" ) + std::string(xbmkdirPath.fCStr( )) + "\" /x " + mDevkitName + " " + xbSubDirDest.fCStr( );
		//if( system( systemArg.c_str( ) ) != 0 )
		const HRESULT res = DmMkdir( xbSubDirDest.fCStr( ) );
		if( res != XBDM_NOERR && res != XBDM_ALREADYEXISTS )
		{
			log_warning( "Error creating directory: " << xbSubDirDest );
			return false;
		}
		return true;
	}

	void tXbDm::fListDevkitFiles( tFilePathPtr root, tFilePathPtrList& fileList, b32 recursive ) const
	{
		PDM_WALK_DIR pWalkDir = NULL;
		DM_FILE_ATTRIBUTES fileAttr;
		while( true ) 
		{
			if( DmWalkDir( &pWalkDir, root.fCStr( ), &fileAttr ) != XBDM_NOERR )
				break;

			tFilePathPtr path = tFilePathPtr::fConstructPath( tFilePathPtr( root ), tFilePathPtr( fileAttr.Name ) );

			if( fileAttr.Attributes & FILE_ATTRIBUTE_DIRECTORY )
				fListDevkitFiles( path, fileList, true );
			else
				fileList.fPushBack( path );
		}
	}

	b32 tXbDm::fCopyResourcesToDevkit( b32 noProgressCounter, b32 fullSync ) const
	{
		if( mXdkBinPath.fLength( ) == 0 )
			return false; // xdk is probably not installed
		if( !fIsConnectedToDevkit( ) )
			return false;

        std::string deployToPath = ToolsPaths::fGetCurrentProjectDeployTo( );

		tFilePathPtrList iniFiles;
		FileSystem::fGetFileNamesInFolder( iniFiles, ToolsPaths::fGetCurrentProjectResFolder( ), true, false, tFilePathPtr( ".ini" ) );

		// always attempt to copy *.ini files
		if( iniFiles.fCount( ) > 0 )
		{
			log_line( 0, "Copying *.ini files to devkit..." );
			fCopyFilesToDevkitFolder( iniFiles, tFilePathPtr( deployToPath ) );
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

		const tFilePathPtr xbcpPath = tFilePathPtr::fConstructPath( mXdkBinPath, tFilePathPtr( "xbcp.exe" ) );
		if( !fullSync )
		{
			// copy xbox resources by spawning xbcp process

			const std::string copyIfMoreRecentOption = ( forceFileExists ? "" : " /d" );
			const std::string  args = "/r /y" + copyIfMoreRecentOption + " /e /t *.* devkit:\\" + deployToPath;
			const tFilePathPtr startDir = ToolsPaths::fGetCurrentProjectGamePlatformFolder( cPlatformXbox360 );

			Win32Util::tTemporaryCurrentDirectoryChange changeDir( startDir.fCStr( ) );
			const std::string systemArg = std::string( "call \"" ) + xbcpPath.fCStr( ) + "\" /x " + mDevkitName + " " + args;
			log_line( 0, "Copying resources to devkit..." );
			if( system( systemArg.c_str( ) ) != 0 )
			{
				log_warning( "Error spawning [xbcp.exe] process." );
				return false;
			}
		}
		else
		{
			const tFilePathPtr xbcpPath = tFilePathPtr::fConstructPath( mXdkBinPath, tFilePathPtr( "xbcp.exe" ) );
			const tFilePathPtr xbdirPath = tFilePathPtr::fConstructPath( mXdkBinPath, tFilePathPtr( "xbdir.exe" ) );
			const tFilePathPtr startPcDir = ToolsPaths::fGetCurrentProjectGamePlatformFolder( cPlatformXbox360 );
			const tFilePathPtr startDevkitDir = tFilePathPtr::fConstructPath( tFilePathPtr( "devkit:\\" ), tFilePathPtr( deployToPath ) );
			const tFilePathPtr devkitGamePath = ToolsPaths::fGetCurrentProjectGamePlatformFolder( cPlatformXbox360 );

			tGrowableArray<tFilePathPtr> excludedFiles;
			excludedFiles.fPushBack( tFilePathPtr::fConstructPath( startDevkitDir, tFilePathPtr( "default.xex" ) ) );
			excludedFiles.fPushBack( tFilePathPtr::fConstructPath( startDevkitDir, tFilePathPtr( "default.pdb" ) ) );
			excludedFiles.fPushBack( tFilePathPtr::fConstructPath( startDevkitDir, tFilePathPtr( "default.xdb" ) ) );
			excludedFiles.fPushBack( tFilePathPtr::fConstructPath( startDevkitDir, tFilePathPtr( "default.exe" ) ) );
			excludedFiles.fPushBack( tFilePathPtr::fConstructPath( startDevkitDir, tFilePathPtr( "defaults.ini" ) ) );
			excludedFiles.fPushBack( tFilePathPtr::fConstructPath( startDevkitDir, tFilePathPtr( "user.ini" ) ) );

			tFilePathPtrList pcFileList;
			FileSystem::fGetFileNamesInFolder( pcFileList, devkitGamePath, true, true );

			// Delete files from the devkit when they're no longer present on the local PC
			log_line( 0, "Creating list of files on devkit..." );
			tFilePathPtrList devkitFileList;
			fListDevkitFiles( startDevkitDir, devkitFileList, true );
			log_line( 0, "Checking for files to remove from devkit..." );
			u32 i;
			for( i = 0; i < devkitFileList.fCount( ); ++i )
			{
				if( !noProgressCounter && ( i % 100 == 0 ) )
					fPrintPercentComplete( i, devkitFileList.fCount( ) );

				b32 excluded = false;
				for( u32 j = 0; j < excludedFiles.fCount( ); ++j )
				{
					if( devkitFileList[ i ] == excludedFiles[ j ] )
					{
						excluded = true;
						break;
					}
				}
				if( excluded )
					continue;

				// Take the devkit path and map it to the PC path
				std::string pcFilePath( devkitFileList[ i ].fCStr( ) );
				StringUtil::fReplaceAllOf( pcFilePath, startDevkitDir.fCStr( ), startPcDir.fCStr( ) );

				if( !FileSystem::fFileExists( tFilePathPtr( pcFilePath ) ) )
				{
					log_line( 0, "Deleting " << devkitFileList[ i ] << " from devkit..." );
					DmDeleteFile( devkitFileList[ i ].fCStr( ), false );
				}
			}
			if( !noProgressCounter )
			{
				fPrintPercentComplete( i, devkitFileList.fCount( ) );
				log_newline( );
			}

			log_line( 0, "Checking for files to add or update on devkit..." );
			for( i = 0; i < pcFileList.fCount( ); ++i )
			{
				if( !noProgressCounter && ( i % 100 == 0 ) )
					fPrintPercentComplete( i, pcFileList.fCount( ) );

				// Take the PC path and map it to the devkit path
				std::string devkitFilePath( pcFileList[ i ].fCStr( ) );
				StringUtil::fReplaceAllOf( devkitFilePath, startPcDir.fCStr( ), startDevkitDir.fCStr( ) );

				DM_FILE_ATTRIBUTES devkitFileAttribs;
				ZeroMemory( &devkitFileAttribs, sizeof( DM_FILE_ATTRIBUTES ) );
				DmGetFileAttributes( devkitFilePath.c_str( ), &devkitFileAttribs );
				u64 devkitFileSize = devkitFileAttribs.SizeHigh;
				devkitFileSize <<= 32;
				devkitFileSize |= devkitFileAttribs.SizeLow;

				u32 pcFileSize = FileSystem::fGetFileSize( pcFileList[ i ] );
				u64 pcModTime = FileSystem::fGetLastModifiedTimeStamp( pcFileList[ i ] );

				// If the size isn't the same, we already know we need to copy
				b32 differ = true;
				if( devkitFileSize == pcFileSize )
				{
					// We need to use 64-bit integers to do the arithmetic on file timestamps
					ULARGE_INTEGER pcTime;
					memcpy( &pcTime.QuadPart, &pcModTime, sizeof( pcModTime ) );

					ULARGE_INTEGER devkitTime;
					memcpy( &devkitTime.QuadPart, &devkitFileAttribs.ChangeTime, sizeof( devkitFileAttribs.ChangeTime ) );

					// Convert the difference between the two timestamps from 100-nanosecond slices to milliseconds
					ULONGLONG diffHundredNs = devkitTime.QuadPart > pcTime.QuadPart ? devkitTime.QuadPart - pcTime.QuadPart : pcTime.QuadPart - devkitTime.QuadPart;
					double diffMs = diffHundredNs * 0.0001;

					// The last modification time is accurate to two seconds
					if( diffMs < 2000 )
						differ = false; // Both size and time match
				}
				if( differ || forceFileExists )
				{
					const std::string systemArg = std::string( "call \"" ) + xbcpPath.fCStr( ) + "\" /x " + mDevkitName + " /r /y /e /t \"" + pcFileList[ i ].fCStr( ) + "\" \"" + devkitFilePath + "\"";
					if( system( systemArg.c_str( ) ) != 0 )
					{
						log_warning( "Error spawning [xbcp.exe] process." );
						return false;
					}
				}
			}
			if( !noProgressCounter )
			{
				fPrintPercentComplete( i, pcFileList.fCount( ) );
				log_newline( );
			}
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
			log_warning( "Error spawning [xbcp.exe] process." );
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

        std::string deployToDir = ToolsPaths::fGetCurrentProjectDeployTo( );

		const tFilePathPtr xexSimplePath = tFilePathPtr( StringUtil::fNameFromPath( xexFullPath.fCStr( ) ) );
		const tFilePathPtr xexOnXboxPath = tFilePathPtr::fConstructPath( tFilePathPtr( "devkit:" ), tFilePathPtr( deployToDir ), xexSimplePath );

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
		fCopyFilesToDevkitFolder( neededFiles, tFilePathPtr( deployToDir ), !copyOnly );

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
				log_warning( "Error spawning [xbreboot.exe] process." );
				return false;
			}
		}

		return true;
	}
}

