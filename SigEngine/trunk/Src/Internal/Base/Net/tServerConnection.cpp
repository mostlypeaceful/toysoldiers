//------------------------------------------------------------------------------
// \file tServerConnection.cpp - 26 Apr 2012
// \author colins
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tServerConnection.hpp"

namespace Sig { namespace Net 
{
#ifdef sig_logging
	const f32 tServerConnection::cTimeoutThreshold = Math::cInfinity;
#else
	const f32 tServerConnection::cTimeoutThreshold = 8.f;
#endif


	//------------------------------------------------------------------------------
	// tServerConnection
	//------------------------------------------------------------------------------
	tServerConnection::tServerConnection( )
		: mState( cStateNull )
		, mRequestSize( 0 )
		, mRequestBuffer( NULL )
		, mSocket( ENET_SOCKET_NULL )
	{
	}

	//------------------------------------------------------------------------------
	tServerConnection::~tServerConnection( )
	{
		fDisconnect( );
	}

	//------------------------------------------------------------------------------
	void tServerConnection::fConnect( const char * serverName, u16 port )
	{
		fDisconnect( );

		log_line( Log::cFlagLSP, "Connecting to server " << serverName << ":" << port );

		// Create the socket
		mSocket = enet_socket_create( ENET_SOCKET_TYPE_STREAM );
		sigassert( mSocket != ENET_SOCKET_NULL );

		// Set to non-blocking
		int result = enet_socket_set_option( mSocket, ENET_SOCKOPT_NONBLOCK, 1 );
		sigassert( result == 0 && "Socket could not be set to non-blocking" );

		mAddress.port = ENET_HOST_TO_NET_16( port );
		result = enet_address_set_host( &mAddress, serverName );
		sigassert( result == 0 && "Failed to convert server name to address" );

		// Connect
		result = enet_socket_connect( mSocket, &mAddress );
		sigassert( result == 0 || ( result == -1 && enet_error_would_block( ) ) && "Failed to connect socket" );

		mTimer.fRestart( );
		mState = cStateConnectingSocket;
	}

	//------------------------------------------------------------------------------
	void tServerConnection::fDisconnect( )
	{
		if( mSocket != ENET_SOCKET_NULL )
		{
			log_line( Log::cFlagLSP, "Disconnecting from server " );
			enet_socket_destroy( mSocket );
			mSocket = ENET_SOCKET_NULL;
		}

		mRequestSize = 0;

		mState = cStateNull;
	}

	//------------------------------------------------------------------------------
	void tServerConnection::fUpdate( )
	{
		switch( mState )
		{
		case cStateNull: break;
		case cStateConnected:
			if( mRequestSize > 0 )
			{
				mState = cStateSendingRequest;
				fSendRequest( true );
			}
			break;
		case cStateConnectingSocket: fUpdateConnectingSocket( ); break;
		case cStateSendingRequest: fUpdateSendingRequest( ); break;
		case cStateWaitingForResponse: fUpdateWaitingForResponse( ); break;

		default: sig_nodefault( );
		}
	}

	//------------------------------------------------------------------------------
	b32	tServerConnection::fReadyToSend( )
	{
		return mRequestSize == 0 && mState == cStateConnected;
	}

	//------------------------------------------------------------------------------
	b32 tServerConnection::fSend( const void * buffer, u32 bytes )
	{
		return fSend( buffer, bytes, tResponseCallback( ) );
	}

	//------------------------------------------------------------------------------
	b32 tServerConnection::fSend( const void * buffer, u32 bytes, const tResponseCallback & cb )
	{
		sigassert( fReadyToSend( ) );
		if( !fReadyToSend( ) )
			return false;

		sigassert( bytes < mBuffer.fCount( ) );

		mRequestCallback = cb;
		mRequestSize = bytes;

		fMemCpy( mBuffer.fBegin( ), buffer, bytes );
		mRequestBuffer = mBuffer.fBegin( );
		return true;
	}

	//------------------------------------------------------------------------------
	void tServerConnection::fUpdateConnectingSocket( )
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
			{
				log_line( Log::cFlagLSP, "Connected to server" );
				mState = cStateConnected;
			}
			else failed = true;
		}
		else failed = true; // Straight up failure

		if( failed )
		{
			log_warning( "Failed to connect to server" );
			fDisconnect( );
		}
	}

	//------------------------------------------------------------------------------
	void tServerConnection::fSendRequest( b32 firstAttempt )
	{
		ENetBuffer buffer = { mRequestSize, mRequestBuffer };

		const int sentBytes = enet_socket_send( mSocket, &mAddress, &buffer, 1 );
		if( sentBytes == 0 ) // WOULDBLOCK
		{
			mState = cStateSendingRequest;

			// If this is the first send, start the timeout timer
			if( firstAttempt )
				mTimer.fRestart( );
		}
		else if( sentBytes < 0 )
		{
			log_warning( "SendRequest failed with result " << sentBytes );

			// Failure
			fDisconnect( );
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
				else fDisconnect( );
			}
		}
	}

	//------------------------------------------------------------------------------
	void tServerConnection::fUpdateSendingRequest( )
	{
		fSendRequest( false );

		// If we're still sending, test the timeout
		if( mState == cStateSendingRequest )
		{
			if( mTimer.fGetElapsedS( ) >= cTimeoutThreshold )
				fDisconnect( );
		}
	}

	//------------------------------------------------------------------------------
	void tServerConnection::fWaitForResponse( b32 firstAttempt )
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
				fDisconnect( ); // The socket is unusable
		}
		else if( recvBytes < 0 )
		{
			// Failure
			fDisconnect( );
		}
		else
		{
			mRequestBuffer += recvBytes;
			mRequestSize += recvBytes;

			sigassert( !mRequestCallback.fNull( ) && "Sanity!" );
			if( mRequestCallback( mBuffer.fBegin( ), mRequestSize ) )
			{
				mState = cStateConnected;
				mRequestSize = 0;
			}
		}
	}

	//------------------------------------------------------------------------------
	void tServerConnection::fUpdateWaitingForResponse( )
	{
		sigassert( mState == cStateWaitingForResponse && "Sanity!" );

		fWaitForResponse( false );

		// If we're still receiving check the timeout
		if( mState == cStateWaitingForResponse )
		{
			if( mTimer.fGetElapsedS( ) >= cTimeoutThreshold )
				fDisconnect( );
		}
	}


}} // ::Sig::Net
