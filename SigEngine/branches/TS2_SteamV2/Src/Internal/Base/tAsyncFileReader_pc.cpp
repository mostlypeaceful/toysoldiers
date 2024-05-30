#include "BasePch.hpp"
#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
#include "tAsyncFileReader.hpp"
#include "tDecompressor.hpp"
#include "Win32Util.hpp"

namespace Sig
{
	namespace
	{
		struct tReadBlock
		{
			OVERLAPPED	mOverlapped;
			byte*		mSectorAlignedMemory;
			u32			mCopyFromOffset;
			u32			mNumBytesToCopy;

			inline tReadBlock( ) { fZeroOut( this ); }
			inline void fReset( )
			{
				const tReadBlock saved = *this;
				fZeroOut( this );
				mOverlapped.hEvent = saved.mOverlapped.hEvent;
				mSectorAlignedMemory = saved.mSectorAlignedMemory;
			}
		};

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

		const u32		cReadBlockCount = 8; ///< Must be power of 2, as we use & instead of %.
		const u32		cReadBlockMod = cReadBlockCount-1;
		const u32		cIdealReadBlockSize = 64 * 1024;
		u32				gSectorSize = 0;
		u32				gBlockSize = 0; ///< A multiple of the sector size, as close to cIdealReadBlockSize as possible.
		tDecompressor	gDecompressor( false, fDecompressorAlloc, fDecompressorFree );
		tFixedArray<tReadBlock,cReadBlockCount>		gReadBlocks;
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

		define_static_function( fAllocReadBlocks )
		{
			if( !GetDiskFreeSpace( 0, 0, ( DWORD* )&gSectorSize, 0, 0 ) )
			{
				log_warning( Log::cFlagFile, "Failed to query the sector size." );
			}

			gBlockSize = fAlignLow( cIdealReadBlockSize, gSectorSize );

			for( u32 i = 0; i < gReadBlocks.fCount( ); ++i )
			{			
				// VirtualAlloc() creates storage that is page aligned and so is disk sector aligned
				gReadBlocks[i].mSectorAlignedMemory = 
					static_cast<byte*>( VirtualAlloc( 0, gBlockSize, MEM_COMMIT, PAGE_READWRITE ) );
				gReadBlocks[i].mOverlapped.hEvent = CreateEvent( 0, false, false, 0 );
			}
		}
	}

