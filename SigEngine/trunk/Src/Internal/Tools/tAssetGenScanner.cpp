#include "ToolsPch.hpp"
#include "tAssetGenScanner.hpp"
#include "tAssetPluginDll.hpp"
#include "FileSystem.hpp"
#include "tFileWriter.hpp"
#include "Win32Util.hpp"
#include "tWin32Window.hpp"
#include "ToolsPaths.hpp"
#include "Sacml.hpp"
#include "Threads/tThread.hpp"
#include "Threads/tProcess.hpp"
#include "wx/progdlg.h"
#include "iAssetGenPlugin.hpp"

namespace Sig
{

	namespace
	{
		static b32 fIsSupportedPlatform( tPlatformId pid )
		{
			if( pid == cPlatformXbox360 )
				return true;
			if( pid == cPlatformPcDx9 )
				return true;
			return false;
		}
	}

	///
	/// \section tInputFileListHandler
	///

	class tInputFileListHandler
	{
		tAssetGenScanner&				mScanner;
		const tAssetGenOptions&			mOpts;
		const tFilePathPtrList&			mFilesInCurDir;
		mutable tFilesProcessed&		mFilesAlreadyProcessed;
		mutable tFilePathPtrList&		mIndirectGenFiles;
		b32								mReconOnly;
		mutable tFilePathPtrList		mOriginalInputs;
		mutable tFilePathPtrList		mOutputs;

	public:

		tInputFileListHandler( 
			tAssetGenScanner& scanner,
			const tAssetGenOptions& opts, 
			const tFilePathPtrList& filesInCurDir, 
			tFilesProcessed& filesAlreadyProcessed,
			tFilePathPtrList& indirectGenFiles,
			b32 reconOnly = false );
		b32 operator()( const tAssetPluginDllPtr& dllPtr, iAssetPlugin& assetPlugin ) const;
		void fDeleteOutputsWithMissingDependencies( );

		const tFilePathPtrList& fOriginalInputs( ) const { return mOriginalInputs; }

	private:

		b32 fVerifyInputs( iAssetGenPlugin::tInputOutput& inOut ) const;
		b32 fVerifyOutput( iAssetGenPlugin::tInputOutput& inOut ) const;
		b32 fTrackProcessed( iAssetGenPlugin::tInputOutput& inOut ) const;
		b32 fComputeUpToDate( iAssetGenPlugin::tInputOutput& inOut ) const;

	};

	tInputFileListHandler::tInputFileListHandler( 
		tAssetGenScanner& scanner,
		const tAssetGenOptions& opts, 
		const tFilePathPtrList& filesInCurDir,
		tFilesProcessed& filesAlreadyProcessed,
		tFilePathPtrList& indirectGenFiles,
		b32 reconOnly )
		: mScanner( scanner )
		, mOpts( opts )
		, mFilesInCurDir( filesInCurDir )
		, mFilesAlreadyProcessed( filesAlreadyProcessed )
		, mIndirectGenFiles( indirectGenFiles )
		, mReconOnly( reconOnly )
	{
	}

	void tInputFileListHandler::fDeleteOutputsWithMissingDependencies( )
	{
		if( mOutputs.fCount( ) != 1 )
			return; // can't be sure about dependencies

		b32 dependsFileMissing = false;
		for( u32 i = 0; i < mIndirectGenFiles.fCount( ); ++i )
		{
			if( !FileSystem::fFileExists( ToolsPaths::fMakeResAbsolute( mIndirectGenFiles[ i ] ) ) ||
				ToolsPaths::fIsUnderSourceFolder( mIndirectGenFiles[ i ] ) )
			{
				dependsFileMissing = true;
				log_warning( "Dependency file [" << mIndirectGenFiles[ i ] << "] does not exist." );
				mIndirectGenFiles.fErase( i );
				--i;
			}
		}
		if( dependsFileMissing )
			ToolsPaths::fDeleteGameFileForAllPlatforms( mOutputs.fFront( ) );
	}

