//------------------------------------------------------------------------------
// \file tLanGameSessionSearch.hpp - 21 Mar 2012
// \author colins
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tLanGameSessionSearch__
#define __tLanGameSessionSearch__

#include "enet/enet.h"

namespace Sig
{
	///
	/// \class tLanGameSessionAdvertiser
	/// \brief Listens on a specified port and responds to LAN GameSession searches
	class tLanGameSessionAdvertiser
	{
	public:
		static const u16 cDefaultPort = 1023;

		tLanGameSessionAdvertiser( )
			: mSocket( ENET_SOCKET_NULL )
			, mSessionSocket( ENET_SOCKET_NULL )
		{
		}

		b32 fStart( const void* sessionDesc, u32 sessionDescSize, const ENetSocket& sessionSocket, u16 advertisePort = cDefaultPort );
		void fStop( );
		void fTick( );

		b32 fStarted( ) const { return ( mSocket != ENET_SOCKET_NULL ); }

	private:
		void fHandleMsg( const ENetAddress& recvAddr, const void* data, u32 dataLength );

	private:
		ENetSocket mSocket;
		tDynamicBuffer mSessionDesc;
		ENetSocket mSessionSocket;
	};

	///
	/// \class tLanGameSessionSearch
	/// \brief Searches for game sessions by broadcasting over the LAN
	class tLanGameSessionSearch
	{
	public:
		enum tState
		{
			cStateInactive,
			cStateSearching,
			cStateFinished
		};

		struct tSearchResult
		{
			ENetAddress hostAddr;
		};

		static const f32 cSendTimeMs; //Time to wait between queries
		static const u16 cDefaultPort = 1023;
		static const char* cDefaultHost;

	public:
		tLanGameSessionSearch( )
			: mSocket( ENET_SOCKET_NULL ), mState( cStateInactive ), mSearchTimeoutS( 0.f )
		{
		}

		b32 fInactive( ) const { return mState == cStateInactive; }
		b32 fSearching( ) const { return mState == cStateSearching; }
		b32 fFinished( ) const { return mState == cStateFinished; }

		void fBegin( const void* sessionDesc, u32 sessionDescSize, f32 searchTimeoutS, const char* host = cDefaultHost, u16 port = cDefaultPort );
		void fTick( );
		void fReset( );

		u32 fResultCount( ) const { return mResults.fCount( ); }
		const tSearchResult& fResult( u32 idx ) { return mResults[ idx ]; }

	private:
		void fInit( );
		void fSendQuery( );

		void fHandleMsg( const ENetAddress& recvAddr, const void* data, u32 dataLength );

	private:
		ENetSocket mSocket;
		tState mState;
		tDynamicBuffer mSessionDesc;
		ENetAddress mHostAddress;
		tGrowableArray< tSearchResult > mResults;
		f32 mSearchTimeoutS;
		Time::tStopWatch mSearchTimer;
		Time::tStopWatch mSendTimer;
	};
}

#endif//__tLanGameSessionSearch__