	b32 tAsyncFileReader::fCreateFileForPlatform( )
	{
		sigassert( mPlatformFileHandle == 0 );
		sigassert( mFileSize == 0 );

		// create the file handle, specifying async reads and no buffering

		HANDLE hFile = CreateFile(
			mFileName.fCStr( ),
			GENERIC_READ,
			FILE_SHARE_READ,
			0,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED | FILE_FLAG_SEQUENTIAL_SCAN,
			0);

		// query for the size of the file

		if( hFile == INVALID_HANDLE_VALUE ||
			!GetFileSizeEx( hFile, ( LARGE_INTEGER* )&mFileSize ) )
			mFileSize = 0;

		if( mFileSize == 0 && hFile != INVALID_HANDLE_VALUE )
		{
			// the file was valid, but it's zero sized; close the file and say we couldn't open anything;
			// conceivably, some systems might simply want to query the existence of a file, but creating
			// a tAsyncFileReader is not the way to do that; these objects are intended for quickly reading
			// blocks of data asynchronously
			CloseHandle( hFile );
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
		HANDLE hFile = ( HANDLE )mPlatformFileHandle;
		if( hFile != 0 && hFile != INVALID_HANDLE_VALUE )
			CloseHandle( hFile );
		mPlatformFileHandle = 0;
		mFileSize = 0;
	}

	u64 tAsyncFileReader::fGetLastModifiedTimeStamp( ) const
	{
		HANDLE hFile = ( HANDLE )mPlatformFileHandle;
		if( !hFile || hFile == INVALID_HANDLE_VALUE )
			return 0;

		FILETIME lastModified={0};
		const BOOL success = GetFileTime( hFile, 0, 0, &lastModified );

		if( !success )
			return 0;

		u64 out;
		sigassert( sizeof( out ) == sizeof( lastModified ) );
		fMemCpy( &out, &lastModified, sizeof( out ) );
		
		return out;
	}

	b32 tAsyncFileReader::fReadFileInThreadForPlatform( )
	{
		HANDLE hFile = ( HANDLE )mPlatformFileHandle;
		sigassert( mState == cStateReading );
		sigassert( mPlatformFileHandle != 0 );
		sigassert( mFileSize > 0 );
		sigassert( hFile != INVALID_HANDLE_VALUE );

		const u32 totalReadBytes = mReadParams.mReadByteCount ? mReadParams.mReadByteCount : ( mFileSize - mReadParams.mReadByteOffset );
		const u32 startingOffset = mReadParams.mReadByteOffset;
		const u32 lastBytePlus1 = startingOffset + totalReadBytes;

		u32 readIdxUnbounded = 0;
		u32 copyIdxUnbounded = 0;
		u32 ioPos = fAlignLow( startingOffset, gBlockSize );
		u32 absolutePos = startingOffset;
		u32 absoluteDecompressedPos = startingOffset;

		if( mReadParams.mDecompressAfterRead )
			gDecompressor.fBeginDecompress( );

		while( absolutePos < lastBytePlus1 )
		{      
			while( readIdxUnbounded - copyIdxUnbounded != cReadBlockCount && ioPos < lastBytePlus1 )
			{
				tReadBlock& readBlock = gReadBlocks[ readIdxUnbounded & cReadBlockMod ];

				readBlock.fReset( );
				readBlock.mOverlapped.Offset = ioPos;
				readBlock.mCopyFromOffset = ioPos < startingOffset ? ( startingOffset % gBlockSize ) : 0;

				u32 bytesToRead = 0;
				const u32 bytesLeft = lastBytePlus1 - ioPos;
				if( bytesLeft >= gBlockSize )
				{
					bytesToRead = gBlockSize;

					sigassert( bytesToRead >= readBlock.mCopyFromOffset );
					readBlock.mNumBytesToCopy = bytesToRead - readBlock.mCopyFromOffset;
				}
				else
				{
					bytesToRead = fAlignHigh( bytesLeft, gSectorSize );

					sigassert( bytesLeft >= readBlock.mCopyFromOffset );
					readBlock.mNumBytesToCopy = bytesLeft - readBlock.mCopyFromOffset;
				}

				const BOOL ret = ReadFile(
					hFile, 
					readBlock.mSectorAlignedMemory, 
					bytesToRead, 
					0, 
					&readBlock.mOverlapped );

				if( !ret )
				{
					const DWORD error = GetLastError( );

					// ERROR_IO_PENDING is the only acceptable error, just means we're reading data
					if( error != ERROR_IO_PENDING )
					{
						log_warning( Log::cFlagFile, 
								   "Error returned by win32 ReadFile:\n\t"
								<< Win32Util::fErrorCodeToString( error ) );

						if( mReadParams.mDecompressAfterRead )
							gDecompressor.fEndDecompress( );

						return false;
					}
				}

				++readIdxUnbounded;
				ioPos += bytesToRead;	
			}

			tReadBlock& readBlock = gReadBlocks[ copyIdxUnbounded & cReadBlockMod ];

			// wait for the current read to be done
			WaitForSingleObject( readBlock.mOverlapped.hEvent, INFINITE );

			// compute offset into recv buffer and read buffer
			const u32	copyToPos	= absoluteDecompressedPos - startingOffset;
			byte*		copyTo		= &mReadParams.mRecvBuffer.fGetBuffer( )[copyToPos];
			const byte* copyFrom	= readBlock.mSectorAlignedMemory + readBlock.mCopyFromOffset;

			// Same assert disabled in Ascend. There are cases where this can be a false positive
			//sigassert( copyToPos + readBlock.mNumBytesToCopy <= mReadParams.mRecvBuffer.fGetBytesAllocated( ) );

			u32 numBytesToAdvanceDecompressed;

			// copy data to recv buffer (if un-compressed), or, if compressed,
			// de-compress into recv buffer...

			if( mReadParams.mDecompressAfterRead )
			{
				numBytesToAdvanceDecompressed = gDecompressor.fDecompress( 
					copyFrom, 
					readBlock.mNumBytesToCopy, 
					copyTo, 
					mReadParams.mRecvBuffer.fGetBytesAllocated( ) );
			}
			else
			{
				fMemCpy( copyTo, copyFrom, readBlock.mNumBytesToCopy );

				numBytesToAdvanceDecompressed = readBlock.mNumBytesToCopy;
			}

			// advance misc. indices and trackers

			++copyIdxUnbounded;
			absolutePos				+= readBlock.mNumBytesToCopy;
			absoluteDecompressedPos += numBytesToAdvanceDecompressed;
		}

		if( mReadParams.mDecompressAfterRead )
			gDecompressor.fEndDecompress( );

		return true;
	}

}
#endif//#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
