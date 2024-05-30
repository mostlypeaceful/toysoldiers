//------------------------------------------------------------------------------
// \file tGameSessionNetwork.cpp - 15 Oct 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tGameSessionNetwork.hpp"
#include "GameArchiveString.hpp"
#include "tVoice.hpp"
#include "tApplication.hpp"
#include "enet/enet.h"
#include <memory>

namespace Sig {

	//------------------------------------------------------------------------------
	// Protocol
	//------------------------------------------------------------------------------
	namespace
	{
		enum tPacketType
		{
			cPacketTypeJoinRequest = 0,
			cPacketTypeJoinAccepted,
			cPacketTypeJoinRejected,
			cPacketTypePeerInfo,
			cPacketTypeArbitrated,
			cPacketTypeVoiceMute,

			cPacketTypeCount
		};

		///
		/// \class tPacketHeader
		/// \brief 
		struct tPacketHeader
		{
			u32 mPacketType;

			explicit tPacketHeader( u32 id = cPacketTypeCount )
				: mPacketType( id ) { }

			template<class tArchive>
			void fSaveLoad( tArchive& archive )
			{
				archive.fSaveLoad( mPacketType );
			}
		};

		///
		/// \class tJoinRequestPacket
		/// \brief 
		struct tJoinRequestPacket : public tPacketHeader
		{
			tUserInfo				mUserInfo;
			b32						mInvited;

			tJoinRequestPacket( )
				: tPacketHeader( cPacketTypeJoinRequest )
				, mInvited( false ) { }

			explicit tJoinRequestPacket( const tUserInfo& user, b32 invited = false )
				: tPacketHeader( cPacketTypeJoinRequest )
				, mUserInfo( user )
				, mInvited( invited ) { }

			template<class tArchive>
			void fSaveLoad( tArchive& archive )
			{
				tPacketHeader::fSaveLoad( archive );
				archive.fSaveLoad( mUserInfo );
				archive.fSaveLoad( mInvited );
			}
		};

		///
		/// \class tJoinAcceptedPacket
		/// \brief 
		struct tJoinAcceptedPacket : public tPacketHeader
		{
			u64 mSessionNonce;
			u32 mSessionCreateFlags;
			u32 mSessionGameMode;
			u32 mSessionGameType;
			u32 mTotalPublicSlots;
			u32 mTotalPrivateSlots;

			tUserInfo mUserInfo;

			tJoinAcceptedPacket( )
				: tPacketHeader( cPacketTypeJoinAccepted )
				, mSessionNonce( 0 ) { }

			template<class tArchive>
			void fSaveLoad( tArchive& archive )
			{
				tPacketHeader::fSaveLoad( archive );
				archive.fSaveLoad( mSessionNonce );
				archive.fSaveLoad( mSessionCreateFlags );
				archive.fSaveLoad( mSessionGameMode );
				archive.fSaveLoad( mSessionGameType );
				archive.fSaveLoad( mTotalPublicSlots );
				archive.fSaveLoad( mTotalPrivateSlots );
				archive.fSaveLoad( mUserInfo );
			}
		};

		///
		/// \class tJoinRejectedPacket
		/// \brief 
		struct tJoinRejectedPacket : public tPacketHeader
		{
			enum tReason
			{
				cReasonInvalid = 0,
				cReasonSessionFull,
			};

			u32 mReason;

			tJoinRejectedPacket( )
				: tPacketHeader( cPacketTypeJoinRejected )
				, mReason( cReasonInvalid ) { }

			template<class tArchive>
			void fSaveLoad( tArchive& archive )
			{
				tPacketHeader::fSaveLoad( archive );
				archive.fSaveLoad( mReason );
			}
		};

		///
		/// \class tPeerInfoPacket
		/// \brief 
		struct tPeerInfoPacket : public tPacketHeader
		{
			tUserInfo	mUserInfo;
			b32			mInvited;
			b32			mLeaving;

			tPeerInfoPacket( )
				: tPacketHeader( cPacketTypePeerInfo )
				, mLeaving( false ) { }

			template<class tArchive>
			void fSaveLoad( tArchive& archive )
			{
				tPacketHeader::fSaveLoad( archive );
				archive.fSaveLoad( mUserInfo );
				archive.fSaveLoad( mInvited );
				archive.fSaveLoad( mLeaving );
			}
		};

		///
		/// \class tArbitratedPacket
		/// \brief 
		struct tArbitratedPacket : public tPacketHeader
		{
			tArbitratedPacket( ) : tPacketHeader( cPacketTypeArbitrated ) { }

			template< class tArchive >
			void fSaveLoad( tArchive & archive )
			{
				tPacketHeader::fSaveLoad( archive );
			}
		};

		///
		/// \class tVoiceMutePacket
		/// \brief 
		struct tVoiceMutePacket : public tPacketHeader
		{
			tVoiceMutePacket( ) : tPacketHeader( cPacketTypeVoiceMute ) { }

			tGrowableBuffer mMuteMsg;

			template< class tArchive >
			void fSaveLoad( tArchive & archive )
			{
				tPacketHeader::fSaveLoad( archive );
				archive.fSaveLoad( mMuteMsg );
			}
		};
	}

	//------------------------------------------------------------------------------
	template<class t>
	static void fDecodePacket( t & out, const byte * data, u32 dataLength )
	{
		tGameArchiveLoad archive( data, dataLength );
		archive.fSaveLoad( out );
	}

	//------------------------------------------------------------------------------
	template<class t>
	static void fSendPacket( t & packet, u32 channelId, const Net::tPeer & peer, b32 reliable, b32 flush = false )
	{
		tGameArchiveSave archive;
		archive.fSaveLoad( packet );
		peer.fSend( 
			archive.fBuffer( ).fBegin( ), 
			archive.fBuffer( ).fTotalSizeOf( ), 
			channelId, 
			reliable ? ENET_PACKET_FLAG_RELIABLE : 0, 
			flush );
	}

	//------------------------------------------------------------------------------
	template<class t>
	static void fSendPacketToPeers( t & packet, u32 channelId, Net::tP2PNetwork & network , b32 reliable, b32 flush = false )
	{
		tGameArchiveSave archive;
		archive.fSaveLoad( packet );
		network.fSendMsgToPeers( channelId, archive.fBuffer( ).fBegin( ), archive.fBuffer( ).fCount( ), true );

		if( flush )
			network.fFlush( );
	}

