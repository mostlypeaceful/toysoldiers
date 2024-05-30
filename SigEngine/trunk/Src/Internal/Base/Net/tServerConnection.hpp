//------------------------------------------------------------------------------
// \file tServerConnection.hpp - 26 Apr 2012
// \author colins
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tServerConnection__
#define __tServerConnection__

#include "enet/enet.h"

namespace Sig { namespace Net 
{
	///
	/// \class tServerConnection
	/// \brief Will query for and create a tcp stream between the client and the
	///		   specified server
	class tServerConnection
	{
	public:

		enum tState
		{
			cStateNull = 0,
			cStateConnectingSocket,
			cStateConnected,
			cStateSendingRequest,
			cStateWaitingForResponse,
		};

		// Response - buffer, bufferSize
		typedef tDelegate< b32 ( const void *, u32 ) > tResponseCallback;

		static const u32 cInvalidRequestId = 0;

	public:

		tServerConnection( );
		~tServerConnection( );

		inline u32 fState( ) const { return mState; }
		inline b32 fConnected( ) const { return mState >= cStateConnected; }

		void fConnect( const char * serverName, u16 port );
		void fDisconnect( );

		void fUpdate( );

		// Indicates if it's ok to call fSend( )
		b32 fReadyToSend( );

		// Request a buffer be sent - only one request is allowed at a time
		// Returns whether the request was successfully enqueued
		b32 fSend( const void * buffer, u32 bytes );

		// If cb is a valid callback, the server assumes you expect a response
		b32 fSend( const void * buffer, u32 bytes, const tResponseCallback & cb );

	private:

		static const f32 cTimeoutThreshold;

	private:

		void fCreateAndConnectSocket( );
		void fUpdateConnectingSocket( );
		void fSendRequest( b32 firstAttempt );
		void fUpdateSendingRequest( );
		void fWaitForResponse( b32 firstAttempt );
		void fUpdateWaitingForResponse( );

	private:

		u32 mState;

		u32 mRequestSize;
		byte * mRequestBuffer;
		tResponseCallback mRequestCallback;

		Time::tStopWatch mTimer;

		ENetSocket mSocket;
		ENetAddress mAddress;

		tFixedArray<byte, 8 * 1024> mBuffer;
	};

}} // ::Sig::Net

#endif//__tServerConnection__
