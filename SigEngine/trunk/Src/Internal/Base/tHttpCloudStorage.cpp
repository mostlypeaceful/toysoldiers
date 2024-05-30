//------------------------------------------------------------------------------
// \file tHttpCloudStorage.cpp - 04 Nov 2013
// \author jwittner
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tHttpCloudStorage.hpp"
#include "tUrlEncode.hpp"

#if defined( platform_xbox360 )
#include <xgetserviceendpoint.h>
#endif

namespace Sig
{
	namespace
	{
		static const u32 cDefaultResponseSize = Memory::fFromKB<u32>( 64 );

		//--------------------------------------------------------------------------------------
		// Helper functions
		//--------------------------------------------------------------------------------------
		std::string fGetClientFileTimeStr( const Time::tDateTime& dateTime )
		{
			std::stringstream ss;

			ss << std::setfill( '0' );
			ss << std::setw( 4 ) << dateTime.fYear( ) << "-";
			ss << std::setw( 2 ) << dateTime.fMonth( ) << "-";
			ss << std::setw( 2 ) << dateTime.fDay( ) << "T";
			ss << std::setw( 2 ) << dateTime.fHour( ) << ":";
			ss << std::setw( 2 ) << dateTime.fMinute( ) << ":";
			ss << std::setw( 2 ) << dateTime.fSecond( ) << "Z";

			std::string encodedStr;
			tUrlEncode::fEncode( ss.str( ), encodedStr );
			return encodedStr;
		}

#if defined( platform_xbox360 )
		tStringPtr fGetServiceEndpoint( )
		{
			tFixedArray< char, 1024 > serviceEndpoint;
			DWORD result = XGetServiceEndpoint
				( "titlestorage"
				, serviceEndpoint.fBegin( )
				, serviceEndpoint.cDimension
				, NULL );

			if( result != ERROR_SUCCESS )
				log_warning( "XGetServiceEndpoint failed with result: " << result );

			return tStringPtr( serviceEndpoint.fBegin( ) );
		}
#else
		tStringPtr fGetServiceEndpoint( )
		{
			log_warning_unimplemented( );
			return tStringPtr::cNullPtr;
		}
#endif
	}

	//--------------------------------------------------------------------------------------
	// tHttpCloudStorage
	//--------------------------------------------------------------------------------------
	tHttpCloudStorage::tHttpCloudStorage( )
		: mFileListValid( false )
		, mQuotaInfoValid( false )
		, mUserIdx( tUser::cMaxLocalUsers )
		, mUserId( tUser::cInvalidUserId )
	{
	}

	//------------------------------------------------------------------------------
	b32 tHttpCloudStorage::fInitGlobal
		( const tStringPtr& groupId
		, u32 userIdx
		, const tFailedCallback& onFailed )
	{
		sigcheckfail( groupId.fLength( ), return false );
		sigcheckfail( userIdx < tUser::cMaxLocalUsers, return false );

		std::stringstream quotaURI;
		std::stringstream filePathURI;

		quotaURI << "/media/titlegroups/" << groupId.fCStr( ) << "/storage";
		filePathURI << "/media/titlegroups/" << groupId.fCStr( ) << "/storage/data/";

		mQuotaURI = tStringPtr( quotaURI.str( ) );
		mFilePathURI = tStringPtr( filePathURI.str( ) );

		mUserIdx = userIdx;
		mOnFailed = onFailed;
		fSetupEndpoint( );

		return !mEndpoint.fNull( );
	}

	//------------------------------------------------------------------------------
	b32 tHttpCloudStorage::fInitUser
		( const tStringPtr& groupId
		, u32 userIdx
		, tPlatformUserId userId
		, const tFailedCallback& onFailed )
	{
		sigcheckfail( groupId.fLength( ), return false );
		sigcheckfail( userIdx < tUser::cMaxLocalUsers , return false );
		sigcheckfail( userId != tUser::cInvalidUserId, return false );

		std::stringstream quotaURI;
		std::stringstream filePathURI;

		quotaURI << "/users/xuid(" << userId << ")/storage/titlestorage/titlegroups/" << groupId.fCStr( );
		filePathURI << "/users/xuid(" << userId << ")/storage/titlestorage/titlegroups/" << groupId.fCStr( ) << "/data/";

		mQuotaURI = tStringPtr( quotaURI.str( ) );
		mFilePathURI = tStringPtr( filePathURI.str( ) );

		mUserIdx = userIdx;
		mUserId = userId;
		mOnFailed = onFailed;
		fSetupEndpoint( );

		return !mEndpoint.fNull( );
	}

