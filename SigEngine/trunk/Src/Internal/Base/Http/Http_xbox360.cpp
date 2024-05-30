//------------------------------------------------------------------------------
// \file Http_xbox360.cpp - 17 Jul 2012
// \author colins
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#if defined( platform_xbox360 )
#include "Http.hpp"

namespace Sig { namespace Http 
{
	//------------------------------------------------------------------------------
	// tGetAuthTokenOp
	//------------------------------------------------------------------------------
	b32 tGetAuthTokenOp::fCreate( u32 userIndex, const tStringPtr& hostUrl, PRELYING_PARTY_TOKEN& tokenOut )
	{
		if( !fPreExecute( ) )
			return false;

		HRESULT hr = XAuthGetToken( userIndex, hostUrl.fCStr( ), hostUrl.fLength( ), &tokenOut, fOverlapped( ) );
		if( hr != S_OK && HRESULT_CODE( hr ) != ERROR_IO_PENDING )
		{
			DWORD lastError = GetLastError( );
			log_warning( "XAuthGetToken failed with error: " << lastError );
			return false;
		}

		return true;
	}

	b32 tGetAuthTokenOp::fCreated( ) const
	{
		return fOverlapped( )->hEvent != INVALID_HANDLE_VALUE;
	}

	b32 tGetAuthTokenOp::fFailed( )
	{
		sigassert( fIsComplete( ) );
		u32 result = 0;
		if( !fGetResult( result ) )
			return true;

		// Result is the length of the token
		return false;
	}

	//------------------------------------------------------------------------------
	// tEndpoint
	//------------------------------------------------------------------------------
	void tEndpoint::fPlatformCtor( )
	{
		mToken = NULL;
		mSessionHandle = NULL;
		mConnectFlags = NULL;
	}

	b32 tEndpoint::fInitialize( const tStringPtr& hostUrl, b32 requireToken, HINTERNET session, DWORD userIndex )
	{
		if( !hostUrl.fExists( ) )
		{
			log_warning( "[Http] Cannot initialize endpoint without a host url" );
			return false;
		}

		// If the URL will need an STS token, query for it and store it until we make the actual request
		if( requireToken && mToken == NULL )
		{
			if( !mGetAuthTokenOp.fCreate( userIndex, hostUrl, mToken ) )
				return false;
		}

		mSessionHandle = session; 

		// initialize request data
		mHostUrl = hostUrl;

		memset(mHostName, 0, cMaxHostnameLength);
		memset(&mUrlComp, 0, sizeof(mUrlComp));

		mUrlComp.dwStructSize = sizeof(mUrlComp);
		mUrlComp.lpszHostName = mHostName;
		mUrlComp.dwHostNameLength = cMaxHostnameLength;
		mUrlComp.lpszUrlPath = NULL;
		mUrlComp.dwUrlPathLength = (DWORD) -1;

		// crack the URL into url components
		if( !XHttpCrackUrl( mHostUrl.fCStr( ),
			mHostUrl.fLength( ),
			ICU_DECODE,
			&mUrlComp ) )
		{
			log_warning( "XHttpCrackUrl failed with error: " << GetLastError( ) );
			return false;
		}

		if (mUrlComp.nScheme == INTERNET_SCHEME_HTTPS)
		{
			mConnectFlags |= XHTTP_FLAG_SECURE;
		}

		return true;
	}

	void tEndpoint::fDestroy()
	{
		mRequests.fSetCount( 0 );

		if( mGetAuthTokenOp.fCreated( ) )
		{
			if( mGetAuthTokenOp.fIsComplete( ) )
				mGetAuthTokenOp.fReset( );
			else
				mGetAuthTokenOp.fCancel( );
		}

		if( mToken )
		{
			XAuthFreeToken( mToken );
			mToken = NULL;
		}
	}

	b32 tEndpoint::fReady( )
	{
		if( mGetAuthTokenOp.fCreated( ) )
			return mGetAuthTokenOp.fIsComplete( ) && !mGetAuthTokenOp.fFailed( );
		return true;
	}

	b32 tEndpoint::fFailed( )
	{
		if( mGetAuthTokenOp.fCreated( ) )
			return mGetAuthTokenOp.fIsComplete( ) && mGetAuthTokenOp.fFailed( );
		return false;
	}