	b32 tInputFileListHandler::operator()( const tAssetPluginDllPtr& dllPtr, iAssetPlugin& assetPlugin ) const
	{
		iAssetGenPlugin* agp = assetPlugin.fGetAssetGenPluginInterface( );
		if( !agp )
			return true;

		// determine list of inputs/outputs
		iAssetGenPlugin::tInputOutputList inputsOutputs;
		agp->fDetermineInputOutputs( inputsOutputs, mFilesInCurDir );

		// go through input/output list and remove any output files that we've already processed;
		// similarly, check if any of the input files are more recent than the output file
		for( u32 i = 0; i < inputsOutputs.fCount( ); ++i )
		{
			b32 remove = false;
			if( fTrackProcessed( inputsOutputs[ i ] ) )
			{
				// file is in 'already' processed list, remove from processing
				log_line( 0, "-> Skipping file [" << inputsOutputs[ i ].mOutput << "] because it has already been processed..." );
				remove = true;
			}
			else if( !fVerifyOutput( inputsOutputs[ i ] ) )
			{
				// something is wrong with the output path; warnings will be issued inside fVerifyOutput
				remove = true;
			}
			else if( !fVerifyInputs( inputsOutputs[ i ] ) )
			{
				// certain input files do not exist, so we can't process the file; warnings will be issued inside fVerifyInputs
				remove = true;
			}
			else if( fComputeUpToDate( inputsOutputs[ i ] ) )
			{
				// file is up to date, remove from processing
				log_line( 0, "-> Skipping file [" << inputsOutputs[ i ].mOutput << "] because it is up-to-date..." );
				remove = true;
			}

			if( remove )
			{
				inputsOutputs.fErase( i );
				--i;
			}
		}

		// convert the files being processed
		for( u32 i = 0; i < inputsOutputs.fCount( ); ++i )
		{
			const tGrowableArray<tPlatformId>& platformsToConvert = inputsOutputs[ i ].mPlatformsToConvert;
			if( platformsToConvert.fCount( ) == 0 )
				continue;

			if( mReconOnly )
			{
				sigassert( inputsOutputs[ i ].mOriginalInput.fLength( ) > 0 );
				mOriginalInputs.fPushBack( inputsOutputs[ i ].mOriginalInput );
			}
			else
			{
				// log the current file being output
				if( !mOpts.mWorkerProc ) // if we're a worker proc, then the main proc already issued a log about processing this file, so don't clutter it all up
					log_line( 0, "-> Converting file [" << inputsOutputs[ i ].mOutput << "]..." );

				// now send the inputs/output back through the plugin
				agp->fProcessInputOutputs( inputsOutputs[ i ], mIndirectGenFiles );
				if ( Log::Detail::tWarning::fWarningCount( ) > 0 )
				{
					log_line( 0, "!ERROR! Not converting file [" << inputsOutputs[ i ].mOutput << "] due to warnings..." );
					Log::Detail::tWarning::fResetErrors( );
					ToolsPaths::fDeleteGameFileForAllPlatforms( inputsOutputs[ i ].mOutput );
				}
				else
					mOutputs.fPushBack( inputsOutputs[ i ].mOutput );
			}

			for( u32 j = 0; j < platformsToConvert.fCount( ); ++j )
				mScanner.fMarkPlatformDirty( platformsToConvert[ j ] );
		}

		return true;
	}

	b32 tInputFileListHandler::fVerifyInputs( iAssetGenPlugin::tInputOutput& inOut ) const
	{
		b32 success = true;
		for( u32 i = 0; i < inOut.mInputs.fCount( ); ++i )
		{
			if( !FileSystem::fFileExists( inOut.mInputs[ i ] ) )
			{
				log_warning( "Input file [" << ToolsPaths::fMakeResRelative( inOut.mInputs[ i ] ) << "] required by output file [" << inOut.mOutput << "] does not exist." );
				success = false;
			}
		}

		return success;
	}

	b32 tInputFileListHandler::fVerifyOutput( iAssetGenPlugin::tInputOutput& inOut ) const
	{
		std::string simpleFileName = StringUtil::fNameFromPath( inOut.mOutput.fCStr( ) );

		const u32 cMaxCharsSimple = 40;

		if( simpleFileName.length( ) > cMaxCharsSimple )
		{
			log_warning( "The file name [" << inOut.mOutput << "] is too long (" << simpleFileName.length( ) << ") - maximum of " << cMaxCharsSimple << " is allowed." );
			return false;
		}

		return true;
	}

	b32 tInputFileListHandler::fTrackProcessed( iAssetGenPlugin::tInputOutput& inOut ) const
	{
		if( mFilesAlreadyProcessed.fFind( inOut.mOutput ) )
			return true; // file has already been processed

		// track this input/output
		mFilesAlreadyProcessed.fInsert( inOut.mOutput, inOut.mOutput );

		// file has not yet been processed
		return false;
	}

