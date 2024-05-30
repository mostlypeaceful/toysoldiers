//------------------------------------------------------------------------------
// \file Http.hpp - 16 Jul 2012
// \author colins
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __Http__
#define __Http__
#ifdef platform_xbox360
	#include <xhttp.h>
	#include <xauth.h>
#endif
#include "tUser.hpp"

namespace Sig { namespace Http 
{
	// Http status codes
	static const u32 cStatusContinue = 100; // OK to continue with request
	static const u32 cStatusSwitchProtocols = 101; // server has switched protocols in upgrade header

	static const u32 cStatusOk = 200; // request completed
	static const u32 cStatusCreated = 201; // object created, reason = new URI
	static const u32 cStatusAccepted = 202; // async completion (TBS)
	static const u32 cStatusPartial = 203; // partial completion
	static const u32 cStatusNoContent = 204; // no info to return
	static const u32 cStatusResetContent = 205; // request completed, but clear form
	static const u32 cStatusPartialContent = 206; // partial GET furfilled

	static const u32 cStatusAmbiguous = 300; // server couldn't decide what to return
	static const u32 cStatusMoved = 301; // object permanently moved
	static const u32 cStatusRedirect = 302; // object temporarily moved
	static const u32 cStatusRedirectMethod = 303; // redirection w/ new access method
	static const u32 cStatusNotModified = 304; // if-modified-since was not modified
	static const u32 cStatusUseProxy = 305; // redirection to proxy, location header specifies proxy to use
	static const u32 cStatusRedirectKeepVerb = 307; // HTTP/1.1: keep same verb

	static const u32 cStatusBadRequest = 400; // invalid syntax
	static const u32 cStatusDenied = 401; // access denied
	static const u32 cStatusPaymentReq = 402; // payment required
	static const u32 cStatusForbidden = 403; // request forbidden
	static const u32 cStatusNotFound = 404; // object not found
	static const u32 cStatusBadMethod = 405; // method is not allowed
	static const u32 cStatusNoneAcceptable = 406; // no response acceptable to client found
	static const u32 cStatusProxyAuthReq = 407; // proxy authentication required
	static const u32 cStatusRequestTimeout = 408; // server timed out waiting for request
	static const u32 cStatusConflict = 409; // user should resubmit with more info
	static const u32 cStatusGone = 410; // the resource is no longer available
	static const u32 cStatusLengthRequired = 411; // the server refused to accept request w/o a length
	static const u32 cStatusPrecondFailed = 412; // precondition given in request failed
	static const u32 cStatusRequestTooLarge = 413; // request entity was too large
	static const u32 cStatusURITooLong = 414; // request URI too long
	static const u32 cStatusUnsupportedMedia = 415; // unsupported media type
	static const u32 cStatusRetryWith = 449; // retry after doing the appropriate action.

	static const u32 cStatusServerError = 500; // internal server error
	static const u32 cStatusNotSupported = 501; // required not supported
	static const u32 cStatusBadGateway = 502; // error response received from gateway
	static const u32 cStatusServiceUnavail = 503; // temporarily overloaded
	static const u32 cStatusGatewayTimeout = 504; // timed out waiting for gateway
	static const u32 cStatusVersionNotSup = 505; // HTTP version not supported


	///
	/// \class tRequest
	class tRequest : public tRefCounter
	{
	public:

		static const u32 cMaxHeaderLength = 16 * 1024;

		enum tState
		{
			cStateIdle,
			cStateInit,
			cStateSendingRequest,
			cStateRequestSent,
			cStateReceivingResponse,
			cStateResponseHeaderAvailable,
			cStateReceivingResponseHeader,
			cStateReceivingResponseBody,
			cStateResponseDataAvailable,
			cStateResponseReceived,
			cStateCompleted,
			cStateError,
		};

	public:

		tRequest( u32 responseBufferSize );
		~tRequest( );

#ifdef platform_xbox360
		b32 fOpen( HINTERNET connectHandle, const char* path, const char* verb, const char* header, const void* data, u32 dataSize );
#endif
		void fClose( );

		void fTick( );
		
		b32 fIsComplete( ) const;
		b32 fSuccess( ) const;
		b32 fFailed( ) const;

		const byte* fReadBuffer( ) const;
		u32 fReadBufferSize( ) const;
		const char* GetETagHeader( ) const;
		u32 fHTTPStatusCode( ) const;
		u32 fContentLength( ) const;
		u32 fContentLengthRemaining( ) const;

	private:

		void fSetState(tState state);

