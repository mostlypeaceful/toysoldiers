#include "BasePch.hpp"
#include "tFileReader.hpp"

namespace Sig
{
	u32 tFileReader::fFileSize( const tFilePathPtr& path )
	{
		tFileReader f( path );
		f.fSeekFromEnd( 0 );
		return f.fTell( );
	}

	tFileReader::tFileReader( )
		: mFile( 0 )
	{
	}

	tFileReader::tFileReader( const tFilePathPtr& path )
		: mFile( 0 )
	{
		if( !path.fNull( ) )
			fOpen( path );
	}

	tFileReader::~tFileReader( )
	{
		fClose( );
	}

	b32 tFileReader::fOpen( const tFilePathPtr& path )
	{
		fClose( );

		mFile = fopen( path.fCStr( ), "rb" );

		return fIsOpen( );
	}

	void tFileReader::fClose( )
	{
		if( mFile )
		{
			fclose( ( FILE* )mFile );
			mFile = 0;
		}
	}

	b32 tFileReader::fIsOpen( ) const
	{
		return mFile!=0;
	}

	void tFileReader::fSeek( u32 absolutePos )
	{
		if( mFile )
			fseek( ( FILE* )mFile, absolutePos, SEEK_SET );
	}

	void tFileReader::fSeekFromCurrent( u32 relativePos )
	{
		if( mFile )
			fseek( ( FILE* )mFile, relativePos, SEEK_CUR );
	}

	void tFileReader::fSeekFromEnd( u32 relativePos )
	{
		if( mFile )
			fseek( ( FILE* )mFile, relativePos, SEEK_END );
	}

	u32 tFileReader::fTell( ) const
	{
		if( !mFile )
			return 0;
		return ftell( ( FILE* ) mFile );
	}

	u32 tFileReader::operator()( void* data, u32 numBytes )
	{
		if( !mFile )
			return 0;
		return ( u32 )fread( data, 1, numBytes, ( FILE* )mFile );
	}

}
