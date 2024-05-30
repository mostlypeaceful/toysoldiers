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
#include "Net/NetConfig.hpp"
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
			cPacketTypeHostMigrated,

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
			Net::tNatType			mNatType;

			tJoinRequestPacket( )
				: tPacketHeader( cPacketTypeJoinRequest )
				, mInvited( false )
				, mNatType( Net::cNatTypeInvalid )
			{ }

			explicit tJoinRequestPacket( const tUserInfo& user, u32 natType, b32 invited )
				: tPacketHeader( cPacketTypeJoinRequest )
				, mUserInfo( user )
				, mInvited( invited )
				, mNatType( natType )
			{ }

			template<class tArchive>
			void fSaveLoad( tArchive& archive )
			{
				tPacketHeader::fSaveLoad( archive );
				archive.fSaveLoad( mUserInfo );
				archive.fSaveLoad( mInvited );
				archive.fSaveLoad( mNatType );
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

			Net::tNatType mHostNatType;

			tUserInfo mHostUserInfo;

			tJoinAcceptedPacket( )
				: tPacketHeader( cPacketTypeJoinAccepted )
				, mSessionNonce( 0 )
				, mHostNatType( Net::cNatTypeInvalid )
			{ }

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
				archive.fSaveLoad( mHostNatType );
				archive.fSaveLoad( mHostUserInfo );
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
				cReasonIncompatibleNatType,
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
			tUserInfo		mUserInfo;
			b32				mInvited;
			b32				mLeaving;
			Net::tNatType	mNatType;

			tPeerInfoPacket( )
				: tPacketHeader( cPacketTypePeerInfo )
				, mLeaving( false )
				, mNatType( Net::cNatTypeInvalid )
			{ }

			template<class tArchive>
			void fSaveLoad( tArchive& archive )
			{
				tPacketHeader::fSaveLoad( archive );
				archive.fSaveLoad( mUserInfo );
				archive.fSaveLoad( mInvited );
				archive.fSaveLoad( mLeaving );
				archive.fSaveLoad( mNatType );
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

		///
		/// \class tHostMigratedPacket
		/// \brief Sent by the new host after the session has migrated
		struct tHostMigratedPacket : public tPacketHeader
		{
			tHostMigratedPacket( ) : tPacketHeader( cPacketTypeHostMigrated ) { }

			tDynamicBuffer mSessionInfoBuffer;

			void fSetSessionInfo( const tGameSessionInfo& info )
			{
				mSessionInfoBuffer.fResize( sizeof( info ) );
				fMemCpy( mSessionInfoBuffer.fBegin( ), &info, sizeof( info ) );
			}

			tGameSessionInfo fGetSessionInfo( ) const
			{
				sigassert( mSessionInfoBuffer.fCount( ) == sizeof( tGameSessionInfo ) );
				const tGameSessionInfo* info = ( const tGameSessionInfo* )mSessionInfoBuffer.fBegin( );
				return *info;
			}

			template< class tArchive >
			void fSaveLoad( tArchive & archive )
			{
				tPacketHeader::fSaveLoad( archive );
				archive.fSaveLoad( mSessionInfoBuffer );
			}
		};
	}

	//------------------------------------------------------------------------------
	template<class t>
	static void fDecodePacket( t & out, const byte * data, u32 dataLength )
	{
		tGameArchiveLoad archive( data, dataLength );
		archive.fLoad( out );
	}

	//------------------------------------------------------------------------------
	template<class t>
	static void fSendPacket( t & packet, u32 channelId, const Net::tPeer & peer, b32 reliable, b32 flush = false )
	{
		tGameArchiveSave archive;
		archive.fSave( packet );
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
		archive.fSave( packet );
		network.fSendMsgToPeers( channelId, archive.fBuffer( ).fBegin( ), archive.fBuffer( ).fCount( ), true );

		if( flush )
			network.fFlush( );
	}

	//------------------------------------------------------------------------------
	// Utility functions
	//------------------------------------------------------------------------------
	namespace
	{
		const char* fStateToString( tGameSessionNetwork::tState state )
		{
#ifdef sig_devmenu
			switch( state )
			{
			case tGameSessionNetwork::cStateInactive: return "inactive";
			case tGameSessionNetwork::cStateConnecting: return "connecting";
			case tGameSessionNetwork::cStateSettingUpUser: return "settingUpUser";
			case tGameSessionNetwork::cStateCreatingSession: return "creatingSession";
			case tGameSessionNetwork::cStateJoiningLocalUsers: return "joiningLocalUsers";
			case tGameSessionNetwork::cStateRemovingLocalUsers: return "removingLocalUsers";
			case tGameSessionNetwork::cStateJoiningRemoteUsers: return "joiningRemoteUsers";
			case tGameSessionNetwork::cStateRemovingRemoteUsers: return "removingRemoteUsers";
			case tGameSessionNetwork::cStateReady: return "ready";
			case tGameSessionNetwork::cStateStarting: return "starting";
			case tGameSessionNetwork::cStateRunning: return "running";
			case tGameSessionNetwork::cStateMigratingHost: return "migrating host";
			case tGameSessionNetwork::cStateStopping: return "stopping";
			case tGameSessionNetwork::cStateDestroying: return "destroying";
			case tGameSessionNetwork::cStateRestarting: return "restarting";
			}
#endif //sig_devmenu

			return "";
		}
	}

	//------------------------------------------------------------------------------
	// tGameSessionNetwork::tSessionUserArray
	//------------------------------------------------------------------------------
	b32 tGameSessionNetwork::tSessionUserArray::fJoinLocal( tGameSession & session )
	{
		const u32 numUsers = fCount( );

		u32 userCount = 0;
		u32 zombieCount = 0;
		malloca_array( u32, indices, numUsers );
		malloca_array( b32, invited, numUsers );
		 

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

		if( userCount && !session.fJoinLocal( userCount, indices.fBegin(), invited.fBegin() ) )
			return false;

		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSessionNetwork::tSessionUserArray::fLeaveLocal( tGameSession & session )
	{	
		const u32 numUsers = fCount( );

		u32 userCount = 0;
		u32 zombieCount = 0;
		malloca_array( u32, indices, numUsers );

		for( u32 u = 0; u < numUsers; ++u )
		{
			sigassert_and_analyze_assume(userCount<numUsers);

			tSessionUser & user = operator[]( u );
			if( !user.mIsZombie )
				indices[ userCount++ ] = user.mUser->fLocalHwIndex( );
			else
				++zombieCount;
		}

		if( zombieCount && !session.fLeaveZombie( zombieCount ) )
			return false;

		if( userCount && !session.fLeaveLocal( userCount, indices.fBegin() ) )
			return false;

		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSessionNetwork::tSessionUserArray::fJoinRemote( tGameSession & session )
	{
		const u32 userCount = fCount( );
		malloca_array( tPlatformUserId, users, userCount );
		malloca_array( b32, invited, userCount );

		for( u32 u = 0; u < userCount; ++u )
		{
			sigassert( !operator[]( u ).mIsZombie && "Cannot do join remote zombie" );

			log_line( Log::cFlagNetwork, operator[]( u ).mUser->fGamerTag( ).fToCString( ) << " has joined." );

			users[ u ] = operator[]( u ).mUser->fPlatformId( );
			invited[ u ] = operator[]( u ).mInvited;
		}

		return session.fJoinRemote( userCount, users.fBegin(), invited.fEnd() );
	}

	//------------------------------------------------------------------------------
	b32 tGameSessionNetwork::tSessionUserArray::fLeaveRemote( tGameSession & session )
	{
		const u32 userCount = fCount( );
		malloca_array( tPlatformUserId, users, userCount );

		for( u32 u = 0; u < userCount; ++u )
		{
			sigassert( !operator[]( u ).mIsZombie && "Cannot leave remote zombie" );
			users[ u ] = operator[]( u ).mUser->fPlatformId( );

			log_line( Log::cFlagNetwork, operator[]( u ).mUser->fGamerTag( ).fToCString( ) << " has left." );
		}

		return session.fLeaveRemote( userCount, users.fBegin() );
	}

	//------------------------------------------------------------------------------
	tGameSessionNetwork::tSessionUser* tGameSessionNetwork::tSessionUserArray::fFindByUserId( const tPlatformUserId& userId )
	{
		const u32 count = fCount( );
		for( u32 u = 0; u < count; ++u )
		{
			if( operator[]( u ).mUser->fPlatformId( ) == userId )
				return &operator[]( u );
		}

		return NULL;
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
	const Net::tPeer* tGameSessionNetwork::fFindPeer( tPlatformUserId userId )
	{
		for( u32 i = 0; i < mConnections.fCount( ); ++i )
		{
			if( mConnections[ i ].mUsers.fFindByUserId( userId ) )
				return &mConnections[ i ].mPeer;
		}

		return NULL;
	}

	//------------------------------------------------------------------------------
	b32 tGameSessionNetwork::fGetUsersByPeer( tGrowableArray< tUserPtr > & out, const Net::tPeer & peer )
	{
		tConnection* connection = mConnections.fFind( peer );
		if( connection )
		{
			for( u32 i = 0; i < connection->mUsers.fCount( ); ++i )
			{
				tUser* curUser = connection->mUsers[ i ].mUser.fGetRawPtr( );
				out.fPushBack( ).fReset( curUser );
			}

			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------
	b32 tGameSessionNetwork::fHost( 
		tUser & user, 
		u32 sessionCreateFlags, 
		u32 publicSlots, 
		u32 privateSlots,
		const void* customData,
		u32 customDataSize )
	{
		sigassert( mState == cStateInactive && !mSession );

		mHostUser.fReset( &user );
		mOwnerUser.fReset( &user );
		mInvited = true;
		mNatType = Net::tPeer::fGetLocalNatType( );

		// Determine if this is a system link game
		{
			const u32 flagsToIgnore = tGameSession::cCreateJoinInProgressDisabled;
			mIsSystemLink = ( ( sessionCreateFlags & ~flagsToIgnore ) == tGameSession::cCreateSystemLink );
		}

		// Create our host session
		mSession.fReset( NEW tGameSession( 
			sessionCreateFlags, 
			publicSlots, 
			privateSlots, 
			make_delegate_memfn( 
				tGameSession::tStateChangedCallback, 
				tGameSessionNetwork, 
				fHandleSessionStateChange ) ) );

		if( !mIsSystemLink )
			mSession->fSetQosListenData( customData, customDataSize );

		// Is this available to the public?
		if( mSession->fIsOnline( ) )
		{
			b32 success = mNetwork.fHost( 
				this, 
				mSession->fTotalSlots( ) - 1, 
				mChannelId + 1, 
				Net::tHost::cDefaultAddress,
				Net::tHost::cDefaultPort );

			if( success && mIsSystemLink )
				success = mLanAdvertiser.fStart( customData, customDataSize, mNetwork.fSocket( ) );

			if( !success )
			{
				fCancelAndLeave( );
				return false;
			}
		}

		fSetState( cStateSettingUpUser );
		return true;
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fSetState( tState state )
	{
		//log_line( Log::cFlagNetwork, "tGameSessionNetwork::fSetState " << fStateToString( mState ) << " -> " << fStateToString( state ) );
		mState = state;
	}

	//------------------------------------------------------------------------------
	b32 tGameSessionNetwork::fJoin( tUser & user, const tGameSessionSearchResult & result, b32 usePrivateSlot )
	{
		mInvited = usePrivateSlot;
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
		sigassert( mState == cStateInactive && !mSession );

		mIsSystemLink = false;
		mJoinInfo = info;
		mOwnerUser.fReset( &user );
		mHostUser.fRelease( );
		tGameSession::fRegisterKey( info );
		mNatType = Net::tPeer::fGetLocalNatType( );

		// Join network and get session info
		b32 success = mNetwork.fJoin( 
			this, 
			mMaxPeers, 
			mChannelId + 1,
			tGameSession::fGetAddress( info ) );

		if( !success )
		{
			log_warning( "Failure to join game session network" );

			tGameSession::fUnregisterKey( info );
			mOwnerUser.fRelease( );
			return false;
		}

		fSetState( cStateConnecting );
		return true;
	}

	//------------------------------------------------------------------------------
	b32	 tGameSessionNetwork::fJoin( tUser & user, u32 hostAddr, u32 hostPort )
	{
		sigassert( mState == cStateInactive && !mSession );

		mIsSystemLink = true;
		fZeroOut( mJoinInfo );
		mOwnerUser.fReset( &user );
		mHostUser.fRelease( );
		mNatType = Net::tPeer::fGetLocalNatType( );

		// Join network and get session info
		b32 success = mNetwork.fJoin( 
			this, 
			mMaxPeers, 
			mChannelId + 1,
			hostAddr,
			hostPort );

		if( !success )
		{
			log_warning( "Failure to join game session network" );

			mOwnerUser.fRelease( );
			return false;
		}

		fSetState( cStateConnecting );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSessionNetwork::fAddLocalUser( tUser & user, b32 addAsZombie )
	{
		// Exists?
		if( !mSession )
		{
			log_warning( "Cannot add local user to session when session does not exist" );
			return false;
		}

		// Join in progress disabled and beyond created state
		if( mSession->fState( ) >= tGameSession::cStateStarting && 
			( mSession->fCreateFlags( ) & tGameSession::cCreateJoinInProgressDisabled ) )
		{
			log_warning( "Cannot add local user to session because session has disabled join in progress" );
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
			log_warning( "Cannot add local user to session because there are no slots available" );
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
			log_warning( "Local user cannot be removed because it is the session owner" );
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
		log_warning( "Local user cannot be removed because it is unknown" );
		return false;
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fWriteStats( 
		tPlatformUserId userId, u32 writeCount, const tGameSessionViewProperties writes[] )
	{
		sigassert( mState == cStateRunning && "Can only write stats from running state" );

		if( !mSession )
		{
			log_warning( "Attempt to write stats with no session" );
			return;
		}

		if( mSession->fState( ) < tGameSession::cStateStarted )
		{
			log_warning( "Attempt to write stats to session that is not started" );
			return;
		}

		if( userId == tUser::cInvalidUserId )
		{
			log_warning( "Attempt to write stats for invalid user" );
			return;
		}
		
		if( !writeCount )
			return;

		b32 userFound = false;

		// Test if the user is a local user
		userFound = ( mLocalUsers.fFindByUserId( userId ) != NULL );

		// If the session is arbitrated stats writes for remote players are allowed and required
		// so if we haven't found it yet then search the remote players
		if( !userFound && mSession->fIsArbitrated( ) )
			userFound = ( mRemoteUsers.fFindByUserId( userId ) != NULL );

		// User unknown
		if( !userFound )
		{
			log_warning( "UserId not found in " << __FUNCTION__ );
			return;
		}

		mSession->fQueueWriteStats( userId, writeCount, writes );
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fFlushStats( )
	{
		sigassert( mState == cStateRunning && "Can only flush stats from running state" );
		mSession->fFlushQueuedStats( );
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
		if( mState == cStateSettingUpUser && !mOwnerUser->fHasPendingOperations( ) )
		{
			// Start creating the session
			b32 success = mSession->fCreate( mOwnerUser->fLocalHwIndex( ) );
			if( !success )
			{
				log_warning( "Failed to start creating session" );
				fSetState( cStateInactive );
				mSession.fRelease( );
				mNetwork.fLeaveNow( );
				return;
			}

			fSetState( cStateCreatingSession );
		}

		if( mLanAdvertiser.fStarted( ) )
			mLanAdvertiser.fTick( );

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

			// Check if we should tell the session to migrate the host
			if( mHostMigrationPending && ( mSession->fIsStarted( ) || mSession->fIsCreated( ) ) )
			{
				if( fIsHost( ) )
				{
					mSession->fMigrateHostLocal( mOwnerUser->fLocalHwIndex( ) );
				}
				else
				{
					mJoinInfo = mMigrationJoinInfo;
					mSession->fMigrateHostRemote( mJoinInfo );
				}

				fSetState( cStateMigratingHost );
				mHostMigrationPending = false;
			}
		}
	}

	//------------------------------------------------------------------------------
	b32 tGameSessionNetwork::fStart( ) 
	{
		sigassert( !mLocalToAdd.fCount( ) && !mRemoteToAdd.fCount( ) );
		sigassert( !mLocalToRemove.fCount( ) && !mRemoteToRemove.fCount( ) );
		sigassert( mState == cStateReady );	
		mSession->fClearViewsWrittenTo( );

		if( mSession->fIsArbitrated( ) )
		{
			if( !mSession->fIsHost( ) )
			{
				b32 success = mSession->fArbitrationRegister( );
				if( !success )
				{
					log_warning( __FUNCTION__ << "Failed to begin arbitration registration" );
					return false;
				}
			}
		}
		else
		{
			b32 success = mSession->fStart( );
			if( !success )
			{
				log_warning( __FUNCTION__ << "Failed to begin starting session" );
				return false;
			}
		}

		fSetState( cStateStarting );
		return true;
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fEnd( )
	{
		sigassert( mState == cStateRunning && "End called while not running" );
		if( mSession->fNoQueuedWrites( ) )
			mSession->fEnd( );

		fSetState( cStateStopping );
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

		
		fSetState( cStateDestroying );
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fRestart( )
	{
		sigassert( mState == cStateRunning && "Restart called while not running" );

		if( mSession->fNoQueuedWrites( ) )
			mSession->fEnd( );

		fSetState( cStateRestarting );
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fCancelAndLeave( )
	{
		// Unregister the key if we're in the process of connecting
		if( mState == cStateConnecting || mState == cStateSettingUpUser )
			tGameSession::fUnregisterKey( mJoinInfo );

		mNetwork.fFlush( );
		// Kill the session
		fKillSession( true );

		if( mLanAdvertiser.fStarted( ) )
			mLanAdvertiser.fStop( );

		mNetwork.fLeaveNow( );
		fSetState( cStateInactive );

		fClearData( );
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fSendMsg( 
		const Net::tPeer & peer, u32 channelId, const byte * data, u32 dataLength, b32 reliable )
	{
		sigassert( channelId < mChannelId && "Cannot send msg on protocol channels" );
		mNetwork.fSendMsg( peer, channelId, data, dataLength, reliable );
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
			fSetState( cStateInactive );
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
					fKillQueuedWritesForUser( oldStates[ hwIdx ].mOnlineId );
				}
				else if( oldStates[ hwIdx ].mState == tUser::cSignInStateSignedInOnline &&
					newStates[ hwIdx ].mState == tUser::cSignInStateSignedInLocally )
				{
					// Host can only close the handle since host deletes require
					// LIVE session calls
					if( fIsHost( ) )
					{
						fKillSession( false );

						if( mLanAdvertiser.fStarted( ) )
							mLanAdvertiser.fStop( );

						mNetwork.fLeaveNow( );
						fClearData( );
						fSetState( cStateInactive );
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
		session->fClearQueuedWrites( );

		// Disable Qos listen to prevent peers from joining while we're destroying the session
		if( session->fIsHost( ) && ( session->fIsCreated( ) || session->fIsStarted( ) ) )
			session->fDisableQosListen( );

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
				log_warning( "Failed to leave local users, presence conflicts may persist" );

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
			fSetState( cStateRemovingLocalUsers );

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
			fSetState( cStateRemovingRemoteUsers );
		}
		else if( mLocalToAdd.fCount( ) && mLocalToAdd.fJoinLocal( *mSession ) )
		{
			// Move the users, set the state
			mPendingAddOrRemove.fJoin( mLocalToAdd );
			mLocalToAdd.fSetCount( 0 );
			fSetState( cStateJoiningLocalUsers );

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
			fSetState( cStateJoiningRemoteUsers );
		}
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fProcessStatsWriteQueue( )
	{
		mSession->fProcessStatsWriteQueue( );
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fKillQueuedWritesForUser( tPlatformUserId userId )
	{
		mSession->fKillQueuedWritesForUser( userId ); 
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
	void tGameSessionNetwork::fRejectPeer( const Net::tPeer & peer, u32 reason )
	{
		// Try to be friendly by sending a join rejected packet
		// but if the peer doesn't get it, no big deal they'll time-out
		// or get our disconnection notice
		tJoinRejectedPacket packet;
		packet.mReason = reason;

		// Flush the host now because we're gonna disconnect 
		// without waiting for outgoing message
		fSendPacket( packet, mChannelId, peer, false, true );
		mNetwork.fRejectPeer( peer );
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fUpdateNatType( )
	{
		const Net::tNatType prevNatType = mNatType;

		mNatType = Net::tPeer::fGetLocalNatType( );
		for( u32 c = 0; c < mConnections.fCount( ); ++c )
			mNatType = fMax( mNatType, mConnections[ c ].mNatType );

		log_line( Log::cFlagNetwork, "Updated nat type from " << prevNatType << " to " << mNatType );
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
				log_warning( "Failed to leave local users, presence conflicts may persist" );

			// End or delete the session
			if( gameSession.fIsStarted( ) )
				gameSession.fEnd( );
			else
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
		case tGameSession::cStateMigratingHost:
			fHandleSessionMigratingHost( success );
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
			// Create the voice network
			if( mSession->fTotalSlots( ) > 1 )
				mVoiceNetwork.fReset( NEW Net::tVoiceNetwork( ) );

			mLocalToAdd.fInsert( 0, tSessionUser( *mOwnerUser, true ) );

			if( mSession->fIsHost( ) )
				mSession->fEnableQosListen( );
		}
		else
		{
			fSetState( cStateInactive );
			mNetwork.fLeaveNow( );
			mSession.fRelease( );

			if( mOwner )
				mOwner->fOnError( cStateCreatingSession );
		}
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fHandleSessionEnding( b32 success )
	{
		mSession->fClearViewsWrittenTo( );

		if( success )
		{
			// We had an error pushing stats
			if( mOwner && mSession->fLastError( ) )
				mOwner->fOnStatsLost( );

			if( mState == cStateStopping )
			{
				if( mOwner )
					mOwner->fOnStopped( );

				fSetState( cStateReady );
			}
			else if( mState == cStateDestroying )
				mSession->fDelete( );
			else if( mState == cStateRestarting )
				mSession->fStart( );
		}
		else
		{
			log_warning( "Failed to end session from state " << mState );
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

			fSetState( cStateReady );
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
				fSetState( cStateReady );

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
			fSetState( mSession->fIsStarted( ) ? cStateRunning : cStateReady );

			// Append the newly added users to the remote list
			mRemoteUsers.fJoin( mPendingAddOrRemove );

			// Register the remote users with the voice system
			const u32 userCount = mPendingAddOrRemove.fCount( );
			for( u32 u = 0; u < userCount; ++u )
			{
				tConnection * connection = mConnections.fFind( mPendingAddOrRemove[ u ].mUser.fGetRawPtr( ) );
				if( !connection )
				{
					log_warning( "Remote user has disconnected after joining session" );
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
			fSetState( mSession->fIsStarted( ) ? cStateRunning : cStateReady );

			// Add the remote users back to the weak list
			mPendingAddOrRemove.fSetCount( 0 );

			if( mOwner )
				mOwner->fOnError( cStateJoiningRemoteUsers );
		}
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fHandleSessionRemovingLocalUsers( b32 success )
	{
		fSetState( mSession->fIsStarted( ) ? cStateRunning : cStateReady );
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

		fSetState( mSession->fIsStarted( ) ? cStateRunning : cStateReady );
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
			fSetState( cStateRunning );

			// Check if we should stop advertising the session
			if( mIsSystemLink && !mSession->fJoinInProgressEnabled( ) )
				mLanAdvertiser.fStop( );

			if( mOwner )
				mOwner->fOnRunning( );
		}
		else
		{
			tState lastState = mState;
			fSetState( cStateReady );
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
					log_warning( __FUNCTION__ << ": Failed to begin starting session" );
					fSetState( cStateReady );
					if( mOwner )
						mOwner->fOnError( cStateStarting );
				}

				tArbitratedPacket msg;
				fSendPacketToPeers( msg, mChannelId, mNetwork, true, false );
			}
		}
		else
		{
			fSetState( cStateReady );
			if( mOwner )
				mOwner->fOnError( cStateStarting );
		}
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fHandleSessionDeleting( b32 success )
	{
		sigassert( success );

		fSetState( cStateInactive );
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
	void tGameSessionNetwork::fHandleSessionMigratingHost( b32 success )
	{
		if( success )
		{
			fSetState( mSession->fIsStarted( ) ? cStateRunning : cStateReady );

			tDynamicBuffer customData;

			if( fIsHost( ) )
			{
				// Notify peers that we are hosting
				tHostMigratedPacket packet;
				packet.fSetSessionInfo( mSession->fSessionInfo( ) );
				fSendPacketToPeers( packet, mChannelId, mNetwork, true, false );

				if( mOwner )
				{
					mOwner->fOnHostMigrated( customData );

					if( mIsSystemLink )
					{
						// Check if we should advertise the session
						if( mSession->fJoinInProgressEnabled( ) || mState == cStateReady )
						{
							if( !mLanAdvertiser.fStart( customData.fBegin( ), customData.fCount( ), mNetwork.fSocket( ) ) )
							{
								fCancelAndLeave( );
								if( mOwner )
									mOwner->fOnError( cStateMigratingHost );

								return;
							}
						}
					}
					else
					{
						mSession->fSetQosListenData( customData.fBegin( ), customData.fCount( ) );
					}
				}

				mSession->fEnableQosListen( );
			}
			else
			{
				if( mOwner )
					mOwner->fOnHostMigrated( customData );
			}
		}
		else
		{
			fCancelAndLeave( );
		}
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fHandleJoinRequestPacket( 
		const byte * data, 
		u32 dataLength,
		const Net::tPeer & peer )
	{
		if( !mSession || !mSession->fIsHost( ) )
		{
			log_line( 0, "Ignoring join request because we are not the host" );
			return;
		}

		if( !mSession->fJoinInProgressEnabled( ) && !mSession->fIsCreatedOrJoining( ) )
		{
			log_line( 0, "Ignoring join request. Join-in-progress is disabled for this session" );
			return;
		}

		tJoinRequestPacket request;
		fDecodePacket( request, data, dataLength );

		// Check if the user already exists on one of the connections.
		// This could happen if the user is reconnecting after disconnecting ungracefully
		for( u32 c = 0; c < mConnections.fCount( ); ++c )
		{
			tPlatformUserId newUserId = request.mUserInfo.mUserId;
			tSessionUser* existingUser = mConnections[ c ].mUsers.fFindByUserId( newUserId );
			if( existingUser )
			{
				log_line( 0, "Ignoring join request. User " << newUserId << " is already in the session" );
				return;
			}
		}

		// Not enough free slots to hold the peer
		if( !fHasFreeSlots( 1 ) || ( !request.mInvited && !fHasFreePublicSlots( 1 ) ) )
		{
			log_line( 0, "Rejecting peer because session was full" );

			fRejectPeer( peer, tJoinRejectedPacket::cReasonSessionFull );
			return;
		}

		// Peer has incompatible NAT type.
		// NAT types are incompatible if they are both strict, or if one is strict and the other is moderate
		if( ( request.mNatType == mNatType && mNatType == Net::cNatTypeStrict ) ||
			( request.mNatType != mNatType && mNatType > Net::cNatTypeOpen && request.mNatType > Net::cNatTypeOpen ) )
		{
			log_line( 0, "Rejecting peer because nat type was incompatible" );

			fRejectPeer( peer, tJoinRejectedPacket::cReasonIncompatibleNatType );
			return;
		}

		// We're good to go
		tJoinAcceptedPacket packet;
		packet.mSessionNonce = mSession->fIsArbitrated( ) ? mSession->fNonce( ) : 0;
		packet.mSessionCreateFlags = mSession->fCreateFlags( );
		packet.mSessionGameMode = mSession->fDetails( ).mGameMode;
		packet.mSessionGameType = mSession->fDetails( ).mGameType;
		packet.mTotalPrivateSlots = mSession->fTotalPrivateSlots( );
		packet.mTotalPublicSlots = mSession->fTotalPublicSlots( );

		packet.mHostNatType = Net::tPeer::fGetLocalNatType( );
		packet.mHostUserInfo = tUserInfo( mHostUser->fUserInfo( ) );
		fSendPacket( packet, mChannelId, peer, true );

		tUserPtr newUser( NEW tUser( request.mUserInfo ) );

		// Build the connection
		mConnections.fPushBack( tConnection( ) );
		mConnections.fBack( ).mPeer = peer;
		mConnections.fBack( ).mUsers.fPushBack( tSessionUser( *newUser, request.mInvited ) );
		mConnections.fBack( ).mNatType = request.mNatType;

		fUpdateNatType( );

		if( mOwner )
			mOwner->fOnUserJoined( peer, newUser );

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
		if( mState != cStateConnecting )
		{
			log_warning( "Join accepted packet received on non-connecting game session network" );
			return;
		}

		if( !mNetwork.fIsConnected( ) )
		{
			log_warning( "Join accepted packet received on non-connected p2p network" );
			return;
		}

		tJoinAcceptedPacket accepted;
		fDecodePacket( accepted, data, dataLength );

		// Sweet, we got in!
		mHostUser.fReset( NEW tUser( accepted.mHostUserInfo ) );

		// Build the connection
		mConnections.fPushBack( tConnection( ) );
		mConnections.fBack( ).mPeer = peer;
		mConnections.fBack( ).mUsers.fPushBack( tSessionUser( *mHostUser, true ) );
		mConnections.fBack( ).mNatType = accepted.mHostNatType;

		fUpdateNatType( );

		if( mOwner )
			mOwner->fOnUserJoined( peer, mHostUser );

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

		// Good to go
		fSetState( cStateSettingUpUser );
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fHandleJoinRejectedPacket(
		const byte * data, 
		u32 dataLength,
		const Net::tPeer & peer )
	{
		if( mState != cStateConnecting )
		{
			log_warning( "Join Rejected packet recieved on non-connecting game session network" );
			return;
		}

		if( !mNetwork.fIsConnected( ) )
		{
			log_warning( "Join rejected packet recieved on non-connected p2p network" );
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
		case tJoinRejectedPacket::cReasonIncompatibleNatType:
			reason = "Nat type was incompatible";
			break;
		default:
			reason = "Unknown";
			break;
		}

		log_line( 0, "Session request was rejected: " << reason );
		tGameSession::fUnregisterKey( mJoinInfo );
		fSetState( cStateInactive );
		if( mOwner )
			mOwner->fOnRejected( );
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fHandlePeerInfoPacket(
		const byte * data, 
		u32 dataLength,
		const Net::tPeer & peer )
	{
		if( mSession->fIsArbitrated( ) && ( !mNetwork.fIsConnectedOrHosting( ) || !mSession->fIsCreated( ) ) )
			return;

		tPeerInfoPacket peerInfo;
		fDecodePacket( peerInfo, data, dataLength );

		tConnection* connection = mConnections.fFind( peer );
		sigassert( connection && "The calling function should have checked this before continuing." );

		tUserPtr newUser( NEW tUser( peerInfo.mUserInfo ) );
		connection->mUsers.fPushBack( tSessionUser( *newUser, peerInfo.mInvited ) );

		connection->mNatType = peerInfo.mNatType;

		fUpdateNatType( );

		if( mOwner )
			mOwner->fOnUserJoined( peer, connection->mUsers.fBack( ).mUser );

		// Add the user
		mRemoteToAdd.fPushBack( connection->mUsers[ 0 ] );
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fHandleArbitratedPacket( 
		const byte * data, 
		u32 dataLength, 
		const Net::tPeer & peer )
	{
		if( mState != cStateStarting )
		{
			log_warning( "Arbitration packets should only be received while in Starting state" );
			return;
		}
		
		tArbitratedPacket arbitrated;
		fDecodePacket( arbitrated, data, dataLength );

		tConnection * connection = mConnections.fFind( peer );
		if( !connection )
		{
			log_warning( "Received arbitration msg from unknown connection" );
			return;
		}

		if( connection->mArbitrated )
		{
			log_warning( "Received duplicate arbitration msg" );
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
					log_warning( "Failed to begin arbitration" );
					fSetState( cStateReady );
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
				log_warning( __FUNCTION__ << ": Failed to begin starting session" );
				fSetState( cStateReady );
				if( mOwner )
					mOwner->fOnError( cStateStarting );
			}
		}
		
		// Who is this?
		else
		{
			log_warning( "Received arbitration packet from non-host" );
		}
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fHandleVoiceMutePacket( 
		const byte * data, u32 dataLength, const Net::tPeer & peer )
	{
		tVoiceMutePacket mutePacket;
		fDecodePacket( mutePacket, data, dataLength );

		tConnection * connection = mConnections.fFind( peer );
		if( !connection )
		{
			log_warning( "Received voice mute msg from unknown connection" );
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
	void tGameSessionNetwork::fHandleHostMigratedPacket( 
		const byte * data, u32 dataLength, const Net::tPeer & peer )
	{
		tHostMigratedPacket packet;
		fDecodePacket( packet, data, dataLength );

		tConnection * connection = mConnections.fFind( peer );
		if( !connection )
		{
			log_warning( "Received host migrated msg from unknown connection" );
			return;
		}

		mNetwork.fMigrateHost( peer );

		mHostUser = connection->mUsers.fFront( ).mUser;

		// Prepare to migrate when ready
		mHostMigrationPending = true;
		mMigrationJoinInfo = packet.fGetSessionInfo( );
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
					log_warning( "Msg received from unknown connection on game channel: " << channelId );
			}
			else
			{
				log_warning( "Game message ignored because no owner was set on tGameSessionNetwork" );
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
		case cPacketTypeHostMigrated:
			fHandleHostMigratedPacket( data, dataLength, peer );
			break;
		}
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fOnConnect( Net::tP2PNetwork & network, const Net::tPeer & peer )
	{
		switch( mState )
		{
		case cStateConnecting:
			{
				// Our host has connected
				sigassert( peer == *network.fHostPeer( ) && "Connection to non-host in connecting state" );

				// It's the server peer who hasn't accepted our connection yet
				// so we should ask for it
				
				// TODO: Allow more than one user
				tJoinRequestPacket packet( mOwnerUser->fUserInfo( ), Net::tPeer::fGetLocalNatType( ), mInvited );
				fSendPacket( packet, mChannelId, peer, true );

			} break;

		default:
			log_assert( mSession->fJoinInProgressEnabled( ) || mSession->fIsCreatedOrJoining( ), "Session does not support join-in-progress" );

			tConnection* connection = mConnections.fFind( peer );

			// Add a connection if it doesn't already exist
			if( !connection )
			{
				mConnections.fPushBack( tConnection( ) );
				mConnections.fBack( ).mPeer = peer;

				// Send our info so the new peer can add our user
				tPeerInfoPacket packet;
				packet.mUserInfo = tUserInfo( mOwnerUser->fUserInfo( ) );
				packet.mInvited = mInvited;
				packet.mLeaving = false;
				packet.mNatType = Net::tPeer::fGetLocalNatType( );

				fSendPacket( packet, mChannelId, peer, true );
			}

			// Make sure the existing connection's peer has the correct Id
			else
			{
				connection->mPeer.fSetId( peer.fId( ) );
			}

			// Ship out our mute list to any new peers
			fOnMuteListChanged( );
			break;
		}

		if( mOwner )
			mOwner->fOnConnect( peer );
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fOnDisconnect( Net::tP2PNetwork & network, const Net::tPeer & peer, u32 reason )
	{
		if( tConnection * connection = mConnections.fFind( peer ) )
		{
			// Remove from voice chat
			if( connection->mCanHear )
				mVoiceNetwork->fRemovePeer( peer.fAddressHost( ), peer.fAddressPort( ) + 1 );

			// Label for removal
			mRemoteToRemove.fJoin( connection->mUsers );

			for( u32 i = 0; i < connection->mUsers.fCount( ); ++i )
				mOwner->fOnUserLeft( peer, connection->mUsers[ i ].mUser );

			mConnections.fErase( fPtrDiff( connection, mConnections.fBegin( ) ) );

			fUpdateNatType( );

			if( mOwner )
				mOwner->fOnDisconnect( peer, reason );
		}
		else if( mState == cStateConnecting )
		{
			// If we got disconnected from the p2p network, cancel the join.
			// This could happen if the peer is no longer hosting but is still reachable.
			if( !network.fIsConnected( ) )
			{
				tGameSession::fUnregisterKey( mJoinInfo );
				fSetState( cStateInactive );
			}

			if( mOwner )
			{
				if( reason == Net::tP2PNetwork::cDisconnectReasonRejected )
					mOwner->fOnRejected( );
				else
					mOwner->fOnDisconnect( peer, reason );
			}
		}
		else
			log_warning( "Disconnect notification from unknown peer" );
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fOnJoinFailure( Net::tP2PNetwork & network )
	{
		tGameSession::fUnregisterKey( mJoinInfo );
		fSetState( cStateInactive );

		if( mOwner )
			mOwner->fOnError( cStateConnecting );
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fOnConnectAttempt( Net::tP2PNetwork & network, const Net::tPeer & peer )
	{
		// Do nothing, because we can only accept after receiving a join request
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fOnBeginHosting( Net::tP2PNetwork & network )
	{
		mHostUser = mOwnerUser;
		mHostMigrationPending = true;
	}

	//------------------------------------------------------------------------------
	u32 tGameSessionNetwork::fGetAddress( const Net::tPlatformAddr& platformAddr )
	{
#ifdef platform_xbox360
		if( !fIsSystemLink( ) )
		{
			IN_ADDR inAddr; fZeroOut( inAddr );
			INT error = XNetXnAddrToInAddr( &platformAddr.xnAddr, &mJoinInfo.sessionID, &inAddr );
			if( error )
			{
				log_warning( "XNetXnAddrToInAddr failed. Error " << error );
				return 0;
			}
			return inAddr.s_addr;
		}
		else
		{
			return platformAddr.xnAddr.ina.S_un.S_addr;
		}
#else
		return platformAddr.inAddr;
#endif
	}

	//------------------------------------------------------------------------------
	u16 tGameSessionNetwork::fGetPort( const Net::tPlatformAddr& platformAddr )
	{
#ifdef platform_xbox360
		if( !fIsSystemLink( ) )
			return Net::tHost::cDefaultPort; //Use the default; xnAddr.wPortOnline is not what we want
		else
			return platformAddr.xnAddr.wPortOnline;
#else
		return platformAddr.port;
#endif
	}

	//------------------------------------------------------------------------------
	b32 tGameSessionNetwork::fGetPlatformAddr( Net::tPlatformAddr& out, u32 address, u32 port )
	{
#ifdef platform_xbox360
		if( !fIsSystemLink( ) )
		{
			IN_ADDR inAddr; fZeroOut( inAddr );
			inAddr.s_addr = address;
			XNKID kid; fZeroOut( kid );
			INT error = XNetInAddrToXnAddr( inAddr, &out.xnAddr, &kid );
			if( error )
			{
				log_warning( "XNetInAddrToXnAddr failed. Error " << error );
				return false;
			}
			return true;
		}
		else
		{
			fZeroOut( out );
			out.xnAddr.ina.S_un.S_addr = address;
			out.xnAddr.wPortOnline = port;
			return true;
		}
#else
		out.inAddr = address;
		out.port = port;
		return true;
#endif
	}

	//------------------------------------------------------------------------------
	void tGameSessionNetwork::fLogStats( std::wstringstream& statsText )
	{
#ifdef sig_devmenu
		const Net::tP2PNetwork::tStats& stats = mNetwork.fStats( );

		statsText << "Network: " << std::endl;
		statsText << " sent: packets(" << stats.mPacketsSent << ") bytes(" << stats.mBytesSent << ")" << std::endl;
		statsText << " recvd: packets(" << stats.mPacketsReceived << ") bytes(" << stats.mBytesReceived << ")" << std::endl;
#endif
	}
}
