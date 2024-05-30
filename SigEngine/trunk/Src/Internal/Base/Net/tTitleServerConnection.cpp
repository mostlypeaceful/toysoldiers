//------------------------------------------------------------------------------
// \file tTitleServerConnection.cpp - 17 Jan 2012
// \author jwittner
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tTitleServerConnection.hpp"

namespace Sig { namespace Net 
{
	const f32 tTitleServerConnection::cFirstFindServerInterval = 5.f;
	const f32 tTitleServerConnection::cFindServerInterval = 5 * 60.f;
#ifdef sig_logging
	const f32 tTitleServerConnection::cTimeoutThreshold = 60.f;
#else
	const f32 tTitleServerConnection::cTimeoutThreshold = 25.f;
#endif


	//------------------------------------------------------------------------------
	// tTitleServerConnection
	//------------------------------------------------------------------------------
	tTitleServerConnection::tTitleServerConnection( )
		: mState( cStateNull )
		, mServiceId( ~0 )
		, mSelectedServer( ~0 )
		, mSecureServerAddress( 0 )
		, mRequestSize( 0 )
		, mRequestBuffer( NULL )
		, mFindServerInterval( cFirstFindServerInterval )
		, mConnectAttempts( 0 )
		, mTimeoutAttempts( 0 )
		, mSocket( ENET_SOCKET_NULL )
	{
		fPlatformCtor( );
	}

	//------------------------------------------------------------------------------
	tTitleServerConnection::~tTitleServerConnection( )
	{
		fPlatformDtor( );
		fDisconnect( );
	}

	//------------------------------------------------------------------------------
	void tTitleServerConnection::fConnect( const char * serverName, u32 serviceId, u16 port )
	{
		fDisconnect( true );
		
		mServerToConnect = serverName ? serverName : "";
		mServiceId = serviceId;
		mServerPort = port;
		mState = cStateNoServer;
		
		fFindServer( );
		mFindServerInterval = cFirstFindServerInterval;
	}

	//------------------------------------------------------------------------------
	void tTitleServerConnection::fDisconnect( )
	{
		fDisconnect( true );
	}

	//------------------------------------------------------------------------------
	void tTitleServerConnection::fUpdate( )
	{
		switch( mState )
		{
		case cStateNull: break;
		case cStateNoServer:
			{
				if( mRequestSize || mTimer.fGetElapsedS( ) > mFindServerInterval )
					fFindServer( );
			} break;
		case cStateFindingServer: fUpdateFindingServer( ); break;
		case cStateIdle: 
			{
				if( mRequestSize > 0 ) 
					fConnectServer( );
			} break;
		case cStateConnectingServer: fUpdateConnectingServer( ); break;
		case cStateConnectingSocket: fUpdateConnectingSocket( ); break;
		case cStateSendingRequest: fUpdateSendingRequest( ); break;
		case cStateWaitingForResponse: fUpdateWaitingForRequest( ); break;

		default: sig_nodefault( );
		}
	}

	//------------------------------------------------------------------------------
	b32	tTitleServerConnection::fIsBusyWithRequest( )
	{
		return mRequestSize > 0 || mState > cStateIdle;
	}

	//------------------------------------------------------------------------------
	b32 tTitleServerConnection::fSend( const void * buffer, u32 bytes )
	{
		return fSend( buffer, bytes, tResponseCallback( ) );
	}

	//------------------------------------------------------------------------------
	b32 tTitleServerConnection::fSend( const void * buffer, u32 bytes, const tResponseCallback & cb )
	{
		sigcheckfail( !fIsBusyWithRequest( ), return false );
		sigcheckfail( bytes < mBuffer.fCount( ), return false );

		mRequestCallback = cb;
		mRequestSize = bytes;

		fMemCpy( mBuffer.fBegin( ), buffer, bytes );
		mRequestBuffer = mBuffer.fBegin( );
		return true;
	}

