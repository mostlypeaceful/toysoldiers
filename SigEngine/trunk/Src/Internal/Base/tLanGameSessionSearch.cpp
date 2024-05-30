//------------------------------------------------------------------------------
// \file tLanGameSessionSearch.cpp - 21 Mar 2012
// \author colins
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tLanGameSessionSearch.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	// Utility functions
	//------------------------------------------------------------------------------
	namespace
	{
		void fSendMsg( tDynamicBuffer& msg, const ENetSocket& socket, ENetAddress addr )
		{
			ENetBuffer sendBuffer;
			sendBuffer.data = msg.fBegin( );
			sendBuffer.dataLength = msg.fCount( );
			int sent = enet_socket_send( socket, &addr, &sendBuffer, 1 );
			if( sent != sendBuffer.dataLength )
				log_line( Log::cFlagNetwork, "Failed to send LAN GameSession message." );
		}
	}

	//------------------------------------------------------------------------------
	// tLanGameSessionAdvertiser
	//------------------------------------------------------------------------------
	b32 tLanGameSessionAdvertiser::fStart( const void* sessionDesc, u32 sessionDescSize, const ENetSocket& sessionSocket, u16 advertisePort )
	{
		sigassert( !fStarted( ) );
		sigassert( sessionDesc && sessionDescSize && "You must pass a valid sessionDesc" );

		mSocket = enet_socket_create( ENET_SOCKET_TYPE_DATAGRAM );
		sigassert( mSocket != ENET_SOCKET_NULL && "Failed to create socket for broadcast" );

		// Set to non-blocking
		int error = enet_socket_set_option( mSocket, ENET_SOCKOPT_NONBLOCK, 1 );
		sigassert( !error && "Failed to set socket to non-blocking" );

		// Bind to the specified port
		ENetAddress addy = { ENET_HOST_ANY, advertisePort };
		error = enet_socket_bind( mSocket, &addy );
		if( error )
		{
			log_warning( "tLanGameSessionSearch failed to bind socket for advertising" );
			fStop( );

			return false;
		}

		mSessionDesc.fInitialize( ( const byte* )sessionDesc, sessionDescSize );
		mSessionSocket = sessionSocket;

		return true;
	}

	void tLanGameSessionAdvertiser::fStop( )
	{
		if( mSocket != ENET_SOCKET_NULL )
		{
			enet_socket_destroy( mSocket );
			mSocket = ENET_SOCKET_NULL;
		}
	}

	void tLanGameSessionAdvertiser::fTick( )
	{
		if( !fStarted( ) )
			return;

		tDynamicBuffer bufferData;
		bufferData.fNewArray( mSessionDesc.fCount( ) );

		// Receive data
		ENetAddress recvAddr;
		b32 doneReceiving = false;
		while( !doneReceiving )
		{
			ENetBuffer recvBuffer;
			recvBuffer.data = bufferData.fBegin( );
			recvBuffer.dataLength = bufferData.fCount( );

			int received = enet_socket_receive( mSocket, &recvAddr, &recvBuffer, 1 );
			if( received > 0 )
				fHandleMsg( recvAddr, recvBuffer.data, recvBuffer.dataLength );
			else
				doneReceiving = true;
		}
	}

	void tLanGameSessionAdvertiser::fHandleMsg( const ENetAddress& recvAddr, const void* data, u32 dataLength )
	{
		if( dataLength != mSessionDesc.fCount( ) )
		{
			log_line( Log::cFlagNetwork, "tLanGameSessionAdvertiser received an invalid message." );
			return;
		}

		// If the descriptors match, send a response
		if( fMemCmp( data, mSessionDesc.fBegin( ), dataLength ) == 0 )
			fSendMsg( mSessionDesc, mSessionSocket, recvAddr );
	}

	//------------------------------------------------------------------------------
	// tLanGameSessionSearch
	//------------------------------------------------------------------------------
	const f32 tLanGameSessionSearch::cSendTimeMs = 500.f;
	const char* tLanGameSessionSearch::cDefaultHost = "255.255.255.255";

	void tLanGameSessionSearch::fBegin( const void* sessionDesc, u32 sessionDescSize, f32 searchTimeoutS, const char* host, u16 port )
	{
		sigassert( sessionDesc && sessionDescSize && "You must pass a valid sessionDesc" );

		fInit( );

		mHostAddress.host = inet_addr( host );
		mHostAddress.port = port;
		mSessionDesc.fInitialize( ( const byte* )sessionDesc, sessionDescSize );
		mSearchTimeoutS = searchTimeoutS;
		mSearchTimer.fRestart( );
		mState = cStateSearching;

		fSendQuery( );
	}

	void tLanGameSessionSearch::fTick( )
	{
		if( mState == cStateInactive )
			return;

		tDynamicBuffer bufferData;
		bufferData.fNewArray( mSessionDesc.fCount( ) );

		// Receive data
		ENetAddress recvAddr;
		b32 doneReceiving = false;
		while( !doneReceiving )
		{
			ENetBuffer recvBuffer;
			recvBuffer.data = bufferData.fBegin( );
			recvBuffer.dataLength = bufferData.fCount( );

			int received = enet_socket_receive( mSocket, &recvAddr, &recvBuffer, 1 );
			if( received > 0 )
				fHandleMsg( recvAddr, recvBuffer.data, recvBuffer.dataLength );
			else
				doneReceiving = true;
		}

		// Check if we should send another query
		if( fSearching( ) && mSendTimer.fGetElapsedMs( ) > cSendTimeMs )
			fSendQuery( );

		// Check if we've searched long enough
		if( fSearching( ) && mSearchTimer.fGetElapsedS( ) > mSearchTimeoutS )
		{
			mState = cStateFinished;
		}
	}

	void tLanGameSessionSearch::fInit( )
	{
		fReset( );

		mSocket = enet_socket_create( ENET_SOCKET_TYPE_DATAGRAM );
		sigassert( mSocket != ENET_SOCKET_NULL && "Failed to create socket for broadcast" );

		// Set to non-blocking
		int error = enet_socket_set_option( mSocket, ENET_SOCKOPT_NONBLOCK, 1 );
		sigassert( !error && "Failed to set socket to non-blocking" );

		// Set to enable broadcast
		error = enet_socket_set_option( mSocket, ENET_SOCKOPT_BROADCAST, 1 );
		sigassert( !error && "Failed to set socket to broadcast" );

		// Bind
		ENetAddress myAddr = { ENET_HOST_ANY, ENET_PORT_ANY };
		error = enet_socket_bind( mSocket, &myAddr );
		sigassert( !error && "Failed to bind socket for searching" );
	}

	void tLanGameSessionSearch::fReset( )
	{
		mResults.fSetCount( 0 );

		// Destroy the socket if it's valid
		if( mSocket != ENET_SOCKET_NULL )
		{
			enet_socket_destroy( mSocket );
			mSocket = ENET_SOCKET_NULL;
		}
		mState = cStateInactive;
	}

	void tLanGameSessionSearch::fSendQuery( )
	{
		fSendMsg( mSessionDesc, mSocket, mHostAddress );

		mSendTimer.fRestart( );
	}

	void tLanGameSessionSearch::fHandleMsg( const ENetAddress& recvAddr, const void* data, u32 dataLength )
	{
		if( dataLength != mSessionDesc.fCount( ) )
		{
			log_line( Log::cFlagNetwork, "tLanGameSessionSearch received an invalid message." );
			return;
		}

		// Store the result
		tSearchResult result;
		result.hostAddr = recvAddr;
		mResults.fPushBack( result );
	}
}
