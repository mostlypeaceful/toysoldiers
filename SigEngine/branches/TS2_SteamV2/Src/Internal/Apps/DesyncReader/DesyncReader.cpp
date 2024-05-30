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
	std::string cmdLineBuffer = GetCommandLine( );

	tStrongPtr< tDesyncReaderConsoleApp > consoleApp( new tDesyncReaderConsoleApp( ) );
	consoleApp->fCreateConsole( "-=-=-=-=-=- dESYNc rEADEr (c) Signal Studios -=-=-=-=-=-\n", true );

	// Allows breaking out with easy, assured console destruction
	for( ;; )
	{
		const tCmdLineOption usage( "?", cmdLineBuffer );

		// Check for usage or no command
		if( usage.fFound( ) )
		{
			fPrintUsage( );
			break;
		}

		// Build the list of desync files
		tFilePathPtrList desyncFiles;
		FileSystem::fGetFileNamesInFolder( desyncFiles, tFilePathPtr::cNullPtr, true, false, tFilePathPtr( ".desync" ) );
		if( !desyncFiles.fCount( ) )
		{
			log_line( 0, "No desync files found to read" );
			break;
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

			tSymbolHelper symbols( ".", false );

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
				if( symbols.fGetSymbolSummary( addresses[ d ], symbol, file ) )
					log_line( 0, "\t" << symbol )
				else
					log_line( 0, "\tAddress 0x" << std::hex << ( void * )addresses[ d ] << " could not be resolved" )
			}
		}

		break; // All finished
	}

	consoleApp->fDestroyConsole( "-=-=-=-=-=- Finished running dESYNc rEADEr -=-=-=-=-=-\n", false );
}