	//------------------------------------------------------------------------------
	void tHttpCloudStorage::fShutdown( )
	{
		mUserIdx = tUser::cMaxLocalUsers;
		mUserId = tUser::cInvalidUserId;

		mFileListValid = false;
		mFileList.fSetCount( 0 );
		mFileListRequest.fRelease( );

		mQuotaInfoValid = false;
		fZeroOut( mQuotaInfo );
		mQuotaInfoRequest.fRelease( );

		mEndpoint.fRelease( );
		mOnFailed.fSetToNull( );
	}

	//------------------------------------------------------------------------------
	void tHttpCloudStorage::fTick( )
	{
		if( mEndpoint.fNull( ) )
			return;

		if( mEndpoint->fFailed( ) )
		{
			log_warning( "[CloudStorage] Failed to connect to: " << mEndpoint->fHostUrl( ) );
			mEndpoint.fRelease( );
			fOnFailed( );
		}

		fProcessFileListRequest( );
		fProcessQuotaInfoRequest( );
	}

	//------------------------------------------------------------------------------
	b32 tHttpCloudStorage::fGetFileList( )
	{
		sigcheckfail( fReadyForRequests( ), return false );

		if( !mFileListRequest )
		{
			mFileListRequest = tRequest::fCreate( 
				mEndpoint->fOpenRequest( "GET", mFilePathURI.fCStr( ), NULL, NULL, 0, cDefaultResponseSize ) );
			if( mFileListRequest.fNull( ) )
			{
				log_warning( "[CloudStorage] Failed to open request for file list" );
				fOnFailed( );
			}
		}

		return fFileListPending( );
	}

	//------------------------------------------------------------------------------
	b32 tHttpCloudStorage::fGetStorageQuota( )
	{
		sigcheckfail( fReadyForRequests( ), return false );

		if( !mQuotaInfoRequest )
		{
			mQuotaInfoRequest = tRequest::fCreate( 
				mEndpoint->fOpenRequest( "GET", mQuotaURI.fCStr( ), NULL, NULL, 0, cDefaultResponseSize ) );
			if( mQuotaInfoRequest.fNull( ) )
			{
				log_warning( "[CloudStorage] Failed to open request for storage quota" );
				fOnFailed( );
			}
		}

		return fQuotaInfoPending( );
	}

	//------------------------------------------------------------------------------
	iCloudStorageRequestPtr tHttpCloudStorage::fGetFile( const tFilePathPtr& filePath )
	{
		sigcheckfail( fReadyForRequests( ), return iCloudStorageRequestPtr( ) );

		const tFileBlob* fileBlob = fFindFileBlob( filePath );
		if( !fileBlob )
		{
			log_warning( "[CloudStorage] Unable to request file: " << filePath );
			return iCloudStorageRequestPtr( );
		}

		std::stringstream uri;
		uri << mFilePathURI.fCStr( ) << fileBlob->fFileNameWithPostfix( );
		const u32 responseBufferSize = fileBlob->mSize + 1; // Adding 1 for logging of text responses
		
		iCloudStorageRequestPtr newRequest = tRequest::fCreate( 
			mEndpoint->fOpenRequest( "GET", uri.str( ).c_str( ), NULL, NULL, 0, responseBufferSize ) );
		return newRequest;
	}

