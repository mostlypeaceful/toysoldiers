//------------------------------------------------------------------------------
// \file DesyncReader.cpp - 01 Feb 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tCmdLineOption.hpp"
#include "FileSystem.hpp"
#include "tFileReader.hpp"
#include "tSymbolHelper.hpp"
#include "Threads\tProcess.hpp"
#include "Win32Util.hpp"
#include "tConsoleApp.hpp"
#include "tStrongPtr.hpp"

using namespace Sig;


//------------------------------------------------------------------------------
static void fPrintUsage( ) 
{
	log_line( 0, " Usage: desyncreader.exe <options>\n" );
	log_line( 0, " -?                 Print this usage statement" );
	log_line( 0, " -xbox < name >     Xbox to copy from, no name means default" );
	log_line( 0, " -nocopy            Parse files in Desync folder without copying new files" );
	log_line( 0, " -move              Delete the desync files on the target box after copying" );
}

//------------------------------------------------------------------------------
static tFilePathPtr fGetXEDKPath( )
{
	std::string xedkVar; // static to fix really strange release bug
	if( Win32Util::fGetEnvVar( "XEDK", xedkVar ) )
		return tFilePathPtr( xedkVar );
	
	log_warning( 0, "XEDK env var could not be found" );
	return tFilePathPtr( );
}

//------------------------------------------------------------------------------
static b32 fLoadXboxDbgHelp( const tFilePathPtr & xedk )
{
	tFilePathPtr path = tFilePathPtr::fConstructPath( 
		xedk, tFilePathPtr( "\\bin\\win32\\dbghelp.dll" ) );

	HMODULE module = LoadLibrary( path.fCStr( ) );
	if( !module )
	{
		log_warning( 0, "Could not load " << path );
		return false;
	}

	return true;
}

///
/// \class tDesyncReaderConsoleApp
/// \brief 
class tDesyncReaderConsoleApp : public tConsoleApp { };

