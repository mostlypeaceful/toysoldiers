//------------------------------------------------------------------------------
// \file SyncCompare.cpp - 16 Nov 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tCmdLineOption.hpp"
#include "tConsoleApp.hpp"
#include "FileSystem.hpp"
#include "tSync.hpp"

using namespace Sig;

void fLogUsage( )
{
	log_line( 0, "Usage: syncompare.exe\n\t -i1 <inputFileName1> - required\n\t -i2 <inputFileName2> - required\n\t" );
}

//------------------------------------------------------------------------------
int main( int argc, const char * argv[] )
{
	tConsoleApp console; 
	console.fCreateConsole( "-=-=-=-=-=- sYNc cOMPARe (c) Signal Studios -=-=-=-=-=-\n", false );

	std::string cmdLineBuffer;
	for( int i = 0; i < argc; ++i)
	{
		cmdLineBuffer += argv[i];
		if( i+1 < argc )
			cmdLineBuffer += ' ';
	}
	
	tCmdLineOption in1("i1", cmdLineBuffer );
	tCmdLineOption in2("i2", cmdLineBuffer );
	tCmdLineOption help("?", cmdLineBuffer );

	for( ;; )
	{
		// Usage request
		if( help.fFound( ) )
		{
			fLogUsage( );
			break;
		}

		// Missing input file 1
		if( !in1.fFound( ) )
		{
			log_warning( 0, "Missing input file 1 (-i1)" );
			fLogUsage( );
			break;
		}

		// Missing input file 2
		if( !in2.fFound( ) )
		{
			log_warning( 0, "Missing input file 2 (-i2)" );
			fLogUsage( );
			break;
		}

		tFilePathPtr file1Path( in1.fGetOption( ) );
		tFilePathPtr file2Path( in2.fGetOption( ) );

		// File 1 doesn't exist
		if( !FileSystem::fFileExists( file1Path ) )
		{
			log_warning( 0, "File 1 ( " << file1Path << " ) could not be found" );
			break;
		}

		// File 2 doesn't exist
		if( !FileSystem::fFileExists( file2Path ) )
		{
			log_warning( 0, "File 2 ( " << file2Path << " ) could not be found" );
			break;
		}

		if( tSync::fFilesEqual( file1Path, file2Path ) )
			log_line( 0, "Files Equal!!!" )
		else
			log_warning( 0, "Files NOT Equal!!!" );
		
		break;
	}

	console.fDestroyConsole( "-=-=-=-=-=- Finished running sYNc cOMPARe -=-=-=-=-=-\n", true );
}