	b32 tInputFileListHandler::fComputeUpToDate( iAssetGenPlugin::tInputOutput& inOut ) const
	{
		// by default, clear all platforms to convert
		inOut.mPlatformsToConvert.fSetCount( 0 );

		// the output file paths are specified as relative to res; we need to go through
		// each requested output platform, convert the relative output path to an absolute
		// path into the game folder for that platform, and then compute whether it's up
		// to date

		for( tPlatformIdIterator pid; !pid.fDone( ); pid.fNext( ) )
		{
			if( !fIsSupportedPlatform( pid ) )
				continue;

			if( !( mOpts.mPlatforms & fPlatformIdFlag( pid ) ) )
			{
				// user didn't want to build for this platform, so
				// we don't add it to the list (i.e., skip it)
				continue;
			}

			if( mOpts.mForce )
			{
				// user specified to force the build, so don't even
				// bother with checking whether it's up to date or not,
				// just add it to the list of platforms to build
				inOut.mPlatformsToConvert.fPushBack( pid );
			}
			else
			{
				const tFilePathPtr absoluteOutputFile = iAssetGenPlugin::fCreateAbsoluteOutputPath( pid, inOut.mOutput );
				if( FileSystem::fAreAnyInAMoreRecentThanB( inOut.mInputs, absoluteOutputFile ) )
				{
					// at least one of the input files is more recent than the output file;
					// this means we need to convert the output file for this platform
					inOut.mPlatformsToConvert.fPushBack( pid );
				}
			}
		}

		return inOut.mPlatformsToConvert.fCount( ) == 0; // return whether to skip the file or not
	}


	///
	/// \section tAssetGenScanner
	///

	tAssetGenScanner::tAssetGenScanner( const tAssetGenOptions& opts )
		: mOpts( opts )
		, mDirtyPlatforms( 0 )
	{
	}

	tAssetGenScanner::~tAssetGenScanner( )
	{
		if( fIsPlatformDirty( cPlatformXbox360 ) )
		{
			const tFilePathPtr xboxGameFolder = ToolsPaths::fGetCurrentProjectGamePlatformFolder( cPlatformXbox360 );
			const tFilePathPtr cleanFilePath = tFilePathPtr::fConstructPath( xboxGameFolder, tFilePathPtr( "clean" ) );
			FileSystem::fDeleteFile( cleanFilePath );
		}

		for( u32 i = cPlatformFirst; i < cPlatformLastPlusOne; ++i )
		{
			if( fIsPlatformDirty( ( tPlatformId )i ) )
			{
				const tFilePathPtr platformGameFolder = ToolsPaths::fGetCurrentProjectGamePlatformFolder( ( tPlatformId )i );
				const tFilePathPtr buildStampFilePath = tFilePathPtr::fConstructPath( platformGameFolder, tFilePathPtr( "zzz.buildstamp" ) );
				FileSystem::fWriteBufferToFile( tDynamicBuffer( ), buildStampFilePath );
			}
		}

		if( mOpts.mGame2Xbox )
		{
			if( fIsPlatformDirty( cPlatformXbox360 ) )
			{
				std::string cmd = "game2xbox.cmd -quiet";
				if( mOpts.mFullSync )
					cmd += " -fullsync -noprogress";
				system( cmd.c_str( ) );
			}
		}
	}

	void tAssetGenScanner::fHandleCurrentFolder( tAssetPluginDllDepot& pluginDepot )
	{
		if( mOpts.mWorkerProc )
			mInputsFromFolders.fPushBack( tFilePathPtr( mOpts.mFile ) );
		else
		{
			fAcquireAllInputsFromFolderRecursive( );
			fCullByExtFilter( mOpts, mInputsFromFolders );
		}

		if( mInputsFromFolders.fCount( ) > 0 )
			fProcessFileList( pluginDepot, mInputsFromFolders );

		if( mOpts.mGenPackFiles )
			fGeneratePackFiles( );
	}