	tRequestPtr tEndpoint::fOpenRequest( const char* verb, const char* path, const char* contentHeader, const char* data, u32 dataSize, u32 responseBufferSize )
	{
		sigcheckfail( fReady( ), return tRequestPtr( ) );

		// connect to the hostname/port for the session object
		HINTERNET connectHandle = XHttpConnect(mSessionHandle,
			mUrlComp.lpszHostName,
			mUrlComp.nPort,
			mConnectFlags);

		if (connectHandle == NULL)
		{
			log_warning( "XHttpConnect failed with error: " << GetLastError() );
			return tRequestPtr( );
		}

		DWORD cbHeader = 0;
		CHAR pszCur[tRequest::cMaxHeaderLength] = {'\0'};

		// TODO: Get these values from somewhere else.
		// Right now this function is assuming that we're communicating with a LIVE service
		static CONST CHAR * STS_TOKEN_HEADER = "Authorization: XBL2.0 x="; // xboxlive specific header used for RESTful services
		static CONST CHAR * STS_TOKEN_HEADER_END = "\r\n";
		static CONST CHAR * CONTENT_TYPE_HEADER = "Content-Type: application/json\r\n";
		static CONST CHAR * CONTRACT_VERSION_HEADER = "x-xbl-contract-version: 1\r\n";
		if (mToken != NULL) // use a token
		{
			strcat_s(pszCur, tRequest::cMaxHeaderLength, STS_TOKEN_HEADER);
			strcat_s(pszCur, tRequest::cMaxHeaderLength, (CONST CHAR *)mToken->pToken);
			strcat_s(pszCur, tRequest::cMaxHeaderLength, STS_TOKEN_HEADER_END);

			if ( contentHeader != NULL )
			{
				strcat_s(pszCur, tRequest::cMaxHeaderLength, contentHeader);
			}
			else
			{
				strcat_s(pszCur, tRequest::cMaxHeaderLength, CONTENT_TYPE_HEADER);
				strcat_s(pszCur, tRequest::cMaxHeaderLength, CONTRACT_VERSION_HEADER); 
			}

			cbHeader = strlen(pszCur); 
		}
		else 
		{
			if ( contentHeader != NULL )
			{
				strcat_s(pszCur, tRequest::cMaxHeaderLength, contentHeader);
			}
			else
			{
				strcat_s(pszCur, tRequest::cMaxHeaderLength, CONTENT_TYPE_HEADER);
				strcat_s(pszCur, tRequest::cMaxHeaderLength, CONTRACT_VERSION_HEADER); 
			}

			cbHeader = strlen(pszCur); 
		}

		// Create the request
		tRequestPtr request = tRequestPtr( NEW tRequest( responseBufferSize ) );
		if( request->fOpen( connectHandle, path, verb, pszCur, data, dataSize ) )
			mRequests.fPushBack( request );
		else
			request.fRelease( );

		return request;
	}

	void tEndpoint::fTick( )
	{
		sigassert(mSessionHandle != NULL);

		for( u32 i = 0; i < mRequests.fCount( ); ++i )
			mRequests[ i ]->fTick( );

		if( !XHttpDoWork( mSessionHandle, 0 ) )
		{
			log_warning( "XHttpDoWork failed with error: " << GetLastError( ) );
		}

		// Erase completed requests
		for( u32 i = 0; i < mRequests.fCount( ); /*++i*/ )
		{
			if( mRequests[ i ]->fIsComplete( ) )
				mRequests.fErase( i );
			else
				++i;
		}
	}

	//------------------------------------------------------------------------------
	// tRequest
	//------------------------------------------------------------------------------
	void tRequest::fPlatformCtor( )
	{
		mRequestHandle = NULL;
		mConnectHandle = NULL;
		mAsyncResult.dwError = S_OK;
	}

	b32 tRequest::fOpen( HINTERNET connectHandle, const char* path, const char* verb, const char* header, const void* data, u32 dataSize )
	{
#ifdef sig_logging
		if( Log::fFlagEnabled( Log::cFlagHttp ) )
		{
			printf("tRequest::fOpen()\n");
				printf("\tpath[%s]\n",path);
				printf("\tverb[%s]\n",verb);
				printf("\theader[");
				fwrite(header,1,strlen(header),stdout);
				printf("]\n\tdata[%u][",dataSize);
				fwrite(data,1,dataSize,stdout);
				printf("]\n");
		}
#endif//sig_logging

		sigassert( verb && "Verb cannot be NULL" );
		sigassert( mState == cStateIdle && "Another request is pending" );

		mConnectHandle = connectHandle;

		mReadBuffer.fZeroOut( );
		mETag.fZeroOut( );
		mHeader.fZeroOut( );

		mVerb = verb;
		mRequestData.fInitialize( ( const byte* )data, dataSize );

		// open the actual request and specify the action verb and url path
		// however the final request will not be submitted until XhttpSendRequest occurs
		mRequestHandle = XHttpOpenRequest( connectHandle,
			verb,
			path,
			NULL,
			NULL,
			NULL,
			0 );
		if( mRequestHandle == NULL )
		{
			log_warning( "XHttpOpenRequest failed with error: " << GetLastError( ) );
			return false;
		}

		// specify the callback routine to be used by XHTTP
		XHttpSetStatusCallback( mRequestHandle,
			StatusCallback,
			XHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS,
			NULL );

		// set the custom header if specified
		strcpy( mHeader.fBegin( ), header );

		// put the auth manager state machine in the "cStateInit" state
		fSetState(cStateInit);

		return true;
	}