	//------------------------------------------------------------------------------
	iCloudStorageRequestPtr tHttpCloudStorage::fGetFileChunk
		( const tFilePathPtr& filePath, u32 chunkOffset, u32 chunkSize )
	{
		sigcheckfail( fReadyForRequests( ), return iCloudStorageRequestPtr( ) );
		sigcheckfail( chunkSize > 0, return iCloudStorageRequestPtr( ) );

		const tFileBlob* fileBlob = fFindFileBlob( filePath );
		if( !fileBlob )
		{
			log_warning( "[CloudStorage] Unable to request file: " << filePath );
			return iCloudStorageRequestPtr( );
		}

		sigcheckfail( chunkOffset < fileBlob->mSize, return iCloudStorageRequestPtr( ) );

		// Clamp the size to the real size of the requested range
		chunkSize = fMin( chunkSize, fileBlob->mSize - chunkOffset );

		std::stringstream contentHeader;
		const u32 chunkEnd = chunkOffset + chunkSize - 1;
		contentHeader << "Range: bytes=" << chunkOffset << "-" << chunkEnd << "\r\n";

		log_line( Log::cFlagHttp, "[CloudStorage] Requesting file: " << fileBlob->mFileName << ", bytes: " << chunkOffset << "-" << chunkEnd );

		std::stringstream urlPath;
		urlPath << mFilePathURI.fCStr( ) << fileBlob->fFileNameWithPostfix( );
		
		iCloudStorageRequestPtr newRequest = tRequest::fCreate( 
			mEndpoint->fOpenRequest( "GET", urlPath.str( ).c_str( ), contentHeader.str( ).c_str( ), NULL, 0, chunkSize ) );
		
		if( newRequest.fNull( ) )
			log_warning( "[CloudStorage] Unable to request file: " << filePath );

		return newRequest;
	}

	//------------------------------------------------------------------------------
	iCloudStorageRequestPtr tHttpCloudStorage::fDeleteFile( const tFilePathPtr& filePath )
	{
		sigcheckfail( fReadyForRequests( ), return iCloudStorageRequestPtr( ) );

		const tFileBlob* fileBlob = fFindFileBlob( filePath );
		if( !fileBlob )
		{
			log_warning( "[CloudStorage] Unable to delete file: " << filePath );
			return iCloudStorageRequestPtr( );
		}

		std::stringstream uri;
		uri << mFilePathURI.fCStr( ) << fileBlob->fFileNameWithPostfix( );
		
		iCloudStorageRequestPtr newRequest = tRequest::fCreate( 
			mEndpoint->fOpenRequest( "DELETE", uri.str( ).c_str( ), NULL, NULL, 0, cDefaultResponseSize ) );

		mFileList.fErase( fPtrDiff( fileBlob, mFileList.fBegin( ) ) );

		return newRequest;
	}

	//------------------------------------------------------------------------------
	iCloudStorageRequestPtr tHttpCloudStorage::fPutFile
		( const tFilePathPtr& filePath, const void* data, u32 dataSize )
	{
		log_line( Log::cFlagHttp, "NOWLOCAL[" << Time::tDateTime::fNowLocal( ) << "]" );
		sigcheckfail( fReadyForRequests( ), return iCloudStorageRequestPtr( ) );

		// Find or create the file blob
		tFileBlob* fileBlob = fFindFileBlob( filePath );
		if( !fileBlob )
		{
			fileBlob = &mFileList.fPushBack( );
			fileBlob->fSetFromPathAndSize( filePath, dataSize );
		}
		else 
			fileBlob->mTimeStamp = Time::tDateTime::fNowLocal( ); //update timestamp to latest

		log_line( Log::cFlagHttp, "[" << fileBlob->mFileName << "] timestamp[" << fileBlob->mTimeStamp << "]" );

		// Build the uri
		std::stringstream uri;
		const std::string clientFileTime = fGetClientFileTimeStr( fileBlob->mTimeStamp );
		uri << mFilePathURI.fCStr( ) << fileBlob->fFileNameWithPostfix( ) << "?" << cFieldClientFileTime << "=" << clientFileTime;
		log_line( Log::cFlagHttp, "[" << fileBlob->mFileName << "] client timestamp[" << clientFileTime << "]" );

		iCloudStorageRequestPtr newRequest = tRequest::fCreate( 
			mEndpoint->fOpenRequest( "PUT", uri.str( ).c_str( ), NULL, ( const char* )data, dataSize, cDefaultResponseSize ) );

		return newRequest;
	}

	//------------------------------------------------------------------------------
	const iCloudStorage::tQuotaInfo* tHttpCloudStorage::fQuotaInfo( )
	{
		if( !mQuotaInfoValid )
			return NULL;

		return &mQuotaInfo;
	}

