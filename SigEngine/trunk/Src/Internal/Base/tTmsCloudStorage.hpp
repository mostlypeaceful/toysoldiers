//------------------------------------------------------------------------------
// \file tTmsCloudStorage.hpp - 04 Nov 2013
// \author jwittner
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tTmsCloudStorage__
#define __tTmsCloudStorage__

#include "tCloudStorage.hpp"

#ifdef platform_xbox360
#include <xtms.h>

namespace Sig
{
	class tTmsCloudStorage : public iCloudStorage
	{
	public:

		tTmsCloudStorage( );
		virtual ~tTmsCloudStorage( );

		virtual b32 fInitGlobal
			( const tStringPtr& groupId
			, u32 userIdx
			, const tFailedCallback& onFailed ) OVERRIDE;

		virtual b32 fInitUser
			( const tStringPtr& groupId
			, u32 userIdx
			, tPlatformUserId userId
			, const tFailedCallback& onFailed ) OVERRIDE;

		virtual void fShutdown( ) OVERRIDE;
		virtual void fTick( ) OVERRIDE;

		virtual b32 fInitialized( ) const OVERRIDE;
		virtual u32 fUserIndex( ) const OVERRIDE;
		virtual b32 fReadyForRequests( ) const OVERRIDE;

		virtual b32 fGetFileList( ) OVERRIDE;
		virtual b32 fGetStorageQuota( ) OVERRIDE;
		virtual iCloudStorageRequestPtr fGetFile( const tFilePathPtr& filePath ) OVERRIDE;
		virtual iCloudStorageRequestPtr fGetFileChunk( const tFilePathPtr& filePath, u32 chunkOffset, u32 chunkSize ) OVERRIDE;
		virtual iCloudStorageRequestPtr fDeleteFile( const tFilePathPtr& filePath ) OVERRIDE;
		virtual iCloudStorageRequestPtr fPutFile( const tFilePathPtr& filePath, const void* data, u32 dataSize ) OVERRIDE;

		virtual b32 fQuotaInfoPending( ) const OVERRIDE { return !mQuotaInfoRequest.fNull( ); }
		virtual b32 fQuotaInfoValid( ) const OVERRIDE { return mQuotaInfoValid; }
		virtual const tQuotaInfo* fQuotaInfo( ) OVERRIDE;

		virtual b32 fFileListPending( ) const OVERRIDE { return !mFileListRequest.fNull( ); }
		virtual b32 fFileListValid( ) const OVERRIDE { return mFileListValid; }
		virtual b32 fFileExists( const tFilePathPtr& filePath ) const OVERRIDE;
		virtual const tFileBlob* fFindFileBlob( const tFilePathPtr& filePath ) const OVERRIDE;
		virtual tArraySleeve<const tFileBlob> fFileBlobs( ) const OVERRIDE;

	private:

		class tRequest : public iCloudStorageRequest
		{
		public:

			tRequest( tTmsCloudStorage* cloudPtr, u32 resultSize );
			virtual ~tRequest( ) { };

			virtual b32 fIsComplete( ) const OVERRIDE;
			virtual b32 fSuccess( ) const OVERRIDE;
			virtual b32 fFailed( ) const OVERRIDE;
			virtual u32 fHTTPStatusCode( ) const OVERRIDE;

			virtual void fWaitToComplete( ) const OVERRIDE;

			virtual const byte* fReadBuffer( ) const OVERRIDE;
			virtual u32 fContentLength( ) const OVERRIDE;
			virtual u32 fContentLengthRemaining( ) const OVERRIDE;

			byte* fResultBuffer( ) { return mResults.fBegin( ); }
			u32 fResultBufferSize( ) const { return mResults.fCount( ); }

			void fOnComplete( b32 success, u32 statusCode );

		private:

			enum tState
			{
				cState_Pending,
				cState_Success,
				cState_Error
			};
			
		private:

			u32 mState;
			u32 mStatusCode;
			tRefCounterPtr< tTmsCloudStorage > mStorage;
			tDynamicBuffer mResults;
		};

	private:

		void fOnFailed( );
		tFileBlob* fFindFileBlob( const tFilePathPtr& filePath );

		static void fOnFileListResults( HRESULT results, HttpResponse *response, void* userCallbackData );
		static void fOnQuotaResults( HRESULT results, HttpResponse *response, void* userCallbackData );
		static void fOnTmsResults( HRESULT results, HttpResponse *response, void* userCallbackData );

	private:

		u32 mUserIdx;
		tPlatformUserId mUserId;

		b32 mFileListValid;
		tFileBlobList mFileList;
		iCloudStorageRequestPtr mFileListRequest;

		b32 mQuotaInfoValid;
		tQuotaInfo mQuotaInfo;
		iCloudStorageRequestPtr mQuotaInfoRequest;

		tFailedCallback mOnFailed;
		HTMSCLIENT mClient;

	};


} // ::Sig

#endif // platform_xbox360


#endif//__tTmsCloudStorage__
