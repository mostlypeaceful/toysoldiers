#include "BasePch.hpp"
#include "FileSystem.hpp"
#include "tFileReader.hpp"
#include "tFileWriter.hpp"

namespace Sig { namespace FileSystem
{
	void fDeleteAllFilesInFolder( const tFilePathPtr& path )
	{
		tFilePathPtrList files;
		fGetFileNamesInFolder( files, path, true, false );

		for( u32 i = 0; i < files.fCount( ); ++i )
			fDeleteFile( files[i] );
	}

	b32 fReadFileToBuffer( void* readInto, u32 numBytes, const tFilePathPtr& path )
	{
		tFileReader reader( path );
		if( !reader.fIsOpen( ) )
			return false;
		const u32 numRead = reader( readInto, numBytes );
		return numRead == numBytes;
	}

	b32 fReadFileToBuffer( tDynamicBuffer& bufferOut, const tFilePathPtr& path, const char* append )
	{
		const u32 fileSize = fGetFileSize( path );
		if( fileSize == 0 )
			return false;

		tFileReader reader( path );
		if( !reader.fIsOpen( ) )
			return false;

		const u32 strLen = append ? ( ( u32 )( strlen( append ) + 1 ) ) : 0;
		const u32 bufferSize = fileSize + strLen;
		if( bufferOut.fCount( ) != bufferSize )
		{
#ifdef target_game
			bufferOut.fNewArray( bufferSize );
#else
			try
			{
				bufferOut.fNewArray( bufferSize );
			}
			catch( std::bad_alloc )
			{
				log_warning("Failed to read "<<path<<" to buffer: Ran out of memory (tried to alloc " << bufferSize/1024/1024 << " MB)");
				return false;
			}
#endif
		}

		const u32 numRead = reader( bufferOut.fBegin( ), fileSize );

		if( append )
			fMemCpy( bufferOut.fBegin( ) + fileSize, append, strLen );

		return numRead == fileSize;
	}
	
	b32 fReadFileToString( std::string& stringOut, const tFilePathPtr& path, const char* append )
	{
		tDynamicBuffer buffer;
		if( !fReadFileToBuffer( buffer, path, append ) ) return false;

		stringOut = std::string( (const char*)buffer.fBegin( ), buffer.fCount( ) );
		return true;
	}

	b32 fWriteBufferToFile( const tDynamicBuffer& buffer, const tFilePathPtr& path, const char* append )
	{
		return fWriteBufferToFile( buffer.fBegin( ), buffer.fCount( ), path, append );
	}

	b32 fWriteBufferToFile( const void* data, u32 numBytes, const tFilePathPtr& path, const char* append )
	{
		tFileWriter writer( path );
		if( !writer.fIsOpen( ) )
			return false;

		writer( data, numBytes );
		if( append )
			writer( append, ( u32 )strlen( append ) + 1 );
		return true;
	}

	b32 fIsAMoreRecentThanB( const tFilePathPtr& A, const tFilePathPtr& B )
	{
		const u64 timeStampA = fGetLastModifiedTimeStamp( A );
		const u64 timeStampB = fGetLastModifiedTimeStamp( B );
		
		return fIsAMoreRecentThanB( timeStampA, timeStampB );
	}
	
	b32 fAreAnyInAMoreRecentThanB( const tFilePathPtrList& A, const tFilePathPtr& B )
	{
		for( u32 i = 0; i < A.fCount( ); ++i )
		{
			if( fIsAMoreRecentThanB( A[i], B ) )
				return true;
		}
		
		return false;
	}
	
}}

