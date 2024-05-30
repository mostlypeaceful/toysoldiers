//------------------------------------------------------------------------------
// \file tTmsCloudStorage.cpp - 04 Nov 2013
// \author jwittner
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tTmsCloudStorage.hpp"
#include "tCloudStorageFileType.hpp"

namespace Sig
{

#ifdef platform_xbox360

#define INVALID_HTMSCLIENT_VALUE (HTMSCLIENT)INVALID_HANDLE_VALUE

	//------------------------------------------------------------------------------
	// tTmsCloudStorage
	//------------------------------------------------------------------------------
	tTmsCloudStorage::tTmsCloudStorage( )
		: mClient( INVALID_HTMSCLIENT_VALUE )
		, mUserIdx( tUser::cMaxLocalUsers )
		, mUserId( tUser::cInvalidUserId )
		, mFileListValid( false )
		, mQuotaInfoValid( false )
	{

	}

	//------------------------------------------------------------------------------
	tTmsCloudStorage::~tTmsCloudStorage( )
	{
		if( mClient != INVALID_HTMSCLIENT_VALUE )
			fShutdown( );
	}

	//------------------------------------------------------------------------------
	b32 tTmsCloudStorage::fInitGlobal
		( const tStringPtr& groupId
		, u32 userIdx
		, const tFailedCallback& onFailed )
	{
		sigcheckfail( mClient == INVALID_HTMSCLIENT_VALUE, return false );
		sigcheckfail( userIdx < tUser::cMaxLocalUsers, return false );

		mUserIdx = userIdx;
		mOnFailed = onFailed;
		
		// Create the client
		mClient = XTmsCreateClient( );
		
		// Initialize the client
		const HRESULT result = XTmsInitialize( mClient, groupId.fCStr( ), userIdx );
		if( result != S_OK )
		{
			log_warning( "XTmsInitialize failed with " << result << " " << GetLastError( ) );
			fShutdown( );
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------
	b32 tTmsCloudStorage::fInitUser
		( const tStringPtr& groupId
		, u32 userIdx
		, tPlatformUserId userId
		, const tFailedCallback& onFailed )
	{
		sigcheckfail( mClient == INVALID_HTMSCLIENT_VALUE, return false );
		sigcheckfail( userIdx < tUser::cMaxLocalUsers, return false );
		sigcheckfail( userId != tUser::cInvalidUserId, return false );

		const b32 initResult = fInitGlobal( groupId, userIdx, onFailed );
		if( initResult )
			mUserId = userId;
		
		return initResult;
	}

	//------------------------------------------------------------------------------
	void tTmsCloudStorage::fShutdown( )
	{
		if( mClient == INVALID_HTMSCLIENT_VALUE ) 
			return;

		// Wait on the tms client to finish
		while( 1 )
		{
			HRESULT result = XTmsShutdown( mClient );
			if( result != TMS_E_CLIENT_PENDING_REQUESTS )
				break;

			fTick( );
		}

		mClient = INVALID_HTMSCLIENT_VALUE;
		mUserIdx = tUser::cMaxLocalUsers;
		mUserId = tUser::cInvalidUserId;
		mFileListRequest.fRelease( );
		mQuotaInfoRequest.fRelease( );
		mOnFailed.fSetToNull( );
	}

	//------------------------------------------------------------------------------
	void tTmsCloudStorage::fTick( )
	{
		sigcheckfail( mClient != INVALID_HTMSCLIENT_VALUE, return );

		const HRESULT result = XTmsDoWork( mClient );
		log_assert( result == S_OK, "XTmsDoWork failed with " << result << ", " << GetLastError( ) );
	}

	//------------------------------------------------------------------------------
	b32 tTmsCloudStorage::fInitialized( ) const
	{
		return mClient != INVALID_HTMSCLIENT_VALUE;
	}

	//------------------------------------------------------------------------------
	u32 tTmsCloudStorage::fUserIndex( ) const
	{
		return mUserIdx;
	}

	//------------------------------------------------------------------------------
	b32 tTmsCloudStorage::fReadyForRequests( ) const
	{
		return fInitialized( );
	}

	//------------------------------------------------------------------------------
	b32 tTmsCloudStorage::fGetFileList( )
	{
		sigcheckfail( fReadyForRequests( ), return false );

		if( !mFileListRequest )
		{
			tRefCounterPtr<tRequest> requestPtr( NEW_TYPED( tRequest )( this, 0 ) );

			HRESULT result;
			if( mUserId == tUser::cInvalidUserId )
				result = XTmsGetGlobalFileList( mClient, "", 0, 0, NULL, fOnFileListResults, this );
			else 
				result = XTmsGetUserFileList( mClient, mUserId, "", 0, 0, NULL, fOnFileListResults, this );

			if( result != S_OK )
			{
				log_warning( "[CloudStorage] Failed to open request for file list" );
				fOnFailed( );

				return false;
			}

			fAddRef( ); // Reference for callback
			mFileListRequest.fReset( requestPtr.fGetRawPtr( ) );
		}

		return fFileListPending( );
	}

	//------------------------------------------------------------------------------
	b32 tTmsCloudStorage::fGetStorageQuota( )
	{
		sigcheckfail( fReadyForRequests( ), return false );

		if( !mQuotaInfoRequest )
		{
			tRefCounterPtr<tRequest> requestPtr( NEW_TYPED( tRequest )( this, 0 ) );

			HRESULT result;
			if( mUserId == tUser::cInvalidUserId )
				result = XTmsGetGlobalQuotaInfo( mClient, fOnQuotaResults, this );
			else
				result = XTmsGetUserQuotaInfo( mClient, mUserId, fOnQuotaResults, this );

			if( result != S_OK )
			{
				log_warning( "[CloudStorage] Failed to open request for storage quota" );
				fOnFailed( );

				return false;
			}

			fAddRef( ); // Ref for callback
			mQuotaInfoRequest.fReset( requestPtr.fGetRawPtr( ) );
		}

		return fQuotaInfoPending( );
	}

	//------------------------------------------------------------------------------
	iCloudStorageRequestPtr tTmsCloudStorage::fGetFile( const tFilePathPtr& filePath )
	{
		sigcheckfail( fReadyForRequests( ), return iCloudStorageRequestPtr( ) );

		const tFileBlob* fileBlob = fFindFileBlob( filePath );
		if( !fileBlob )
		{
			log_warning( "[CloudStorage] Unable to request file: " << filePath );
			return iCloudStorageRequestPtr( );
		}

		tRefCounterPtr<tRequest> requestPtr( NEW_TYPED( tRequest )( this, fileBlob->mSize ) );

		HRESULT result;
		if( mUserId == tUser::cInvalidUserId )
		{
			result = XTmsGetGlobalFile
				( mClient
				, fileBlob->mFileName.c_str( )
				, fGetCloudStoragePostFix( fileBlob->mFileType ).c_str( )
				, requestPtr->fResultBuffer( )
				, requestPtr->fResultBufferSize( )
				, ""
				, ""
				, ""
				, IGNORE_ETAG
				, fOnTmsResults
				, requestPtr.fGetRawPtr( ) );
		}
		else
		{
			result = XTmsGetUserFile
				( mClient
				, mUserId
				, fileBlob->mFileName.c_str( )
				, fGetCloudStoragePostFix( fileBlob->mFileType ).c_str( )
				, requestPtr->fResultBuffer( )
				, requestPtr->fResultBufferSize( )
				, ""
				, ""
				, ""
				, IGNORE_ETAG
				, fOnTmsResults
				, requestPtr.fGetRawPtr( ) );
		}

		if( result != S_OK )
		{
			log_warning( "XTmsGet*File failed with: " << result << ", " << GetLastError( ) );
			return iCloudStorageRequestPtr( );
		}

		// Add reference to be cleared when callback happens
		requestPtr->fAddRef( );
		return iCloudStorageRequestPtr( requestPtr.fGetRawPtr( ) );
	}

	//------------------------------------------------------------------------------
	iCloudStorageRequestPtr tTmsCloudStorage::fGetFileChunk( const tFilePathPtr& filePath, u32 chunkOffset, u32 chunkSize )
	{
		sigcheckfail( chunkSize, return iCloudStorageRequestPtr( ) );
		sigcheckfail( fReadyForRequests( ), return iCloudStorageRequestPtr( ) );

		const tFileBlob* fileBlob = fFindFileBlob( filePath );
		if( !fileBlob )
		{
			log_warning( "[CloudStorage] Unable to request file chunk: " << filePath );
			return iCloudStorageRequestPtr( );
		}

		sigcheckfail( chunkOffset < fileBlob->mSize, return iCloudStorageRequestPtr( ) );

		// Reduce the chunk size to fit real file space
		chunkSize = fMin( chunkSize, fileBlob->mSize - chunkOffset );
		tRefCounterPtr<tRequest> requestPtr( NEW_TYPED( tRequest )( this, chunkSize ) );

		HRESULT result;
		if( mUserId == tUser::cInvalidUserId )
		{
			result = XTmsGetGlobalFileWithRange
				( mClient
				, fileBlob->mFileName.c_str( )
				, fGetCloudStoragePostFix( fileBlob->mFileType ).c_str( )
				, requestPtr->fResultBuffer( )
				, requestPtr->fResultBufferSize( )
				, ""
				, ""
				, chunkOffset
				, chunkOffset + chunkSize - 1
				, ""
				, IGNORE_ETAG
				, fOnTmsResults
				, requestPtr.fGetRawPtr( ) );
		}
		else
		{
			result = XTmsGetUserFileWithRange
				( mClient
				, mUserId
				, fileBlob->mFileName.c_str( )
				, fGetCloudStoragePostFix( fileBlob->mFileType ).c_str( )
				, requestPtr->fResultBuffer( )
				, requestPtr->fResultBufferSize( )
				, ""
				, ""
				, chunkOffset
				, chunkOffset + chunkSize - 1
				, ""
				, IGNORE_ETAG
				, fOnTmsResults
				, requestPtr.fGetRawPtr( ) );
		}

		if( result != S_OK )
		{
			log_warning( "XTmsGet*FileWithRange failed with: " << result << ", " << GetLastError( ) );
			return iCloudStorageRequestPtr( );
		}

		// Add reference to be cleared when callback happens
		requestPtr->fAddRef( );
		return iCloudStorageRequestPtr( requestPtr.fGetRawPtr( ) );
	}

	//------------------------------------------------------------------------------
	iCloudStorageRequestPtr tTmsCloudStorage::fDeleteFile( const tFilePathPtr& filePath )
	{
		sigcheckfail( fReadyForRequests( ), return iCloudStorageRequestPtr( ) );
		sigcheckfail( mUserId != tUser::cInvalidUserId, return iCloudStorageRequestPtr( ) );

		const tFileBlob* fileBlob = fFindFileBlob( filePath );
		if( !fileBlob )
		{
			log_warning( "[CloudStorage] Unable to delete file: " << filePath );
			return iCloudStorageRequestPtr( );
		}

		tRefCounterPtr<tRequest> requestPtr( NEW_TYPED( tRequest )( this, 0 ) );

		const HRESULT result = XTmsDeleteUserFile
			( mClient
			, mUserId
			, fileBlob->mFileName.c_str( )
			, fGetCloudStoragePostFix( fileBlob->mFileType ).c_str( )
			, ""
			, IGNORE_ETAG
			, fOnTmsResults
			, requestPtr.fGetRawPtr( ) );

		if( result != S_OK )
			return iCloudStorageRequestPtr( );

		mFileList.fErase( fPtrDiff( fileBlob, mFileList.fBegin( ) ) );

		requestPtr->fAddRef( );
		return iCloudStorageRequestPtr( requestPtr.fGetRawPtr( ) );
	}

	//------------------------------------------------------------------------------
	iCloudStorageRequestPtr tTmsCloudStorage::fPutFile( const tFilePathPtr& filePath, const void* data, u32 dataSize )
	{
		sigcheckfail( fReadyForRequests( ), return iCloudStorageRequestPtr( ) );
		sigcheckfail( mUserId != tUser::cInvalidUserId, return iCloudStorageRequestPtr( ) );

		// Find or create the file blob
		tFileBlob* fileBlob = fFindFileBlob( filePath );
		if( !fileBlob )
		{
			fileBlob = &mFileList.fPushBack( );
			fileBlob->fSetFromPathAndSize( filePath, dataSize );
		}
		else 
			fileBlob->mTimeStamp = Time::tDateTime::fNowLocal( ); //update timestamp to latest

		tRefCounterPtr<tRequest> requestPtr( NEW_TYPED( tRequest )( this, 0) );

		const HRESULT result = XTmsPutUserFile
			( mClient
			, mUserId
			, fileBlob->mFileName.c_str( )
			, fGetCloudStoragePostFix( fileBlob->mFileType ).c_str( )
			, NULL
			, ""
			, ( const CHAR* )data
			, dataSize
			, 0
			, ""
			, IGNORE_ETAG
			, fOnTmsResults
			, requestPtr.fGetRawPtr( ) );

		if( result != S_OK )
			return iCloudStorageRequestPtr( );

		requestPtr->fAddRef( );
		return iCloudStorageRequestPtr( requestPtr.fGetRawPtr( ) );
	}

	//------------------------------------------------------------------------------
	const iCloudStorage::tQuotaInfo* tTmsCloudStorage::fQuotaInfo( )
	{
		if( !mQuotaInfoValid )
			return NULL;

		return &mQuotaInfo;
	}

	//------------------------------------------------------------------------------
	b32 tTmsCloudStorage::fFileExists( const tFilePathPtr& filePath ) const
	{
		return fFindFileBlob( filePath ) ? true : false;
	}

	//------------------------------------------------------------------------------
	void tTmsCloudStorage::fOnFailed( )
	{
		mFileListRequest.fRelease( );
		mQuotaInfoRequest.fRelease( );
		if( !mOnFailed.fNull( ) )
			mOnFailed( );
	}

	//------------------------------------------------------------------------------
	const iCloudStorage::tFileBlob* tTmsCloudStorage::fFindFileBlob( const tFilePathPtr& filePath ) const
	{
		for( u32 i = 0; i < mFileList.fCount( ); ++i )
		{
			if( mFileList[ i ].mFilePath == filePath )
				return &mFileList[ i ];
		}

		return NULL;
	}

	//------------------------------------------------------------------------------
	tArraySleeve<const iCloudStorage::tFileBlob> tTmsCloudStorage::fFileBlobs( ) const
	{
		return mFileList.fSleeve( );
	}

	//------------------------------------------------------------------------------
	iCloudStorage::tFileBlob* tTmsCloudStorage::fFindFileBlob( const tFilePathPtr& filePath )
	{
		const tTmsCloudStorage* constThis = ( const tTmsCloudStorage* )this;
		return const_cast< tFileBlob* >( constThis->fFindFileBlob( filePath ) );
	}

	//------------------------------------------------------------------------------
	void tTmsCloudStorage::fOnFileListResults( HRESULT results, HttpResponse *response, void* userCallbackData )
	{
		b32 success = false;

		tTmsCloudStorage* thisPtr = ( tTmsCloudStorage* )userCallbackData;
		if( results == S_OK )
		{
			if( response->returnCode == Http::cStatusOk || 
				response->returnCode == Http::cStatusNotFound )
			{
				thisPtr->mFileListValid = thisPtr->mFileList.fLoad( (const byte*)response->buffer, StringUtil::fStrLen( response->buffer ) );
				thisPtr->mFileListRequest.fRelease( );
				success = true;
			}
		}
		
		if( !success )
		{
			log_warning( "[CloudStorage] File list request failed: " << response->returnCode );
			thisPtr->fOnFailed( );
		}

		thisPtr->fDecRef( );
	}

	//------------------------------------------------------------------------------
	void tTmsCloudStorage::fOnQuotaResults( HRESULT results, HttpResponse *response, void* userCallbackData )
	{
		b32 success = false;

		tTmsCloudStorage* thisPtr = ( tTmsCloudStorage* )userCallbackData;
		if( results == S_OK && response->returnCode == Http::cStatusOk )
		{
			thisPtr->mQuotaInfoValid = thisPtr->mQuotaInfo.fLoad( ( const byte* )response->buffer, StringUtil::fStrLen( response->buffer ) );
			thisPtr->mQuotaInfoRequest.fRelease( );
		}

		if( !success )
		{
			log_warning( "[CloudStorage] Quota request failed: " << response->returnCode );
			thisPtr->fOnFailed( );
		}

		thisPtr->fDecRef( );
	}

	//------------------------------------------------------------------------------
	void tTmsCloudStorage::fOnTmsResults( HRESULT results, HttpResponse *response, void* userCallbackData )
	{
		tRequest* request = (tRequest*)userCallbackData;
		request->fOnComplete( results == S_OK, response->returnCode );
		request->fDecRef( );
	}

	//------------------------------------------------------------------------------
	// tTmsCloudStorage::tRequest
	//------------------------------------------------------------------------------
	tTmsCloudStorage::tRequest::tRequest( tTmsCloudStorage* cloudPtr, u32 resultSize )
		: mResults( resultSize )
		, mState( cState_Pending )
		, mStatusCode( Http::cStatusBadRequest )
		, mStorage( cloudPtr )
	{

	}

	//------------------------------------------------------------------------------
	b32 tTmsCloudStorage::tRequest::fIsComplete( ) const
	{
		return mState != cState_Pending;
	}

	//------------------------------------------------------------------------------
	b32 tTmsCloudStorage::tRequest::fSuccess( ) const
	{
		return mState == cState_Success;
	}

	//------------------------------------------------------------------------------
	b32 tTmsCloudStorage::tRequest::fFailed( ) const
	{
		return mState == cState_Error;
	}

	//------------------------------------------------------------------------------
	u32 tTmsCloudStorage::tRequest::fHTTPStatusCode( ) const
	{
		return mStatusCode;
	}

	//------------------------------------------------------------------------------
	void tTmsCloudStorage::tRequest::fWaitToComplete( ) const
	{
		while( !fIsComplete( ) )
			mStorage->fTick( );
	}

	//------------------------------------------------------------------------------
	const byte* tTmsCloudStorage::tRequest::fReadBuffer( ) const
	{
		return mResults.fBegin( );
	}

	//------------------------------------------------------------------------------
	u32 tTmsCloudStorage::tRequest::fContentLength( ) const
	{
		return mResults.fCount( );
	}

	//------------------------------------------------------------------------------
	u32 tTmsCloudStorage::tRequest::fContentLengthRemaining( ) const
	{
		// We have no insight into XTMS process - it's all or nothing
		return fIsComplete( ) ? 0 : fContentLength( );
	}

	//------------------------------------------------------------------------------
	void tTmsCloudStorage::tRequest::fOnComplete( b32 success, u32 statusCode )
	{
		mState = success ? cState_Success : cState_Error;
		mStatusCode = statusCode;
	}

#endif // platform_xbox360

} // ::Sig