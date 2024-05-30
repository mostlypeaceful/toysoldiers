//------------------------------------------------------------------------------
// \file tAsyncFileDownloader.cpp - 26 Jul 2012
// \author colins
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tAsyncFileDownloader.hpp"
#include "tCloudStorage.hpp"

namespace Sig
{
	devvar( u32, FileDownloader_ChunkSize, Memory::fFromMB<u32>( 5 ) );

	//--------------------------------------------------------------------------------------
	// tAsyncFileDownloader
	//--------------------------------------------------------------------------------------
	tAsyncFileDownloader::tAsyncFileDownloader( const tFilePathPtr& filePath, u32 fileSize, const iCloudStoragePtr& cloudStorage )
		: mFilePath( filePath )
		, mFileSize( fileSize )
		, mChunkOffset( 0 )
		, mChunkSize( FileDownloader_ChunkSize )
		, mWriteBuffer( NULL )
		, mWriteBufferSize( 0 )
		, mWriteOffset( 0 )
		, mCloudStorage( cloudStorage )
	{
	}

	tAsyncFileDownloader::~tAsyncFileDownloader( )
	{
	}

	tAsyncFileDownloaderPtr tAsyncFileDownloader::fCreate( const tFilePathPtr& path, const iCloudStoragePtr& cloudStorage )
	{
		const iCloudStorage& constCloudStorage = ( const iCloudStorage& )*cloudStorage;
		const iCloudStorage::tFileBlob* fileBlob = constCloudStorage.fFindFileBlob( path );
		if( !fileBlob )
			return tAsyncFileDownloaderPtr( );

		tAsyncFileDownloaderPtr downloader = tAsyncFileDownloaderPtr( NEW tAsyncFileDownloader( path, fileBlob->mSize, cloudStorage ) );
		return downloader;
	}

	void tAsyncFileDownloader::fStart( byte* buffer, u32 bufferSize, const tFilePathPtr& fileRootPath )
	{
		sigassert( !buffer || bufferSize >= mFileSize && "Cannot start file download because the buffer is too small" );

		mWriteBuffer = buffer;
		mWriteBufferSize = bufferSize;
		mWriteOffset = 0;

		if( fileRootPath.fExists( ) )
		{
			std::string fileName = StringUtil::fNameFromPath( mFilePath.fCStr( ) );
			mFileWriter.fOpen( tFilePathPtr::fConstructPath( fileRootPath, tFilePathPtr( fileName ) ) );
		}

		fRequestNextChunk( );
	}

	void tAsyncFileDownloader::fUpdate( )
	{
		if( mCloudRequest.fNull( ) )
			return;

		if( !mCloudRequest->fIsComplete( ) )
			return;

		// Check if we have a new chunk
		if( mCloudRequest->fSuccess( ) && mChunkOffset < mFileSize )
		{
			const u32 contentLength = mCloudRequest->fContentLength( );

			// Write to memory
			if( mWriteBuffer )
			{
				byte* destBuffer = mWriteBuffer + mWriteOffset;
				memcpy( destBuffer, mCloudRequest->fReadBuffer( ), contentLength );
			}

			// Write to disk
			if( mFileWriter.fIsOpen( ) )
			{
				mFileWriter( mCloudRequest->fReadBuffer( ), contentLength );
			}

			mChunkOffset += contentLength;
			mWriteOffset += contentLength;

			if( mChunkOffset < mFileSize )
				fRequestNextChunk( );
			else if( mFileWriter.fIsOpen( ) )
				mFileWriter.fClose( );
		}
	}

	b32 tAsyncFileDownloader::fIsComplete( ) const
	{
		return ( fSuccess( ) || fFailed( ) );
	}

	b32 tAsyncFileDownloader::fSuccess( ) const
	{
		return ( mCloudRequest && mChunkOffset == mFileSize );
	}

	b32 tAsyncFileDownloader::fFailed( ) const
	{
		return ( mCloudRequest && mCloudRequest->fFailed( ) );
	}

	const tFilePathPtr& tAsyncFileDownloader::fFilePath( ) const
	{
		return mFilePath;
	}

	u32 tAsyncFileDownloader::fFileSize( ) const
	{
		return mFileSize;
	}

	u32 tAsyncFileDownloader::fChunkOffset( ) const
	{
		return mChunkOffset;
	}

	u32 tAsyncFileDownloader::fChunkDownloaded( ) const
	{
		if( mCloudRequest )
		{
			return mCloudRequest->fContentLength( ) 
				- mCloudRequest->fContentLengthRemaining( );
		}

		return 0;
	}

	void tAsyncFileDownloader::fRequestNextChunk( )
	{
		mCloudRequest = mCloudStorage->fGetFileChunk( mFilePath, mChunkOffset, mChunkSize );
	}

} // ::Sig
