//------------------------------------------------------------------------------
// \file tCompanionHost_xbox360.cpp - 14 June 2012
// \author colins
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#if defined( platform_xbox360 )
#include "tCompanionHost.hpp"

namespace Sig
{
	//--------------------------------------------------------------------------------------
	// tCompanionHost
	//--------------------------------------------------------------------------------------
	b32 tCompanionHost::fInitialize( )
	{
		mNewClients = false;

		HRESULT hr = XbcInitialize( fXbcStatusCallback, this );
		if( FAILED( hr ) )
		{
			log_warning( "XbcInitialize failed with result: " << hr );
			return false;
		}

		return true;
	}

	void tCompanionHost::fShutdown( )
	{
		XbcShutdown( );
	}

	void tCompanionHost::fTick( )
	{
		mNewClients = false;

		HRESULT hr = XbcDoWork( );
		if( hr != S_OK )
			log_warning( "XbcDoWork failed with result: " << hr );
	}

	VOID CALLBACK tCompanionHost::fXbcStatusCallback( HRESULT hr, PXBC_EVENT_PARAMS params, VOID* statePtr )
	{
		tCompanionHost* thisPtr = (tCompanionHost*)statePtr;
		Threads::tMutex lock( thisPtr->mCritSec );

		switch (params->Type)
		{
		case XBC_EVENT_CLIENT_CONNECTED:
			thisPtr->OnClientConnected( params->nClientId );
			break;

		case XBC_EVENT_CLIENT_DISCONNECTED:
			thisPtr->OnClientDisconnected( params->nClientId );
			break;

		case XBC_EVENT_JSON_SEND_COMPLETE:
			thisPtr->fSendJSONInternal( );
			break;

		case XBC_EVENT_JSON_RECEIVE_COMPLETE:
			break;
		}
	}

	void tCompanionHost::fSendJSON( tClientId clientId, const tJsonWriterPtr & writer )
	{
		sigassert( writer && "Cannot send JSON without valid writer" );
		
		Threads::tMutex lock( mCritSec );

		mOutbox.fPushBack( tOutboxEntry( clientId, writer ) );

		if( mOutbox.fCount( ) == 1 )
			fSendJSONInternal( );
	}

	void tCompanionHost::fSendJSONInternal( )
	{
		if( !mOutbox.fCount( ) )
			return;

		const tOutboxEntry& outboxEntry = mOutbox.fFront( );

		HRESULT hr = XbcSendJSON( XBC_DELIVERY_RELIABLE, outboxEntry.mClientId, outboxEntry.mWriter->fHandle( ), this );
		if( SUCCEEDED( hr ) || E_PENDING == hr )
			mOutbox.fPopFront( );

#ifdef sig_logging
		// Warn about messages that fail because they are too big
		if( hr == JSON_E_BUFFER_TOO_SMALL )
		{
			u32 size = 1024 * 1024;
			char *buf = new char[ size ];
			sigassert( buf );
			outboxEntry.mWriter->fGetBuffer( buf, &size );
			printf( "!WARNING! >> tCompanionHost::fSendJSON with buffer[%p][%.32s] size[%u] is too much data for JSON!\n", buf, buf, size );
			sigassert( false && "This JSON edge-case must be handled before ship!" );
			delete[] buf;
		}
#endif
	}
	
}
#endif//#if defined( platform_xbox360 )