		void fPlatformCtor( );

#ifdef platform_xbox360
		static VOID CALLBACK StatusCallback(HINTERNET hInternet,
			DWORD_PTR dwpContext,
			DWORD dwInternetStatus,
			LPVOID lpvStatusInformation,
			DWORD dwStatusInformationSize);
		HRESULT GetResponseStatusCode(HINTERNET hRequest,
			DWORD * pdwStatusCode);
		HRESULT GetResponseETagHeader(HINTERNET hRequest,
			CHAR * szETag);
		HRESULT GetResponseContentLength(HINTERNET hRequest,
			DWORD * pdwContentLength);
		HRESULT GetResponseHeader(HINTERNET hRequest,
			DWORD dwInfoLevel,
			CONST CHAR * pszHeader,
			VOID * pvBuffer,
			DWORD * pdwBufferLength);
#endif

	private:

		tState mState;

		tDynamicBuffer mReadBuffer; // read buffer used for HTTP reading
		tDynamicArray< char > mETag; // buffer for ETag
		tDynamicArray< char > mHeader; // request header (this generally contains the STS token, and additional header info like "Content-Type")

		const char* mVerb; // GET, PUT, POST, DELETE
		tDynamicBuffer mRequestData;

		u32 mBytesRead; // member variable used to track how many bytes last read

		u32 mHttpStatusCode;
		u32 mContentLength;
		u32 mContentLengthRemaining;
		u32 mBytesToRead;

#ifdef platform_xbox360
		HINTERNET mRequestHandle;
		HINTERNET mConnectHandle;
		XHTTP_ASYNC_RESULT mAsyncResult;
#endif
	};
	typedef tRefCounterPtr< tRequest > tRequestPtr;

#ifdef platform_xbox360
	class tGetAuthTokenOp : public XtlUtil::tOverlappedOp
	{
	public:
		b32 fCreate( u32 userIndex, const tStringPtr& hostUrl, PRELYING_PARTY_TOKEN& tokenOut );

		b32 fCreated( ) const;
		b32 fFailed( );
	};
#endif

	///
	/// \class tEndpoint
	class tEndpoint : public tRefCounter
	{
	public:
		static const u32 cMaxHostnameLength = 128;

	public:

		tEndpoint( );
		~tEndpoint( );

#ifdef platform_xbox360
		b32 fInitialize( const tStringPtr& hostUrl, b32 requireToken, HINTERNET session, DWORD userIndex );
#endif
		void fDestroy( );

		b32 fReady( );
		b32 fFailed( );

		tRequestPtr fMakeSyncRequest(
			const char* verb,
			const char* path,
			const char* contentHeader,
			const char* data,
			u32 dataSize,
			u32 responseBufferSize );

		tRequestPtr fOpenRequest(
			const char* verb,
			const char* path,
			const char* contentHeader,
			const char* data,
			u32 dataSize,
			u32 responseBufferSize );

		void fCloseRequest( );

		void fTick();

		const tStringPtr& fHostUrl( ) const { return mHostUrl; }

	private:

		// Platform specific
		void fPlatformCtor( );
		
	private:

		tStringPtr mHostUrl; // e.g. https://foo.bar.com, http://123.123.123.123
		char mHostName[ cMaxHostnameLength ]; // e.g. foo.bar.com, 123.123.123.123

		tGrowableArray< tRequestPtr > mRequests;
		
#ifdef platform_xbox360
		PRELYING_PARTY_TOKEN mToken;
		tGetAuthTokenOp mGetAuthTokenOp;

		URL_COMPONENTS mUrlComp;
		DWORD mConnectFlags;

		// HINTERNET handles
		HINTERNET mSessionHandle;
#endif
	};
	typedef tRefCounterPtr< tEndpoint > tEndpointPtr;

	///
	/// \class tSystem
	/// \brief Handles global Http initialization/shutdown, user authentication,
	///		and endpoint creation/removal/updating
	class tSystem
	{
		declare_singleton_define_own_ctor_dtor( tSystem );
		tSystem( );

	public:

		// Startup flags
		static const u32 cStartupBypassSecurity;

	public:

		b32 fInitialize( u32 startupFlags, const tStringPtr& userAgent );
		b32 fIsInitialized( ) const { return mIsInitialized; }
		void fShutdown( );
		void fTick( );

		void fOnUserSigninChange( const tUserSigninInfo oldStates[], const tUserSigninInfo newStates[] );

		tEndpointPtr fFindOrCreateEndpoint( const tStringPtr& hostUrl, b32 requireToken, u32 userIdx );
		void fRemoveEndpoint( tEndpoint* endpoint );

	private:

		typedef tGrowableArray< tEndpointPtr > tEndpointList;

	private:

		void fPlatformCtor( );
		b32 fPlatformInitialize( );
		void fPlatformShutdown( );

		void fRemoveEndpointsForUserIdx( u32 userIdx );

	private:

#ifdef platform_xbox360
		HINTERNET mSessionHandle;
#endif
		tDynamicArray< tEndpointList > mEndpointsByUserIdx; // Indexed by user hardware idx
		b32 mIsInitialized;

		// Initialization params
		u32 mStartupFlags;
		tStringPtr mUserAgent;
	};

} } // ::Sig::Http


#endif//__Http__