	void tRequest::fClose( )
	{
		if( mRequestHandle != NULL )
		{
			XHttpCloseHandle( mRequestHandle );
			mRequestHandle = NULL;
		}
		if( mConnectHandle != NULL )
		{
			XHttpCloseHandle( mConnectHandle );
			mConnectHandle = NULL;
		}

		mState = cStateIdle;
		mReadBuffer[0] = 0;
		mETag[0] = 0;
	}

	void tRequest::fTick()
	{
		HRESULT hr = S_OK;

		switch( mState )
		{
		case cStateIdle:
			break; // not expected to do anything

		case cStateInit:
			{
				DWORD headerLen = strlen( mHeader.fBegin( ) ); 

				// here we pass the header and the initial custom request data
				// NOTE: totalsize will always be custom data size due to keeping the class simple and keeping the request as a single call
				if( XHttpSendRequest( mRequestHandle,
					mHeader.fBegin( ),
					headerLen,
					mRequestData.fBegin( ),
					mRequestData.fTotalSizeOf( ),
					mRequestData.fTotalSizeOf( ),
					(DWORD_PTR)this ) )
				{
					fSetState(cStateSendingRequest);
				}
				else
				{
					log_warning( "[Http] XHttpSendRequest failed with error: " << GetLastError( ) );
					fSetState( cStateError );
				}
			} 
			break;

		case cStateSendingRequest:
			break; // statemachine will be updated in the callback

		case cStateRequestSent:
			{
				if( XHttpReceiveResponse( mRequestHandle, NULL ) )
				{
					fSetState( cStateReceivingResponseHeader );
				}
				else
				{
					log_warning( "[Http] XHttpReceiveResponse failed with error: " << GetLastError( ) );
					fSetState( cStateError );
				}
			}
			break;

		case cStateReceivingResponse:
			break; // statemachine will be updated in the callback

		case cStateReceivingResponseHeader:
			break; // statemachine will be updated in the callback, expected move: ReponseHeaderAvailable

		case cStateResponseHeaderAvailable:
			{
				hr = GetResponseETagHeader( mRequestHandle, mETag.fBegin( ) );
				if( hr != HRESULT_FROM_WIN32( ERROR_XHTTP_HEADER_NOT_FOUND ) && FAILED( hr ) )
				{
					log_warning( "[Http] GetResponseETagHEader failed with error: " << GetLastError( ) );
					fSetState( cStateError );
				}

				DWORD httpStatusCode = 0;
				hr = GetResponseStatusCode( mRequestHandle, &httpStatusCode );
				mHttpStatusCode = httpStatusCode;
				if( FAILED( hr ) )
				{
					log_warning( "[Http] GetResponseStatusCode failed with error: " << GetLastError( ) );
					fSetState( cStateError );
				}
				else if( mHttpStatusCode >= cStatusBadRequest )
				{
					log_warning( "[Http] Server returned error status code: " << mHttpStatusCode );
					fSetState( cStateError );
				}
				else // NOTE: if we get a 403/404 we do still have data that maybe helpful
				{
					DWORD contentLength = 0;
					hr = GetResponseContentLength( mRequestHandle, &contentLength );
					mContentLength = contentLength;
					if( FAILED( hr ) )
					{
						log_warning( "[Http] GetResponseContentLength failed with error: " << GetLastError( ) );
						fSetState( cStateError );
					}
					else if( mContentLength > 0 )
					{
						log_line( Log::cFlagHttp, "GetResponseContentLength size: " << mContentLength );
						mContentLengthRemaining = mContentLength;
						
						const u32 requiredBufferSize = mContentLength;
						if( requiredBufferSize > mReadBuffer.fTotalSizeOf( ) )
						{
							log_warning( "Read buffer for HttpRequest had to be resized from: " << mReadBuffer.fTotalSizeOf( ) << " to: " << requiredBufferSize );

							mReadBuffer.fResize( requiredBufferSize );
							mReadBuffer.fZeroOut( );
						}

						fSetState( cStateReceivingResponseBody );
					}
					else
					{
						fSetState( cStateResponseReceived );
					}
				}
			}
			break;

		case cStateReceivingResponseBody:
			{
				fSetState( cStateReceivingResponse );
				mBytesToRead = fMin( mContentLengthRemaining, mReadBuffer.fCount( ) );
				log_line( Log::cFlagHttp, "bytes to read: " << mBytesToRead);
				if( mBytesToRead == 0 )
				{
					fSetState(cStateResponseReceived);
				}
				else if( !XHttpReadData( mRequestHandle,
					mReadBuffer.fBegin( ) + (mContentLength - mContentLengthRemaining),
					mBytesToRead,
					NULL ) )
				{
					log_warning( "[Http] XHttpReadData failed with error: " << GetLastError( ) );
					fSetState(cStateError);
				}
			}
			break;

		case cStateResponseDataAvailable:
			{
				mContentLengthRemaining -= mBytesRead;
				log_line( Log::cFlagHttp, "Received data: " << mReadBuffer.fBegin( ) );
				fSetState( cStateReceivingResponseBody );
			}
			break;

		case cStateResponseReceived:
			fSetState( cStateCompleted );
			break;

		case cStateError: __fallthrough
		case cStateCompleted:
			break; // final state

		default:
			sigassert( !"Invalid state" );
			break;
		}
	}

