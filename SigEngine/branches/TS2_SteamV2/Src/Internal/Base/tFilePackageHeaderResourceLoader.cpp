#include "BasePch.hpp"
#include "tFilePackageHeaderResourceLoader.hpp"
#include "tLoadInPlaceResourceBuffer.hpp"
#include "tFilePackageFile.hpp"
#include "tResource.hpp"

namespace Sig
{

	tFilePackageHeaderResourceLoader::tFilePackageHeaderResourceLoader( tResource* res, const tAsyncFileReaderPtr& fileReader )
		: tStandardResourceLoader( res )
		, mHeaderSize( 0 )
	{
		mFileReader = fileReader;
	}

	void tFilePackageHeaderResourceLoader::fInitiate( )
	{
		fSetResourceBuffer( NEW tLoadInPlaceResourceBuffer( ) );
		fSetSelfOnResource( );
	}

	void tFilePackageHeaderResourceLoader::fOnOpenSuccess( )
	{
		if( fGetCancel( ) )
			return; // don't allocate or begin reading if we've been cancelled

		const u32 resBufferSize = sizeof( tFilePackageFile );

		tGenericBuffer* resBuffer = fGetResourceBuffer( );

		resBuffer->fAlloc( resBufferSize, false, fMakeStamp( ) );

		mFileReader->fRead( tAsyncFileReader::tReadParams( 
				tAsyncFileReader::tRecvBuffer( resBuffer->fGetBuffer( )/*buffer*/, resBufferSize/*bytesAllocated*/ ),
				resBufferSize/*readByteCount*/,
				0/*readByteOffset*/,
			false/*decompressAfterRead*/) );
	}

	void tFilePackageHeaderResourceLoader::fOnReadSuccess( )
	{
		mFileReader->fForgetBuffer( );

		fCleanupAfterSuccess( );
	}

	void tFilePackageHeaderResourceLoader::fCleanupAfterSuccess( )
	{
		if( mHeaderSize == 0 && !fGetCancel( ) )
		{
			// we've only just read the first few bytes so that we could
			// actually find out just how big the actual header data is;
			// hence, we now allocate our buffer to proper size and perform
			// a full read on the rest of the header table...

			tGenericBuffer* resBuffer = fGetResourceBuffer( );
			sigassert( resBuffer->fGetBufferSize( ) == sizeof( tFilePackageFile ) );

			const tFilePackageFile* filePackageFileStubPtr = fGetResource( )->fCast<tFilePackageFile>( );
			sigassert( filePackageFileStubPtr );

			// copy out the header to a temporary buffer, bcz as soon as we call fFree on the resBuffer,
			// the current pointer (filePackageFileStubPtr) will become invalid
			tFixedArray<Sig::byte,sizeof(tFilePackageFile)> filePackageFileStub;
			fMemCpy( &filePackageFileStub, filePackageFileStubPtr, sizeof( filePackageFileStub ) );
			mHeaderSize = filePackageFileStubPtr->fGetHeaderTableSize( );
			resBuffer->fFree( );

			resBuffer->fAlloc( mHeaderSize, false, fMakeStamp( ) );
			fMemCpy( resBuffer->fGetBuffer( ), &filePackageFileStub, sizeof( filePackageFileStub ) );

			const u32 skipSize = sizeof( filePackageFileStub );
			const u32 leftSize = mHeaderSize - skipSize;

			mFileReader->fRead( tAsyncFileReader::tReadParams( 
					tAsyncFileReader::tRecvBuffer( resBuffer->fGetBuffer( ) + skipSize/*buffer*/, leftSize/*bytesAllocated*/ ),
					leftSize/*readByteCount*/,
					skipSize/*readByteOffset*/,
				false/*decompressAfterRead*/) );
		}
		else
		{
			// now we're actually done reading the whole of the file package header table...
			mFileReader.fRelease( );
		}
	}

}
