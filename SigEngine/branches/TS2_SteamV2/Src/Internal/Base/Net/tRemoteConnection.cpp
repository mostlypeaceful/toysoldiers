#include "BasePch.hpp"
#include "tRemoteConnection.hpp"
#include "enet/enet.h"

namespace Sig { namespace Net
{
	//------------------------------------------------------------------------------
	tRemoteConnection::tRemoteConnection( const Net::tPeer & peer )
		: mInvitedToSession( false )
		, mAcceptedToSession( false )
		, mPeer( peer )
	{
	}

	//------------------------------------------------------------------------------
	void tRemoteConnection::fDisconnect( b32 immediate )
	{
		if( !fConnected( ) )
			return;

		if( immediate )
			mPeer.fDisconnectNow( );
		else
			mPeer.fDisconnect( );
	}

	//------------------------------------------------------------------------------
	void tRemoteConnection::fSendMessage( 
		const tGameArchiveSave& archive, 
		const tHost & host, 
		u32 channelId, 
		u32 packetFlags )
	{
		host.fSend( 
			mPeer, 
			archive.fBuffer( ).fBegin( ), 
			archive.fBuffer( ).fCount( ), 
			channelId, 
			packetFlags, 
			true );
	}
}}


namespace Sig { namespace Net
{
	void tRemoteConnection::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tRemoteConnection,Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		classDesc
			.Prop("GamerTag", &tRemoteConnection::fGamerTag)
			.Var("AcceptedToSession", &tRemoteConnection::mAcceptedToSession)
			;
		vm.fRootTable( ).Bind( _SC("RemoteConnection"), classDesc );
	}

}}