	VOID CALLBACK tRequest::StatusCallback( HINTERNET hInternet,
		DWORD_PTR dwpContext,
		DWORD dwInternetStatus,
		LPVOID lpvStatusInformation,
		DWORD dwStatusInformationSize )
	{
		tRequest* thisPtr = (tRequest*) dwpContext;

		switch( dwInternetStatus )
		{
		case XHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE:
			{
				sigcheckfail( thisPtr, break );
				thisPtr->fSetState(cStateRequestSent);
			}
			break;

		case XHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE:
			{
				sigcheckfail( thisPtr, break );
				sigassert(thisPtr->mState == cStateReceivingResponseHeader);
				thisPtr->fSetState(cStateResponseHeaderAvailable);
			}
			break;

		case XHTTP_CALLBACK_STATUS_READ_COMPLETE:
			{
				sigcheckfail( thisPtr, break );
				thisPtr->mBytesRead = dwStatusInformationSize;
				log_line( Log::cFlagHttp, "read complete with sizeof: " << dwStatusInformationSize );
				thisPtr->fSetState(cStateResponseDataAvailable);
			}
			break;

		case XHTTP_CALLBACK_STATUS_REQUEST_ERROR:
			{
				sigcheckfail( thisPtr, break );
				memcpy_s( &thisPtr->mAsyncResult, sizeof(XHTTP_ASYNC_RESULT), (XHTTP_ASYNC_RESULT*) lpvStatusInformation, sizeof(XHTTP_ASYNC_RESULT) );

				log_warning( "[Http] encountered request error " << thisPtr->mAsyncResult.dwError );
				thisPtr->fSetState(cStateError);
			}
			break;

		case XHTTP_CALLBACK_STATUS_WRITE_COMPLETE: __fallthrough 
		case XHTTP_CALLBACK_STATUS_INTERMEDIATE_RESPONSE: 
			{
				sigcheckfail( thisPtr, break );
				log_warning( "[Http] Unsupported state" );
				thisPtr->fSetState(cStateError);
			}
			break;

		case XHTTP_CALLBACK_STATUS_REDIRECT: __fallthrough
		case XHTTP_CALLBACK_STATUS_HANDLE_CLOSING:
			break; // do nothing

		default:
			sigassert( "[Http] invalid state" );
		}
	}

	HRESULT tRequest::GetResponseStatusCode(
		HINTERNET hRequest,
		DWORD * pdwStatusCode
		)
	{
		HRESULT hr = S_OK;
		DWORD dwBufferLength = sizeof(DWORD);

		sigassert(pdwStatusCode != NULL);
		hr = GetResponseHeader(hRequest,
			(XHTTP_QUERY_STATUS_CODE | XHTTP_QUERY_FLAG_NUMBER),
			XHTTP_HEADER_NAME_BY_INDEX,
			pdwStatusCode,
			&dwBufferLength);

		return hr;
	}


	HRESULT tRequest::GetResponseETagHeader( HINTERNET hRequest, CHAR * szETag	)
	{
		HRESULT hr = S_OK;
		DWORD dwBufferLength = cMaxHeaderLength;

		sigassert(szETag != NULL);
		hr = GetResponseHeader(hRequest,
			XHTTP_QUERY_CUSTOM,
			"ETag",
			szETag,
			&dwBufferLength);

		return hr;
	}

