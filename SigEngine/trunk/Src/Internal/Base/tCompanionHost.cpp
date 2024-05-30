//------------------------------------------------------------------------------
// \file tCompanionHost.cpp - 01 Oct 2012
// \author jwittner
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tCompanionHost.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	// tCompanionHost
	//------------------------------------------------------------------------------
	void tCompanionHost::fClientIds( tClientIdList & out )
	{
		Threads::tMutex lock( mCritSec );
		out = mClientIds;
	}

	u32 tCompanionHost::fOutboxCount()
	{
		Threads::tMutex lock( mCritSec );
		return mOutbox.fCount( );
	}

	void tCompanionHost::OnClientConnected( tClientId clientId )
	{
		mNewClients = true;

		mClientIds.fPushBack( clientId );
	}

	void tCompanionHost::OnClientDisconnected( tClientId clientId )
	{
		mClientIds.fFindAndErase( clientId );

		// remove dead messages
		for( s32 i = mOutbox.fCount( ) - 1; i >= 0; --i )
		{
			if( mOutbox[ i ].mClientId == clientId )
				mOutbox.fEraseOrdered( i );
		}
	}

} // ::Sig