//------------------------------------------------------------------------------
int main( )
{
	// convert command line to our format
	tCmdLineBufferPtr cmdLineBuffer = tCmdLineBuffer::fConvert( GetCommandLine( ) );

	tStrongPtr< tDesyncReaderConsoleApp > consoleApp( new tDesyncReaderConsoleApp( ) );
	consoleApp->fCreateConsole( "-=-=-=-=-=- dESYNc rEADEr (c) Signal Studios -=-=-=-=-=-\n", true );

	// Allows breaking out with easy, assured console destruction
	for( ;; )
	{
		const tCmdLineOption usage( "?", cmdLineBuffer );
		const tCmdLineOption xbox( "xbox", cmdLineBuffer );
		const tCmdLineOption noCopy( "nocopy", cmdLineBuffer );
		const tCmdLineOption move( "move", cmdLineBuffer );

		// Check for usage or no command
		if( usage.fFound( ) || ( !xbox.fFound( ) && !noCopy.fFound( ) )  )
		{
			fPrintUsage( );
			break;
		}

		// Construct the target folder
		tFilePathPtr desyncFolder = tFilePathPtr::fConstructPath(
			ToolsPaths::fGetCurrentProjectRootFolder( ), tFilePathPtr( "\\Test\\Desyncs" ) );

		// Create the target folder if it doesn't exist
		if( !FileSystem::fFolderExists( desyncFolder ) )
			FileSystem::fCreateDirectory( desyncFolder );

		// Capture the XEDK path
		tFilePathPtr xedkPath = fGetXEDKPath( );
		if( xedkPath.fNull( ) )
		{
			log_warning( 0, "XEDK path is currently required, aborting." );
			break;
		}

		// If preservation isn't specified, clear the desync 
		// folder and copy new desyncs and executables over
		if( !noCopy.fFound( ) )
		{
			sigassert( xbox.fFound( ) );

			std::string xboxName = xbox.fGetOption( );

			// Clear the old files
			log_line( 0, "Deleting old desync files..." );
			FileSystem::fDeleteAllFilesInFolder( desyncFolder );

			// Construct the xbcp.exe path
			tFilePathPtr xbcpPath = tFilePathPtr::fConstructPath( 
				xedkPath, tFilePathPtr( "\\bin\\win32\\xbcp.exe" ) );

			// Options storage
			std::string options;

			// Build options to copy desync files
			{
				std::stringstream ss;
				ss << "/R /Q"; // options
				if( xboxName.length( ) ) ss << " /X:" << xboxName; // xboxName
				ss << " xE:\\" << ToolsPaths::fGetCurrentProjectName( ) << "\\Desyncs"; // source
				ss << " " << desyncFolder.fCStr( ); // dest
				options = ss.str( );
			}

			// Execute xbox copy of desync files
			log_line( 0, "Copying desync files from xbox..." );
			Threads::tProcess desyncCopy( xbcpPath.fCStr( ), options.c_str( ) );
			if( !desyncCopy.fCreatedSuccessfully( ) )
				log_warning( 0, "Failed to copy desync files" );

			// Build options to copy executables
			{
				std::stringstream ss;
				ss << "/Q"; // options
				if( xboxName.length( ) ) ss << " /X:" << xboxName; // xboxName
				ss << " xE:\\" << ToolsPaths::fGetCurrentProjectName( ); // source
				ss << "\\default.* " << desyncFolder.fCStr( ) << "\\."; // dest
				options = ss.str( );
			}

			// Execute xbox copy of executables
			log_line( 0, "Copying executables and pdb..." );
			Threads::tProcess exeCopy( xbcpPath.fCStr( ), options.c_str( ) );
			if( !exeCopy.fCreatedSuccessfully( ) )
				log_warning( 0, "Failed to copy default.* from xbox" );

			// Wait for the copies to finish
			log_line( 0, "Waiting for desync copies to finish..." );
			desyncCopy.fWaitUntilFinished( );

			if( move.fFound( ) )
			{
				// Construct the xbdel.exe path
				tFilePathPtr xbdelPath = tFilePathPtr::fConstructPath( 
					xedkPath, tFilePathPtr( "\\bin\\win32\\xbdel.exe" ) );

				{
					std::stringstream ss;
					if( xboxName.length( ) ) ss << "/X:" << xboxName; // xboxName
					ss << " xE:\\" << ToolsPaths::fGetCurrentProjectName( ) << "\\Desyncs\\*.desync"; // files
					options = ss.str( );
				}

				log_line( 0, "Deleting desync files from xbox..." );
				if( !Threads::tProcess::fSpawnAndForget( xbdelPath.fCStr( ), options.c_str( ) ) )
					log_warning( 0, "Failed to delete desync files off xbox" );
			}

			// Wait for exe copy to finish
			log_line( 0, "Waiting for exe copies to finish..." );
			exeCopy.fWaitUntilFinished( );
		}

		// Build the list of desync files
		tFilePathPtrList desyncFiles;
		FileSystem::fGetFileNamesInFolder( desyncFiles, desyncFolder, true, false, tFilePathPtr( ".desync" ) );
		if( !desyncFiles.fCount( ) )
		{
			log_line( 0, "No desync files found to read" );
			break;
		}

		// Load xbox version of dbghelp.dll so that symsrv.dll loads correctly
		fLoadXboxDbgHelp( xedkPath );

		// Search path storage
		std::string symSearchPath;

		// Build the symbols search path
		{
			std::stringstream ss;
			ss << "SRV*" << xedkPath.fCStr( ) << "\\bin\\xbox\\symsrv;";
			ss << desyncFolder;
			symSearchPath = ss.str( );
		}
		
		// Share memory for each file
		tGrowableArray<Sig::byte> buffer; 

		log_line( 0, "Processing files..." );

		// Process each file
		const u32 fileCount = desyncFiles.fCount( );
		for( u32 f = 0; f < fileCount; ++ f )
		{
			const tFilePathPtr & filePath = desyncFiles[ f ];
			log_line( 0, "============= " << StringUtil::fNameFromPath( filePath.fCStr( ) ) << " =============");

			// Open the file
			tFileReader reader( filePath );

			// If it couldn't be opened just skip it
			if( !reader.fIsOpen( ) )
			{
				log_warning( 0, "Could not open file, skipping" );
				continue;
			}

			// Get the file size
			reader.fSeekFromEnd( 0 );
			u32 size = reader.fTell( );
			reader.fSeek( 0 );

			// Allocate and read file into memory
			buffer.fSetCount( size );
			u32 read = reader( buffer.fBegin( ), buffer.fTotalSizeOf( ) );
			reader.fClose( );

			// Check for read errors
			if( read != size )
			{
				log_warning( 0, "Failed to read file, skipping" );
				continue;
			}

			log_line( 0, "Parsing modules..." );
			
			// Load the modules
			tModuleHelper modules( false );
			const Sig::byte * stack = modules.fLoad( buffer.fBegin( ), buffer.fTotalSizeOf( ) );
			const u32 depth = *( const u32* )stack; stack += sizeof( u32 );
			const u64 * addresses = ( const u64 * )stack;

			log_line( 0, modules.fModuleCount( ) << " modules found" );
			log_line( 0, depth << " addresses found" );

			tSymbolHelper symbols( symSearchPath.c_str( ) );

			log_line( 0, "Loading modules..." );
			const u32 moduleCount = modules.fModuleCount( );
			for( u32 m = 0; m < moduleCount; ++m )
			{
				const tModule & module = modules.fModule( m );
				if( !symbols.fLoadSymbolsForModule( module ) )
					log_warning( 0, module.mName << " - Failed to load symbols at " << module.mSymbolsSignature.mPath.fCStr( ) );
			}
			
			log_line( 0, "Callstack:" );
			for( u32 d = 0; d < depth; ++d )
			{
				std::string symbol, file;
				if( symbols.fGetSymbolSummary( ( void * )addresses[ d ], symbol, file ) )
					log_line( 0, "\t" << symbol )
				else
					log_line( 0, "\tAddress 0x" << std::hex << ( void * )addresses[ d ] << " could not be resolved" )
			}
		}

		break; // All finished
	}

	consoleApp->fDestroyConsole( "-=-=-=-=-=- Finished running dESYNc rEADEr -=-=-=-=-=-\n", false );
}

