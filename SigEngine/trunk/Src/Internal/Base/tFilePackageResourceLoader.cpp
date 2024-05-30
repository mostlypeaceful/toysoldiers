#include "BasePch.hpp"
#include "tFilePackageResourceLoader.hpp"
#include "tResource.hpp"
#include "tLoadInPlaceResourceBuffer.hpp"

namespace Sig
{

	tFilePackageResourceLoader::tFilePackageResourceLoader( 
		tResource* resource,
		const tAsyncFileReaderPtr& fileReader,
		u32 fileOffset, 
		u32 fileSizeActual,
		u32 fileSizeUncompressed, 
		b32 decompress )
			: tStandardResourceLoader( resource )
			, mFileOffset( fileOffset )
			, mFileSizeActual( fileSizeActual )
			, mFileSizeUncompressed( fileSizeUncompressed )
			, mDecompress( decompress )
	{
		sigassert( mFileSizeUncompressed >= mFileSizeActual );
		mFileReader = fileReader;
	}

	void tFilePackageResourceLoader::fInitiate( )
	{
		fSetFileTimeStamp( 0 );
		fSetResourceBuffer( NEW tLoadInPlaceResourceBuffer( ) );
		fSetSelfOnResource( );
	}

	void tFilePackageResourceLoader::fOnOpenSuccess( )
	{
		if( fGetCancel( ) )
			return; // don't allocate or begin reading if we've been cancelled

		tGenericBuffer* resBuffer = fGetResourceBuffer( );
		resBuffer->fAlloc( mFileSizeUncompressed, fMakeStamp( mFileSizeUncompressed ) );

		mFileReader->fRead( tAsyncFileReader::tReadParams( 
				tAsyncFileReader::tRecvBuffer( resBuffer->fGetBuffer( )/*buffer*/, mFileSizeUncompressed/*bytesAllocated*/ ),
				mFileSizeActual/*readByteCount*/,
				mFileOffset/*readByteOffset*/,
			mDecompress/*decompressAfterRead*/) );
	}

}