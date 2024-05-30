#ifndef __tRemoteConnection__
#define __tRemoteConnection__
#include "tUser.hpp"
#include "tHost.hpp"
#include "tGameArchive.hpp"

typedef _ENetPeer ENetPeer;

namespace Sig { namespace Net
{
	///
	/// \class tRemoteConnection
	/// \brief Wrapper around user information and a peer connection
	class tRemoteConnection
	{
	public:

		tRemoteConnection( ) 
			: mInvitedToSession( false )
			, mAcceptedToSession( false ) { }

		explicit tRemoteConnection( const Net::tPeer & peer );

		inline b32 operator==( tPlatformUserId uid ) const { return mUser ? mUser->fPlatformId( ) == uid : uid == 0; }
		inline b32 operator==( const tPeer & peer ) const { return peer == mPeer; }
		inline b32 operator==( const ENetPeer * peer ) const { return mPeer == peer; }

		inline b32 fAcceptedToSession( ) const { return mAcceptedToSession; }
		inline void fSetAcceptedToSession( b32 accepted ) { mAcceptedToSession = accepted; }
		inline b32 fInvitedToSession( ) const { return mInvitedToSession; }
		inline void fSetInvitedToSession( b32 invited ) { mInvitedToSession = invited; }

		// Peer helper functions
		inline tPeer & fPeer( ) { return mPeer; }
		inline const tPeer & fPeer( ) const { return mPeer; }

		inline b32 fConnected( ) const { return mPeer.fValid( ); }
		void fDisconnect( b32 immediate = true );
		void fSendMessage( 
			const tGameArchiveSave& archive, 
			const tHost & host, 
			u32 channelId, 
			u32 packetFlags );

		template<class t>
		void fArchiveAndSend(
			t & msg,
			const tHost & host, 
			u32 channelId, 
			u32 packetFlags )
		{
			tGameArchiveSave save;
			save.fSave( msg );
			fSendMessage( save, host, channelId, packetFlags );
		}
		
		// tUser functions
		inline const tUserPtr & fUser( ) const { return mUser; }
		void fSetUser( const tUserPtr & user ) { mUser = user; }

		tPlatformUserId fUserId( ) const { return mUser ? mUser->fPlatformId( ) : tUser::cInvalidUserId; }
		u32 fAddOnFlags( ) const { return mUser ? mUser->fAddOnsInstalled( ) : 0; }
		const tLocalizedString& fGamerTag( ) const { return mUser->fGamerTag( ); }
		

		static void fExportScriptInterface( tScriptVm& vm );

	private:

		tPeer mPeer;
		tUserPtr mUser;
		b32 mInvitedToSession;
		b32 mAcceptedToSession;
	};

	typedef tGrowableArray<tRemoteConnection> tRemoteConnectionList;

}}

#endif//__tRemoteConnection__
