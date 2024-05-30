//------------------------------------------------------------------------------
// \file tTitleServerConnection.hpp - 17 Jan 2012
// \author jwittner
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tTitleServerConnection__
#define __tTitleServerConnection__

#include "enet/enet.h"

namespace Sig { namespace Net 
{
	///
	/// \class tTitleServerConnection
	/// \brief Will query for and create a tcp stream between the client and the
	///		   specified server
	class tTitleServerConnection 
	{
	public:

		enum tState
		{
			cStateNull = 0,
			cStateNoServer,
			cStateFindingServer,
			cStateIdle,
			cStateConnectingServer,
			cStateConnectingSocket,
			cStateSendingRequest,
			cStateWaitingForResponse,
		};

		// Response - buffer, bufferSize
		typedef tDelegate< b32 ( const void *, u32 ) > tResponseCallback;

		static const u32 cInvalidRequestId = 0;

	public:

		tTitleServerConnection( );
		~tTitleServerConnection( );

		inline const tStringPtr& fServerName( ) const 
		{ 
			return mSelectedServer < mServers.fCount( ) ? fSelectedServer( ).mName : tStringPtr::cNullPtr; 
		}
		inline u32 fState( ) const { return mState; }
		inline b32 fIdle( ) const { return mState == cStateIdle; }

		void fClearServersToIgnore( ) { mServersToIgnore.fSetCount( 0 ); }
		void fAddServerToIgnore( const tStringPtr& ignore ) { mServersToIgnore.fFindOrAdd( ignore ); }

		void fConnect( const char * serverName, u32 serviceId, u16 port );
		void fDisconnect( );

		void fUpdate( );
		

		// Indicates if the system is currently working on a request
		b32	fIsBusyWithRequest( );

		// Request a buffer be sent - only one request is allowed at a time
		// Returns whether the request was successfully enqueued
		b32 fSend( const void * buffer, u32 bytes );

		// If cb is a valid callback, the title server assumes you expect a response
		b32 fSend( const void * buffer, u32 bytes, const tResponseCallback & cb );

	private:

		static const u32 cMaxServers = 50;
		static const u32 cMaxConnectAttempts = 5;
		static const u32 cMaxTimeoutAttempts = 5;
		static const f32 cFirstFindServerInterval;
		static const f32 cFindServerInterval;
		static const f32 cTimeoutThreshold;

	private:

		struct tServerInfo
		{
			tStringPtr mName;
			u32 mAddress;
		};
	
	private:

		void fDisconnect( b32 dropServer );
		void fCreateAndConnectSocket( );

		void fSelectRandomServer( );
		void fSelectNextServer( );
		const tServerInfo& fSelectedServer( ) const { return mServers[ mSelectedServer ]; }

		
		// Platform specific
		void fPlatformCtor( );
		void fPlatformDtor( );
		void fFindServer( );
		void fUpdateFindingServer( );
		void fConnectServer( );
		void fUpdateConnectingServer( );
		void fUpdateConnectingSocket( );
		void fSendRequest( b32 firstAttempt );
		void fUpdateSendingRequest( );
		void fWaitForResponse( b32 firstAttempt );
		void fUpdateWaitingForRequest( );
		void fPlatformDisconnect( );


	private:

		u32 mState;

		std::string mServerToConnect;

		u16 mServerPort;
		u32 mServiceId;
		u32 mSelectedServer;
		u32 mSecureServerAddress;

		u32 mRequestSize;
		byte * mRequestBuffer;
		tResponseCallback mRequestCallback;

		Time::tStopWatch mTimer;
		f32 mFindServerInterval;
		u32 mConnectAttempts;
		u32 mTimeoutAttempts;

		ENetSocket mSocket;

		tFixedArray<byte, 200 * 1024> mBuffer;

		tGrowableArray< tStringPtr > mServersToIgnore;
		tGrowableArray< tServerInfo > mServers;

#ifdef platform_xbox360
		XOVERLAPPED mOverlapped;
		HANDLE mServerEnumerator;
		tDynamicBuffer mServerInfoBuffer;
#endif
		
	};

}} // ::Sig::Net

#endif//__tTitleServerConnection__