	//------------------------------------------------------------------------------
	b32 tHttpCloudStorage::fFileExists( const tFilePathPtr& filePath ) const
	{
		return fFindFileBlob( filePath ) ? true : false;
	}

	//------------------------------------------------------------------------------
	void tHttpCloudStorage::fSetupEndpoint( )
	{
		tStringPtr hostUrl = fGetServiceEndpoint( );
		mEndpoint = Http::tSystem::fInstance( ).fFindOrCreateEndpoint( hostUrl, true, mUserIdx );

		if( mEndpoint.fNull( ) )
		{
			log_warning( "[CloudStorage] Failed to create endpoint for CloudStorage host" );
			fOnFailed( );
		}
	}

	//------------------------------------------------------------------------------
	void tHttpCloudStorage::fOnFailed( )
	{
		mFileListRequest.fRelease( );
		mQuotaInfoRequest.fRelease( );
		if( !mOnFailed.fNull( ) )
			mOnFailed( );
	}

	//------------------------------------------------------------------------------
	void tHttpCloudStorage::fProcessFileListRequest( )
	{
		if( mFileListRequest && mFileListRequest->fIsComplete( ) )
		{
			// Handle file list request success; cStatusNotFound indicates an empty list
			if( mFileListRequest->fSuccess( ) || mFileListRequest->fHTTPStatusCode( ) == Http::cStatusNotFound )
			{
				mFileListValid = mFileList.fLoad( mFileListRequest->fReadBuffer( ), mFileListRequest->fContentLength( ) );
				mFileListRequest.fRelease( );
			}
			else
			{
				log_warning( "[CloudStorage] File list request failed: " << mFileListRequest->fHTTPStatusCode( ) );
				fOnFailed( );
			}
		}
	}

	//------------------------------------------------------------------------------
	void tHttpCloudStorage::fProcessQuotaInfoRequest( )
	{
		if( mQuotaInfoRequest && mQuotaInfoRequest->fIsComplete( ) )
		{
			// Handle file list request success; cStatusNotFound indicates an empty list
			if( mQuotaInfoRequest->fSuccess( ) )
			{
				mQuotaInfoValid = mQuotaInfo.fLoad( mQuotaInfoRequest->fReadBuffer( ), mQuotaInfoRequest->fContentLength( ) );
				mQuotaInfoRequest.fRelease( );
			}
			else
			{
				log_warning( "[CloudStorage] Quota request failed: " << mQuotaInfoRequest->fHTTPStatusCode( ) );
				fOnFailed( );
			}
		}
	}

	//------------------------------------------------------------------------------
	iCloudStorage::tFileBlob* tHttpCloudStorage::fFindFileBlob( const tFilePathPtr& filePath )
	{
		const tHttpCloudStorage* constThis = ( const tHttpCloudStorage* )this;
		return const_cast< tFileBlob* >( constThis->fFindFileBlob( filePath ) );
	}

	//------------------------------------------------------------------------------
	const iCloudStorage::tFileBlob* tHttpCloudStorage::fFindFileBlob( const tFilePathPtr& filePath ) const
	{
		for( u32 i = 0; i < mFileList.fCount( ); ++i )
		{
			if( mFileList[ i ].mFilePath == filePath )
				return &mFileList[ i ];
		}

		return NULL;
	}

	//------------------------------------------------------------------------------
	tArraySleeve<const iCloudStorage::tFileBlob> tHttpCloudStorage::fFileBlobs( ) const
	{
		return mFileList.fSleeve( );
	}

	//------------------------------------------------------------------------------
	// tHttpCloudStorage::tRequest
	//------------------------------------------------------------------------------
	iCloudStorageRequestPtr tHttpCloudStorage::tRequest::fCreate( const Http::tRequestPtr& ptr )
	{
		if( ptr.fNull( ) )
			return iCloudStorageRequestPtr( );

		return iCloudStorageRequestPtr( NEW_TYPED( tRequest )( ptr ) );
	}

	//------------------------------------------------------------------------------
	void tHttpCloudStorage::tRequest::fWaitToComplete( ) const
	{
		while( !fIsComplete( ) )
			Http::tSystem::fInstance( ).fTick( );
	}

} // ::Sig