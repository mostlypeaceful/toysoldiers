#include "BasePch.hpp"
#if defined( platform_ios )
#include "tAsyncFileReader.hpp"
#include "tDecompressor.hpp"
#include "FileSystem.hpp"


namespace Sig
{
	namespace
	{
		// these values were determined empirically, tracking the behavior
		// of the zlib runtime, seeing how many allocations there were and
		// how big each one was... there are asserts in place in case a
		// situation is encountered which breaks the empirical assumptions
		const u32		cAllocBlockSize = 32*1024;
		const u32		cAllocBlockCount = 2;

		struct tAllocBlock
		{
			tFixedArray<byte, cAllocBlockSize>	mBuffer;
			b32								mInUse;

			inline tAllocBlock( ) : mInUse( false ) { }
		};

		voidpf	fDecompressorAlloc( voidpf opaque, uInt items, uInt size );
		void	fDecompressorFree( voidpf opaque, voidpf address );

		tDecompressor	gDecompressor( false, fDecompressorAlloc, fDecompressorFree );
		tFixedArray<tAllocBlock,cAllocBlockCount>	gAllocBlocks;

		///
		/// Static allocation function using empirically derived assumptions,
		/// so that we can provide very fast and thread-safe allocations for
		/// decompression library.
		voidpf fDecompressorAlloc( voidpf opaque, uInt items, uInt size )
		{
			const size_t allocSize = items * size;

			sigassert( allocSize <= cAllocBlockSize );

			for( u32 i = 0; i < gAllocBlocks.fCount( ); ++i )
			{
				if( !gAllocBlocks[i].mInUse )
				{
					gAllocBlocks[i].mInUse = true;
					return gAllocBlocks[i].mBuffer.fBegin( );
				}
			}

			sigassert( !"should never reach this in fDecompressorAlloc" );
			return 0;
		}

		///
		/// Static free function using empirically derived assumptions,
		/// so that we can provide very fast and thread-safe allocations for
		/// decompression library.
		void fDecompressorFree( voidpf opaque, voidpf address )
		{
			for( u32 i = 0; i < gAllocBlocks.fCount( ); ++i )
			{
				if( address == gAllocBlocks[i].mBuffer.fBegin( ) )
				{
					sigassert( gAllocBlocks[i].mInUse );
					gAllocBlocks[i].mInUse = false;
					break;
				}
			}
		}
	}

	b32 tAsyncFileReader::fCreateFileForPlatform( )
	{
		sigassert( mPlatformFileHandle == 0 );
		sigassert( mFileSize == 0 );

		// create the file handle, specifying async reads and no buffering
		
		FILE* hFile = fopen( mFileName.fCStr( ), "rb" );

		// query for the size of the file

		mFileSize = 0;
		if( hFile )
		{
			fseek( hFile, 0, SEEK_END );
			mFileSize = ftell( hFile );
			rewind( hFile );
		}

		if( !hFile || mFileSize == 0 )
		{
			// the file was valid, but it's zero sized; close the file and say we couldn't open anything;
			// conceivably, some systems might simply want to query the existence of a file, but creating
			// a tAsyncFileReader is not the way to do that; these objects are intended for quickly reading
			// blocks of data asynchronously
			fclose( hFile );
		}

		if( mFileSize > 0 )
		{
			mPlatformFileHandle = ( u64 )hFile;
			return true;
		}

		return false;
	}

	void tAsyncFileReader::fCloseFileForPlatform( )
	{
		FILE* hFile = ( FILE* )mPlatformFileHandle;
		if( hFile )
			fclose( hFile );
		mPlatformFileHandle = 0;
		mFileSize = 0;
	}

	u64 tAsyncFileReader::fGetLastModifiedTimeStamp( ) const
	{
		return FileSystem::fGetLastModifiedTimeStamp( mFileName );
	}

	b32 tAsyncFileReader::fReadFileInThreadForPlatform( )
	{
		FILE* hFile = ( FILE* )mPlatformFileHandle;
		sigassert( mState == cStateReading );
		sigassert( mPlatformFileHandle != 0 );
		sigassert( mFileSize > 0 );
		sigassert( hFile );
		
		const u32 totalReadBytes = mReadParams.mReadByteCount ? mReadParams.mReadByteCount : ( mFileSize - mReadParams.mReadByteOffset );
		const u32 startingOffset = mReadParams.mReadByteOffset;
//		const u32 lastBytePlus1 = startingOffset + totalReadBytes;
		
		if( mReadParams.mDecompressAfterRead )
		{
			gDecompressor.fBeginDecompress( );
			
			log_warning_unimplemented( 0 );
//			numBytesToAdvanceDecompressed = gDecompressor.fDecompress( 
//				  copyFrom, 
//				  readBlock.mNumBytesToCopy, 
//				  copyTo, 
//				  mReadParams.mRecvBuffer.fGetBytesAllocated( ) );
			
			gDecompressor.fEndDecompress( );
		}
		else
		{
			const u32	copyToPos	= startingOffset;
			byte*		copyTo		= &mReadParams.mRecvBuffer.fGetBuffer( )[copyToPos];
			const u32	bytesRead	= fread( copyTo, 1, totalReadBytes, hFile );
			log_assert( bytesRead == totalReadBytes, "Invalid file read, bytesRead != totalReadBytes." );
		}

		
		return true;
	}
}
#endif//#if defined( platform_ios )

