#include "BasePch.hpp"
#include "tWin32Window.hpp"
#include "FileSystem.hpp"
#include "tFileWriter.hpp"
#include "tCmdLineOption.hpp"
#include "resource.h"

using namespace Sig;

namespace
{
	///
	/// \brief Convert the specified 'inputFile' to a text file suitable for including
	/// into the middle of a declared array of bytes in a .cpp file. I.e., the generated
	/// 'outputFile' could be used like this:
	/// const byte gSomeArray[]={
	///		#include "outputFile"
	/// };
	s32 fConvertFile( const char* inputFile, const char* outputFile, b32 appendNull )
	{
		tDynamicBuffer srcBytes;
		if( !FileSystem::fReadFileToBuffer( srcBytes, tFilePathPtr( inputFile ), appendNull ? "" : 0 ) )
		{
			log_warning( "Couldn't open input file [" << inputFile << "] for reading, aborting." );
			return -1;
		}

		tFileWriter dstFile = tFileWriter( tFilePathPtr( outputFile ) );
		if( !dstFile.fIsOpen( ) )
		{
			log_warning( "Couldn't open output file [" << inputFile << "] for writing, aborting." );
			return -1;
		}

		std::stringstream comment;
		comment << "// auto-generated from " << inputFile << std::endl;
		dstFile( comment.str( ).c_str( ), ( u32 )comment.str( ).size( ) );

		for( u32 i = 0; i < srcBytes.fCount( ); ++i )
		{
			std::stringstream value;
			value << ( u32 )srcBytes[i] << ", ";

			if( ( i + 1 ) % 8 == 0 )
				value << std::endl;

			dstFile( value.str( ).c_str( ), ( u32 )value.str( ).size( ) );
		}

		return 0;
	}
}


int WINAPI WinMain( HINSTANCE h1, HINSTANCE h2, LPSTR cmdLine, int showCmd )
{
	const std::string cmdLineBuffer = cmdLine;
	const tCmdLineOption inputFile( "i", cmdLineBuffer );
	const tCmdLineOption outputFile( "o", cmdLineBuffer );
	const tCmdLineOption appendNull( "n", cmdLineBuffer );

	if( !inputFile.fFound( ) || !outputFile.fFound( ) )
	{
		log_warning( "Input (-i) and Output (-o) filename options are required to run FileToByteArray; Append Null (-n) is optional." );
		return -1;
	}

	return fConvertFile( 
				inputFile.fGetOption( ).c_str( ), 
				outputFile.fGetOption( ).c_str( ),
				appendNull.fFound( ) );
}


