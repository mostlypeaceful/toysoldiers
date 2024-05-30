#include "UnitTestsPch.hpp"
#include "tFileWriter.hpp"
#include "tAsyncFileReader.hpp"
#include "tCompressor.hpp"

using namespace Sig;

define_unittest( TestAsyncFileRead )
{
	const b32 doCompressedTest = true;

	tFilePathPtr tempPath = ToolsPaths::fCreateTempEngineFilePath( );
	tFilePathPtr tempPathCompressed = ToolsPaths::fCreateTempEngineFilePath( );

	const u32 writeFileSize = 13*1024*1024+972;
	u32 compressedFileSize = 0;
	sigassert( fAlignHigh( writeFileSize, sizeof(int) ) == writeFileSize );
	Sig::byte* refBuf = new Sig::byte[writeFileSize];
	Sig::byte* compressedRefBuf = new Sig::byte[writeFileSize + tCompressor::cCompressionOverhead];
	{
		int i = 0;
		for( int* p = ( int* )refBuf; i < writeFileSize; ++p, i+=sizeof(int) )
			*p = i / sizeof(int);

		tCompressor compr;
		compressedFileSize = compr.fCompress( refBuf, writeFileSize, compressedRefBuf );
	}

	// write reference buffer to file
	{
		tFileWriter o( tempPath );
		o( refBuf, writeFileSize );
	}
	// write compressed buffer to file
	{
		tFileWriter o( tempPathCompressed );
		o( compressedRefBuf, compressedFileSize );
	}

	tAsyncFileReaderPtr fileRead;
	if( doCompressedTest )
		fileRead = tAsyncFileReader::fCreate( tempPathCompressed ); // for compressed test
	else
		fileRead = tAsyncFileReader::fCreate( tempPath );

	fAssertNotNull( fileRead );

	fileRead->fBlockUntilOpen( );

	fAssertEqual( fileRead->fInFailedState( ), false );
	fAssertEqual( fileRead->fGetState( ), tAsyncFileReader::cStateOpenSuccess );

	const size_t fileSize = fileRead->fGetFileSize( );
	if( doCompressedTest )
		fAssertNotEqual( fileSize, writeFileSize ); // for compressed test
	else
		fAssertEqual( fileSize, writeFileSize );

	srand( 314876 );

	const u32 numAttempts = doCompressedTest ? 8 : 1024;

	for( u32 attempt = 0; attempt < numAttempts; ++attempt )
	{
		const u32 readOffset = doCompressedTest ? 0 : ( fAlignHigh( rand( ) % ( u32 )( fileSize - sizeof(int) + 1 ), sizeof(int) ) );
		sigassert( readOffset < fileSize );
		const size_t numBytesAllocated = writeFileSize - readOffset;
		Sig::byte* originalRecvBuffer = new Sig::byte[numBytesAllocated];

		fileRead->fRead( tAsyncFileReader::tReadParams( tAsyncFileReader::tRecvBuffer( originalRecvBuffer, ( u32 )numBytesAllocated ), 0, readOffset, doCompressedTest  ) );

		fileRead->fBlockUntilReadComplete( );

		fAssertEqual( fileRead->fInFailedState( ), false );
		fAssertEqual( fileRead->fGetState( ), tAsyncFileReader::cStateReadSuccess );

		// calling this twice in succession should yield our original pointer the first time, and null the second
		Sig::byte* reclaimedRecvBuffer = fileRead->fForgetBuffer( );
		Sig::byte* shouldBeNullRecvBuffer = fileRead->fForgetBuffer( );

		fAssertEqual( originalRecvBuffer, reclaimedRecvBuffer );
		fAssertNull( shouldBeNullRecvBuffer );

		{
			int i = readOffset;
			for(	int* pa = ( int* )( refBuf + readOffset ), *pb = ( int* )reclaimedRecvBuffer; 
					i < writeFileSize; 
					++pa, ++pb, i += sizeof(int) )
			{
				fAssertEqual( *pa, i / (int)sizeof(int) );
				fAssertEqual( *pa, *pb );
			}
		}

		delete [] reclaimedRecvBuffer;
	}

	delete [] refBuf;
	delete [] compressedRefBuf;
}
