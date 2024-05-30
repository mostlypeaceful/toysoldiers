//------------------------------------------------------------------------------
// \file tTitleServerConnection_Xbox360.cpp - 17 Jan 2012
// \author jwittner
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tTitleServerConnection.hpp"
#include "XtlUtil.hpp"

namespace Sig { namespace Net 
{
	

	//------------------------------------------------------------------------------
	// tTitleServerConnection
	//------------------------------------------------------------------------------
	void tTitleServerConnection::fPlatformCtor( )
	{
		static_assert( SOCKET_ERROR == -1 );
		static_assert( sizeof( IN_ADDR ) <= sizeof_member( tTitleServerConnection::tServerInfo, mAddress ) );
		static_assert( sizeof( IN_ADDR ) <= sizeof_member( tTitleServerConnection, mSecureServerAddress ) );

		mServerEnumerator = INVALID_HANDLE_VALUE;
		fZeroOut( &mOverlapped );
	}

	//------------------------------------------------------------------------------
	void tTitleServerConnection::fPlatformDtor( )
	{
		sigcheckfail_xoverlapped_done_else_wait_cancel( &mOverlapped );
	}

	//------------------------------------------------------------------------------
	void tTitleServerConnection::fFindServer( )
	{
		sigassert( mState == cStateNoServer && "Sanity!" );

		DWORD bufferSize = 0;
		DWORD result = XTitleServerCreateEnumerator( mServerToConnect.c_str( ), cMaxServers, &bufferSize, &mServerEnumerator );

		if( result == ERROR_SUCCESS )
		{
			if( mServerInfoBuffer.fCount( ) < bufferSize )
				mServerInfoBuffer.fNewArray( bufferSize );

			fZeroOut( &mOverlapped );
			result = XEnumerate( 
				mServerEnumerator, 
				mServerInfoBuffer.fBegin( ), 
				bufferSize, NULL, &mOverlapped );

			if( result == ERROR_IO_PENDING )
				mState = cStateFindingServer;
		}
		else
			log_warning( "XTitleServerCreateEnumerator failed with result: " << result );

		// Something went wrong
		if( mState != cStateFindingServer )
		{
			if( mServerEnumerator != INVALID_HANDLE_VALUE )
			{
				XCloseHandle( mServerEnumerator );
				mServerEnumerator = INVALID_HANDLE_VALUE;
			}

			fDisconnect( true );
		}
	}

	//------------------------------------------------------------------------------
	void tTitleServerConnection::fUpdateFindingServer( )
	{
		sigassert( mState == cStateFindingServer && "Sanity!" );

		if( !XHasOverlappedIoCompleted( &mOverlapped ) )
			return;

		HRESULT result = XGetOverlappedExtendedError( &mOverlapped );
		if( result == ERROR_SUCCESS && mOverlapped.InternalHigh > 0 /*Count is stored in InternalHigh*/ )
		{
			XTITLE_SERVER_INFO * info = (XTITLE_SERVER_INFO*)( mServerInfoBuffer.fBegin( ) );
			const unsigned int numServers = mOverlapped.InternalHigh;

			log_line( Log::cFlagLSP, "Found " << numServers << " title servers" );

			// If we found at least 1 gather the keepers
			if( numServers )
			{
				sigcheckfail( mServers.fCount( ) == 0, mServers.fSetCount( 0 ) );

				mServers.fReserve( numServers );
				for( u32 i = 0; i < numServers; ++i )
				{
					const XTITLE_SERVER_INFO& serverInf = info[ i ];

					tStringPtr serverName( serverInf.szServerInfo );
					if( !mServersToIgnore.fFind( serverName ) )
					{
						log_line( Log::cFlagLSP, "Keeping " << serverName );

						tServerInfo& servToKeep = mServers.fPushBack( );
						servToKeep.mName = serverName;
						servToKeep.mAddress = *((u32*)&serverInf.inaServer);
					}
					else log_line( Log::cFlagLSP, "Ignoring " << serverName );
				}
			}

			// If we found some servers then randomly select a start point and mark us ready
			if( mServers.fCount( ) )
			{
				log_line( Log::cFlagLSP, "Total servers available " << mServers.fCount( ) );

				fSelectRandomServer( );
				mState = cStateIdle;
			}
			else fDisconnect( true );
		}
		else
		{
			log_warning( "Unable to find a title server. Result: 0x" << std::hex << result );

			fDisconnect( true );
		}

		XCloseHandle( mServerEnumerator );
		mServerEnumerator = INVALID_HANDLE_VALUE;
	}

	//------------------------------------------------------------------------------
	void tTitleServerConnection::fConnectServer( )
	{
		sigcheckfail( mState == cStateIdle && "Sanity!", return );

		const tServerInfo& server = fSelectedServer( );

		//Transform the raw IP into a secure address that we can connect on
		INT result = XNetServerToInAddr( 
			*(const IN_ADDR*)(&server.mAddress), // Server's ip address
			mServiceId,							 // Service id - from server
			(IN_ADDR*)(&mSecureServerAddress)   // Output secure ip address
		);

		log_line( Log::cFlagLSP, "ConnectServer: mServiceId " << mServiceId << ", result " << result );

		// Converted?
		if( result == ERROR_SUCCESS )
		{
			result = XNetConnect( *(const IN_ADDR*)(&mSecureServerAddress) );

			log_line( Log::cFlagLSP, "XNetConnect: result " << result );

			// Connecting?
			if( result == ERROR_SUCCESS )
			{
				mState = cStateConnectingServer;
				mConnectAttempts = 0;
			}
			else 
			{
				XNetUnregisterInAddr( *(const IN_ADDR*)(&mSecureServerAddress) );
				mSecureServerAddress = 0;
			}
		}

		// Error?
		if( mState != cStateConnectingServer )
		{
			if( ++mConnectAttempts >= cMaxConnectAttempts )
				fDisconnect( true );
			else
			{
				fSelectNextServer( );
				mState = cStateIdle;
			}
		}
	}

	//------------------------------------------------------------------------------
	void tTitleServerConnection::fUpdateConnectingServer( )
	{
		sigcheckfail( mState == cStateConnectingServer && "Sanity!", return );

		DWORD result = XNetGetConnectStatus( *(const IN_ADDR*)(&mSecureServerAddress) );
		switch( result )
		{
		case XNET_CONNECT_STATUS_PENDING: break; //Still waiting
		case XNET_CONNECT_STATUS_CONNECTED: 
			log_line( Log::cFlagLSP, "Connected to title server" );
			fCreateAndConnectSocket( ); break;
		case XNET_CONNECT_STATUS_IDLE: // Intentional fall-through
		case XNET_CONNECT_STATUS_LOST:
			{
				log_line( Log::cFlagLSP, "Failed to connect to title server after attempt " << mConnectAttempts+1 );
				if( ++mConnectAttempts >= cMaxConnectAttempts )
					fDisconnect( true );
				else
				{
					fSelectNextServer( );
					mState = cStateIdle;
				}
			} break;
		}
	}
	
	//------------------------------------------------------------------------------
	void tTitleServerConnection::fPlatformDisconnect( )
	{
		if( mSecureServerAddress )
		{
			XNetUnregisterInAddr( *(const IN_ADDR*)(&mSecureServerAddress) );
			mSecureServerAddress = 0;
		}
	}

}} // ::Sig::Net

