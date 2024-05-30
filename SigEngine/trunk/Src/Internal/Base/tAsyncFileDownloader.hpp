//------------------------------------------------------------------------------
// \file tAsyncFileDownloader.hpp - 26 Jul 2012
// \author colins
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tAsyncFileDownloader__
#define __tAsyncFileDownloader__
#include "Http/Http.hpp"
#include "tFileWriter.hpp"

namespace Sig
{
	class iCloudStorage;
	class iCloudStoragePtr;
	class iCloudStorageRequest;

	class tAsyncFileDownloader;
	typedef tRefCounterPtr< tAsyncFileDownloader > tAsyncFileDownloaderPtr;

	///
	/// \class tAsyncFileDownloader
	/// \brief Encapsulates asynchronous file download operations.
	class tAsyncFileDownloader : public tUncopyable, public tRefCounter
	{
	private:

		tRefCounterPtr< iCloudStorage > mCloudStorage;
		tRefCounterPtr< iCloudStorageRequest > mCloudRequest;
		tFilePathPtr mFilePath;
		u32 mFileSize;
		u32 mChunkOffset;
		u32 mChunkSize;

		byte* mWriteBuffer;
		u32 mWriteBufferSize;
		tFileWriter mFileWriter;
		u32 mWriteOffset;

	public:

		~tAsyncFileDownloader( );

		///
		/// \brief Starts the download, writing to a buffer and/or file
		void fStart( byte* buffer, u32 bufferSize, const tFilePathPtr& fileRootPath );

		///
		/// \brief Requests the next file chunk if appropriate
		void fUpdate( );

		///
		/// \brief Create a smart pointer for downloading the specified file
		static tAsyncFileDownloaderPtr fCreate( const tFilePathPtr& path, const iCloudStoragePtr& cloudStorage );

		b32 fIsComplete( ) const;
		b32 fSuccess( ) const;
		b32 fFailed( ) const;

		const tFilePathPtr& fFilePath( ) const;
		u32 fFileSize( ) const;
		u32 fChunkOffset( ) const;
		u32 fChunkDownloaded( ) const;

	private:

		void fRequestNextChunk( );

		tAsyncFileDownloader( const tFilePathPtr& filePath, u32 fileSize, const iCloudStoragePtr& cloudStorage );
	};

} // ::Sig


#endif//__tAsyncFileDownloader__