	void tAssetGenScanner::fAcquireAllInputsFromFolderRecursive( )
	{
		// save off the current directory
		std::string curDir;
		Win32Util::fGetCurrentDirectory( curDir );

		// verify that this folder is a sub-folder of res
		tFilePathPtr curDirRelativeToRes = ToolsPaths::fMakeResRelative( tFilePathPtr( curDir.c_str( ) ), true );
		if( curDirRelativeToRes.fNull( ) )
		{
			log_warning( "The current directory [" << curDir << "] is not a subfolder of the current project's Res folder; aborting." );
			return;
		}

		// add res back onto the front to make it more legible for outputting
		curDirRelativeToRes = tFilePathPtr::fConstructPath( tFilePathPtr( "Res" ), curDirRelativeToRes );

		// verify that the folder doesn't have the ignore tag
		if( ToolsPaths::fIsUnderSourceFolder( curDirRelativeToRes ) )
		{
			// no reason to warn, this is a perfectly normal situation; we just don't want to process
			// anything more in this folder or any sub-folders of this folder.
			return;
		}

		const u32 oldSize = mInputsFromFolders.fCount( );
		if( mOpts.mFile.length( ) == 0 )
		{
			// gather all the files in the immediate directory (not recursive)
			FileSystem::fGetFileNamesInFolder(
				mInputsFromFolders, tFilePathPtr( curDir.c_str( ) ), true, false );
		}
		else
		{
			const tFilePathPtr inputFile = tFilePathPtr::fConstructPath( 
					tFilePathPtr( curDir.c_str( ) ),
					tFilePathPtr( mOpts.mFile.c_str( ) ) );

			if( FileSystem::fFileExists( inputFile ) )
			{
				// add the specific option file instead 
				// of getting the files in the current folder
				mInputsFromFolders.fPushBack( inputFile );
			}
		}

		const u32 numAdded = mInputsFromFolders.fCount( ) - oldSize;
		log_line( 0, ">> Scanning folder [" << curDirRelativeToRes << "] - added " << numAdded << " files to be processed." );

		if( mOpts.mRecursive )
		{
			// get all the immediate sub folders
			tFilePathPtrList subFolders;
			FileSystem::fGetFolderNamesInFolder(
				subFolders, tFilePathPtr( curDir.c_str( ) ), true, false );

			// now process all the sub folders
			for( u32 iSubFolder = 0; iSubFolder < subFolders.fCount( ); ++iSubFolder )
			{
				// set current directory to sub folder
				Win32Util::tTemporaryCurrentDirectoryChange changeCurDirToSubFolder( subFolders[iSubFolder].fCStr( ) );

				// recurse
				fAcquireAllInputsFromFolderRecursive( );
			}
		}
	}

	void tAssetGenScanner::fCullByExtFilter( const tAssetGenOptions& opts, tFilePathPtrList& files )
	{
		// No extensions
		const u32 cExtensionCount = opts.mExt.fCount( );
		if( !cExtensionCount )
			return;
	
		// if the user specified any extension filters, then cull files lacking all of those extensions
		for( s32 f = files.fCount( ) - 1; f >= 0; --f )
		{
			b32 found = false;

			// Try to find the file's extension in the list
			for( u32 e = 0; !found && e < cExtensionCount; ++e )
				found = StringUtil::fCheckExtension( files[ f ].fCStr( ), opts.mExt[ e ].c_str( ) );

			// If it wasn't found, erase the file from processing
			if( !found )
				files.fEraseOrdered( f );
		}
	}

	namespace
	{
		static tGrowableArray<std::string> gBufferedLogs;
		static void fWorkerLogCapture( const char* text, u32 flag )
		{
			gBufferedLogs.fPushBack( text );
		}
	}

	void tAssetGenScanner::fProcessFileList( tAssetPluginDllDepot& pluginDepot, const tFilePathPtrList& files )
	{
		Win32Util::tTemporaryCurrentDirectoryChange changeCurDirToRes( ToolsPaths::fGetCurrentProjectResFolder( ).fCStr( ) );

		tFilePathPtrList indirectGenFiles;

		if( mOpts.mWorkerProc )
			Log::fAddOutputFunction( fWorkerLogCapture );

		// process all the files in the specified list by iterating over all the registered plugins
		tInputFileListHandler pluginFileHandler( *this, mOpts, files, mOutputFilesProcessed, indirectGenFiles, mOpts.mMultiProcMaster );
		pluginDepot.fForEachPlugin( pluginFileHandler );
		pluginFileHandler.fDeleteOutputsWithMissingDependencies( );

		if( mOpts.mWorkerProc )
		{
			Log::fRemoveOutputFunction( fWorkerLogCapture );
			fWriteProcessDependencies( mOpts.mFile, indirectGenFiles, gBufferedLogs );
			return;
		}

		if( mOpts.mMultiProcMaster )
			fDelegateWork( pluginFileHandler.fOriginalInputs( ), indirectGenFiles );

		sigassert( !mOpts.mWorkerProc );

		if( !mOpts.mDisableDependencyFiles )
		{
			fCullByExtFilter( mOpts, indirectGenFiles );
			if( indirectGenFiles.fCount( ) > 0 )
				fProcessFileList( pluginDepot, indirectGenFiles );
		}
	}

