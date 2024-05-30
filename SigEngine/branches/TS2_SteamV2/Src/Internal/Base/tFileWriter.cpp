#include "BasePch.hpp"
#include "tFileWriter.hpp"
#include "FileSystem.hpp"

namespace Sig
{
	tFileWriter::tFileWriter( )
		: mFile( 0 )
	{
	}

	tFileWriter::tFileWriter( const tFilePathPtr& path, b32 append )
		: mFile( 0 )
	{
		if( !path.fNull( ) )
			fOpen( path, append );
	}

	tFileWriter::~tFileWriter( )
	{
		fClose( );
	}

	b32 tFileWriter::fOpen( const tFilePathPtr& path, b32 append )
	{
		fClose( );

		// ensure the directory is created
		FileSystem::fCreateDirectory( 
			tFilePathPtr( StringUtil::fDirectoryFromPath( path.fCStr( ) ).c_str( ) ) );

		mFile = fopen( path.fCStr( ), append ? "a+" : "wb" );

		return fIsOpen( );
	}

	void tFileWriter::fClose( )
	{
		if( mFile )
		{
			fflush( ( FILE* )mFile );
			fclose( ( FILE* )mFile );
			mFile = 0;
		}
	}

	b32 tFileWriter::fIsOpen( ) const
	{
		return mFile!=0;
	}

	void tFileWriter::fSeek( u32 absolutePos )
	{
		if( mFile )
			fseek( ( FILE* )mFile, absolutePos, SEEK_SET );
	}

	void tFileWriter::fSeekFromCurrent( u32 relativePos )
	{
		if( mFile )
			fseek( ( FILE* )mFile, relativePos, SEEK_CUR );
	}

	void tFileWriter::fSeekFromEnd( u32 relativePos )
	{
		if( mFile )
			fseek( ( FILE* )mFile, relativePos, SEEK_END );
	}

	void tFileWriter::fFlush( )
	{
		if( mFile )
			fflush( ( FILE* )mFile );
	}

	void tFileWriter::operator()( const void* data, u32 numBytes )
	{
		if( !mFile || numBytes == 0 )
			return;
		fwrite( data, 1, numBytes, ( FILE* )mFile );
	}

}
