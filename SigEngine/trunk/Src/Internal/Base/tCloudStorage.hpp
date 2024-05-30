//------------------------------------------------------------------------------
// \file tCloudStorage.hpp - 17 Jul 2012
// \author colins
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __tCloudStorage__
#define __tCloudStorage__

#include "Http/Http.hpp"
#include "tJsonReader.hpp"
#include "tCloudStorageFileType.hpp"

namespace Sig
{
	class iCloudStorageRequest : public tRefCounter
	{
	public:

		virtual ~iCloudStorageRequest( ) { };

		virtual b32 fIsComplete( ) const = 0;
		virtual b32 fSuccess( ) const = 0;
		virtual b32 fFailed( ) const = 0;
		virtual u32 fHTTPStatusCode( ) const = 0;

		virtual void fWaitToComplete( ) const = 0;

		virtual const byte* fReadBuffer( ) const = 0;
		virtual u32 fContentLength( ) const = 0;
		virtual u32 fContentLengthRemaining( ) const = 0;
	};

	typedef tRefCounterPtr< iCloudStorageRequest > iCloudStorageRequestPtr;

	class iCloudStorage : public tRefCounter
	{
	public:

		/// \class	tFileBlob
		/// \brief	Represents the TMS++ metadata about a single file
		struct tFileBlob
		{
			tFileBlob( );

			tFilePathPtr			mFilePath;	///< Local-style filename (e.g. "{path\file}b" or "{path\file}j")
			std::string				mFileName;	///< Remote-style filename (e.g."{path/file}" or "{path/file}")
			u32						mSize;		///< Size of the file in bytes
			std::string				mEtag;		///< HTTP ETag header.
			Time::tDateTime			mTimeStamp;	///< JSON file list "clientFileTime" entry
			tCloudStorageFileType	mFileType;	///< URL binary/json postfix

			std::string fFileNameWithPostfix( ) const;

			b32 fLoad( tJsonReader& reader );
			void fSetFromPathAndSize( const tFilePathPtr& filePath, u32 size );
		};

		class tFileBlobList : public tGrowableArray< tFileBlob >
		{
		public:

			b32 fLoad( const byte* buffer, u32 bufferSize );
		};

		/// \class	tQuotaInfo
		/// \brief	Represents the TMS++ metadata about the current path's quota settings (Be that per-user or title-storage wide.)
		struct tQuotaInfo
		{
			tQuotaInfo( );

			u32 mQuotaBytesForUnverifiedStorage;
			u32 mQuotaBytesForVerifiedStorage;
			u32 mUsedBytesInUnverifiedStorage;
			u32 mUsedBytesInVerifiedStorage;

			b32 fLoad( const byte* buffer, u32 bufferSize );
		};

		typedef tDelegate< void ( ) > tFailedCallback;

		static const tStringPtr cFieldClientFileTime;

		virtual ~iCloudStorage( ) { }

		virtual b32 fInitGlobal
			( const tStringPtr& groupId
			, u32 userIdx
			, const tFailedCallback& onFailed ) = 0;

		virtual b32 fInitUser
			( const tStringPtr& groupId
			, u32 userIdx
			, tPlatformUserId userId
			, const tFailedCallback& onFailed ) = 0;

		virtual void fShutdown( ) = 0;
		virtual void fTick( ) = 0;

		virtual b32 fInitialized( ) const = 0;
		virtual u32 fUserIndex( ) const = 0;
		virtual b32 fReadyForRequests( ) const = 0;

		virtual b32 fGetFileList( ) = 0;
		virtual b32 fGetStorageQuota( ) = 0;
		virtual iCloudStorageRequestPtr fGetFile( const tFilePathPtr& filePath ) = 0;
		virtual iCloudStorageRequestPtr fGetFileChunk( const tFilePathPtr& filePath, u32 chunkOffset, u32 chunkSize ) = 0;
		virtual iCloudStorageRequestPtr fDeleteFile( const tFilePathPtr& filePath ) = 0;
		virtual iCloudStorageRequestPtr fPutFile( const tFilePathPtr& filePath, const void* data, u32 dataSize ) = 0;

		virtual b32 fQuotaInfoPending( ) const = 0;
		virtual b32 fQuotaInfoValid( ) const = 0;
		virtual const tQuotaInfo* fQuotaInfo( ) = 0;

		virtual b32 fFileListPending( ) const = 0;
		virtual b32 fFileListValid( ) const = 0;
		virtual b32 fFileExists( const tFilePathPtr& filePath ) const = 0;
		virtual const tFileBlob* fFindFileBlob( const tFilePathPtr& filePath ) const = 0;
		virtual tArraySleeve<const tFileBlob> fFileBlobs( ) const = 0;
	};

	define_smart_ptr( base_export, tRefCounterPtr, iCloudStorage );

} // ::Sig


#endif//__tCloudStorage__