	void tAssetGenScanner::fDelegateWork( const tFilePathPtrList& inputs, tFilePathPtrList& indirectGenFiles )
	{
		if( inputs.fCount( ) == 0 )
			return; // no work to be done

		u32 ithInFile = 0, ithOutFile = 0;
		const u32 numProcs = fMin( inputs.fCount( ), Threads::tThread::fHardwareThreadCount( ) );
		tDynamicArray< Threads::tProcessPtr > procs( numProcs );

		// start initial procs
		for( ; ithInFile < numProcs; ++ithInFile )
			procs[ ithInFile ].fReset( fSpawnWorkerProcess( inputs[ ithInFile ], ithInFile ) );

		// track procs as they finish and add more
		for( u32 icounter = 0; true; ++icounter )
		{
			const u32 ithProc = icounter % numProcs;
			if( !procs[ ithProc ] )
				continue;

			if( procs[ ithProc ]->fIsRunning( ) )
				fSleep( 1 );
			else
			{
				fReadProcessDependencies( *procs[ ithProc ], indirectGenFiles );
				++ithOutFile;

				if( ithOutFile == inputs.fCount( ) )
				{
					for( u32 i = 0; i < procs.fCount( ); ++i )
						sigassert( !procs[ i ] || !procs[ i ]->fIsRunning( ) );
					break;
				}

				if( ithInFile < inputs.fCount( ) )
				{
					procs[ ithProc ].fReset( fSpawnWorkerProcess( inputs[ ithInFile ], ithProc ) );
					++ithInFile;
				}
				else
					procs[ ithProc ].fRelease( ); // no work for proc to do, release it
			}
		}
	}

	Threads::tProcess* tAssetGenScanner::fSpawnWorkerProcess( const tFilePathPtr& inputFile, u32 ithProc )
	{
		const tFilePathPtr resRelativePath = ToolsPaths::fMakeResRelative( inputFile );
		log_line( 0, "-> Spawning process [" << ithProc << "] for file [" << resRelativePath << "]." );

		const char* assetGenName =
#ifdef _DEBUG
			"\\AssetGend.exe";
#else
			"\\AssetGen.exe";
#endif
		const std::string assetGenPath = std::string( ToolsPaths::fGetEngineBinFolder( ).fCStr( ) ) + assetGenName;

		std::stringstream cmdLine;
		cmdLine << "-workerproc -file " << inputFile.fCStr( );
		if( mOpts.mForce )
			cmdLine << " -force";
		if( mOpts.mPlatforms & fPlatformIdFlag( cPlatformWii ) )
			cmdLine << " -wii";
		if( mOpts.mPlatforms & fPlatformIdFlag( cPlatformPcDx9 ) )
			cmdLine << " -pcdx9";
		if( mOpts.mPlatforms & fPlatformIdFlag( cPlatformPcDx10 ) )
			cmdLine << " -pcdx10";
		if( mOpts.mPlatforms & fPlatformIdFlag( cPlatformXbox360 ) )
			cmdLine << " -xbox360";
		if( mOpts.mPlatforms & fPlatformIdFlag( cPlatformPs3Ppu ) )
			cmdLine << " -ps3ppu";
		return new Threads::tProcess( assetGenPath.c_str( ), cmdLine.str( ).c_str( ), 0 );
	}