	//------------------------------------------------------------------------------
	// tGameSessionNetwork::tSessionUserArray
	//------------------------------------------------------------------------------
	b32 tGameSessionNetwork::tSessionUserArray::fJoinLocal( tGameSession & session )
	{
		const u32 numUsers = fCount( );

		u32 userCount = 0;
		u32 zombieCount = 0;
		u32 * indices = ( u32 * )alloca( sizeof( u32 ) * numUsers );
		b32 * invited = ( b32 * )alloca( sizeof( b32 ) * numUsers );
		 

		for( u32 u = 0; u < numUsers; ++u )
		{
			tSessionUser & user = operator[]( u );
			if( !user.mIsZombie )
			{
				indices[ userCount ] = user.mUser->fLocalHwIndex( );
				invited[ userCount ] = user.mInvited;
				++userCount;
			}
			else
			{
				++zombieCount;
			}
		}

		if( zombieCount && !session.fJoinZombie( zombieCount ) )
			return false;

		if( userCount && !session.fJoinLocal( userCount, indices, invited ) )
			return false;

		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSessionNetwork::tSessionUserArray::fLeaveLocal( tGameSession & session )
	{	
		const u32 numUsers = fCount( );

		u32 userCount = 0;
		u32 zombieCount = 0;
		u32 * indices = ( u32 * )alloca( sizeof( u32 ) * numUsers );

		for( u32 u = 0; u < numUsers; ++u )
		{
			tSessionUser & user = operator[]( u );
			if( !user.mIsZombie )
				indices[ userCount++ ] = user.mUser->fLocalHwIndex( );
			else
				++zombieCount;
		}

		if( zombieCount && !session.fLeaveZombie( zombieCount ) )
			return false;

		if( userCount && !session.fLeaveLocal( userCount, indices ) )
			return false;

		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSessionNetwork::tSessionUserArray::fJoinRemote( tGameSession & session )
	{
		const u32 userCount = fCount( );
		tPlatformUserId * users = ( tPlatformUserId * )alloca( sizeof( tPlatformUserId ) * userCount );
		b32 * invited = ( b32 * )alloca( sizeof( b32 ) * userCount );

		for( u32 u = 0; u < userCount; ++u )
		{
			sigassert( !operator[]( u ).mIsZombie && "Cannot do join remote zombie" );

			users[ u ] = operator[]( u ).mUser->fPlatformId( );
			invited[ u ] = operator[]( u ).mInvited;
		}

		return session.fJoinRemote( userCount, users, invited );
	}

	//------------------------------------------------------------------------------
	b32 tGameSessionNetwork::tSessionUserArray::fLeaveRemote( tGameSession & session )
	{
		const u32 userCount = fCount( );
		tPlatformUserId * users = ( tPlatformUserId * )alloca( sizeof( tPlatformUserId ) * userCount );

		for( u32 u = 0; u < userCount; ++u )
		{
			sigassert( !operator[]( u ).mIsZombie && "Cannot leave remote zombie" );
			users[ u ] = operator[]( u ).mUser->fPlatformId( );
		}

		return session.fLeaveRemote( userCount, users );
	}

	//------------------------------------------------------------------------------
	// tGameSessionNetwork::tConnectionArray
	//------------------------------------------------------------------------------
	b32 tGameSessionNetwork::tConnectionArray::fArbitrated( ) const
	{
		const u32 count = fCount( );
		for( u32 c = 0; c < count; ++c )
		{
			if( !operator[]( c ).mArbitrated )
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------
	// tGameSessionNetwork
	//------------------------------------------------------------------------------
	tGameSessionNetwork::tGameSessionNetwork( u32 gameChannelCount )
		: mChannelId( gameChannelCount )
		, mState( cStateInactive )
		, mOwner( NULL )
		, mInvited( false )
		, mMaxPeers( 15 )
#if defined( use_steam )
		, mLobbyDataRequested( false )
		, mOnLobbyDataUpdate( this, &tGameSessionNetwork::fOnLobbyDataUpdate )
#endif
	{
		mViewsWrittenTo.fFill( ~0 );
	}

	//------------------------------------------------------------------------------
	b32	tGameSessionNetwork::fIsReadyAndFull( ) const
	{
		if( !fIsReady( ) )
			return false;

		if( mSession->fFreeSlots( ) )
			return false;

		return true;
	}

	//------------------------------------------------------------------------------
	b32	tGameSessionNetwork::fIsReadyAndIdle( ) const
	{
		if( !fIsReady( ) )
			return false;

		if( mLocalToAdd.fCount( ) ||
			mLocalToRemove.fCount( ) ||
			mRemoteToAdd.fCount( ) ||
			mRemoteToRemove.fCount( ) )
			return false;

		return true;
	}

	//------------------------------------------------------------------------------
	b32	tGameSessionNetwork::fIsHost( ) const
	{
		return mHostUser == mOwnerUser;
	}

	//------------------------------------------------------------------------------
	b32	tGameSessionNetwork::fIsConnected( ) const
	{
		return mNetwork.fIsConnectedOrHosting( );
	}

	//------------------------------------------------------------------------------
	b32	tGameSessionNetwork::fIsOnline( ) const
	{
		if( mSession )
			return mSession->fIsOnline( );

		return false;
	}

	//------------------------------------------------------------------------------
	b32	tGameSessionNetwork::fIsRunningWithConnections( ) const
	{
		return fIsRunning( ) && fConnectionCount( );
	}

	//------------------------------------------------------------------------------
	b32	tGameSessionNetwork::fLongStateTime( f32 longTime ) const
	{
		if( !mSession )
			return false;

		if( mSession->fState( ) == tGameSession::cStateNull ||
			mSession->fState( ) == tGameSession::cStateCreated ||
			mSession->fState( ) == tGameSession::cStateStarted )
			return false;

		return mSession->fStateTimer( ) > longTime;
	}

	//------------------------------------------------------------------------------
	b32 tGameSessionNetwork::fIsSameSession( const tGameSessionInfo& info ) const
	{
		if( !mSession )
			return false;

		return mSession->fIsSameSession( info );
	}

	//------------------------------------------------------------------------------
	b32 tGameSessionNetwork::fHost( 
		tUser & user, 
		u32 sessionCreateFlags, 
		u32 publicSlots, 
		u32 privateSlots )
	{
		sigassert( mState == cStateInactive && !mSession );

		mHostUser.fReset( &user );
		mOwnerUser.fReset( &user );
		mInvited = true;

		// Create our host session
		mSession.fReset( NEW tGameSession( 
			sessionCreateFlags, 
			publicSlots, 
			privateSlots, 
			make_delegate_memfn( 
				tGameSession::tStateChangedCallback, 
				tGameSessionNetwork, 
				fHandleSessionStateChange ) ) );

		// Is this available to the public?
		if( mSession->fIsOnline( ) )
		{
			b32 success = mNetwork.fHost( 
				this, 
				mSession->fTotalSlots( ) - 1, 
				mChannelId + 1, 
				Net::tHost::cDefaultAddress,
				Net::tHost::cDefaultPort );

			if( !success )
			{
				mSession.fRelease( );
				return false;
			}
		}

		b32 success = mSession->fCreate( user.fLocalHwIndex( ) );
		if( !success )
		{
			mNetwork.fLeaveNow( );
			mSession.fRelease( );
			return false;
		}

		mState = cStateCreatingSession;
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSessionNetwork::fJoin( tUser & user, const tGameSessionSearchResult & result)
	{
		mInvited = false;
		return fJoin( user, result.fSessionInfo( ) );
	}


	//------------------------------------------------------------------------------
	b32 tGameSessionNetwork::fJoin( tUser & user, const tGameInvite & invite  )
	{
		mInvited = true;
		return fJoin( user, invite.fSessionInfo( ) );
	}

	//------------------------------------------------------------------------------
	b32	 tGameSessionNetwork::fJoin( tUser & user, const tGameSessionInfo & info )
	{
#if defined( use_steam )
		sigassert( ( ( mState == cStateInactive ) || ( mState == cStateRequestingLobbyData ) ) && !mSession );
#else
		sigassert( mState == cStateInactive && !mSession );
#endif

		mJoinInfo = info;

#if defined( use_steam )
		if ( mInvited )
		{
			auto host = tGameSession::fGetAddress( info );
			if ( !host )
			{
				if ( !mLobbyDataRequested )
				{
					// There was no known host address for the lobby, so we
					// request the lobby data from Steam and try this again
					// when the lobby data is returned.
					log_line( Log::cFlagNetwork, __FUNCTION__ " requesting lobby data for lobby " << info.mId.ConvertToUint64( ) );
					b32 connected = SteamMatchmaking( )->RequestLobbyData( info.mId );
					mLobbyDataRequested = true;
					mOwnerUser.fReset( &user );
					mState = cStateRequestingLobbyData;
					return connected;
				}
				else
				{
					log_warning( 0, "Failure to join game session network. Unknown game server for lobby " );
					mState = cStateInactive;
					return false;
				}
			}
			else if ( !mLobbyDataRequested )
			{
				mOwnerUser.fReset( &user );
			}
			mLobbyDataRequested = false;
		}
		else
		{
			mOwnerUser.fReset( &user );
		}
#else
		mOwnerUser.fReset( &user );
#endif

		mHostUser.fRelease( );
		tGameSession::fRegisterKey( info );

		// Join network and get session info
		b32 success = mNetwork.fJoin( 
			this, 
			mMaxPeers, 
			mChannelId + 1,
			tGameSession::fGetAddress( info ) );

		if( !success )
		{
			log_warning( 0, "Failure to join game session network" );

			tGameSession::fUnregisterKey( info );
			mOwnerUser.fRelease( );
			return false;
		}

		mState = cStateConnecting;
		return true;
	}

#if defined( use_steam )
	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fOnLobbyDataUpdate( LobbyDataUpdate_t* params )
	{
		if ( mLobbyDataRequested )
		{
			// Try the join again
			fJoin( *mOwnerUser, mJoinInfo );
		}
	}
#endif

	//------------------------------------------------------------------------------
	b32 tGameSessionNetwork::fAddLocalUser( tUser & user, b32 addAsZombie )
	{
		// Exists?
		if( !mSession )
		{
			log_warning( 0, "Cannot add local user to session when session does not exist" );
			return false;
		}

		// Join in progress disabled and beyond created state
		if( mSession->fState( ) >= tGameSession::cStateStarting && 
			( mSession->fCreateFlags( ) & tGameSession::cCreateJoinInProgressDisabled ) )
		{
			log_warning( 0, "Cannot add local user to session because session has disabled join in progress" );
			return false;
		}

		// Haven't removed it yet
		if( mLocalToRemove.fFindAndErase( &user ) )
			return true;

		// Already marked for add
		if( mLocalToAdd.fFind( &user ) )
			return true;

		// Already added
		if( tSessionUser * sUser = mLocalUsers.fFind( &user ) )
		{
			// Test if we're currently removing it and if so then add it to the add list again
			if( mState == cStateRemovingLocalUsers && mPendingAddOrRemove.fFind( &user ) )
				mLocalToAdd.fPushBack( *sUser );

			return true;
		}

		// New add so check the slots
		if( !fHasFreeSlots( 1 ) )
		{
			log_warning( 0, "Cannot add local user to session because there are no slots available" );
			return false;
		}

		// Success!
		mLocalToAdd.fPushBack( tSessionUser( user, true, addAsZombie ) );
		return true;
	}


	//------------------------------------------------------------------------------
	b32 tGameSessionNetwork::fRemoveLocalUser( tUser & user )
	{
		if( mOwnerUser == &user )
		{
			log_warning( 0, "Local user cannot be removed because it is the session owner" );
			return false;
		}

		// Haven't added it yet
		if( mLocalToAdd.fFindAndErase( &user ) )
			return true;

		// Already marked for removal
		if( mLocalToRemove.fFind( &user ) )
			return true;

		// Has added it
		tSessionUser * sUser = mLocalUsers.fFind( &user );

		// Adding it now
		if( !sUser && mState == cStateJoiningLocalUsers )
			sUser = mPendingAddOrRemove.fFind( &user );

		// We've got it
		if( sUser )
		{
			fKillQueuedWritesForUser( sUser->mUser->fPlatformId( ) );
			mLocalToRemove.fPushBack( *sUser );
			return true;
		}

		// We don't know about this user
		log_warning( 0, "Local user cannot be removed because it is unknown" );
		return false;
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fWriteStats( 
		tPlatformUserId userId, u32 writeCount, const tGameSessionViewProperties writes[] )
	{
		sigassert( mState == cStateRunning && "Can only write stats from running state" );

		if( !mSession )
		{
			log_warning( 0, "Attempt to write stats with no session" );
			return;
		}

		if( mSession->fState( ) < tGameSession::cStateStarted )
		{
			log_warning( 0, "Attempt to write stats to session that is not started" );
			return;
		}

		if( userId == tUser::cInvalidUserId )
		{
			log_warning( 0, "Attempt to write stats for invalid user" );
			return;
		}
		
		if( !writeCount )
			return;

		b32 userFound = false;

		// Test if the user is a local user
		const u32 localCount = mLocalUsers.fCount( );
		for( u32 u = 0; u < localCount; ++u )
		{
			if( mLocalUsers[ u ].mUser->fPlatformId( ) == userId )
			{
				userFound = true;
				break;
			}
		}

		// If the session is arbitrated stats writes for remote players are allowed and required
		// so if we haven't found it yet then search the remote players
		if( !userFound && ( mSession->fCreateFlags( ) & tGameSession::cCreateUsesArbitration ) )
		{
			const u32 remoteCount = mRemoteUsers.fCount( );
			for( u32 u = 0; u < remoteCount; ++u )
			{
				if( mRemoteUsers[ u ].mUser->fPlatformId( ) == userId )
				{
					userFound = true;
					break;
				}
			}
		}

		// User unknown
		if( !userFound )
		{
			log_warning( 0, "UserId not found in " << __FUNCTION__ );
			return;
		}

		if( mQueuedWrites.fNumItems( ) + writeCount > mQueuedWrites.fCapacity( ) )
			mQueuedWrites.fResize( ( mQueuedWrites.fCapacity( ) + writeCount ) * 2  );

		// Store the writes to the queue
		for( u32 w = 0; w < writeCount; ++w )
		{
			const tGameSessionViewProperties & view = writes[ w ];

			tStatsWritePtr ptr( NEW tStatsWrite( ) );

			// Setup the write
			ptr->mUserId = userId;
			ptr->mViewId = view.mViewId;
			ptr->mProperties.fInitialize( view.mProperties, view.mNumProperties );

			// Push the write
			mQueuedWrites.fPut( ptr );
		}
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fFlushStats( )
	{
		sigassert( mState == cStateRunning && "Can only flush stats from running state" );

		if( mQueuedWrites.fNumItems( ) + 1 > mQueuedWrites.fCapacity( ) )
			mQueuedWrites.fResize( ( mQueuedWrites.fCapacity( ) + 1 ) * 2 );

		tStatsWritePtr forceFlush( NEW tStatsWrite( ) );
		forceFlush->mUserId = tUser::cInvalidUserId; // invalid indicates flush command
		mQueuedWrites.fPut( forceFlush );
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fWait( )
	{
		if( mSession )
			mSession->fWait( );
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fTick( f32 dt )
	{
		mNetwork.fService( );

		if( mVoiceNetwork )
			mVoiceNetwork->fTick( dt );

		// Process dead sessions
		for( s32 ds = mDeadSessions.fCount( ) - 1; ds >= 0; --ds )
		{
			// If the session is laid to rest erase it
			if( fAdvanceDeadSession( *mDeadSessions[ ds ] ) )
				mDeadSessions.fErase( ds );
		}

		// Tick the session
		if( mSession )
			mSession->fTick( dt );
		
		// Check again, as the tick may have destroyed the session
		if( mSession )
		{
			// and is idling in the created state then we try to do our add/removes of users
			if( mSession->fIsCreated( ) )
				fProcessAddRemoveLists( );
			else if( mSession->fIsStarted( ) )
			{
				fProcessStatsWriteQueue( );

				if( mSession->fIsStarted( ) )
					fProcessAddRemoveLists( );
			}
		}
	}

	//------------------------------------------------------------------------------
	b32 tGameSessionNetwork::fStart( ) 
	{
		log_line( Log::cFlagSession, __FUNCTION__ );
		sigassert( !mLocalToAdd.fCount( ) && !mRemoteToAdd.fCount( ) );
		sigassert( !mLocalToRemove.fCount( ) && !mRemoteToRemove.fCount( ) );
		sigassert( !mQueuedWrites.fNumItems( ) );
		sigassert( mState == cStateReady );

		mViewsWrittenTo.fFill( ~0 );			

		if( mSession->fCreateFlags( ) & tGameSession::cCreateUsesArbitration )
		{
			if( !mSession->fIsHost( ) )
			{
				b32 success = mSession->fArbitrationRegister( );
				if( !success )
				{
					log_warning( 0, __FUNCTION__ << "Failed to begin arbitration registration" );
					return false;
				}
			}
		}
		else
		{
			b32 success = mSession->fStart( );
			if( !success )
			{
				log_warning( 0, __FUNCTION__ << "Failed to begin starting session" );
				return false;
			}
		}

		mState = cStateStarting;
		return true;
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fEnd( )
	{
		log_line( Log::cFlagSession, __FUNCTION__ );
		sigassert( mState == cStateRunning && "End called while not running" );
		
		// If we have no queued stat writes and the session is idle in the start phase
		// then begin the end phase
		if( !mQueuedWrites.fNumItems( ) && mSession->fIsStarted( ) )
			mSession->fEnd( );

		mState = cStateStopping;
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fDestroy( )
	{
		sigassert( ( mState == cStateRunning || mState == cStateReady ) && "End called while not running" );

		mNetwork.fLeave( );

		// If we're active we need to stop first
		if( mState == cStateRunning ) 
		{
			fEnd( );
		}

		// Then we must be in the ready state, in which case we can destroy
		else
		{
			mSession->fDelete( );
		}

		
		mState = cStateDestroying;
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fRestart( )
	{
		sigassert( mState == cStateRunning && "Restart called while not running" );

		if( !mQueuedWrites.fNumItems( ) && mSession->fIsStarted( ) )
			mSession->fEnd( );

		mState = cStateRestarting;
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fCancelAndLeave( )
	{
		// Unregister the key if we're in the connecting state
		if( mState == cStateConnecting )
			tGameSession::fUnregisterKey( mJoinInfo );

		mNetwork.fFlush( );
		// Kill the session
		fKillSession( true );

		mNetwork.fLeaveNow( );
		mState = cStateInactive;

		fClearData( );
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fSendMsgToPeers( 
		u32 channelId, const byte * data, u32 dataLength, b32 reliable )
	{
		sigassert( channelId < mChannelId && "Cannot send msg to peers on protocol channels" );
		mNetwork.fSendMsgToPeers( channelId, data, dataLength, reliable );
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fSendMsgToHost( 
		u32 channelId, const byte * data, u32 dataLength, b32 reliable )
	{
		sigassert( channelId < mChannelId && "Cannot send msg to host on protocol channels" );
		mNetwork.fSendMsgToHost( channelId, data, dataLength, reliable );
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fFlush( )
	{
		mNetwork.fFlush( );
	}

	//------------------------------------------------------------------------------
	b32 tGameSessionNetwork::fOnUserSignInChange(
			const tUserSigninInfo oldStates[], 
			const tUserSigninInfo newStates[] )
	{
		if( mState == cStateInactive )
			return true;

		// Owner user signed out entirely, so the session was silently deleted
		if( newStates[ mOwnerUser->fLocalHwIndex( ) ].mState == tUser::cSignInStateNotSignedIn )
		{
			mNetwork.fLeaveNow( );
			fClearData( );
			mState = cStateInactive;
			return false;
		}

		// Check for live connectivity loss
		else
		{
			const u32 localUserCount = mLocalUsers.fCount( );
			for( u32 u = 0; u < localUserCount; ++u )
			{
				const u32 hwIdx = mLocalUsers[ u ].mUser->fLocalHwIndex( );

				if( newStates[ hwIdx ].mState == tUser::cSignInStateNotSignedIn )
				{
					fRemoveLocalUser( *mLocalUsers[ u ].mUser );

					// Have to use the old user id since the id is now an invalid one
					fKillQueuedWritesForUser( oldStates[ hwIdx ].mUserId );
				}
				else if( oldStates[ hwIdx ].mState == tUser::cSignInStateSignedInOnline &&
					newStates[ hwIdx ].mState == tUser::cSignInStateSignedInLocally )
				{
					// Host can only close the handle since host deletes require
					// LIVE session calls
					if( fIsHost( ) )
					{
						fKillSession( false );

						mNetwork.fLeaveNow( );
						fClearData( );
						mState = cStateInactive;
						return false;
					}
					else
					{
						fCancelAndLeave( );
						return false;
					}
				}
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fOnMuteListChanged( )
	{
		if( tVoice::fInitialized( ) && mNetwork.fIsConnectedOrHosting( ) )
		{
			tVoiceMutePacket mutePacket;

			tVoice & voice = tVoice::fInstance( );
			voice.fUpdateSystemMutes( mutePacket.mMuteMsg );

			fSendPacketToPeers( mutePacket, mChannelId, mNetwork, true, false );
		}
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fClearData( )
	{
		if( tVoice::fInitialized( ) )
			tVoice::fInstance( ).fUnregisterAllRemote( );

		mLocalToAdd.fSetCount( 0 );
		mRemoteToAdd.fSetCount( 0 );
		mLocalToRemove.fSetCount( 0 );
		mRemoteToRemove.fSetCount( 0 );

		mPendingAddOrRemove.fSetCount( 0 );

		mLocalUsers.fSetCount( 0 );
		mRemoteUsers.fSetCount( 0 );
		mConnections.fSetCount( 0 );

		mVoiceNetwork.fRelease( );
		mSession.fRelease( );
		mHostUser.fRelease( );
		mOwnerUser.fRelease( );
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fKillSession( b32 canDelete )
	{
		// No session to kill
		if( !mSession )
			return;

		tGameSessionPtr session = mSession;
		mSession.fRelease( );

		// Clear the write queue
		mQueuedWrites.fReset( );

		// Change the state changed callback
		if( canDelete )
		{
			session->fSetStateChangedCallback(
				make_delegate_memfn( 
					tGameSession::tStateChangedCallback, 
					tGameSessionNetwork, 
					fHandleDeadSessionStateChange ) );
		}
		else
		{
			// Clear the callback if it can't be deleted
			session->fSetStateChangedCallback( tGameSession::tStateChangedCallback( ) );
		}

		// If the session needs time to die add it to the dead list
		if( !fAdvanceDeadSession( *session ) )
			mDeadSessions.fPushBack( session );
	}

	//------------------------------------------------------------------------------
	b32 tGameSessionNetwork::fAdvanceDeadSession( tGameSession & gameSession )
	{
		// Advance the state
		gameSession.fTick( 0.f );

		if( gameSession.fIsNull( ) )
			return true;
		else if( gameSession.fIsCreated( ) || gameSession.fIsStarted( ) )
		{
			// Try to remove any local users
			u32 localUserCount; 
			if( gameSession.fLeavelAllLocal( localUserCount ) )
				return false;

			if( localUserCount )
				log_warning( 0, "Failed to leave local users, presence conflicts may persist" );

			// Must be a valid shutdown
			if( gameSession.fHasStateChangedCallback( ) )
				gameSession.fDelete( );

			// We set no callback on sessions that are already dead due
			// to user sign in status changes, that means once they
			// enter an idle state then the local users have been removed
			// and the session can be deleted
			else
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fProcessAddRemoveLists( )
	{
		// We can't add/remove users until the old sessions are gone
		if( mDeadSessions.fCount( ) )
			return;

		sigassert( !mPendingAddOrRemove.fCount( ) );

		if( mLocalToRemove.fCount( ) && mLocalToRemove.fLeaveLocal( *mSession ) )
		{
			// Move the users, set the state
			mPendingAddOrRemove.fJoin( mLocalToRemove );
			mLocalToRemove.fSetCount( 0 );
			mState = cStateRemovingLocalUsers;

			// If we're not actually removing local users then we likely only removed
			// zombies so we need to falsify the state change here
			if( mSession->fState( ) != tGameSession::cStateRemovingLocalUsers )
				fHandleSessionRemovingLocalUsers( true );
		}
		else if( mRemoteToRemove.fCount( ) && mRemoteToRemove.fLeaveRemote( *mSession ) )
		{
			// Move the users, set the state
			mPendingAddOrRemove.fJoin( mRemoteToRemove );
			mRemoteToRemove.fSetCount( 0 );
			mState = cStateRemovingRemoteUsers;
		}
		else if( mLocalToAdd.fCount( ) && mLocalToAdd.fJoinLocal( *mSession ) )
		{
			// Move the users, set the state
			mPendingAddOrRemove.fJoin( mLocalToAdd );
			mLocalToAdd.fSetCount( 0 );
			mState = cStateJoiningLocalUsers;

			// If we're not actually joining local users then we likely only added
			// zombies so we need to falsify the state change here
			if( mSession->fState( ) != tGameSession::cStateJoiningLocalUsers )
				fHandleSessionJoiningLocalUsers( true );
		}
		else if( mRemoteToAdd.fCount( ) && mRemoteToAdd.fJoinRemote( *mSession ) )
		{
			// Move the users, set the state
			mPendingAddOrRemove.fJoin( mRemoteToAdd );
			mRemoteToAdd.fSetCount( 0 );
			mState = cStateJoiningRemoteUsers;
		}
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fProcessStatsWriteQueue( )
	{
		tGrowableArray< tStatsWritePtr > toWrite;
		tPlatformUserId writeUserId = tUser::cInvalidUserId;

#if defined( use_steam )
		if( tGameSession::fIsWriteActive( ) )
			return;
#endif
		while( mQueuedWrites.fNumItems( ) )
		{
			tStatsWritePtr ptr = mQueuedWrites.fBack( );
			
			// A user was removed
			if( ptr.fNull( ) )
			{
				mQueuedWrites.fGet( );
				continue;
			}

			if( writeUserId == tUser::cInvalidUserId )
				writeUserId = ptr->mUserId;

			// We can only do writes for one user at a time
			if( writeUserId != ptr->mUserId )
				break;

			// Check if we need to do a flush before we can do the write
			b32 canWrite = false;

			// Special flush commands have invalid user id
			if( writeUserId != tUser::cInvalidUserId )
			{
				for( u32 v = 0; v < mViewsWrittenTo.fCount( ); ++v )
				{
					// Set the view if the array isn't full
					if( mViewsWrittenTo[ v ] == ~0 )
						mViewsWrittenTo[ v ] = ptr->mViewId;

					// The view is valid
					if( mViewsWrittenTo[ v ] == ptr->mViewId )
					{
						canWrite = true;
						break;
					}
				}
			}
			else if( !toWrite.fCount( ) )
			{
				// Remove the special flush command because we're going to flush below
				mQueuedWrites.fGet( );
			}

			// If we can't write this one 
			if( !canWrite )
			{
				// and we didn't validate any before it, then we need to do a flush
				if( !toWrite.fCount( ) )
				{
					if( mSession->fFlushStats( ) )
						mViewsWrittenTo.fFill( ~0 );
					else
						log_warning( 0, "Failed to flush stats" );
					return;
				}

				// otherwise we can just break out and write those we can
				else
					break;
			}

			// Remove it from the queue because the write will occur
			mQueuedWrites.fGet( );
			toWrite.fPushBack( ptr );
		}

		const u32 toWriteCount = toWrite.fCount( );

		if( !toWriteCount )
			return;

		tGameSessionViewProperties * properties = 
			(tGameSessionViewProperties *)alloca( sizeof(tGameSessionViewProperties) * toWriteCount );

		for( u32 w = 0; w < toWriteCount; ++w )
		{
			const tStatsWritePtr & ptr = toWrite[ w ];
			properties[ w ].mViewId = ptr->mViewId;
			properties[ w ].mNumProperties = ptr->mProperties.fCount( );
			properties[ w ].mProperties = ptr->mProperties.fBegin( );
		}

		if( !mSession->fWriteStats( writeUserId, toWriteCount, properties ) )
		{
			log_warning( 0, "Stats write failed to start, requeueing" );
			for( u32 w = 0; w < toWriteCount; ++w )
				mQueuedWrites.fPut( toWrite[ w ] );
		}
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fKillQueuedWritesForUser( tPlatformUserId userId )
	{
		for( u32 w = 0; w < mQueuedWrites.fNumItems( ); ++w )
		{
			if( mQueuedWrites[ w ]->mUserId == userId )
				mQueuedWrites[ w ].fRelease( );
		}
	}

	//------------------------------------------------------------------------------
	s32 tGameSessionNetwork::fActiveSlots( ) const
	{
		return	( mLocalToAdd.fCount( ) + mRemoteToAdd.fCount( ) ) - 
				( mRemoteToRemove.fCount( ) + mLocalToRemove.fCount( ) );
	}

	//------------------------------------------------------------------------------
	b32 tGameSessionNetwork::fHasFreeSlots( u32 count ) const
	{
		sigassert( mSession && "Cannot determine free slot count without active session" );
		return mSession->fFreeSlots( ) - fActiveSlots( ) >= count;
	}

	//------------------------------------------------------------------------------
	b32 tGameSessionNetwork::fHasFreePublicSlots( u32 count ) const
	{
		sigassert( mSession && "Cannot determine free slot count without active session" );
		u32 freePublic = mSession->fFreePublicSlots( );
		u32 freePrivate = mSession->fFreePrivateSlots( );

		// Are we short without counting active slots?
		if( freePublic < count )
			return false;

		// Adjust for active locals
		const u32 localToAddCount = mLocalToAdd.fCount( );
		for( u32 u = 0; u < localToAddCount; ++u )
		{
			if( mLocalToAdd[ u ].mInvited && freePrivate )
				--freePrivate;
			else
			{
				if( --freePublic < count )
					return false;
			}
		}

		// Adjust for active remotes
		const u32 remoteToAddCount = mRemoteToAdd.fCount( );
		for( u32 u = 0; u < remoteToAddCount; ++u )
		{
			if( mRemoteToAdd[ u ].mInvited && freePrivate )
				--freePrivate;
			else
			{
				if( --freePublic < count )
					return false;
			}
		}

		// Do we still have enough?
		return freePublic >= count;
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fHandleDeadSessionStateChange( 
		tGameSession & gameSession, u32 oldState, b32 success )
	{
		if( gameSession.fIsStarted( ) || gameSession.fIsCreated( ) )
		{
			// Try to remove all the local users
			u32 localUserCount;
			if( gameSession.fLeavelAllLocal( localUserCount ) )
				return;

			if( localUserCount )
				log_warning( 0, "Failed to leave local users, presence conflicts may persist" );

			// Delete the session
			gameSession.fDelete( );
		}
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fHandleSessionStateChange( 
		tGameSession& gameSession, u32 oldState, b32 success )
	{
		sigassert( &gameSession == mSession.fGetRawPtr( ) && "Sanity: Invalid session" );

		switch( oldState )
		{
		case tGameSession::cStateEnding:
			fHandleSessionEnding( success );
			break;
		case tGameSession::cStateCreating:
			fHandleSessionCreating( success );
			break;
		case tGameSession::cStateJoiningLocalUsers:
			fHandleSessionJoiningLocalUsers( success );
			break;
		case tGameSession::cStateJoiningRemoteUsers:
			fHandleSessionJoiningRemoteUsers( success );
			break;
		case tGameSession::cStateRemovingLocalUsers:
			fHandleSessionRemovingLocalUsers( success );
			break;
		case tGameSession::cStateRemovingRemoteUsers:
			fHandleSessionRemovingRemoteUsers( success );
			break;
		case tGameSession::cStateStarting:
			fHandleSessionStarting( success );			
			break;
		case tGameSession::cStateRegistering:
			fHandleSessionRegistering( success );
			break;
		case tGameSession::cStateWritingStats:
			fHandleSessionWritingStats( success );
			break;
		case tGameSession::cStateFlushingStats:
			fHandleSessionFlushingStats( success );
			break;
		case tGameSession::cStateCreated:
			if( gameSession.fState( ) != tGameSession::cStateNull )
				break;
		case tGameSession::cStateDeleting:
			fHandleSessionDeleting( success );
			break;
		}
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fHandleSessionCreating( b32 success )
	{
		if( success )
		{
#if defined( use_steam )
			if( mOwner && !mOwner->fIsCompatible( ) )
			{
				// for some reason the two games are incompatible (eg different game versions)
				// so we can't join the session, abort.
				mState = cStateInactive;
				mNetwork.fLeaveNow( );
				mSession.fRelease( );

				if( mOwner )
					mOwner->fOnError( cStateRequestingLobbyData );
			}
			else
#endif // defined( use_steam )
			{
				// Create the voice network
				if( mSession->fTotalSlots( ) > 1 )
					mVoiceNetwork.fReset( NEW Net::tVoiceNetwork( ) );

				mLocalToAdd.fInsert( 0, tSessionUser( *mOwnerUser, true ) );
			}
		}
		else
		{
			mState = cStateInactive;
			mNetwork.fLeaveNow( );
			mSession.fRelease( );

			if( mOwner )
				mOwner->fOnError( cStateCreatingSession );
		}
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fHandleSessionEnding( b32 success )
	{
		mViewsWrittenTo.fFill( ~0 ); 

		if( success )
		{
			// We had an error pushing stats
			if( mOwner && mSession->fLastError( ) )
				mOwner->fOnStatsLost( );

			if( mState == cStateStopping )
			{
				if( mOwner )
					mOwner->fOnStopped( );

				mState = cStateReady;
			}
			else if( mState == cStateDestroying )
				mSession->fDelete( );
			else if( mState == cStateRestarting )
				mSession->fStart( );
		}
		else
		{
			log_warning( 0, "Failed to end session from state " << mState );
		}
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fHandleSessionJoiningLocalUsers( b32 success )
	{
		b32 wasOwner = false;
		const u32 userCount = mPendingAddOrRemove.fCount( );
		for( u32 u = 0; u < userCount; ++u )
		{
			if( mPendingAddOrRemove[ u ].mUser == mOwnerUser )
			{
				wasOwner = true;
				break;
			}
		}

		if( success )
		{
			mLocalUsers.fJoin( mPendingAddOrRemove );
			mPendingAddOrRemove.fSetCount( 0 );

			mState = cStateReady;
			if( mOwner )
			{
				mOwner->fOnUsersChanged( );
				if( wasOwner )
					mOwner->fOnReady( );
				
				// Test if the session is full and alert the owner
				if( !mSession->fFreeSlots( ) )
					mOwner->fOnSessionFull( );
			}
		}
		else 
		{
			if( wasOwner )
				fCancelAndLeave( );
			else
				mState = cStateReady;

			mPendingAddOrRemove.fSetCount( 0 );

			if( mOwner )
				mOwner->fOnError( cStateJoiningLocalUsers );
		}
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fHandleSessionJoiningRemoteUsers( b32 success )
	{
		if( success )
		{
			mState = cStateReady;

			// Append the newly added users to the remote list
			mRemoteUsers.fJoin( mPendingAddOrRemove );

			// Register the remote users with the voice system
			const u32 userCount = mPendingAddOrRemove.fCount( );
			for( u32 u = 0; u < userCount; ++u )
			{
				tConnection * connection = mConnections.fFind( mPendingAddOrRemove[ u ].mUser.fGetRawPtr( ) );
				if( !connection )
				{
					log_warning( 0, "Remote user has disconnected after joining session" );
					continue;
				}

				// Add the connection to the voice network
				if( !connection->mCanHear )
				{
					mVoiceNetwork->fAddPeer( 
						connection->mPeer.fAddressHost( ), 
						connection->mPeer.fAddressPort( ) + 1
					);

					connection->mCanHear = true;
				}

				// Add the voice to the voice system
				if( tVoice::fInitialized( ) )
				{
					tVoice::fInstance( ).fRegisterRemote(  
						mPendingAddOrRemove[ u ].mUser->fPlatformId( ) );

					// If we've stored a mute msg for this connection then 
					// try to process it now
					if( connection->mMuteMsg.fCount( ) )
					{
						// If any of the connections users are not added yet, then
						// we have to wait to apply the mute msg until they are
						b32 allAdded = true;
						const u32 userCount = connection->mUsers.fCount( ); 
						for( u32 u = 0; u < userCount; ++u )
						{
							if( !mRemoteUsers.fFind( connection->mUsers[ u ].mUser.fGetRawPtr( ) ) )
							{
								allAdded = false;
								break;
							}
						}

						if( allAdded )
						{
							tVoice::fInstance( ).fHandleMuteMsg( 
								connection->mMuteMsg.fBegin( ), 
								connection->mMuteMsg.fCount( ) );

							connection->mMuteMsg.fDeleteArray( );
						}
					}

					// Ship out our mute list for new voice actors
					fOnMuteListChanged( );
				}
			}

			mPendingAddOrRemove.fSetCount( 0 );

			if( mOwner )
				mOwner->fOnUsersChanged( );

			// Test if the session is full and alert the owner
			if( !mSession->fFreeSlots( ) )
			{
				if( mOwner )
					mOwner->fOnSessionFull( );
			}
		}
		else
		{
			mState = cStateReady;

			// Add the remote users back to the weak list
			mPendingAddOrRemove.fSetCount( 0 );

			if( mOwner )
				mOwner->fOnError( cStateJoiningRemoteUsers );
		}
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fHandleSessionRemovingLocalUsers( b32 success )
	{
		mState = mSession->fIsStarted( ) ? cStateRunning : cStateReady;
		if( success )
		{
			const u32 userCount = mPendingAddOrRemove.fCount( );
			for( u32 u = 0; u < userCount; ++u )
				mLocalUsers.fFindAndErase( mPendingAddOrRemove[ u ].mUser.fGetRawPtr( ) );

			mPendingAddOrRemove.fSetCount( 0 );

			if( mOwner )
				mOwner->fOnUsersChanged( );
		}
		else
		{
			mPendingAddOrRemove.fSetCount( 0 );

			if( mOwner )
				mOwner->fOnError( cStateRemovingLocalUsers );
		}
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fHandleSessionRemovingRemoteUsers( b32 success )
	{
		//sigassert( 0 && "Removing remote users should not be happening..." );

		mState = mSession->fIsStarted( ) ? cStateRunning : cStateReady;
		if( success )
		{
			if( tVoice::fInitialized( ) )
			{
				const u32 userCount = mPendingAddOrRemove.fCount( );
				for( u32 u = 0; u < userCount; ++u )
					tVoice::fInstance( ).fUnregisterRemote( mPendingAddOrRemove[ u ].mUser->fPlatformId( ) );
			}

			// Remove from the valid list
			const u32 userCount = mPendingAddOrRemove.fCount( );
			for( u32 u = 0; u < userCount; ++u )
				mRemoteUsers.fFindAndErase( mPendingAddOrRemove[ u ].mUser.fGetRawPtr( ) );

			mPendingAddOrRemove.fSetCount( 0 );
			if( mOwner )
				mOwner->fOnUsersChanged( );
		}
		else
		{
			mPendingAddOrRemove.fSetCount( 0 );

			if( mOwner )
				mOwner->fOnError( cStateRemovingRemoteUsers );
		}
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fHandleSessionStarting( b32 success )
	{
		if( success )
		{
			mState = cStateRunning;
			if( mOwner )
				mOwner->fOnRunning( );
		}
		else
		{
			tState lastState = mState;
			mState = cStateReady;
			if( mOwner )
				mOwner->fOnError( lastState );
		}
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fHandleSessionRegistering( b32 success )
	{
		if( success )
		{
			if( !mSession->fIsHost( ) )
			{
				tArbitratedPacket msg;
				fSendPacket( msg, mChannelId, *mNetwork.fHostPeer( ), true, false );
			}
			else
			{
				// Now we've registered, check the results
				tGrowableArray< tPlatformUserId > users;
				b32 success = mSession->fGetArbitrationResults( users );
				sigassert( success && "Could not access arbitration results" );

				// TODO: Expand error handling here
				success = mSession->fStart( );
				if( !success )
				{
					log_warning( 0, __FUNCTION__ << ": Failed to begin starting session" );
					mState = cStateReady;
					if( mOwner )
						mOwner->fOnError( cStateStarting );
				}

				tArbitratedPacket msg;
				fSendPacketToPeers( msg, mChannelId, mNetwork, true, false );
			}
		}
		else
		{
			mState = cStateReady;
			if( mOwner )
				mOwner->fOnError( cStateStarting );
		}
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fHandleSessionDeleting( b32 success )
	{
		sigassert( success );

		mState = cStateInactive;
		fClearData( );

		if( mOwner )
			mOwner->fOnDestroyed( );
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fHandleSessionWritingStats( b32 success )
	{
		// N.B. This function is called from fHandleSessionFlushingStats, so if the
		// behavior doesn't remain consistent for these two states then update that
		// function to behave properly

		sigassert( mSession->fIsStarted( ) );
		if( mState == cStateDestroying || mState == cStateStopping || mState == cStateRestarting )
		{
			fProcessStatsWriteQueue( );

			// After processing the write queue, if the session is still in the
			// started state then we can begin the end phase
			if( mSession->fIsStarted( ) )
				mSession->fEnd( );
		}
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fHandleSessionFlushingStats( b32 success )
	{
		fHandleSessionWritingStats( success );

		if( !success && mOwner )
			mOwner->fOnStatsLost( );
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fHandleJoinRequestPacket( 
		const byte * data, 
		u32 dataLength,
		const Net::tPeer & peer )
	{
		log_line( Log::cFlagSession, __FUNCTION__ );
		if( !mSession || !mSession->fIsHosting( ) )
		{
			log_warning( 0, "Join request received while not hosting" );
			return;
		}

		tJoinRequestPacket request;
		fDecodePacket( request, data, dataLength );

		// TODO: Check for this user already being added

		// Not enough free slots to hold the peer
		if( !fHasFreeSlots( 1 ) || ( !request.mInvited && !fHasFreePublicSlots( 1 ) ) )
		{
			log_line( 0, "Rejecting player because session was full" );

			// Try to be friendly by sending a join rejected packet
			// but if the peer doesn't get it, no big deal they'll time-out
			// or get our disconnection notice
			tJoinRejectedPacket packet;
			packet.mReason = tJoinRejectedPacket::cReasonSessionFull;

			// Flush the host now because we're gonna disconnect 
			// without waiting for outgoing message
			fSendPacket( packet, mChannelId, peer, false, true );
			mNetwork.fRejectPeer( peer );
			return;
		}

		// We're good to go
		tJoinAcceptedPacket packet;
		packet.mSessionNonce = mSession->fNonce( );
		packet.mSessionCreateFlags = mSession->fCreateFlags( );
		packet.mSessionGameMode = mSession->fDetails( ).mGameMode;
		packet.mSessionGameType = mSession->fDetails( ).mGameType;
		packet.mTotalPrivateSlots = mSession->fTotalPrivateSlots( );
		packet.mTotalPublicSlots = mSession->fTotalPublicSlots( );

		packet.mUserInfo = tUserInfo( mHostUser->fUserInfo( ) );
		fSendPacket( packet, mChannelId, peer, true );

		tUserPtr newUser( NEW tUser( request.mUserInfo ) );

		// Build the connection
		mConnections.fPushBack( tConnection( ) );
		mConnections.fBack( ).mPeer = peer;
		mConnections.fBack( ).mUsers.fPushBack( tSessionUser( *newUser, request.mInvited ) );

		// Add the user
		mRemoteToAdd.fPushBack( mConnections.fBack( ).mUsers[ 0 ] );
		mNetwork.fAcceptPeer( peer );
	}
 
	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fHandleJoinAcceptedPacket(
		const byte * data, 
		u32 dataLength,
		const Net::tPeer & peer )
	{
		log_line( Log::cFlagSession, __FUNCTION__ );
		if( mState != cStateConnecting )
		{
			log_warning( 0, "Join accepted packet received on non-connecting game session network" );
			return;
		}

		if( !mNetwork.fIsConnected( ) )
		{
			log_warning( 0, "Join accepted packet received on non-connected p2p network" );
			return;
		}

		tJoinAcceptedPacket accepted;
		fDecodePacket( accepted, data, dataLength );

		// Sweet, we got in!
		mHostUser.fReset( NEW tUser( accepted.mUserInfo ) );

		// Build the connection
		mConnections.fPushBack( tConnection( ) );
		mConnections.fBack( ).mPeer = peer;
		mConnections.fBack( ).mUsers.fPushBack( tSessionUser( *mHostUser, true ) );

		// Add the host as a weak user so it gets added to the session
		mRemoteToAdd.fPushBack( mConnections.fBack( ).mUsers[ 0 ] );

		// Set the user contexts for the session
		mOwnerUser->fSetContext( tUser::cUserContextGameMode, accepted.mSessionGameMode );
		mOwnerUser->fSetContext( tUser::cUserContextGameType, accepted.mSessionGameType );

		// Create the session
		mSession.fReset( NEW tGameSession( 
			accepted.mSessionCreateFlags, 
			mJoinInfo, 
			accepted.mTotalPublicSlots,
			accepted.mTotalPrivateSlots,
			make_delegate_memfn( 
				tGameSession::tStateChangedCallback, 
				tGameSessionNetwork, 
				fHandleSessionStateChange ) ) );

		mSession->fSetNonce( accepted.mSessionNonce );

		// Start creating the session
		b32 success = mSession->fCreate( mOwnerUser->fLocalHwIndex( ) );
		if( !success )
		{
			log_warning( 0, "Failed to start creating session" );
			mState = cStateInactive;
			mSession.fRelease( );
			mNetwork.fLeaveNow( );
			return;
		}

		// Good to go
		mState = cStateCreatingSession;
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fHandleJoinRejectedPacket(
		const byte * data, 
		u32 dataLength,
		const Net::tPeer & peer )
	{
		log_line( Log::cFlagSession, __FUNCTION__ );
		if( mState != cStateConnecting )
		{
			log_warning( 0, "Join Rejected packet recieved on non-connecting game session network" );
			return;
		}

		if( !mNetwork.fIsConnected( ) )
		{
			log_warning( 0, "Join rejected packet recieved on non-connected p2p network" );
			return;
		}

		tJoinRejectedPacket rejected;
		fDecodePacket( rejected, data, dataLength );

		std::string reason;
		switch( rejected.mReason )
		{
		case tJoinRejectedPacket::cReasonSessionFull:
			reason = "Session was full";
			break;
		default:
			reason = "Unknown";
			break;
		}

		log_line( 0, "Session request was rejected: " << reason );
		tGameSession::fUnregisterKey( mJoinInfo );
		mState = cStateInactive;
		if( mOwner )
			mOwner->fOnRejected( );
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fHandlePeerInfoPacket(
		const byte * data, 
		u32 dataLength,
		const Net::tPeer & peer )
	{
		log_line( Log::cFlagSession, __FUNCTION__ );
		if( !mNetwork.fIsConnectedOrHosting( ) || !mSession->fIsCreated( ) )
			return;

		tPeerInfoPacket peerInfo;
		fDecodePacket( peerInfo, data, dataLength );

		log_warning_unimplemented( 0 );
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fHandleArbitratedPacket( 
		const byte * data, 
		u32 dataLength, 
		const Net::tPeer & peer )
	{
		log_line( Log::cFlagSession, __FUNCTION__ );
		if( mState != cStateStarting )
		{
			log_warning( 0, "Arbitration packets should only be received while in Starting state" );
			return;
		}
		
		tArbitratedPacket arbitrated;
		fDecodePacket( arbitrated, data, dataLength );

		tConnection * connection = mConnections.fFind( peer );
		if( !connection )
		{
			log_warning( 0, "Received arbitration msg from unknown connection" );
			return;
		}

		if( connection->mArbitrated )
		{
			log_warning( 0, "Received duplicate arbitration msg" );
			return;
		}

		// Message from client that they've finished arbitration
		if( mSession->fIsHost( ) )
		{
			connection->mArbitrated = true;
			
			if( mConnections.fArbitrated( ) )
			{
				if( !mSession->fArbitrationRegister( ) )
				{
					log_warning( 0, "Failed to begin arbitration" );
					mState = cStateReady;
					if( mOwner )
						mOwner->fOnError( cStateStarting );
				}
			}
		}

		// Message from host that arbitration is completed and we should start the session
		else if( peer == *mNetwork.fHostPeer( ) )
		{
			if( !mSession->fStart( ) )
			{
				log_warning( 0, __FUNCTION__ << ": Failed to begin starting session" );
				mState = cStateReady;
				if( mOwner )
					mOwner->fOnError( cStateStarting );
			}
		}
		
		// Who is this?
		else
		{
			log_warning( 0, "Received arbitration packet from non-host" );
		}
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fHandleVoiceMutePacket( 
		const byte * data, u32 dataLength, const Net::tPeer & peer )
	{
		log_line( Log::cFlagSession, __FUNCTION__ );
		tVoiceMutePacket mutePacket;
		fDecodePacket( mutePacket, data, dataLength );

		tConnection * connection = mConnections.fFind( peer );
		if( !connection )
		{
			log_warning( 0, "Received voice mute msg from unknown connection" );
			return;
		}

		if( tVoice::fInitialized( ) )
		{
			// If any of the connections users are not added yet, then
			// we have to wait to apply the mute msg until they are
			const u32 userCount = connection->mUsers.fCount( ); 
			for( u32 u = 0; u < userCount; ++u )
			{
				if( !mRemoteUsers.fFind( connection->mUsers[ u ].mUser.fGetRawPtr( ) ) )
				{
					connection->mMuteMsg.fInitialize( 
						mutePacket.mMuteMsg.fBegin( ),
						mutePacket.mMuteMsg.fCount( ) );
					break;
				}
			}

			// Process the message if they were all added
			if( !connection->mMuteMsg.fCount( ) )
				tVoice::fInstance( ).fHandleMuteMsg( mutePacket.mMuteMsg.fBegin( ), mutePacket.mMuteMsg.fCount( ) );
		}
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fOnReceive( 
		Net::tP2PNetwork & network, 
		const Net::tPeer & peer, 
		u32 channelId,
		const byte * data, 
		u32 dataLength )
	{
		// Is it on a game channel?
		if( channelId < mChannelId )
		{
			if( mOwner )
			{
				tConnection * connection = mConnections.fFind( peer );
				if( connection )
					mOwner->fOnGameMessage( peer, channelId, data, dataLength );
				else
					log_warning( 0, "Msg received from unknown connection on game channel: " << channelId );
			}
			else
			{
				log_warning( 0, "Game message ignored because no owner was set on tGameSessionNetwork" );
			}

			return;
		}

		// Ok, it should be one of our packets then
		tPacketHeader header; 
		fDecodePacket( header, data, dataLength );

		switch( header.mPacketType )
		{
		case cPacketTypeJoinRequest:
			fHandleJoinRequestPacket( data, dataLength, peer );
			break;
		case cPacketTypeJoinAccepted:
			fHandleJoinAcceptedPacket( data, dataLength, peer );
			break;
		case cPacketTypeJoinRejected:
			fHandleJoinRejectedPacket( data, dataLength, peer );
			break;
		case cPacketTypePeerInfo:
			fHandlePeerInfoPacket( data, dataLength, peer );
			break;
		case cPacketTypeArbitrated:
			fHandleArbitratedPacket( data, dataLength, peer );
			break;
		case cPacketTypeVoiceMute:
			fHandleVoiceMutePacket( data, dataLength, peer );
			break;
		}
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fOnConnect( Net::tP2PNetwork & network, const Net::tPeer & peer )
	{
		log_line( Log::cFlagSession, __FUNCTION__ );
		switch( mState )
		{
		case cStateConnecting:
			{
				// Our host has connected
				sigassert( peer == *network.fHostPeer( ) && "Connection to non-host in connecting state" );

				// It's the server peer who hasn't accepted our connection yet
				// so we should ask for it
				
				// TODO: Allow more than one user
				tJoinRequestPacket packet( mOwnerUser->fUserInfo( ), mInvited );
				fSendPacket( packet, mChannelId, peer, true );

			} break;

		default:
			// Ship out our mute list to any new peers
			fOnMuteListChanged( );
			break;
		}
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fOnDisconnect( Net::tP2PNetwork & network, const Net::tPeer & peer, u32 reason )
	{
		log_line( Log::cFlagSession, __FUNCTION__ );
		if( tConnection * connection = mConnections.fFind( peer ) )
		{
			// Label for removal
			mRemoteToRemove.fJoin( connection->mUsers );
			mConnections.fErase( fPtrDiff( connection, mConnections.fBegin( ) ) );

			if( mOwner )
				mOwner->fOnDisconnect( );
		}
		else if( mState == cStateConnecting )
		{
			tGameSession::fUnregisterKey( mJoinInfo );

			if( mOwner )
			{
				if( reason == Net::tP2PNetwork::cDisconnectReasonRejected )
					mOwner->fOnRejected( );
				else
					mOwner->fOnDisconnect( );
			}
		}
		else
			log_warning( 0, "Disconnect notification from unknown peer" );
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fOnJoinFailure( Net::tP2PNetwork & network )
	{
		log_line( Log::cFlagSession, __FUNCTION__ );
		tGameSession::fUnregisterKey( mJoinInfo );
		mState = cStateInactive;

		if( mOwner )
			mOwner->fOnError( cStateConnecting );
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fOnConnectAttempt( Net::tP2PNetwork & network, const Net::tPeer & peer )
	{
		// Do nothing, because we can only accept after receiving a join request
	}
}
