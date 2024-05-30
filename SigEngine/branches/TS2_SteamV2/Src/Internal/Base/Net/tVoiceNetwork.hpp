//------------------------------------------------------------------------------
// \file tVoiceNetwork.hpp - 16 Dec 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tVoiceNetwork__
#define __tVoiceNetwork__
#include "tHost.hpp"
#include "enet\enet.h"

namespace Sig { namespace Net
{
	///
	/// \class tVoiceNetwork
	/// \brief 
	class tVoiceNetwork : public tUncopyable, public tRefCounter
	{
	public:

		tVoiceNetwork( tAddr host = tHost::cDefaultAddress, u32 port = tHost::cDefaultPort + 1 );
		~tVoiceNetwork( );

		void fAddPeer( tAddr host, u32 port );
		void fRemovePeer( tAddr host, u32 port );
		void fRemoveAllPeers( );

		void fTick( f32 dt );

	private:
		
		struct tAddress
		{
			inline b32 operator==( const tAddress & o ) { return o.mHost == mHost && o.mPort == mPort; }
			tAddr mHost;
			u32 mPort;
		};

	private:

		void fSendData( );
		void fReceiveData( );

		b32 fIsAPeer( tAddr host, u32 port ) const;
		
	private:

		ENetSocket mSocket;
		tGrowableArray< tAddress > mPeers;
	};

	define_smart_ptr( base_export, tRefCounterPtr, tVoiceNetwork );
}}

#endif//__tVoiceNetwork__
