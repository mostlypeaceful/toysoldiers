//------------------------------------------------------------------------------
// \file tHttpCloudStorage.hpp - 04 Nov 2013
// \author jwittner
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tHttpCloudStorage__
#define __tHttpCloudStorage__

#include "tCloudStorage.hpp"

namespace Sig
{
	/// \class	tHttpCloudStorage
	/// \brief	Provides access to file stored in the cloud
	class tHttpCloudStorage : public iCloudStorage
	{
	public:

		tHttpCloudStorage( );
		virtual ~tHttpCloudStorage( ) { }

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

		virtual b32 fInitialized( ) const OVERRIDE { return mUserIdx != ~0; }
		virtual u32 fUserIndex( ) const OVERRIDE { return mUserIdx; }
		virtual b32 fReadyForRequests( ) const  { return mEndpoint && mEndpoint->fReady( ); }
		
		virtual b32 fGetStorageQuota( ) OVERRIDE;
		virtual b32 fGetFileList( ) OVERRIDE;

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

			static iCloudStorageRequestPtr fCreate( const Http::tRequestPtr& ptr );

		public:

			tRequest( Http::tRequestPtr request ) : mRequest( request ) { }
			virtual ~tRequest( ) { };

			virtual b32 fIsComplete( ) const OVERRIDE { return mRequest->fIsComplete( ); }
			virtual b32 fSuccess( ) const OVERRIDE { return mRequest->fSuccess( ); }
			virtual b32 fFailed( ) const OVERRIDE { return mRequest->fFailed( ); }
			virtual u32 fHTTPStatusCode( ) const OVERRIDE { return mRequest->fHTTPStatusCode( ); }
			
			virtual void fWaitToComplete( ) const;

			virtual const byte* fReadBuffer( ) const OVERRIDE { return mRequest->fReadBuffer( ); }
			virtual u32 fContentLength( ) const OVERRIDE { return mRequest->fContentLength( ); }
			virtual u32 fContentLengthRemaining( ) const OVERRIDE { return mRequest->fContentLengthRemaining( ); }

		private:

			Http::tRequestPtr mRequest;
		};

	private:

		void fSetupEndpoint( );
		void fOnFailed( );

		void fProcessFileListRequest( );
		void fProcessQuotaInfoRequest( );

		tFileBlob* fFindFileBlob( const tFilePathPtr& filePath );

	private:

		Http::tEndpointPtr mEndpoint;

		tStringPtr mQuotaURI;
		tStringPtr mFilePathURI;
		
		b32 mFileListValid;
		tFileBlobList mFileList;
		iCloudStorageRequestPtr mFileListRequest;

		b32 mQuotaInfoValid;
		tQuotaInfo mQuotaInfo;
		iCloudStorageRequestPtr mQuotaInfoRequest;

		u32 mUserIdx;
		tPlatformUserId mUserId;
		tFailedCallback mOnFailed;
	};

	define_smart_ptr( base_export, tRefCounterPtr, tHttpCloudStorage );

} // ::Sig

#endif//__tHttpCloudStorage__
