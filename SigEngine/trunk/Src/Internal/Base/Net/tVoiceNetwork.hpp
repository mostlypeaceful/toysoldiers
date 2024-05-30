//------------------------------------------------------------------------------
// \file tVoiceNetwork.hpp - 16 Dec 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tVoiceNetwork__
#define __tVoiceNetwork__
#include "tHost.hpp"

namespace Sig { namespace Net
{
	///
	/// \class tVoiceNetwork
	/// \brief 
	class tVoiceNetwork : public tUncopyable, public tRefCounter
	{
	public:

		tVoiceNetwork( u32 host = tHost::cDefaultAddress, u32 port = tHost::cDefaultPort + 1 );
		~tVoiceNetwork( );

		void fAddPeer( u32 host, u32 port );
		void fRemovePeer( u32 host, u32 port );
		void fRemoveAllPeers( );

		void fTick( f32 dt );

	private:
		
		struct tAddress
		{
			inline b32 operator==( const tAddress & o ) { return o.mHost == mHost && o.mPort == mPort; }
			u32 mHost;
			u32 mPort;
		};

	private:

		void fSendData( );
		void fReceiveData( );

		b32 fIsAPeer( u32 host, u32 port ) const;
		
	private:

		u64 mSocket;
		tGrowableArray< tAddress > mPeers;
	};

	define_smart_ptr( base_export, tRefCounterPtr, tVoiceNetwork );
}}

#endif//__tVoiceNetwork__
