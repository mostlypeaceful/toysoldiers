//------------------------------------------------------------------------------
// \file tCompanionHost.hpp - 14 June 2012
// \author colins
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tCompanionHost__
#define __tCompanionHost__
#ifdef platform_xbox360
	#include <xbc.h>
#endif
#include "tJsonWriter.hpp"

namespace Sig
{
	///
	/// \class tCompanionHost
	/// \brief Functionality for communicating with Companion apps
	class tCompanionHost
	{
	public:

		// typedefs
		typedef DWORD tClientId;
		typedef tGrowableArray<tClientId> tClientIdList;

		struct tOutboxEntry
		{
			tClientId mClientId;
			tJsonWriterPtr mWriter;
			tOutboxEntry( )
				: mClientId( ~0 )
			{
			}
			tOutboxEntry( const tClientId& id, const tJsonWriterPtr& writer )
				: mClientId( id )
				, mWriter( writer )
			{
			}
		};
		typedef tGrowableArray<tOutboxEntry> tOutbox;

	public:

		b32 fInitialize( );
		void fShutdown( );
		void fTick( );

		void fSendJSON( tClientId clientId, const tJsonWriterPtr & writer );

		void fClientIds( tClientIdList & out );
		u32 fOutboxCount( );

		b32 fNewClients( ) const { return mNewClients; }

	private:

		void fSendJSONInternal( );
		void OnClientConnected( tClientId clientId );
		void OnClientDisconnected( tClientId clientId );

		
#ifdef platform_xbox360
		// Xbox Companion callback handler
		static VOID CALLBACK fXbcStatusCallback( HRESULT hr, PXBC_EVENT_PARAMS params, VOID* statePtr );
#endif

	private:

		Threads::tCriticalSection mCritSec;
		tClientIdList mClientIds;
		b32 mNewClients;
		
		tOutbox mOutbox;
	};

}
#endif//__tCompanionHost__