	//------------------------------------------------------------------------------
	void tTitleServerConnection::fDisconnect( b32 dropServer )
	{
		log_line( Log::cFlagLSP, "Disconnect: dropServer " << dropServer );

		fPlatformDisconnect( );

		if( mSocket != ENET_SOCKET_NULL )
		{
			enet_socket_destroy( mSocket );
			mSocket = ENET_SOCKET_NULL;
		}

		mRequestSize = 0;
		mConnectAttempts = 0;

		if( dropServer )
		{
			mTimer.fRestart( );
			mTimeoutAttempts = 0;
			mState = cStateNoServer;
			
			mSelectedServer = ~0;
			mServers.fSetCount( 0 );
		}
		else
		{
			fSelectNextServer( );
			mState = cStateIdle;
		}
	}

	//------------------------------------------------------------------------------
	void tTitleServerConnection::fCreateAndConnectSocket( )
	{
		sigassert( mState == cStateConnectingServer && "Sanity!" );

		mConnectAttempts = 0;

		// We've got a secure connection, open up a stream
		mSocket = enet_socket_create( ENET_SOCKET_TYPE_STREAM );
		sigassert( mSocket != ENET_SOCKET_NULL );

		// We don't want the socket to block
		int result = enet_socket_set_option( mSocket, ENET_SOCKOPT_NONBLOCK, 1 );
		sigassert( result == 0 && "Socket could not be set to non-blocking" );

		ENetAddress addy;
		addy.port = ENET_HOST_TO_NET_16( mServerPort );
		addy.host = mSecureServerAddress;

		log_line( Log::cFlagLSP, "Connecting socket: secureAddr " << mSecureServerAddress << ":" << mServerPort );

		// Ignore result as it'll report that it would have caused a block
		// Error handling is in update
		enet_socket_connect( mSocket, &addy );

		mTimer.fRestart( );
		mState = cStateConnectingSocket;
	}

	//------------------------------------------------------------------------------
	void tTitleServerConnection::fSelectRandomServer( )
	{
		sigcheckfail( mServers.fCount( ), return );

		mSelectedServer = tRandom::fSubjectiveRand( ).fIndex( mServers.fCount( ) );

		const tServerInfo& info = fSelectedServer( );
		log_line( Log::cFlagLSP, "Randomly chose server " << info.mName << " " << info.mAddress );
	}

	//------------------------------------------------------------------------------
	void tTitleServerConnection::fSelectNextServer( )
	{
		sigcheckfail( mServers.fCount( ), return );

		mSelectedServer = ( mSelectedServer + 1 ) % mServers.fCount( );

		const tServerInfo& info = fSelectedServer( );
		log_line( Log::cFlagLSP, "Next server is " << info.mName << " " << info.mAddress );
	}

	//------------------------------------------------------------------------------
	void tTitleServerConnection::fUpdateConnectingSocket( )
	{
		ENetSocketSet sockWriteQuery;
		ENET_SOCKETSET_EMPTY( sockWriteQuery );
		ENET_SOCKETSET_ADD( sockWriteQuery, mSocket );

		ENetSocketSet sockErrorQuery;
		ENET_SOCKETSET_EMPTY( sockErrorQuery );
		ENET_SOCKETSET_ADD( sockErrorQuery, mSocket );

		b32 failed = false;
		int selectResult = enet_socketset_select( 0, NULL, &sockWriteQuery, &sockErrorQuery, 0 );
		if( selectResult == 0 ) // 0 indicates a timeout
		{
			if( mTimer.fGetElapsedS( ) >= cTimeoutThreshold )
				failed = true;
		}
		else if( selectResult != -1 ) // No error
		{
			const int errorSet = ENET_SOCKETSET_CHECK( sockErrorQuery, mSocket );
			const int writeSet = ENET_SOCKETSET_CHECK( sockWriteQuery, mSocket );
			
			if( !errorSet && writeSet ) 
				fSendRequest( true );
			else failed = true;
		}
		else failed = true; // Straight up failure

		if( failed )
			fDisconnect( true );
	}