	void tAssetGenScanner::fReadProcessDependencies( Threads::tProcess& proc, tFilePathPtrList& indirectGenFiles )
	{
		std::stringstream ss; ss << "depends" << proc.fProcessId( );
		const tFilePathPtr dependsPath = ToolsPaths::fCreateTempEngineFilePath( ".txt", tFilePathPtr( "assetgen" ), ss.str( ).c_str( ) );
		tDynamicBuffer depends;
		if( FileSystem::fReadFileToBuffer( depends, dependsPath, "" ) )
		{
			char* start = ( char* )depends.fBegin( );

			const u32* numFiles = ( const u32* )start; start += sizeof( u32 );
			const u32* numLogs = ( const u32* )start; start += sizeof( u32 );

			tFilePathPtrList dependsFiles;
			std::string outputHeader;
			b32 outputHeaderWritten = false;

			char* end = strstr( start, "\n" );
			while( start && *start )
			{
				sigassert( end );
				*end = '\0';

				if( outputHeader.length( ) == 0 )
					outputHeader = start;
				else if( dependsFiles.fCount( ) < *numFiles )
					dependsFiles.fPushBack( tFilePathPtr( start ) );
				else
				{
					if( !outputHeaderWritten )
					{
						log_line( 0, outputHeader );
						outputHeaderWritten = true;
					}
					log_line( 0, start );
				}

				start = end + 1;
				end = strstr( start, "\n" );
			}

			for( u32 i = 0; i < dependsFiles.fCount( ); ++i )
			{
				if(  FileSystem::fFileExists( ToolsPaths::fMakeResAbsolute( dependsFiles[ i ] ) ) && 
					!ToolsPaths::fIsUnderSourceFolder( dependsFiles[ i ] ) )
					indirectGenFiles.fPushBack( dependsFiles[ i ] );
				else
				{
					if( !outputHeaderWritten )
					{
						log_line( 0, outputHeader );
						outputHeaderWritten = true;
					}
					log_warning( "Dependency file [" << dependsFiles[ i ] << "] does not exist, or is under a ~src folder." );
					dependsFiles.fErase( i );
					--i;
				}
			}

			FileSystem::fDeleteFile( dependsPath );
		}
	}

	void tAssetGenScanner::fWriteProcessDependencies( const std::string& file, const tFilePathPtrList& dependFiles, const tGrowableArray<std::string>& bufferedLogs )
	{
		if( dependFiles.fCount( ) == 0 && bufferedLogs.fCount( ) == 0 )
			return;
		std::stringstream ss; ss << "depends" << GetCurrentProcessId( );
		const tFilePathPtr dependsPath = ToolsPaths::fCreateTempEngineFilePath( ".txt", tFilePathPtr( "assetgen" ), ss.str( ).c_str( ) );

		const u32 numFiles = dependFiles.fCount( );
		const u32 numLogs = bufferedLogs.fCount( );

		tFileWriter depends = tFileWriter( dependsPath );
		depends( &numFiles, sizeof( numFiles ) );
		depends( &numLogs, sizeof( numLogs ) );
		// write output header 
		{
			std::stringstream ss; ss << "Output from file [" << ToolsPaths::fMakeResRelative( tFilePathPtr( file ) ).fCStr( ) << "]:" << std::endl;
			std::string logHeader = ss.str( );
			depends( logHeader.c_str( ), logHeader.length( ) );
		}
		for( u32 i = 0; i < dependFiles.fCount( ); ++i )
		{
			depends( dependFiles[ i ].fCStr( ), dependFiles[ i ].fLength( ) );
			depends( "\n", strlen( "\n" ) );
		}
		for( u32 i = 0; i < bufferedLogs.fCount( ); ++i )
			depends( bufferedLogs[ i ].c_str( ), bufferedLogs[ i ].length( ) );
	}