	HRESULT tRequest::GetResponseContentLength(
		HINTERNET hRequest,
		DWORD * pdwContentLength
		)
	{
		HRESULT hr = S_OK;
		DWORD dwBufferLength = sizeof(DWORD);
		sigassert(pdwContentLength != NULL);
		hr = GetResponseHeader(hRequest,
			(XHTTP_QUERY_CONTENT_LENGTH | XHTTP_QUERY_FLAG_NUMBER),
			XHTTP_HEADER_NAME_BY_INDEX,
			pdwContentLength,
			&dwBufferLength);

		return hr;
	}

	HRESULT tRequest::GetResponseHeader(
		HINTERNET hRequest,
		DWORD dwInfoLevel,
		CONST CHAR * pszHeader,
		VOID * pvBuffer,
		DWORD * pdwBufferLength
		)
	{
		HRESULT hr = S_OK;

		sigassert(pdwBufferLength != NULL);
		if (!XHttpQueryHeaders(hRequest,
			dwInfoLevel,
			pszHeader,
			pvBuffer,
			pdwBufferLength,
			XHTTP_NO_HEADER_INDEX)) 
		{
			DWORD error = GetLastError();
			hr = HRESULT_FROM_WIN32( error );
		}

		return hr;
	}

	//------------------------------------------------------------------------------
	// tSystem
	//------------------------------------------------------------------------------
	const u32 tSystem::cStartupBypassSecurity = XAUTH_FLAG_BYPASS_SECURITY;

	void tSystem::fPlatformCtor( )
	{
		mSessionHandle = NULL;
	}

	b32 tSystem::fPlatformInitialize( )
	{
		if( !XHttpStartup( 0, NULL ) )
		{
			DWORD error = GetLastError( );
			log_warning( "XHttpStartup failed with error: " << error );
			return false;
		}

		XAUTH_SETTINGS settings;
		memset(&settings, 0, sizeof(settings));
		settings.SizeOfStruct = sizeof(settings);
		settings.Flags = mStartupFlags;

		HRESULT hr = XAuthStartup( &settings );
		if( FAILED( hr ) )
		{
			log_warning( "XAuthStartup failed with result: " << hr );
			return false;
		}

		// Open an XHTTP session
		mSessionHandle = XHttpOpen( mUserAgent.fCStr( ),
			XHTTP_ACCESS_TYPE_DEFAULT_PROXY,
			NULL,
			NULL,
			XHTTP_FLAG_ASYNC);
		if( !mSessionHandle )
		{
			DWORD error = GetLastError( );
			log_warning( "XHttpOpen failed with error: " << error );
			return false;
		}

		return true;
	}

	void tSystem::fPlatformShutdown( )
	{
		if( mSessionHandle )
		{
			XHttpCloseHandle( mSessionHandle );
			mSessionHandle = NULL;
		}

		for( u32 userIdx = 0; userIdx < mEndpointsByUserIdx.fCount( ); ++userIdx )
		{
			tEndpointList& endpoints = mEndpointsByUserIdx[ userIdx ];
			for( u32 i = 0; i < endpoints.fCount( ); ++i )
			{
				// Explicitly call fDestroy( ) to invalidate the endpoint.
				// This ensures that the endpoint cannot be used via any existing tEndpointPtr variables
				endpoints[ i ]->fDestroy( );
			}

			endpoints.fDeleteArray( );
		}

		XAuthShutdown();
		XHttpShutdown();
	}

	tEndpointPtr tSystem::fFindOrCreateEndpoint( const tStringPtr& hostUrl, b32 requireToken, u32 userIdx )
	{
		sigassert( userIdx < mEndpointsByUserIdx.fCount( ) && "userIdx out of range" );

		tEndpointList& endpoints = mEndpointsByUserIdx[ userIdx ];

		// If we already have a matching endpoint, return that one
		for( u32 i = 0; i < endpoints.fCount( ); ++i )
		{
			tEndpointPtr& endpoint = endpoints[ i ];
			if( endpoint->fHostUrl( ) == hostUrl )
				return endpoint;
		}

		// Create the endpoint
		tEndpointPtr endpoint = tEndpointPtr( NEW tEndpoint( ) );
		if( endpoint->fInitialize( hostUrl, requireToken, mSessionHandle, userIdx ) )
			endpoints.fPushBack( endpoint );
		else
			endpoint.fRelease( );

		return endpoint;
	}

} } // ::Sig::Http
#endif//#if defined( platform_xbox360 )