	//------------------------------------------------------------------------------
	void tTitleServerConnection::fSendRequest( b32 firstAttempt )
	{
		sigassert((	mState == cStateConnectingSocket || 
					mState == cStateSendingRequest ) && "Sanity!" );

		ENetBuffer buffer = { mRequestSize, mRequestBuffer };

		const int sentBytes = enet_socket_send( mSocket, NULL, &buffer, 1 );
		log_line( Log::cFlagLSP, "SendRequest: sentBytes " << sentBytes );
		if( sentBytes == 0 ) // WOULDBLOCK
		{
			mState = cStateSendingRequest;

			// If this is the first send, start the timeout timer
			if( firstAttempt )
				mTimer.fRestart( );
		}
		else if( sentBytes < 0 )
		{
			// Failure
			fDisconnect( true );
		}
		else // We sent some data!
		{
			sigassert( sentBytes <= (s32)mRequestSize && "Sanity!" );
			mRequestSize -= sentBytes;

			// Send more if need be
			if( mRequestSize )
			{
				mRequestBuffer += sentBytes;
				fSendRequest( true );
			}
			else
			{
				mRequestBuffer = NULL;
				if( !mRequestCallback.fNull( ) )
				{
					mBuffer.fZeroOut( );
					mRequestSize = 0;
					mRequestBuffer = mBuffer.fBegin( );
					fWaitForResponse( true );
				}
				else fDisconnect( false );
			}
		}
	}

	//------------------------------------------------------------------------------
	void tTitleServerConnection::fUpdateSendingRequest( )
	{
		fSendRequest( false );

		// If we're still sending, test the timeout
		if( mState == cStateSendingRequest )
		{
			if( mTimer.fGetElapsedS( ) >= cTimeoutThreshold )
				fDisconnect( true );
		}
	}

	//------------------------------------------------------------------------------
	void tTitleServerConnection::fWaitForResponse( b32 firstAttempt )
	{
		sigassert((	mState == cStateConnectingSocket || 
					mState == cStateSendingRequest ||
					mState == cStateWaitingForResponse ) && "Sanity!" );

		// -1 to guarantee that we've always got space for a terminating character
		ENetBuffer buffer = { mBuffer.fCount( ) - mRequestSize - 1, mRequestBuffer };

		int recvBytes = enet_socket_receive( mSocket, NULL, &buffer, 1 );
		if( recvBytes == 0 ) // WOULDBLOCK || CONNRESET
		{
			mState = cStateWaitingForResponse;

			// If this is the first receive 
			if( firstAttempt )
				mTimer.fRestart( );

			if( !enet_error_would_block( ) )
				fDisconnect( true ); // The socket is unusable
		}
		else if( recvBytes < 0 )
		{
			// Failure
			fDisconnect( true );
		}
		else
		{
			mRequestBuffer += recvBytes;
			mRequestSize += recvBytes;

			mTimer.fRestart( );
			mTimeoutAttempts = 0;
					
			sigassert( !mRequestCallback.fNull( ) && "Sanity!" );
			if( mRequestCallback( mBuffer.fBegin( ), mRequestSize ) )
				fDisconnect( false );
		}
	}

	//------------------------------------------------------------------------------
	void tTitleServerConnection::fUpdateWaitingForRequest( )
	{
		sigassert( mState == cStateWaitingForResponse && "Sanity!" );

		fWaitForResponse( false );

		// If we're still receiving check the timeout
		if( mState == cStateWaitingForResponse )
		{
			if( mTimer.fGetElapsedS( ) >= cTimeoutThreshold )
				fDisconnect( ++mTimeoutAttempts > fMin( cMaxTimeoutAttempts, mServers.fCount( ) ) );
		}
	}


}} // ::Sig::Net