	void tAssetGenScanner::fGeneratePackFiles( )
	{
		mDirtyPlatforms |= mOpts.mPlatforms;

		// get all sacml paths (recursively, everywhere under res)
		tFilePathPtrList sacmlPaths;

		if( mOpts.mFile.length( ) > 0 )
		{
			std::string curDir;
			Win32Util::fGetCurrentDirectory( curDir );

			const tFilePathPtr inputFile = tFilePathPtr::fConstructPath( 
					tFilePathPtr( curDir.c_str( ) ),
					tFilePathPtr( mOpts.mFile.c_str( ) ) );

			if( FileSystem::fFileExists( inputFile ) )
				sacmlPaths.fPushBack( inputFile );
		}
		else
		{
			FileSystem::fGetFileNamesInFolder(
				sacmlPaths, ToolsPaths::fGetCurrentProjectResFolder( ), true, true, tFilePathPtr( Sacml::fGetFileExtension( ) ) );
		}

		// filter out any paths that came from a folder with a leading ~
		for( u32 i = 0; i < sacmlPaths.fCount( ); ++i )
		{
			const std::string pathOnly = StringUtil::fDirectoryFromPath( sacmlPaths[ i ].fCStr( ) );
			if( ToolsPaths::fIsUnderSourceFolder( tFilePathPtr( pathOnly ) ) )
			{
				sacmlPaths.fErase( i );
				--i;
			}
		}

		if( sacmlPaths.fCount( ) == 0 )
			return; // no files to process

		log_line( 0, ">> Processing sac package files -> " << sacmlPaths.fCount( ) << " files found..." );

		if( sacmlPaths.fCount( ) > 1 && mOpts.mSacmlOutput.length( ) > 0 )
		{
			log_warning( "Multiple input .sacml files found, but a single output path was specified: " << mOpts.mSacmlOutput );
		}

		for( u32 i = 0; i < sacmlPaths.fCount( ); ++i )
		{
			Sacml::tFile sacml;
			if( !sacml.fLoadXml( sacmlPaths[ i ] ) )
				continue;
			log_line( 0, "-> Converting file [" << Sacml::fSacmlPathToSacb( ToolsPaths::fMakeResRelative( sacmlPaths[ i ] ) ) << "]..." );
			sacml.fGeneratePackFile( sacmlPaths[ i ], mOpts.mPlatforms, mOpts.mSacmlOutput );
		}
	}

	void tAssetGenScanner::fMarkPlatformDirty( tPlatformId pid )
	{
		mDirtyPlatforms |= fPlatformIdFlag( pid );
	}

	b32 tAssetGenScanner::fIsPlatformDirty( tPlatformId pid ) const
	{
		return mDirtyPlatforms & fPlatformIdFlag( pid );
	}

	namespace
	{
		class tools_export tAssetGenOutputLog : public tUncopyable
		{
		private:
			b32 mDisplayResults;
			b32 mWarned;
			wxProgressDialog* mProgressDialog;
			std::stringstream mLogBuffer;
		private:
			static tAssetGenOutputLog* gInstance;
			static void fLogReRoute( const char* text, u32 flag ) { if( gInstance ) gInstance->fOnLog( text ); }
			void fOnLog( const char* text )
			{
				const b32 isWarning = StringUtil::fStrStrI( text, "!WARNING!" ) != 0; 
				mWarned = mWarned || isWarning;
				mProgressDialog->Pulse( text );
				if( isWarning )
					mLogBuffer << text;
				tWin32Window::fMessagePump( );
			}
		public:
			explicit tAssetGenOutputLog( b32 displayResults )
				: mDisplayResults( displayResults )
				, mWarned( false )
				, mProgressDialog( 0 )
			{
				if( mDisplayResults )
				{
					gInstance = this;
					Log::fAddOutputFunction( fLogReRoute );
					mProgressDialog = new wxProgressDialog( "AssetGen Conversion", "", 100, 0, wxPD_APP_MODAL );
					mProgressDialog->SetSize( wxSize( 425, 125 ) );
					mProgressDialog->Center( );
				}
			}
			~tAssetGenOutputLog( )
			{
				if( mDisplayResults )
				{
					Log::fRemoveOutputFunction( fLogReRoute );
					delete mProgressDialog;
					gInstance = 0;
					if( mWarned )
					{
						const std::string result = mLogBuffer.str( );
						wxMessageBox( result.c_str( ), "AssetGen Warnings:" );
					}
				}
			}
		};
		tAssetGenOutputLog* tAssetGenOutputLog::gInstance=0;
	}


	void tAssetGenScanner::fProcessSingleFile( const tFilePathPtr& absolutePath, b32 displayResults, u32 platformFlags )
	{
		tAssetGenOutputLog assetGenLog( displayResults );

		log_line( 0, std::endl << "AssetGen beginning..." << std::endl );

		tAssetGenOptions opts;

		opts.mPlatforms = platformFlags;
		opts.mMultiProcMaster = true;
		opts.mGame2Xbox = true;

		tAssetGenScanner assetGenScanner( opts );
		tFilePathPtrList files;
		files.fPushBack( ToolsPaths::fMakeResRelative( absolutePath ) );
		assetGenScanner.fProcessFileList( tAssetPluginDllDepot::fInstance( ), files );

		log_line( 0, std::endl << "...AssetGen complete!" );
	}

}
