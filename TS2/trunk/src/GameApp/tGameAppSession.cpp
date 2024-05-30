#include "GameAppPch.hpp"
#include "tGameApp.hpp"
#include "tGameSession.hpp"
#include "tGameSessionSearch.hpp"
#include "tLevelLogic.hpp"
#include "GameNetMessages.hpp"
#include "FileSystem.hpp"
#include "GameSession.hpp"
#include "tSync.hpp"
#include "tGameSessionStats.hpp"
#include "tGameLoadAppState.hpp"


namespace Sig
{
	devvar( bool, Game_Record, false );
	devvar( bool, Game_Record_Sync, false );
	devvar( bool, Game_Sync_Verbose, false );
	devvar( int, Game_Net_LvlIdx, -1 );
	devvar( bool, Game_Sync_SkipAssert, true );
	devvar( tStringPtr, Game_Net_LvlOverride, tStringPtr( ) );

	namespace
	{
		static const u32 cMinInputBufferSize = 1;
		static const u32 cMaxInputBufferSize = 3;
		static const u32 cResultCount = 10;
		static const u32 cUserCount = 1;

		enum tGameChannels
		{
			cGameChannelDefault = 0,
			cGameChannelSync,

			cGameChannelCount
		};
		

		///
		/// \class tReplayContextInfo
		/// \brief 
		struct tReplayContextInfo
		{
			struct tReplayUser
			{
				b32				mWasLocal;
				tUserInfo		mUserInfo;

				template<class tArchive>
				void fSaveLoad( tArchive & archive )
				{
					archive.fSaveLoad( mWasLocal );
					archive.fSaveLoad( mUserInfo );
				}
			};

			u32 mObjectiveSeed;
			tLevelLoadInfo mLevelInfo;

			u32 mHostUserIndex;
			tDynamicArray<tReplayUser> mUsers;

			template<class tArchive>
			void fSaveLoad( tArchive & archive )
			{
				archive.fSaveLoad( mObjectiveSeed );
				archive.fSaveLoad( mLevelInfo );
				archive.fSaveLoad( mHostUserIndex );
				archive.fSaveLoad( mUsers );
			}
		};

		enum tReplayContext
		{
			cReplayContextInfo = 0, // Gameplay info
			cReplayContextCount
		};
	}

	//------------------------------------------------------------------------------
	// Packet helpers
	//------------------------------------------------------------------------------
	template<class t>
	static b32 fDecodeMsg( t & out, const byte * data, u32 dataLength )
	{
		tGameArchiveLoad archive( data, dataLength );
		archive.fSaveLoad( out );
		return true;
	}

	//------------------------------------------------------------------------------
	template<class t>
	static void fSendMsgToPeers( 
		tGameSessionNetwork & network, t & msg, u32 channelId, b32 reliable, b32 flush = false )
	{
		if( !network.fIsConnected( ) )
		{
			//log_warning( Log::cFlagNetwork, __FUNCTION__ << "Msg not sent because network was not functioning" );
			return;
		}

		tGameArchiveSave archive;
		archive.fSaveLoad( msg );

		network.fSendMsgToPeers(
			channelId, 
			archive.fBuffer( ).fBegin( ), 
			archive.fBuffer( ).fTotalSizeOf( ), 
			reliable );

		if( flush )
			network.fFlush( );
	}

	//------------------------------------------------------------------------------
	template<class t>
	static void fSendMsgToHost( 
		tGameSessionNetwork & network, t & msg, u32 channelId, b32 reliable )
	{
		tGameArchiveSave archive;
		archive.fSaveLoad( msg );

		network.fSendMsgToHost( 
			channelId, 
			archive.fBuffer( ).fBegin( ), 
			archive.fBuffer( ).fTotalSizeOf( ), 
			reliable );
	}

	//------------------------------------------------------------------------------
	static void fLogMsgError( 
		const Net::tPeer & peer, u32 channelId, const char * msg )
	{
		log_warning( 0, "Unrecognized msg error: " << msg << 
							" Channel: " << channelId << 
							" Address: " << std::hex << peer.fAddressHost( ) );
	}

	//------------------------------------------------------------------------------
	// tGameAppSession
	//------------------------------------------------------------------------------
	tGameAppSession::tGameAppSession( )
		: mGameSessionNetwork( cGameChannelCount )
		, mLoadingLevel( false )
		, mLevelUnloaded( false )
		, mHasAllInput( false )
		, mStartingReplaySession( false )
		, mWaitingForProfiles( false )
		, mFrontEndLoadBehavior( cFrontEndLoadBehaviorDestroySession )
		, mIsQuickMatch( false )
	{

		mGameSessionNetwork.fSetOwner( this );

		mErrorPrequeue.fReserve( 2 );
		mErrorQueue.fReserve( 2 );

		mCaptureInputFrame = mApplyInputFrame = 0;
		for( u32 i = 0; i < mQuickMatchMapRand.fCount( ); ++i )
			mQuickMatchMapRand[ i ] = tRandom( tRandom::fSubjectiveRand( ).fUInt( ) );
	}

	//------------------------------------------------------------------------------
	tGameAppSession::~tGameAppSession( )
	{
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fInitialize( )
	{
		mNetUI.fReset( NEW Gui::tNetUI( ) );
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fOnAppTick( f32 dt )
	{
		profile_pix("tGameAppSes*::fOnAppTick");
		if( mGameSessionSearch && mGameSessionSearch->fSearching( ) )
		{
			mGameSessionSearch->fTick( );
			if( !mGameSessionSearch->fSearching( ) )
				fSendSessionEvent( ApplicationEvent::cSessionSearchComplete, mGameSessionSearch->fSucceeded( ) );
		}

		if( !mLoadingLevel && mQueuedSessionEvents.fCount( ) )
		{
			for( u32 e = 0; e < mQueuedSessionEvents.fCount( ); ++e )
				fSendSessionEvent( mQueuedSessionEvents[ e ].mEventId, mQueuedSessionEvents[ e ].mContext );
			mQueuedSessionEvents.fSetCount( 0 );
		}

		tGameApp & gameApp = tGameApp::fInstance( );
		if( !mLoadingLevel && gameApp.fSimulate( ) && mErrorQueue.fCount( ) )
		{
			const tErrorDialogInfo error = mErrorQueue.fPopFront( );
			gameApp.fReallyShowErrorDialog( error.mErrorCode, error.mUserId );
		}

		fDrawStats( );

		// Ticks the session if it exists, this uses the real elapsed time
		// regardless of whether we're fixed or not
		mGameSessionNetwork.fTick( tGameApp::fInstance( ).fGetFrameDeltaTime( ) );

		if( mLoadingLevel && mGameSessionNetwork.fLongStateTime( 20.f ) )
			mNetUI->fShowNetWaitUI( );
		else
			mNetUI->fHideNetWaitUI( );
	}

#ifdef sig_devmenu
	devvar( bool, Debug_GameSessionDebugInfo_Render, false );
	namespace
	{
		static Gui::tTextPtr gStatsText;
	}
#endif

	namespace
	{
		void fUserStatsText( std::ostream& o, u32 i, tUser* user )
		{
			sigassert( user );

			o << "    " << i << " HW " << user->fLocalHwIndex() << " - " << user->fGamerTag( ).fToCString( ) << " (";
			if( user->fIsLocal( ) )
			{
				o << ( ( user->fSignedIn( ) )? "s" : "-" );
				o << ( ( user->fSignedInOnline( ) )? "o" : "-" );
				o << ( ( user->fSignedInOnline( ) && user->fHasPrivilege( tUser::cPrivilegeMultiplayer ) )? "m" : "-" );
				o << ( ( user->fIsGuest( ) )? "g" : "-" );
				o << ( ( user->fIsInActiveParty( ) )? "p" : "-" );
			}
			else
			{
				o << ".....";
			}
			o << ( ( user->fAddOnsLicensed( ) & GameFlags::cDLC_NAPALM )? "n" : "-" );
			o << ( ( user->fAddOnsLicensed( ) & GameFlags::cDLC_EVIL_EMPIRE )? "e" : "-" );
			o << ")" << std::endl;
		}
	}

	void tGameAppSession::fDrawStats( ) const
	{
#ifdef sig_devmenu
		if( !Debug_GameSessionDebugInfo_Render )
			return;

		tGameAppBase* gameApp = tApplication::fInstance( ).fDynamicCast< tGameAppBase >( );
		if( !gameApp )
			return;

		if( !gStatsText )
		{
			gStatsText.fReset( new Gui::tText( ) );
			gStatsText->fSetDevFont( );
		}

		std::stringstream statsText;
		statsText << "GameAppSession Debug" << std::endl;
		statsText << "LocalUsers (" << fLocalUserCount( ) << "):" << std::endl;
		for( u32 i = 0; i < fLocalUserCount( ); ++i )
			fUserStatsText( statsText, i, fLocalUser( i ) );
		statsText << "RemoteUsers (" << fRemoteUserCount( ) << "):" << std::endl;
		for( u32 i = 0; i < fRemoteUserCount( ); ++i )
			fUserStatsText( statsText, i, fRemoteUser( i ) );
		
		if( fLocalUserCount( ) + fRemoteUserCount( ) > 0 )
		{
			// Legend
			statsText << std::endl;
			statsText << "Legend: (somgpne)" << std::endl;
			statsText << "s - Signed in locally" << std::endl;
			statsText << "o - Signed in online" << std::endl;
			statsText << "m - Has multiplayer privileges" << std::endl;
			statsText << "g - Is a guest" << std::endl;
			statsText << "p - Is in active party" << std::endl;
			statsText << "n - Has Napalm DLC" << std::endl;
			statsText << "e - Has Evil Empire DLC" << std::endl;
			statsText << ". - Remote user, no data" << std::endl;
		}

		gStatsText->fBakeBox( 300, statsText.str( ).c_str( ), 0, Gui::tText::cAlignLeft );
		gStatsText->fSetPosition( Math::tVec3f( 1216.0f - 430.0f, 40.0f, 0.0f ) ); // Top right
		gStatsText->fSetRgbaTint( Math::tVec4f( 1.f, 1.f, 1.f, 1.f ) );
		gameApp->fScreen( )->fAddScreenSpaceDrawCall( gStatsText->fDrawCall( ) );
#endif
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fPollInputDevices( tUserArray & localUsers )
	{
		tGameApp & gameApp = tGameApp::fInstance( );

		// Single player games or games where the network has shut down or lost its connections
		if( !gameApp.fCurrentLevelLoadInfo( ).mGameMode.fIsNet( ) || 
			( mGameSessionNetwork.fIsInactive( ) || !mGameSessionNetwork.fConnectionCount( ) ) )
		{

			// Transfer from the prequeue straightforwardly for single box games
			if( mErrorPrequeue.fCount( ) )
			{
				mErrorQueue.fJoin( mErrorPrequeue );
				mErrorPrequeue.fSetCount( 0 );
			}

			if( !mReplayer )
			{
				const f32 frameDelta = gameApp.fGetFrameDeltaTime( );
				const u32 userCount = localUsers.fCount( );
				for( u32 u = 0; u < userCount; ++u )
				{
					tUserPtr user = localUsers[ u ];
					if( user )
						user->fPollInputDevices( frameDelta );
				}

				// Recording
				fRecordInputFrame( );
			}

			// Replayed input
			else if( !mLoadingLevel )
			{
				mReplayer->fMarkInputFrame( );

				// When we run out of input we quit
				if( !mReplayer->fHasInputAvailable( ) )
				{
					gameApp.fQuitAsync( );
				}
				else
				{
					const tDynamicArray< tPlayerPtr > & players = gameApp.fPlayers( );
					const u32 playerCount = players.fCount( );

					sigassert( playerCount == mReplayer->fGetInputCount( ) );	
					for( u32 p = 0; p < playerCount; ++p )
						players[ p ]->fUser( )->fAddGamepadState( mReplayer->fGetInput( p ) );
				}
			}
		}

		// It's a net game, but only process if the game session is valid
		else if( mGameSessionNetwork.fIsRunning( ) )
		{
			const f32 frameDelta = gameApp.fGetFrameDeltaTime( );

			// Can we apply our input?
			if( fReadyForInput( ) )
			{
				// Only if we're in the game do we apply input
				if( gameApp.fIngameSimulationState( ) )
				{
					b32 hadAllInput = mHasAllInput;
					mHasAllInput = fHasAllInputForThisFrame( );

					// Only simulate if we have all the input
					gameApp.fSetSimulate( mHasAllInput );

					// Apply an input frame if possible
					fTryApplyInputFrame( localUsers );
				}

				// But no matter what we poll for input

				// Poll input for users not used by the game
				const u32 localUserCount = localUsers.fCount( );
				for( u32 u = 0; u < localUserCount; ++u )
				{
					tUserPtr user = localUsers[ u ];
					if( !user || user->fIsUsedByGame( ) )
						continue;

					user->fPollInputDevices( frameDelta );
				}

				// Capture an input frame if necessary
				fTryCaptureInputFrame( localUsers );
			}

			if( gameApp.fSimulate( ) )
				mNetUI->fHideLagUI( );
			else
				mNetUI->fShowLagUI( frameDelta );
		}
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fAddToStatText( std::stringstream & statsText )
	{
		statsText << "network:" << std::endl;
		statsText << "   avg rtt = " << fAverageRTT( ) << " ms" << std::endl;
		statsText << "   avg loss ratio = " << fAveragePacketLoss( ) << std::endl;

		for( u32 u = 0; u < mInput.fCount( ); ++u )
			statsText << "   " << mInput[ u ].mUserId << " = " << mInput[ u ].mFrames.fNumItems( ) << std::endl;
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fOnLoadLevel( b32 alreadyLoading )
	{
		fResetSyncData( );

		// Hide the net UI
		mNetUI->fHideLagUI( );

		tGameApp & gameApp = tGameApp::fInstance( );

		// Recording/Playback
		mPlaycorder.fRelease( );
		if( mReplayer && !mLoadingLevel )
		{
			mReplayer.fRelease( );
			gameApp.fQuitAsync( );
		}

		// Always simulate while loading
		gameApp.fSetSimulate( true );

		mLoadingLevel = true;
		mLevelUnloaded = false;

		if( !alreadyLoading )
			gameApp.fLoadLevelDelayed( );

		if( !gameApp.fIsFullVersion( ) )
		{
			tGameApp::fInstance( ).fLoadLevelNowThatSessionIsReady( );
		}
		else if( mGameSessionNetwork.fIsInactive( ) )
		{
			// The network isn't being used because the session failed to create

			// Create preview session if we're starting from a preview
			if( gameApp.mNextLevelLoadInfo.mPreview )
				fCreateSessionForPreview( );

			// Otherwise just advance
			else
				tGameApp::fInstance( ).fLoadLevelNowThatSessionIsReady( );
		}
		else if( mGameSessionNetwork.fIsReadyAndIdle( ) )
		{
			mGameSessionNetwork.fStart( );
			fSendProfiles( );
		}
		else if( mGameSessionNetwork.fIsRunning( ) )
		{
			if( gameApp.fNextLevelLoadInfo( ).mGameMode.fIsFrontEnd( ) )
			{
				// Do the appropriate behavior
				switch( mFrontEndLoadBehavior )
				{
				case cFrontEndLoadBehaviorEndSession:
					mGameSessionNetwork.fEnd( );
					break;
				case cFrontEndLoadBehaviorDestroySession:
				default:
					mGameSessionNetwork.fDestroy( );
					break;
				}

				mFrontEndLoadBehavior = cFrontEndLoadBehaviorDestroySession;
			}
			else
			{
				mGameSessionNetwork.fRestart( );
				fSendProfiles( ); // Send all the profiles once more
			}
		}
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fOnLevelUnloadBegin( )
	{
		mLevelUnloaded = true;
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fOnLevelLoadBegin( )
	{
		// Players should absolutely exist by now
		fApplyProfiles( ); 
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fOnLevelSpawn( )
	{
		mLoadingLevel = false;
		mLevelUnloaded = false;
		mHasAllInput = false;

		// Hide the desync ui if it was being displayed
		mNetUI->fHideDesyncUI( );

		tGameApp & gameApp = tGameApp::fInstance( );

		// Net version is the same for replay or not
		if( gameApp.fCurrentLevelLoadInfo( ).mGameMode.fIsNet( ) )
		{
			// If we're running and have connections
			if( mGameSessionNetwork.fIsRunningWithConnections( ) )
			{
				// Reset local user gamepad data to a null pad state
				fNullifySessionPads( );

				// Reset sync state information
				log_line( 0, "Setting objective rand seed: " << mObjectiveSeed );
				tRandom::fSyncObjectiveSeed( mObjectiveSeed );
				gameApp.fSceneGraph( )->fResetLogicGuids( );

				//gameApp.fSetSimulate( false );
				gameApp.fSetFixedTimeStep( true );

				Net::tReadyForInput ready; // Let our peers know we're ready

				// Tag any local player inputs as ready
				const tDynamicArray<tPlayerPtr> & players = gameApp.fPlayers( );
				const u32 playerCount = players.fCount( );
				for( u32 p = 0; p < playerCount; ++p )
				{
					if( players[ p ]->fUser( )->fIsLocal( ) )
					{
						ready.mUserId = players[ p ]->fUser( )->fPlatformId( );
						fSendMsgToPeers( mGameSessionNetwork, ready, cGameChannelDefault, true );
						fMarkReadyForInput( ready.mUserId );
					}
				}
			}

			else
			{
				// Begin standard input processing
				gameApp.fSetSimulate( true );
				//gameApp.mCurrentLevelLoadInfo.mGameMode.fRemoveOnlineFlag( );

				// Reset non-local profiles
				const tDynamicArray<tPlayerPtr> & players = gameApp.fPlayers( );
				const u32 playerCount = players.fCount( );
				for( u32 p = 0; p < playerCount; ++p )
				{
					if( !players[ p ]->fUser( )->fIsLocal( ) )
						players[ p ]->fResetProfile( );
				}
			}

			mClientData.fSetCount( 0 ); // clear the client data
		}

		// Single player replay needs a fixed time step and synced seed
		else if( mReplayer )
		{
			tRandom::fSyncObjectiveSeed( mObjectiveSeed );
			gameApp.fSetSimulate( true );
			gameApp.fSetFixedTimeStep( true );
		}

		// Regular single player
		else
		{
			// Null out the local users pads so no input is read from the load screen
			Input::tGamepad::tStateData nullData;
			const tUserArray & localUsers = gameApp.fLocalUsers( );
			const u32 localUserCount = localUsers.fCount( );
			for( u32 u = 0; u < localUserCount; ++u )
			{
				const tUserPtr & user = localUsers[ u ];
				user->fAddGamepadState( nullData );
				user->fAddGamepadState( nullData );
			}

			gameApp.fSetSimulate( true );
			gameApp.fSetFixedTimeStep( true );
		}

		// Create the playcorder
		fCreatePlaycorder( );

		tFilePathPtr logPath, comparePath;
		if( mPlaycorder )
			logPath = mPlaycorder->fSyncPath( );
		if( mReplayer )
			comparePath = mReplayer->fSyncPath( );

		if( !gameApp.fGameMode( ).fIsFrontEnd( ) )
		{
			sync_start( logPath, comparePath );
		}
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fOnInGameSimulationState( b32 inSim )
	{
		if( !inSim )
			return;

		tGameApp & gameApp = tGameApp::fInstance( );
		if( gameApp.fCurrentLevelLoadInfo( ).mGameMode.fIsNet( ) && 
			mGameSessionNetwork.fIsRunningWithConnections( ) )
			gameApp.fSetSimulate( false );
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fLoadReplay( const tFilePathPtr & path )
	{
		sigassert( !mLoadingLevel && "Cannot replay while loading level" );

		std::string error = "Replay not implemented";
		tGameApp & gameApp = tGameApp::fInstance( );

		// Load contexts
		for( ;; )
		{
			if( !FileSystem::fFileExists( path ) )
			{
				error = "Replay file does not exist";
				break;
			}

			mReplayer.fReset( NEW tReplayer( ) );

			if( !mReplayer->fOpen( path ) )
			{
				error = "Could not open replay path";
				break;
			}

			const tDynamicArray<byte>& contextData = mReplayer->fGetContext( cReplayContextInfo );
			tGameArchiveLoad archive( contextData.fBegin( ), contextData.fCount( ) );
			
			tReplayContextInfo context;
			archive.fSaveLoad( context );

			// If it's networked we'll need to do some session setup
			if( context.mLevelInfo.mGameMode.fIsNet( ) )
			{
				sigassert( context.mUsers.fCount( ) == 2 && "Only 1v1 net games are supported in replays" );
				const tReplayContextInfo::tReplayUser & hostUser = context.mUsers[ context.mHostUserIndex ];
				const tReplayContextInfo::tReplayUser & clientUser = context.mUsers[ !context.mHostUserIndex ];

				// TODO: Find the best user
				tUser & user = *gameApp.fGetPlayer( 0 )->fUser( );

				const u32 gameType = tUser::cUserContextGameTypeStandard;
				const u32 gameMode = GameSession::cContextGameModeReplay;
				//const u32 gameMode = GameSession::cContextGameModeVersus;

				// Contexts
				{
					user.fSetContext( tUser::cUserContextGameType, gameType );
					user.fSetContext( tUser::cUserContextGameMode, gameMode );
				}

				// Replay propertys
				{
					tUserData data; data.fSet( (s64)hostUser.mUserInfo.mUserId );
					user.fSetProperty( GameSession::cPropertyReplayHostId, data );

					data.fSet( (s64)clientUser.mUserInfo.mUserId );
					user.fSetProperty( GameSession::cPropertyReplayClientId, data );
				}

				user.fWaitForOperations( ); // Since we can, just wait for the ops to finish

				const u32 createFlags = tGameSession::cCreateJoinInProgressDisabled |
										tGameSession::cCreateUsesMatchmaking |
										tGameSession::cCreateUsesPeerNetwork;

				// We were the host
				if( hostUser.mWasLocal )
				{
					b32 success = mGameSessionNetwork.fHost( user, createFlags, 1, 1 );
					sigassert( success && "Failed to host game session network" );
					mStartingReplaySession = true;
				}

				// We were the client
				else
				{
					// Search for our replay game
					while( 1 )
					{
						tGameSessionSearchPtr search( NEW tGameSessionSearch( ) );

						// Properties
						{
							tUserData data; data.fSet( (s64)hostUser.mUserInfo.mUserId );
							search->fSetProperty( GameSession::cPropertyReplayHostId, data );

							data.fSet( (s64)clientUser.mUserInfo.mUserId );
							search->fSetProperty( GameSession::cPropertyReplayClientId, data );
						}

						const u32 procedure = GameSession::cMatchQueryReplay;
						//const u32 procedure = GameSession::cMatchQueryStandard;
						search->fBegin( user.fLocalHwIndex( ), gameType, gameMode, procedure, 1, 1 );

						while( search->fSearching( ) )
							search->fTick( );

						sigassert( !search->fFailed( ) && "Search failed in replay mode" );

						const u32 idx = search->fFindOpenResult( 1 );

						// If we've found the host of the replay set the data and break
						if( idx != ~0 )
						{
							b32 success = mGameSessionNetwork.fJoin( user, search->fResult( 0 ) );
							sigassert( success && "Failed to join game session network" );
							mStartingReplaySession = true;
							break;
						}

						// TODO: add a timeout.
					}
				}

				// Now we grind till someone connects and we can start the level
				// TODO: add a timeout
				while( !mGameSessionNetwork.fIsReadyAndFull( ) )
				{
					mGameSessionNetwork.fWait( );
					mGameSessionNetwork.fTick( 0 );
				}

				// Set the session event to the standard callback now
				mStartingReplaySession = false;
				fResetInput( );
			}

			mLoadingLevel = true;
			gameApp.fSetFixedTimeStep( true );

			mObjectiveSeed = context.mObjectiveSeed;
			gameApp.fLoadLevel( context.mLevelInfo );
			return;
		}

		log_warning( 0, error << " - Quitting" );

		mReplayer.fRelease( );
		gameApp.fQuitAsync( );
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fCreateSessionForPreview( )
	{
		tGameApp& gameApp = tGameApp::fInstance( );

		sigassert( gameApp.fLoadingLevelLoadInfo( mLevelUnloaded ).mPreview );

		if( gameApp.fLoadingLevelLoadInfo( mLevelUnloaded ).mGameMode.fInvalid( ) )
			return; // no next level TODO HAXOR?

		// setup new game session for previews if possible
		if( !gameApp.fLoadingLevelLoadInfo( mLevelUnloaded ).mGameMode.fIsFrontEnd( ) )
		{
			if( gameApp.fLoadingLevelLoadInfo( mLevelUnloaded ).mGameMode.fIsSinglePlayer( ) )
			{
				log_line( 0, "Creating game session for single player preview level" );

				tUser & user = *gameApp.fGetPlayer( 0 )->fUser( );
				user.fSetContext( tUser::cUserContextGameType, tUser::cUserContextGameTypeStandard );
				user.fSetContext( tUser::cUserContextGameMode, GameSession::cContextGameModeCampaign );

				fCreateSinglePlayerSession( user );
				return;
			}
			else
			{
				log_line( 0, "Sessions for preview levels only supported in single player modes" );
			}
		}
		else
		{
			log_line( 0, "Game Mode does not require a game session - no session created." );
		}

		if( mGameSessionNetwork.fIsInactive( ) )
		{
			// no game session required, go directly into loading next level
			tGameApp::fInstance( ).fLoadLevelNowThatSessionIsReady( );
		}
	}

	//------------------------------------------------------------------------------
	u32	tGameAppSession::fLocalUserCount( ) const
	{
		return mGameSessionNetwork.fLocalUserCount( );
	}

	//------------------------------------------------------------------------------
	tUser * tGameAppSession::fLocalUser( u32 i ) const
	{
		return mGameSessionNetwork.fLocalUser( i ).fGetRawPtr( );
	}

	//------------------------------------------------------------------------------
	u32	tGameAppSession::fRemoteUserCount( ) const
	{
		return mGameSessionNetwork.fRemoteUserCount( );
	}

	//------------------------------------------------------------------------------
	tUser *	tGameAppSession::fRemoteUser( u32 i ) const
	{
		return mGameSessionNetwork.fRemoteUser( i ).fGetRawPtr( );
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fAddLocalUserToSession( tUser * user )
	{
		sigassert( user->fIsLocal( ) );
		mGameSessionNetwork.fAddLocalUser( *user, false );
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fAddLocalZombieToSession( tUser * user )
	{
		sigassert( user->fIsLocal( ) );
		mGameSessionNetwork.fAddLocalUser( *user, true );
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fRemoveLocalUserFromSession( tUser * user )
	{
		sigassert( user->fIsLocal( ) );
		mGameSessionNetwork.fRemoveLocalUser( *user );
	}

	//------------------------------------------------------------------------------
	b32	tGameAppSession::fIsOnline( ) const
	{
		return mGameSessionNetwork.fIsOnline( );
	}

	//------------------------------------------------------------------------------
	bool tGameAppSession::fIsHost( ) const
	{
		return mGameSessionNetwork.fIsHost( ) ? true : false;
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fBeginGameSearch( tUser& user, u32 gameType, u32 gameMode )
	{
		sigassert( !mGameSessionSearch || !mGameSessionSearch->fSearching( ) );

		mGameSessionSearch.fReset( NEW tGameSessionSearch( ) );


		// We don't support ranked matches
		gameType = tUser::cUserContextGameTypeStandard;
		
		user.fSetContext( tUser::cUserContextGameType, gameType );
		user.fSetContext( tUser::cUserContextGameMode, gameMode );

		b32 success = mGameSessionSearch->fBegin( 
			user.fLocalHwIndex( ), 
			gameType, 
			gameMode, 
			GameSession::cMatchQueryStandard, 
			cResultCount, 
			cUserCount );

		sigassert( success && "Game session search failed to start" );
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fJoinSessionFromInvite( const tGameInvite & invite )
	{
		// Quick drop any current session
		mGameSessionNetwork.fCancelAndLeave( );

		mIsQuickMatch = false;

		// Join the new session
		b32 success = mGameSessionNetwork.fJoin( 
			*tGameApp::fInstance( ).fLocalUsers( )[ invite.mLocalHwIndex ],
			invite
		);

		sigassert( success && "Failed to join game session network from invite" );
	}
	
	//------------------------------------------------------------------------------
	void tGameAppSession::fCancelGameSearch( )
	{
		if( mGameSessionSearch )
		{
			mGameSessionSearch->fCancel( );
			mGameSessionSearch.fRelease( );
		}
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fCancelSession( )
	{
		mGameSessionNetwork.fCancelAndLeave( );

		// TODO HAXOR?
		tGameApp::fInstance( ).mNextLevelLoadInfo.mGameMode.fSetInvalid( );
	}

	//------------------------------------------------------------------------------
	u32 tGameAppSession::fFirstJoinableSession( ) const
	{
		if( mGameSessionSearch )
			return mGameSessionSearch->fFindOpenResult( );

		return ~0;
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fSyncFrame( )
	{
		// Only sync the frame if it's in progress
		if( !sync_in_progress( ) )
			return;

		tGameApp & gameApp = tGameApp::fInstance( );

		// If this is a net game and we didn't simulate this frame then buffer up
		if( gameApp.fGameMode( ).fIsNet( ) && !mHasAllInput )
			return;

		// We cannot be processing threads in the following code, so if we're running the sync engine
		// sync all the logic frames here
		gameApp.fSceneGraph( )->fWaitForLogicRunListsToComplete( );

		tSync::tFrame syncFrame;
		sync_frame_f( &syncFrame, Game_Sync_Verbose );

		// If we simulated this frame and we're in a networked game session
		if( gameApp.fGameMode( ).fIsNet( ) )
		{
			Net::tSyncCheck * syncCheck = NEW Net::tSyncCheck( );
			syncCheck->mHash = syncFrame.mHash;
			syncCheck->mInputFrame = mApplyInputFrame; // Mark the applied frame this represents

#ifndef sync_system_detect_only

			syncCheck->mBuffer.fSwap( syncFrame.mBuffer );
			syncCheck->mCallstackBuffer.fSwap( syncFrame.mCallstackBuffer );
#endif

			//log_line( 0, 
			//	"Sending SyncCheck - Hash: " << syncCheck.mHash << 
			//	" BufferSize: " << syncCheck.mBuffer.fCount( ) );
			if( mGameSessionNetwork.fIsRunning( ) )
				fSendMsgToPeers( mGameSessionNetwork, *syncCheck, cGameChannelSync, true );

			fPushSyncFrame( syncCheck, true );
		}
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fHostOnlineGame( tUser& user, u32 gameMode, u32 gameType )
	{
		sigassert( mGameSessionNetwork.fIsInactive( ) );
		sigassert( gameType == tUser::cUserContextGameTypeRanked || gameType == tUser::cUserContextGameTypeStandard );

		mIsQuickMatch = false;

		//gameType = tUser::cUserContextGameTypeStandard; //TODO: support arbitration

		// We don't actually support arbitrated sessions
		user.fSetContext( tUser::cUserContextGameType, tUser::cUserContextGameTypeStandard );
		user.fSetContext( tUser::cUserContextGameMode, gameMode );

		// Build game mode flags
		u32 gameModeFlags = tGameMode::cFlagOnline;
		if( gameMode == GameSession::cContextGameModeCampaign )
			gameModeFlags |= tGameMode::cFlagCoOp;
		else if( gameMode == GameSession::cContextGameModeSurvival )
			gameModeFlags |= tGameMode::cFlagCoOp;
		else if( gameMode == GameSession::cContextGameModeMinigame )
			gameModeFlags |= tGameMode::cFlagCoOp;
		else if( gameMode == GameSession::cContextGameModeVersus )
			gameModeFlags |= tGameMode::cFlagVersus;

		tGameApp & gameApp = tGameApp::fInstance( );

		gameApp.mNextLevelLoadInfo.mGameMode.fSetState( 
			tGameMode::cStateNormal, gameModeFlags );

		// Create the session now
		u32 sessionCreateFlags =	tGameSession::cCreateUsesPresence | 
									tGameSession::cCreateUsesStats | 
									tGameSession::cCreateJoinInProgressDisabled;

		// Check the multiplayer privelege, actually being signed in online, and owning the full version
		if( gameApp.fIsFullVersion( ) && user.fSignedInOnline( ) && user.fHasPrivilege( tUser::cPrivilegeMultiplayer ) )
			sessionCreateFlags |= tGameSession::cCreateUsesPeerNetwork;
		else
		{
			sessionCreateFlags |= tGameSession::cCreateJoinViaPresenceDisabled;
			sessionCreateFlags |= tGameSession::cCreateInvitesDisabled;
		}

		u32 privateSlots = 2, publicSlots = 0;
		if( gameType == tUser::cUserContextGameTypeRanked )
		{
			sessionCreateFlags |= tGameSession::cCreateUsesArbitration;
			sessionCreateFlags |= tGameSession::cCreateUsesMatchmaking; // Assumes ranked is for quick match

			// Since it's not ranked now let's not allow invites or join via presence
			sessionCreateFlags |= tGameSession::cCreateJoinViaPresenceDisabled;
			sessionCreateFlags |= tGameSession::cCreateInvitesDisabled;

			privateSlots = 1;
			publicSlots = 1;
		}

		b32 success = mGameSessionNetwork.fHost( user, sessionCreateFlags, publicSlots, privateSlots );
		sigassert( success && " Could not host game session network" );
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fJoinRemoteSessionFromSearchIndex( tUser& user, u32 sessionIndex )
	{
		sigassert( mGameSessionNetwork.fIsInactive( ) );

		fCreateJoinSession( user, sessionIndex );	
	}

	u32 tGameAppSession::fRandomQuickMatchLevelIndex( u32 mapType )
	{
		if( mapType != GameFlags::cMAP_TYPE_HEADTOHEAD && mapType != GameFlags::cMAP_TYPE_SURVIVAL )
		{
			log_warning( 0, "Invalid map type passed to tGameAppSession::fStartQuickMatchMap (" << mapType << ")" );
			return ~0;
		}

		if( !mGameSessionNetwork.fIsReadyAndIdle( ) )
		{
			log_warning( 0, "Failed to start quick match map because session network was not ready" );
			return ~0;
		}

		const u32 addOns = fIntersectedAddOns( );

		u32 mapIndex = tGameApp::fInstance( ).fRandomLevelIndex( mQuickMatchMapRand[ mapType ], ( GameFlags::tMAP_TYPE )mapType, addOns );

		if( Game_Net_LvlIdx != -1 )
			mapIndex = Game_Net_LvlIdx;

		if( mapIndex == ~0 )
			log_warning( 0, "Couldn't select a valid random map in tGameAppSession::fStartQuickMatchMap (mapType = " << mapType << ")" );

		return mapIndex;
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fStartQuickMatchMap( u32 mapType, u32 mapIndex, Sqrat::Function& levelLoadInfoCallback )
	{
		fStartMap( mapType, mapIndex, levelLoadInfoCallback );
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fAddGameModeFlags( tLevelLoadInfo& info )
	{
		info.mGameMode.fAddOnlineFlag( );
		const u32 sessionGameMode = mGameSessionNetwork.fSessionDetails( ).mGameMode;
		if( sessionGameMode == GameSession::cContextGameModeCampaign )
			info.mGameMode.fAddCoOpFlag( );
		else if( sessionGameMode == GameSession::cContextGameModeSurvival )
			info.mGameMode.fAddCoOpFlag( );
		else if( sessionGameMode == GameSession::cContextGameModeMinigame )
			info.mGameMode.fAddCoOpFlag( );
		else if( sessionGameMode == GameSession::cContextGameModeVersus )
			info.mGameMode.fAddVersusFlag( );
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fStartMap( u32 mapType, u32 mapIndex, Sqrat::Function& loadLevelInfoCallback )
	{
		tLevelLoadInfo info = tGameApp::fInstance( ).fQueryLevelLoadInfoFromTable( ( GameFlags::tMAP_TYPE )mapType, mapIndex );
		if( !loadLevelInfoCallback.IsNull( ) )
			loadLevelInfoCallback.Execute( &info );

		fAddGameModeFlags( info );

		// Build and send the map packet
		{
			tGameApp & gameApp = tGameApp::fInstance( );

			Net::tStartMap startMap;
			startMap.mLoadLevelInfo = info;
			startMap.mObjectiveSeed = mObjectiveSeed = tRandom().fSubjectiveRand( ).fUInt( );

			fSendMsgToPeers( mGameSessionNetwork, startMap, cGameChannelDefault, true );
		}

		fLoadMapForNetGame( info );
	}

	void tGameAppSession::fRestartMap( )
	{
		sigassert( fIsHost( ) );

		// Build and send the map packet
		tGameApp & gameApp = tGameApp::fInstance( );

		Net::tStartMap startMap;
		startMap.mObjectiveSeed = mObjectiveSeed = tRandom().fSubjectiveRand( ).fUInt( );
		startMap.mType = Net::tStartMap::cRestart;

		fSendMsgToPeers( mGameSessionNetwork, startMap, cGameChannelDefault, true );

		fReloadMapForNetGame( );
	}

	void tGameAppSession::fStartNextMap( )
	{
		// Build and send the map packet
		tGameApp & gameApp = tGameApp::fInstance( );

		Net::tStartMap startMap;
		startMap.mObjectiveSeed = mObjectiveSeed = tRandom().fSubjectiveRand( ).fUInt( );
		startMap.mType = Net::tStartMap::cNext;

		fSendMsgToPeers( mGameSessionNetwork, startMap, cGameChannelDefault, true );

		fLoadNextMapForNetGame( );
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fQueryRemoteUsers( tUserArray & out )
	{
		const u32 remoteUserCount = mGameSessionNetwork.fRemoteUserCount( );
		for( u32 u = 0; u < remoteUserCount; ++u )
			out.fPushBack( mGameSessionNetwork.fRemoteUser( u ) );
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fOnUserSignInChange( 
			const tUserSigninInfo oldStates[], 
			const tUserSigninInfo newStates[] )
	{
		tGameApp & gameApp = tGameApp::fInstance( );
		if( !mGameSessionNetwork.fOnUserSignInChange( oldStates, newStates ) )
		{
			// Safety in case this happens when we're in a state of shite internet
			gameApp.fSetSimulate( true );

			if( mLoadingLevel )
				gameApp.fLoadLevelNowThatSessionIsReady( );
		}

		//if( !mGameSessionNetwork.fOnUserSignInChange( oldStates, newStates ) )
		//	fSendSessionEvent( ApplicationEvent::cSessionDropped, false );

		
		const tDynamicArray< tPlayerPtr > players = gameApp.fPlayers( );
		const u32 playerCount = players.fCount( );
		for( u32 p = 0; p < playerCount; ++p )
		{
			const tPlayerPtr & player = players[ p ];
			if( !player->fUser( )->fIsLocal( ) )
				continue;

			// Player user was never signed in
			if( !player->fHasValidUserId( ) )
			{
				// Refresh the cached user ids
				player->fRefreshUserIds( );

				// Is he signed in now?
				if( player->fHasValidUserId( ) )
					fSendSessionEvent( ApplicationEvent::cOnPlayerSignIn, p );
			}

			// If the player offline user ids don't match then
			// this player does not have the user it originally pointed to
			else if( !player->fOfflineUserIdsMatch( ) )
			{
				player->fRefreshUserIds( );
				fSendSessionEvent( ApplicationEvent::cOnPlayerSignOut, p );
			}

			// If the player user ids don't match then this
			// player has signed in or out of live. We checked fo general
			// signed out above.
			else if( !player->fUserIdsMatch( ) )
			{
				u32 userIndex = player->fUser( )->fLocalHwIndex( );
				player->fRefreshUserIds( );

				// This means we went from live to no-live.
				if( newStates[ userIndex ].mState == tUser::cSignInStateSignedInLocally )
					fSendSessionEvent( ApplicationEvent::cOnPlayerNoLive, p  );

				// Otherwise we went from no-live to live.
				else
					fSendSessionEvent( ApplicationEvent::cOnPlayerYesLive, p  );
			}
		}

		//fSendSessionEvent( ApplicationEvent::cOnUserSignInChange, false );
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fOnMuteListChanged( )
	{
		mGameSessionNetwork.fOnMuteListChanged( );
	}

	//------------------------------------------------------------------------------
	b32 tGameAppSession::fCanAcceptInvite( ) const
	{
		if( mGameSessionNetwork.fState( ) == tGameSessionNetwork::cStateInactive )
			return true;

		if( !mGameSessionNetwork.fHasSession( ) )
			return true;

		if( mGameSessionNetwork.fIsSameSession( tGameApp::fInstance( ).mGameInvite->fSessionInfo( ) ) )
			return false;

		return true;
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fOnGameInviteAccepted( )
	{
		tGameApp & gameApp = tGameApp::fInstance( );

		fSendSessionEvent( 
			ApplicationEvent::cSessionInviteAccepted, 
			gameApp.mGameInvite->mLocalHwIndex );
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fOnGameInviteRejected( u32 localHwIndex )
	{
		fSendSessionEvent( ApplicationEvent::cSessionInviteRejected, localHwIndex );
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fOnGameInviteNeedFullVer( )
	{
		tGameApp & gameApp = tGameApp::fInstance( );

		fSendSessionEvent( 
			ApplicationEvent::cSessionInviteNeedFullVer, 
			gameApp.mGameInvite->mLocalHwIndex );
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fQueueErrorDialog( u32 error, tPlatformUserId userId )
	{
		sigassert( userId != tUser::cInvalidUserId );

		tErrorDialogInfo info;
		info.mErrorCode = error;
		info.mUserId = userId;

		mErrorPrequeue.fPushBack( info );
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fLoadMapForNetGame( tLevelLoadInfo info )
	{
		tGameApp& gameApp = tGameApp::fInstance( );

		// Reset the input here because once we're loading we have a fixed session
		fResetInput( );

		fSendSessionEvent( ApplicationEvent::cSessionLoadLevel, true );

		tStringPtr netLvlOverride = Game_Net_LvlOverride;
		if( !netLvlOverride.fNull( ) )
			info = gameApp.fQueryLevelLoadInfoFromPath( tFilePathPtr( netLvlOverride ) );

		gameApp.fLoadLevel( info );
	}

	void tGameAppSession::fReloadMapForNetGame( )
	{
		tGameApp& gameApp = tGameApp::fInstance( );

		// Reset the input here because once we're loading we have a fixed session
		fResetInput( );

		fSendSessionEvent( ApplicationEvent::cSessionLoadLevel, true );

		gameApp.fReloadCurrentLevel( );
	}

	void tGameAppSession::fLoadNextMapForNetGame( )
	{
		tGameApp& gameApp = tGameApp::fInstance( );

		// Reset the input here because once we're loading we have a fixed session
		fResetInput( );

		fSendSessionEvent( ApplicationEvent::cSessionLoadLevel, true );

		Sqrat::Function f( gameApp.fCurrentLevel( )->fOwnerEntity( )->fScriptLogicObject( ), "LoadNextLevel" );
		if( !f.IsNull( ) )
			f.Execute( );
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fWriteStatFromScript( tPlayer * player, u32 viewId, u32 propId, u32 value )
	{
		tUserProperty prop;
		prop.mId = propId;
		prop.mData.fSet( (s64) value );

		fWriteStats( player, viewId, 1, &prop );
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fCreateSinglePlayerSession( tUser & user )
	{
		mIsQuickMatch = false;

		b32 success = mGameSessionNetwork.fHost( user, tGameSession::cCreateSinglePlayerWithStats, 0, 1 );
		sigassert( success && "Could not host single player game session network" );
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fCreateJoinSession( tUser & user, u32 resultIndex )
	{
		sigassert( mGameSessionSearch && mGameSessionSearch->fSucceeded( ) );

		mIsQuickMatch = false;

		b32 success = mGameSessionNetwork.fJoin( user, mGameSessionSearch->fResult( resultIndex ) );
		sigassert( success && " Failed to join game session network" );
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fCreatePlaycorder( )
	{
		if( !Game_Record || !mGameSessionNetwork.fHasSession( ) || mReplayer )
			return;

		tGameApp & gameApp = tGameApp::fInstance( );

		gameApp.fSetFixedTimeStep( true );

		// TODO: Parameterize creation 
		sigassert( gameApp.fPlayers( ).fCount( ) == mGameSessionNetwork.fFilledSlots( ) );

		std::string sessionName = mGameSessionNetwork.fSessionName( );
		std::string userName; 
		{
			std::stringstream ss;
			ss << std::hex << gameApp.fFrontEndPlayer( )->fUser( )->fPlatformId( );
			userName = ss.str( );
		}

		mPlaycorder.fReset( NEW tPlaycorder( 
			cReplayContextCount, 
			gameApp.fPlayers( ).fCount( ), 
			gameApp.fResourceDepot( )->fRootPath( ),
			gameApp.fCurrentLevelLoadInfo( ).mGameMode.fIsNet( ) ? 2 : 1,
			sessionName.c_str( ),
			userName.c_str( ),
			Game_Record_Sync ) );

		// Archive for contexts
		tGameArchiveSave archive;

		// cReplayContextInfo
		{
			tReplayContextInfo context;
			archive.fBuffer( ).fSetCount( 0 );

			// Session info
			context.mObjectiveSeed = tRandom::fObjectiveRand( ).fState( );
			context.mLevelInfo = gameApp.fCurrentLevelLoadInfo( );

			context.mHostUserIndex = ~0;
			const tPlatformUserId hostId = mGameSessionNetwork.fHostUser( )->fPlatformId( );

			// User info
			const u32 userCount = gameApp.fPlayers( ).fCount( );
			context.mUsers.fNewArray( gameApp.fPlayers( ).fCount( ) );
			for( u32 u = 0; u < userCount; ++u )
			{
				const tPlayerPtr & ptr = gameApp.fPlayers( )[ u ];
				tReplayContextInfo::tReplayUser & user = context.mUsers[ u ];

				user.mUserInfo = ptr->fUser( )->fUserInfo( );
				user.mWasLocal = ptr->fUser( )->fIsLocal( );

				if( hostId == user.mUserInfo.mUserId )
					context.mHostUserIndex = u;
			}

			sigassert( context.mHostUserIndex != ~0 && "Host user could not be found" );

			archive.fSaveLoad( context );
			mPlaycorder->fSetContext( 
				cReplayContextInfo, 
				archive.fBuffer( ).fTotalSizeOf( ), 
				archive.fBuffer( ).fBegin( ) );
		}
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fRecordInputFrame( )
	{
		if( !mPlaycorder )
			return;

		mPlaycorder->fMarkInputFrame( );

		const tDynamicArray< tPlayerPtr > & players = tGameApp::fInstance( ).fPlayers( );
		const u32 playerCount = players.fCount( );
		for( u32 p = 0; p < playerCount; ++p )
			mPlaycorder->fSetInput( p, players[ p ]->fUser( )->fGetGamepadState( ) );
	}

	//------------------------------------------------------------------------------
	u32 tGameAppSession::fIntersectedAddOns( ) const
	{
		u32 combinedLicense = 0;

		// combine licenses
		const u32 localUserCount = mGameSessionNetwork.fLocalUserCount( );
		for( u32 u = 0; u < localUserCount; ++u )
			combinedLicense |= mGameSessionNetwork.fLocalUser( u )->fAddOnsLicensed( );
		const u32 remoteUserCount = mGameSessionNetwork.fRemoteUserCount( );
		for( u32 u = 0; u < remoteUserCount; ++u )
			combinedLicense |= mGameSessionNetwork.fRemoteUser( u )->fAddOnsLicensed( );

		// remove uninstalled licenses.
		for( u32 u = 0; u < localUserCount; ++u )
			combinedLicense &= mGameSessionNetwork.fLocalUser( u )->fAddOnsInstalled( );
		for( u32 u = 0; u < remoteUserCount; ++u )
			combinedLicense &= mGameSessionNetwork.fRemoteUser( u )->fAddOnsInstalled( );

		return combinedLicense;
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fWriteLeaderboards( )
	{
		tGameApp & gameApp = tGameApp::fInstance( );

		// Needs a level logic
		tLevelLogic* level = gameApp.fCurrentLevel( );
		if( !level || level->fMapType( ) >= GameFlags::cMAP_TYPE_COUNT )
			return;

		// Only full versions can write to leaderboards
		if( !gameApp.fIsFullVersion( ) )
		{
			// Flag players as needing to write cached score data
			for( u32 p = 0; p < tGameApp::fInstance( ).fPlayers( ).fCount( ); ++p )
			{
				const tPlayerPtr & player = tGameApp::fInstance( ).fPlayers( )[ p ];
				player->fProfile( )->fSetSetting( tUserProfile::cSettingsScoresCached, true );
			}
			return;
		}

		b32 isCampaign = ( level->fMapType( ) == GameFlags::cMAP_TYPE_CAMPAIGN );
		b32 arbitrated = mGameSessionNetwork.fHasSession( ) ? ( mGameSessionNetwork.fSessionCreateFlags( ) & tGameSession::cCreateUsesArbitration ) != 0 : false;

		const tDynamicArray< tPlayerPtr > & players = gameApp.fPlayers( );
		const u32 playerCount = players.fCount( );

		u32 levelDiff = gameApp.fDifficulty( );
		if( gameApp.fCurrentLevelLoadInfo( ).mMapType == GameFlags::cMAP_TYPE_SURVIVAL )
		{
			levelDiff = gameApp.fCurrentLevelLoadInfo( ).mChallengeMode;
		}
		else if( gameApp.fCurrentLevelLoadInfo( ).mMapType == GameFlags::cMAP_TYPE_MINIGAME )
		{
			levelDiff = 0;
		}

		// grab a bunch of data to write, then we're going to correlate it
		struct tPlayerData
		{
			tGrowableArray<tGameSessionViewProperties> mWrites;
			tScoreProps mProps;
			tPlayer* mPlayer;
		};
		tGrowableArray<tPlayerData> playerData;
		playerData.fSetCount( playerCount );

		for( u32 p = 0; p < playerCount; ++p )
		{
			const tPlayerPtr & player = players[ p ];
			playerData[ p ].mPlayer = player.fGetRawPtr( );

			if( !arbitrated && !player->fUser( )->fIsLocal( ) )
				continue;

			tLevelScores* scores = level->fLevelScores( player.fGetRawPtr( ) );
			fCreateScoreWrites( scores, levelDiff, player.fGetRawPtr( ), playerData[ p ].mProps, playerData[ p ].mWrites, true );
		}

		tPlayerData* mostStats = &playerData[ 0 ];
		tPlayerData* otherStats = playerCount > 1 ? &playerData[ 1 ] : NULL;

		if( otherStats && otherStats->mWrites.fCount( ) > mostStats->mWrites.fCount( ) )
			fSwap( mostStats, otherStats );
		sigassert( mostStats );

		// write the stats correlated by board. assuming that the arrays are the same, if they're not, it should be good enough.
		for( u32 i = 0; i < mostStats->mWrites.fCount( ); ++i )
		{
			fWriteStatsToRightPlace( mostStats->mPlayer, 1, &mostStats->mWrites[ i ] );
			if( otherStats && i < otherStats->mWrites.fCount( ) )
				fWriteStatsToRightPlace( otherStats->mPlayer, 1, &otherStats->mWrites[ i ] );
		}
	}

	void tGameAppSession::fWriteStatsToRightPlace( tPlayer* player, u32 writeCount, const tGameSessionViewProperties writes[] )
	{
		if( mGameSessionNetwork.fHasSession( ) )
		{
			mGameSessionNetwork.fWriteStats( 
				player->fUser( )->fPlatformId( ),
				writeCount,
				writes );
		}
		else
		{
			tGameApp::fInstance( ).fCachedScoreWriter( ).fQueueStats(
				player->fUser( )->fPlatformId( ),
				writeCount,
				writes );
		}
	}

	void tGameAppSession::fCreateScoreWrites( tLevelScores* scores, u32 difficulty, const tPlayer* player, tScoreProps& props, tGrowableArray<tGameSessionViewProperties>& writes, b32 fromSession )
	{
		// ! this function is not intended to write stats for minigames ! //
		if( scores->fGetMapType( ) == GameFlags::cMAP_TYPE_MINIGAME )
			return;

		if( scores->fGetMapType( ) == GameFlags::cMAP_TYPE_HEADTOHEAD )
		{
			if( !fromSession )
				return; // Can't write stats from cached data for H2H match types because we don't cache wins or loses

			if( !mIsQuickMatch )
				return; //dont write scores for private matches.
			props.fPushBack( tGrowableArray<tUserProperty>() );
			tGrowableArray<tUserProperty>& back = props.fBack( );
			back.fPushBack( tUserProperty( GameSession::cPropertyWinLose ) );
			back.fPushBack( tUserProperty( GameSession::cPropertyWins ) );
			back.fPushBack( tUserProperty( GameSession::cPropertyLosses ) );


			b32 victory = player->fVictorious( );

			// TODO: Does this work without adding the 32 bit parts separately?
			back[ 0 ].mData.fSet( ( s64 ) ( victory ? (1ull << 32 ) : 1ull ) );
			back[ 1 ].mData.fSet( ( s64 ) ( victory ? 1ull : 0ull ) );
			back[ 2 ].mData.fSet( ( s64 ) ( victory ? 0ull : 1ull ) );

			writes.fPushBack( tGameSessionViewProperties( ) );
			writes.fBack( ).mViewId = GameSession::cLeaderboardH2HTotals;
			writes.fBack( ).mNumProperties = back.fCount( );
			writes.fBack( ).mProperties = back.fBegin( );

			writes.fPushBack( tGameSessionViewProperties( ) );
			writes.fBack( ).mViewId = GameSession::cLeaderboardH2H[ scores->fGetIndex( ) ];
			writes.fBack( ).mNumProperties = back.fCount( );
			writes.fBack( ).mProperties = back.fBegin( );
		}
		else
		{
			sigassert( scores );

			b32 victory = player->fVictorious( );

			u32 challengeProgress = scores->fRankProgress( );
			u32 overallMedal = (u32)scores->fMedalProgress( difficulty, tLevelScores::cOverallMedal );
			u32 levelScore = (u32)player->fStats( ).fStat( GameFlags::cSESSION_STATS_SCORE );
			u32 totalScore = (u32)player->fProfile( )->fGetTotalLevelScore( );
			s64 win = ( s64 ) ( victory ? 1 : 0 );
			s64 lose = ( s64 ) ( victory ? 0 : 1 );
			s64 kills = ( s64 ) player->fStats( ).fStat( GameFlags::cSESSION_STATS_KILLS );
			f64 time = ( f64 ) player->fStats( ).fStat( GameFlags::cSESSION_STATS_TOTAL_TIME );
			s64 money = ( s64 ) player->fInGameMoney( );

			if( !fromSession )
			{
				// If these stats aren't from the current session grab what we can from the profile data and write it to the leaderboards
				// score, medal, and challenge progress are saved in the profile everything else can only be written as zero
				// we can't not write the other stats because they will show up on the leaderboards at {NULL}
				levelScore = scores->fGetHighScore( difficulty );
				win = 0;
				lose = 0;
				kills = 0;
				time = 0;
			}

			// This is updating the props for the overalls leaderboard
			props.fPushBack( tGrowableArray<tUserProperty>() );
			tGrowableArray<tUserProperty>& arcadeProps = props.fBack( );
			arcadeProps.fPushBack( tUserProperty( GameSession::cPropertyScore ) );
			arcadeProps.fPushBack( tUserProperty( GameSession::cPropertyKills ) );
			arcadeProps.fPushBack( tUserProperty( GameSession::cPropertyMoney ) );

			// This is updating the props for each individual level
			props.fPushBack( tGrowableArray<tUserProperty>() );
			tGrowableArray<tUserProperty>& levelProps = props.fBack( );
			levelProps.fPushBack( tUserProperty( GameSession::cPropertyScore ) );
			levelProps.fPushBack( tUserProperty( GameSession::cPropertyWins ) );
			levelProps.fPushBack( tUserProperty( GameSession::cPropertyLosses ) );
			levelProps.fPushBack( tUserProperty( GameSession::cPropertyMedal ) );
			levelProps.fPushBack( tUserProperty( GameSession::cPropertyKills ) );
			levelProps.fPushBack( tUserProperty( GameSession::cPropertyTime ) );
			levelProps.fPushBack( tUserProperty( GameSession::cPropertyChallengeProgress ) );

			arcadeProps[ 0 ].mData.fSet( ( s64 ) totalScore );
			arcadeProps[ 1 ].mData.fSet( ( s64 ) kills );
			arcadeProps[ 2 ].mData.fSet( ( s64 ) money );
			writes.fPushBack( tGameSessionViewProperties( GameSession::cLeaderboardArcade, arcadeProps.fCount( ), arcadeProps.fBegin( ) ) );

			levelProps[ 0 ].mData.fSet( ( s64 ) levelScore );
			levelProps[ 1 ].mData.fSet( ( s64 ) win );
			levelProps[ 2 ].mData.fSet( ( s64 ) lose );
			levelProps[ 3 ].mData.fSet( ( s64 ) overallMedal );
			levelProps[ 4 ].mData.fSet( ( s64 ) kills );
			levelProps[ 5 ].mData.fSet( ( f64 ) time );
			levelProps[ 6 ].mData.fSet( ( s64 ) challengeProgress );

			u32 levelDiffMax = GameFlags::cDIFFICULTY_COUNT;

			if( scores->fGetMapType( ) == GameFlags::cMAP_TYPE_SURVIVAL )
			{
				levelDiffMax = GameFlags::cCHALLENGE_MODE_COUNT;
			}
			
			writes.fPushBack( tGameSessionViewProperties( fGetLevelLeaderboardId( scores->fGetMapType( ), scores->fGetIndex( ), difficulty ), levelProps.fCount( ), levelProps.fBegin( ) ) );

			//write challenge progress to other difficulties
			for( u32 i = 0; i < levelDiffMax; ++i )
			{
				props.fPushBack( tGrowableArray<tUserProperty>() );
				tGrowableArray<tUserProperty>& challengeProp = props.fBack( );
				if( i == difficulty )
					continue;
				challengeProp.fSetCount( 0 );
				challengeProp.fPushBack( tUserProperty( GameSession::cPropertyChallengeProgress ) );
				challengeProp[ 0 ].mData.fSet( ( s64 ) challengeProgress );
				writes.fPushBack( tGameSessionViewProperties( fGetLevelLeaderboardId( scores->fGetMapType( ), scores->fGetIndex( ), i ), challengeProp.fCount( ), challengeProp.fBegin( ) ) );
			}


			// Need to update the hidden stat for the stats screen
			props.fPushBack( tGrowableArray<tUserProperty>() );
			tGrowableArray<tUserProperty>& statProp = props.fBack( );
			statProp.fPushBack( tUserProperty( GameSession::cPopertyPlayerStats[ GameFlags::cSESSION_STATS_SCORE ] ) );

			statProp[ 0 ].mData.fSet( ( s64 ) totalScore  );
			writes.fPushBack( tGameSessionViewProperties( GameSession::cLeaderboardPlayerStats, statProp.fCount( ), statProp.fBegin( ) ) );
		}
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fWriteStats( tPlayer* player, u32 viewId, u32 propCount, const tUserProperty props[] )
	{
		sigassert( player );

		// Only full versions can write to leaderboards
		if( !tGameApp::fInstance( ).fIsFullVersion( ) )
			return;

		if( mGameSessionNetwork.fIsRunning( ) )
		{
			tGameSessionViewProperties view;

			view.mViewId = viewId;
			view.mNumProperties = propCount;
			view.mProperties = props;
			
			mGameSessionNetwork.fWriteStats(
				player->fUser( )->fPlatformId( ), 1, &view );
		}
		else 
		{
			tGameSessionViewProperties view;

			view.mViewId = viewId;
			view.mNumProperties = propCount;
			view.mProperties = props;

			tGameApp::fInstance( ).fCachedScoreWriter( ).fQueueStats(
				player->fUser( )->fPlatformId( ), 1, &view );
		}
	}
	//------------------------------------------------------------------------------
	void tGameAppSession::fFlushStats()
	{
		if( mGameSessionNetwork.fIsRunning( ) )
			mGameSessionNetwork.fFlushStats( );
		else
			log_warning( 0, "No game session to flush stats." );
	}

	//------------------------------------------------------------------------------
	u32 tGameAppSession::fGameMode( ) const
	{
		if( mGameSessionNetwork.fHasSession( ) )
			return mGameSessionNetwork.fSessionDetails( ).mGameMode;

		return ~0;
	}

	//------------------------------------------------------------------------------
	u32 tGameAppSession::fGameType( ) const
	{
		if( mGameSessionNetwork.fHasSession( ) )
			return mGameSessionNetwork.fSessionDetails( ).mGameType;

		return ~0;
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fResetInput( )
	{
		mApplyInputFrame = mCaptureInputFrame = 0;
		
		fNullifySessionPads( );

		// 1 for everybody
		u32 inputCount = mGameSessionNetwork.fFilledSlots( );
		
		mInput.fSetCount( inputCount );
		const u32 count = mInput.fCount( );
		for( u32 i = 0; i < count; ++i )
		{
			mInput[ i ].mUserId = tUser::cInvalidUserId;

			mInput[ i ].mFrames.fResize( cMaxInputBufferSize );
			mInput[ i ].mFrames.fReset( );
		}

		fResetSyncData( );
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fResetSyncData( )
	{
		// End the sync first
		sync_end( );

		Net::tSyncCheck * check;
		while( mSyncData.mLocalSyncs.fGet( check ) )
			delete check;
		mSyncData.mLocalSyncs.fReset( );

		while( mSyncData.mRemoteSyncs.fGet( check ) )
			delete check;
		mSyncData.mRemoteSyncs.fReset( );
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fNullifySessionPads( )
	{
		// Reset local user gamepad data to a null pad state
		Input::tGamepad::tStateData nullData; 
		const u32 localUserCount = mGameSessionNetwork.fLocalUserCount( );
		for( u32 u = 0; u < localUserCount; ++u )
		{
			const tUserPtr & user = mGameSessionNetwork.fLocalUser( u );
			user->fAddGamepadState( nullData );
			user->fAddGamepadState( nullData );
		}

		// Reset remote user gamepad data to a null pad state
		const u32 remoteUserCount = mGameSessionNetwork.fRemoteUserCount( );
		for( u32 u = 0; u < remoteUserCount; ++u )
		{
			const tUserPtr & user = mGameSessionNetwork.fRemoteUser( u );
			user->fAddGamepadState( nullData );
			user->fAddGamepadState( nullData );
		}
	}

	//------------------------------------------------------------------------------
	b32 tGameAppSession::fHasAllProfiles( ) const
	{
		const u32 userCount = mGameSessionNetwork.fRemoteUserCount( );
		for( u32 u = 0; u < userCount; ++u )
		{
			if( !mClientData.fFind( mGameSessionNetwork.fRemoteUser( u )->fPlatformId( ) ) )
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fSendProfiles( )
	{
		if( mGameSessionNetwork.fRemoteUserCount( ) )
		{	
			Net::tPlayerInfo info;
			const tDynamicArray< tPlayerPtr > & players = tGameApp::fInstance( ).fPlayers( );
			const u32 playerCount = players.fCount( );
			for( u32 p = 0; p < playerCount; ++p )
			{
				tPlayerPtr player = players[ p ];
				if( player->fUser( )->fIsLocal( ) )
				{
					tGameArchiveSave archive;
					player->fProfile( )->fSaveLoad( archive );
					info.mUserId = player->fUser( )->fPlatformId( );
					info.mProfile.fInitialize( archive.fBuffer( ).fBegin( ), archive.fBuffer( ).fTotalSizeOf( ) );

					fSendMsgToPeers( mGameSessionNetwork, info, cGameChannelDefault, true );
				}
			}
		}
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fApplyProfiles( )
	{
		tGameApp & gameApp = tGameApp::fInstance( );

		// Tag any local player inputs as ready
		const tDynamicArray<tPlayerPtr> & players = gameApp.fPlayers( );
		const u32 playerCount = players.fCount( );
		for( u32 p = 0; p < playerCount; ++p )
		{
			if( !players[ p ]->fUser( )->fIsLocal( ) )
			{
				tClientData * data = mClientData.fFind( players[ p ]->fUser( )->fPlatformId( ) );
				if( data && data->mProfile != players[ p ]->fProfile( ) )
				{
					players[ p ]->fSetProfile( data->mProfile.fGetRawPtr( ) );
				}
			}
		}
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fNetRewindBegin( )
	{
		fResetInput( );
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fRewindMap( tUser* user, u32 waveIndex )
	{
		tGameApp & gameApp = tGameApp::fInstance( );

		log_line( 0, "REWINDING MAP TO " << waveIndex );

		std::string path =  tSaveGame::fGenerateFileName( waveIndex, true );
		log_line( Log::cFlagRewind, " LoadingSaveGame: " << path );
		tSaveGameStorageDesc saveDesc( gameApp.fGetPlayerByUser( user )->fSaveDeviceId( ), tStringPtr( path.c_str( ) ), tLocalizedString( L"QuickieSave" ) );
		tSaveGameStorageReader saveGameReader;
		saveGameReader.fBeginReading( tUserPtr( user ), saveDesc );
		while( !saveGameReader.fIsFinished( ) ) //SUPER HAX - BLOCK UNTIL LOAD
			fSleep( 1 );

		if( saveGameReader.fIsErrored( ) )
		{
			fRestartMap( );
			return;
		}

		log_output( 0, this << " tGameAppSession::fRewindMap() " << &saveGameReader << std::endl );

		tDynamicArray<byte> temp;
		saveGameReader.fGetBuffer( temp );
		tGrowableArray<byte> encrypted;
		encrypted.fInsert( 0, temp.fBegin( ), temp.fCount( ) );

		// reading from a save done in tGameApp, this will be encrypted so decrypt it
		tGrowableArray<byte> decrypted;
		tEncryption::fDecrypt( encrypted, decrypted );

		Net::tLoadFromSave loadMsg;
		loadMsg.mSaveGame.fInitialize( 
			decrypted.fBegin( ),
			decrypted.fCount( ) );

		loadMsg.mObjectiveSeed = mObjectiveSeed = tRandom::fSubjectiveRand( ).fUInt( );

		fSendMsgToPeers( mGameSessionNetwork, loadMsg, cGameChannelDefault, true, false );

		fLoadFromSave( decrypted.fBegin( ), decrypted.fCount( ) );

	}

	//------------------------------------------------------------------------------
	b32 tGameAppSession::fReadyForInput( ) const
	{
		return fFindClientInput( tUser::cInvalidUserId ) == NULL;
	}

	//------------------------------------------------------------------------------
	b32 tGameAppSession::fHasAllInputForThisFrame( ) const
	{
		const u32 inputCount = mInput.fCount( );
		for( u32 i = 0; i < inputCount; ++i )
		{
			const tClientInput & input = mInput[ i ];
			if( input.mUserId == tUser::cInvalidUserId )
				return false;

			// Must have enough buffered
			if( input.mFrames.fNumItems( ) < cMinInputBufferSize )
				return false;

			sigassert( input.mFrames.fBack( ).mFrameId == mApplyInputFrame );
		}

		return true;
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fTryCaptureInputFrame( tUserArray & localUsers )
	{
		const u32 captured = mCaptureInputFrame - mApplyInputFrame;

		// If we have maxed out our input frame buffer then don't capture anything
		if( captured >= cMaxInputBufferSize )
			return;

		tGameApp & gameApp = tGameApp::fInstance( );

		// NOT replay
		if( !mReplayer )
		{
			const f32 frameDelta = gameApp.fGetOptions( ).mFixedTimeStep;
			b32 consumedError = false;

			// Capture all the state data of our local users for the capture frame
			const u32 userCount = localUsers.fCount( );
			for( u32 u = 0; u < userCount; ++u )
			{
				tUserPtr user = localUsers[ u ];
				if( !user || !user->fHasValidPlatformId( ) || !user->fIsUsedByGame( ) )
					continue;

				Net::tInputFrame frame;
				frame.mFrameId = mCaptureInputFrame;
				frame.mUserId = user->fPlatformId( );

				// Find an error code to apply if we have one
				const u32 errorCount = mErrorPrequeue.fCount( );
				for( u32 e = 0; e < errorCount; ++e )
				{
					if( mErrorPrequeue[ e ].mUserId == frame.mUserId )
					{
						frame.mErrorCode = mErrorPrequeue[ e ].mErrorCode;
						mErrorPrequeue.fErase( e );

						consumedError = true;
						break;
					}
				}

				Input::tGamepad gamepad;
				gamepad.fCaptureStateUnbuffered( frame.mStateData, user->fLocalHwIndex( ), frameDelta );

				fPushInputFrame( frame );
				fSendMsgToPeers( mGameSessionNetwork, frame, cGameChannelDefault, true );
			}

			sigassert( consumedError || !mErrorPrequeue.fCount( ) );

			++mCaptureInputFrame;
		}

		// Replayed input
		else if( !mLoadingLevel )
		{
			mReplayer->fMarkInputFrame( );

			// When we run out of input we quit
			if( !mReplayer->fHasInputAvailable( ) )
			{
				gameApp.fQuitAsync( );
			}
			else
			{
				const tDynamicArray< tPlayerPtr > & players = gameApp.fPlayers( );
				const u32 playerCount = players.fCount( );

				sigassert( playerCount == mReplayer->fGetInputCount( ) );

				Net::tInputFrame frame;
				frame.mFrameId = mCaptureInputFrame;

				for( u32 p = 0; p < playerCount; ++p )
				{
					if( !players[ p ]->fUser( )->fIsLocal( ) )
						continue;

					frame.mUserId = players[ p ]->fUser( )->fPlatformId( );
					frame.mStateData = mReplayer->fGetInput( p );
					fPushInputFrame( frame );
					fSendMsgToPeers( mGameSessionNetwork, frame, cGameChannelDefault, true );
				}
			}

			++mCaptureInputFrame;
		}
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fTryApplyInputFrame( tUserArray & localUsers )
	{
		tGameApp & gameApp = tGameApp::fInstance( );

		// If we're gonna simulate we need to pull input from the queue
		if( !mHasAllInput )
			return;

		const f32 frameDelta = gameApp.fGetFrameDeltaTime( );
		const u32 localUserCount = localUsers.fCount( );

		// Sized to something longer than possible. We store a list
		// of errors here because we need to push them into the error
		// queue based on the order of the players so that all players
		// get the same errors in the same order
		u32 errorCount = 0;
		tFixedArray< tErrorDialogInfo, 16 > errors;

		for( u32 u = 0; u < localUserCount; ++u )
		{
			tUserPtr user = localUsers[ u ];
			if( !user || !user->fHasValidPlatformId( ) || !user->fIsUsedByGame( ) )
				continue;

			Net::tInputFrame frame; frame.mUserId = user->fPlatformId( );
			fPopInputFrame( frame );

			if( frame.mErrorCode != ~0 )
			{
				tErrorDialogInfo & error = errors[ errorCount++ ];
				error.mUserId = frame.mUserId;
				error.mErrorCode = frame.mErrorCode;
			}

			user->fAddGamepadState( frame.mStateData );
			user->fStepDevMenu( frameDelta );
		}

		tUserArray remoteUsers;
		fQueryRemoteUsers( remoteUsers );

		const u32 remoteUserCount = remoteUsers.fCount( );
		for( u32 u = 0; u < remoteUserCount; ++u )
		{
			tUserPtr user = remoteUsers[ u ];
			if( !user || !user->fPlatformId( ) )
				continue;
			
			Net::tInputFrame frame; frame.mUserId = user->fPlatformId( );
			fPopInputFrame( frame );

			if( frame.mErrorCode != ~0 )
			{
				tErrorDialogInfo & error = errors[ errorCount++ ];
				error.mUserId = frame.mUserId;
				error.mErrorCode = frame.mErrorCode;
			}

			user->fAddGamepadState( frame.mStateData );
			user->fStepDevMenu( frameDelta );
		}

		++mApplyInputFrame;

		// If we had errors then apply them in player order
		if( errorCount )
		{
			tGameApp & gameApp = tGameApp::fInstance( );
			const u32 playerCount = gameApp.fPlayerCount( );
			for( u32 p = 0; p < playerCount; ++p )
			{
				const tPlayer * player = gameApp.fGetPlayer( p );
				const tPlatformUserId userId = player->fUser( )->fPlatformId( );
				for( u32 e = 0; e < errorCount; ++e )
				{
					const tErrorDialogInfo & error = errors[ e ];
					if( userId == error.mUserId )
						mErrorQueue.fPushBack( error );
				}
			}
		}

		// Recording
		fRecordInputFrame( );
	}

	//------------------------------------------------------------------------------
	tGameAppSession::tClientInput * tGameAppSession::fFindClientInput( tPlatformUserId userId )
	{
		// Search for the user input or a free one
		const u32 inputCount = mInput.fCount( );
		for( u32 i = 0; i < inputCount; ++i )
		{
			tClientInput & test = mInput[ i ];
			if( test.mUserId == userId )
				return &test;
		}

		return NULL;
	}

	//------------------------------------------------------------------------------
	const tGameAppSession::tClientInput * tGameAppSession::fFindClientInput( tPlatformUserId userId ) const
	{
		// Search for the user input or a free one
		const u32 inputCount = mInput.fCount( );
		for( u32 i = 0; i < inputCount; ++i )
		{
			const tClientInput & test = mInput[ i ];
			if( test.mUserId == userId )
				return &test;
		}

		return NULL;
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fPushInputFrame( const Net::tInputFrame & frame )
	{
		tClientInput * input = fFindClientInput( frame.mUserId );

		if( !input )
			return;

		sigassert( !input->mFrames.fNumItems( ) || input->mFrames.fFront( ).mFrameId == frame.mFrameId - 1 );
		if( input->mFrames.fNumItems( ) == input->mFrames.fCapacity( ) )
			input->mFrames.fResize( ( input->mFrames.fCapacity( ) + 1 ) * 2 );

		input->mFrames.fPut( frame );
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fPopInputFrame( Net::tInputFrame & frame )
	{
		tClientInput * input = fFindClientInput( frame.mUserId );

		sigassert( input && "Client input could not be found in fPopInputFrame" ); 
		sigassert( input->mFrames.fNumItems( ) && input->mFrames.fBack( ).mFrameId == mApplyInputFrame );

		b32 success = input->mFrames.fGet( frame );

		sigassert( success && "InputFrame could not be popped in fPopInputFrame" );
		sigassert( frame.mUserId == input->mUserId && "Input frame contained in wrong client input" );
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fMarkReadyForInput( tPlatformUserId userId )
	{
		tClientInput * input = NULL;

		// Search for the user input or a free one
		const u32 inputCount = mInput.fCount( );
		for( u32 i = 0; i < inputCount; ++i )
		{
			tClientInput & test = mInput[ i ];
			if( test.mUserId == tUser::cInvalidUserId )
			{
				test.mUserId = userId;
				return;
			}

			if( test.mUserId == userId )
			{
				log_warning( 0, "Client input for" << userId << " already marked as ready" );
				return;
			}
		}

		log_warning( 0, "Client input not available for user " << userId );
	}

	namespace { const tFilePathPtr cFileDesyncs( "Desyncs" ); }

	//------------------------------------------------------------------------------
	void tGameAppSession::fPushSyncFrame( Net::tSyncCheck * syncFrame, b32 local )
	{
		tRingBuffer< Net::tSyncCheck * > * buffer = local ? &mSyncData.mLocalSyncs : &mSyncData.mRemoteSyncs;

		if( buffer->fNumItems( ) == buffer->fCapacity( ) )
			buffer->fResize( ( buffer->fCapacity( ) + 1 ) * 2 );

		buffer->fPut( syncFrame );

		// Only do checks when we push a new sync frame
		if( !local )
			return;

		tFilePathPtr desyncRoot = tFilePathPtr::fConstructPath( 
			tGameApp::fInstance( ).fResourceDepot( )->fRootPath( ), cFileDesyncs );

		// Compare all available sync info
		while( mSyncData.mLocalSyncs.fNumItems( ) && mSyncData.mRemoteSyncs.fNumItems( ) )
		{
			tSync::tFrame localFrame, remoteFrame;
			Net::tSyncCheck * localSync, * remoteSync;

			mSyncData.mLocalSyncs.fGet( localSync );
			localFrame.mHash = localSync->mHash;

			mSyncData.mRemoteSyncs.fGet( remoteSync );
			remoteFrame.mHash = remoteSync->mHash;

#ifndef sync_system_detect_only

			localFrame.mBuffer.fSwap( localSync->mBuffer );
			localFrame.mCallstackBuffer.fSwap( localSync->mCallstackBuffer );

			remoteFrame.mBuffer.fSwap( remoteSync->mBuffer );
			remoteFrame.mCallstackBuffer.fSwap( remoteSync->mCallstackBuffer );
#endif

			// Make sure we're comparing the same frame inputs
			sigassert( localSync->mInputFrame == remoteSync->mInputFrame );

			u32 inputFrame = localSync->mInputFrame;

			delete localSync; localSync = NULL;
			delete remoteSync; remoteSync = NULL;

			std::string error;
			if( !tSync::fFramesEqual( localFrame, remoteFrame, error, desyncRoot) )
			{
				log_warning( 0, "Desync error ( " << inputFrame << " ): " << error );
				sigassert( Game_Sync_SkipAssert && "Desync" );

				if( mGameSessionNetwork.fIsHost( ) )
				{
					// Reset input - resets and ends syncing too
					fResetInput( );

					tGameApp & gameApp = tGameApp::fInstance( );

					mNetUI->fShowDesyncUI( );

					// Inform our peer that we've desynced
					Net::tDesyncDetected desyncMsg;
					fSendMsgToPeers( mGameSessionNetwork, desyncMsg, cGameChannelDefault, true, false );

					// Create a save game
					tSaveGameData saveGameData;
					tSaveGameRewindPreview ignored;

					if( gameApp.fCurrentLevel( )->fMapType( ) != GameFlags::cMAP_TYPE_MINIGAME && gameApp.fBuildSaveData( saveGameData, ignored ) )
					{
						saveGameData.mId = gameApp.mCurrentLevelLoadInfo.mHighestWaveReached;

						tGameArchiveSave archive;
						archive.fSaveLoad( saveGameData );
						// not encrypted

						Net::tLoadFromSave loadMsg;
						loadMsg.mSaveGame.fInitialize( 
							archive.fBuffer( ).fBegin( ), 
							archive.fBuffer( ).fCount( ) );

						loadMsg.mObjectiveSeed = mObjectiveSeed = tRandom::fSubjectiveRand( ).fUInt( );

						fSendMsgToPeers( mGameSessionNetwork, loadMsg, cGameChannelDefault, true, false );

						fLoadFromSave( archive.fBuffer( ).fBegin( ), archive.fBuffer( ).fCount( ) );
					}
					else
					{
						fRestartMap( );
					}
				}
				break;
			}
		}
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fLoadFromSave( const byte * data, u32 dataLength )
	{
		// this expects the data to already be decrypted
		tGameArchiveLoad archive( data, dataLength );

		tGameApp & gameApp  = tGameApp::fInstance( );

		gameApp.fSetLevelLoadState( NEW tGameLoadAppState( gameApp.fDefaultLoadScriptPath( ), archive, true, false ) ); 
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fOnGameMessage( 
		const Net::tPeer & peer, u32 channelId, const byte * data, u32 dataLength )
	{
		sigassert( channelId <= cGameChannelCount && "Sanity: Msg on invalid channel passed to GameAppSession" );

		Net::tBaseMessage header;
		if( !fDecodeMsg( header, data, dataLength ) )
		{
			fLogMsgError( peer, channelId, "Too small for header size" );
			return;
		}

		switch( header.mMessageType )
		{
		case Net::cMessageStartMap:
			fHandleStartMapMsg( peer, channelId, data, dataLength );
			break;
		case Net::cMessageLoadFromSave:
			fHandleLoadFromSaveMsg( peer, channelId, data, dataLength );
			break;
		case Net::cMessageReadyForInput:
			fHandleReadyForInputMsg( peer, channelId, data, dataLength );
			break;
		case Net::cMessagePlayerInfo:
			fHandlePlayerInfo( peer, channelId, data, dataLength );
			break;
		case Net::cMessageInputFrame:
			fHandleInputMsg( peer, channelId, data, dataLength );
			break;
		case Net::cMessageSyncCheck:
			fHandleSyncCheckMsg( peer, channelId, data, dataLength );
			break;
		case Net::cMessageDesyncDetected:
			fHandleDesyncMsg( peer, channelId, data, dataLength );
			break;
		case Net::cMessageClientState:
			fHandleLobbyPlayerStateChange( peer, channelId, data, dataLength );
			break;
		case Net::cMessageLevelSelectStatus:
			fHandleLobbyMenuStateChange( peer, channelId, data, dataLength );
			break;
		default:
			fLogMsgError( peer, channelId, "Unrecognized packet type" );
			break;
		}
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fOnReady( )
	{
		// Normal
		if( !mStartingReplaySession )
		{
			fSendSessionEvent( ApplicationEvent::cSessionCreated, true );

			if( mLoadingLevel && mGameSessionNetwork.fIsReadyAndIdle( ) )
				mGameSessionNetwork.fStart( );
		}
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fOnRejected( )
	{
		mGameSessionNetwork.fCancelAndLeave( );
		fSendSessionEvent( ApplicationEvent::cSessionRejected, false );
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fOnUsersChanged( )
	{
		if( !mStartingReplaySession )
		{
			// Send profile information to all peers, we do this whenever users change
			// to assure that all peers have received up to date profile information
			fSendProfiles( );
			fSendSessionEvent( ApplicationEvent::cSessionUsersChanged, true );

			// If we're trying to load the level, but waiting for the users to get updated
			// then recheck if it's totally ready and then start
			if( mLoadingLevel && mGameSessionNetwork.fIsReadyAndIdle( ) )
				mGameSessionNetwork.fStart( );
		}
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fOnSessionFull( )
	{
		// Normal
		if( !mStartingReplaySession )
		{
			tGameApp & gameApp = tGameApp::fInstance( );

			// Start the session if the next level is queued and is single player
			if( mGameSessionNetwork.fIsReadyAndIdle( ) )
			{
				if( mLoadingLevel || ( gameApp.mNextLevelQueued && gameApp.mNextLevelLoadInfo.mGameMode.fIsSinglePlayer( ) ) )
					mGameSessionNetwork.fStart( );
			}

			fSendSessionEvent( ApplicationEvent::cSessionFilled, true );
		}
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fOnRunning( )
	{
		if( !mStartingReplaySession )
		{
			tGameApp & gameApp = tGameApp::fInstance( );

			// If have profiles for all the remote users ( which may be none ) then start the session
			if( fHasAllProfiles( ) )
				gameApp.fLoadLevelNowThatSessionIsReady( );
			else
				mWaitingForProfiles = true;

			fSendSessionEvent( ApplicationEvent::cSessionStarted, true );
		}
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fOnDisconnect( )
	{
		// If we're not currently on the front end then load the front end
		//if( !tGameApp::fInstance( ).fLoadingLevelLoadInfo( mLevelUnloaded ).mGameMode.fIsFrontEnd( ) )
		//	tGameApp::fInstance( ).fLoadFrontEnd( );			

		// Disconnect is expected when the session network is stopping
		if( mGameSessionNetwork.fState( ) == tGameSessionNetwork::cStateDestroying )
			return;

		mNetUI->fHideLagUI( );

		// Signal the disconnect
		fSendSessionEvent( ApplicationEvent::cOnDisconnect, true );

		// Begin standard input processing
		tGameApp & gameApp = tGameApp::fInstance( );
		gameApp.fSetSimulate( true );

		if( mLoadingLevel )
			gameApp.fLoadLevelNowThatSessionIsReady( );
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fOnStopped( )
	{
		if( !mStartingReplaySession )
		{
			tGameApp & gameApp = tGameApp::fInstance( );

			if( mLoadingLevel )
				gameApp.fLoadLevelNowThatSessionIsReady( );
			else if( !gameApp.fCurrentLevelLoadInfo( ).mGameMode.fIsFrontEnd( ) )
				gameApp.fLoadFrontEnd( );

			fSendSessionEvent( ApplicationEvent::cSessionEnded, true );
		}
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fOnDestroyed( )
	{
		if( !mStartingReplaySession )
		{
			tGameApp & gameApp = tGameApp::fInstance( );

			// Create a session for the next level
			if( mLoadingLevel )
			{
				if( gameApp.fLoadingLevelLoadInfo( mLevelUnloaded ).mPreview )
					fCreateSessionForPreview( );
				else
					gameApp.fLoadLevelNowThatSessionIsReady( );
			}
			else if( !gameApp.fCurrentLevelLoadInfo( ).mGameMode.fIsFrontEnd( ) )
				gameApp.fLoadFrontEnd( );

			fSendSessionEvent( ApplicationEvent::cSessionDeleted, true );
		}
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fOnError( u32 failedState )
	{
		// Normal
		if( !mStartingReplaySession )
		{
			switch( failedState )
			{
			case tGameSessionNetwork::cStateCreatingSession:
			case tGameSessionNetwork::cStateConnecting:

				// If the target is a Net game, this needs to notify the player it's not going to happen
				// otherwise the player needs to be notified that stats won't be saved
				log_warning( 0, "!!!Sending Session Creation Failure Message!!!" );
				fSendSessionEvent( ApplicationEvent::cSessionCreated, false );

				if( mLoadingLevel )
					tGameApp::fInstance( ).fLoadLevelNowThatSessionIsReady( );
				break;
			case tGameSessionNetwork::cStateJoiningLocalUsers:
			case tGameSessionNetwork::cStateJoiningRemoteUsers:
				fSendSessionEvent( ApplicationEvent::cSessionUsersChanged, false );
				break;
			}
		}

		// Replay
		else
		{
			sigassert( 0 && "Error starting replay session" );
		}
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fOnStatsLost( )
	{
		fSendSessionEvent( ApplicationEvent::cSessionStatsLost, false );
	}

#ifdef sig_logging
	namespace{
		tStringPtr fEventIdToString( u32 id )
		{
			static tHashTable<u32, tStringPtr> sEventNames;
			sEventNames[ ApplicationEvent::cOnSystemUiChange ] = tStringPtr( "ON_SYSTEM_UI_CHANGE" );
			sEventNames[ ApplicationEvent::cOnPartyMembersChange ] = tStringPtr( "ON_PARTY_MEMBERS_CHANGE" );	

			sEventNames[ ApplicationEvent::cSessionCreated ] = tStringPtr( "SESSION_CREATED" );
			sEventNames[ ApplicationEvent::cSessionStarted ] = tStringPtr( "SESSION_STARTED" );
			sEventNames[ ApplicationEvent::cSessionEnded ] = tStringPtr( "SESSION_ENDED" );
			sEventNames[ ApplicationEvent::cSessionDeleted ] = tStringPtr( "SESSION_DELETED" );
			sEventNames[ ApplicationEvent::cSessionSearchComplete ] = tStringPtr( "SESSION_SEARCH_COMPLETE" );	
			sEventNames[ ApplicationEvent::cSessionUsersChanged ] = tStringPtr( "SESSION_USERS_CHANGED" );	
			sEventNames[ ApplicationEvent::cSessionFilled ] = tStringPtr( "SESSION_FILLED" );
			sEventNames[ ApplicationEvent::cSessionLoadLevel ] = tStringPtr( "SESSION_LOAD_LEVEL" );
			sEventNames[ ApplicationEvent::cSessionInviteAccepted ] = tStringPtr( "SESSION_INVITE_ACCEPTED" );
			sEventNames[ ApplicationEvent::cSessionInviteNeedFullVer ] = tStringPtr( "SESSION_INVITE_NEED_FULL_VERISON" );
			sEventNames[ ApplicationEvent::cSessionInviteRejected ] = tStringPtr( "SESSION_INVITE_REJECTED" );
			sEventNames[ ApplicationEvent::cSessionRejected ] = tStringPtr( "SESSION_REJECTED" );
			sEventNames[ ApplicationEvent::cOnDisconnect ] = tStringPtr( "ON_DISCONNECT" );

			sEventNames[ ApplicationEvent::cOnPlayerNoLive ] = tStringPtr( "ON_PLAYER_NO_LIVE" );	
			sEventNames[ ApplicationEvent::cOnPlayerYesLive ] = tStringPtr( "ON_PLAYER_YES_LIVE" );	
			sEventNames[ ApplicationEvent::cOnPlayerSignOut ] = tStringPtr( "ON_PLAYER_SIGN_OUT" );
			sEventNames[ ApplicationEvent::cOnPlayerSignIn ] = tStringPtr( "ON_PLAYER_SIGN_IN" );
			sEventNames[ ApplicationEvent::cOnPlayerLoseInput ] = tStringPtr( "ON_PLAYER_LOSE_INPUT" );

			sEventNames[ ApplicationEvent::cOnMenuStateChange ] = tStringPtr( "LOBBY_MENU_STATE_CHANGE" );	
			sEventNames[ ApplicationEvent::cOnClientStateChange ] = tStringPtr( "LOBBY_CLIENT_STATE_CHANGE" );

			sEventNames[ ApplicationEvent::cProflieStorageDeviceRemoved ] = tStringPtr( "ON_PROFILE_STORAGE_DEVICE_REMOVED" );
			sEventNames[ ApplicationEvent::cOnUpgradeToFullVersion ] = tStringPtr( "ON_UPGRADE_TO_FULL_VERSION" );

			tStringPtr* name = sEventNames.fFind( id );
			if( name )
				return *name;
			static const tStringPtr unknown( "UNKNOWN SESSION EVENT" );
			return unknown;
		}
	}//unnamed namespace
#endif//sig_logging

	//------------------------------------------------------------------------------
	void tGameAppSession::fSendSessionEvent( u32 eventId, u32 context )
	{
		// Queue up session events during the load phase
		if( mLoadingLevel )
		{
			tSessionEvent e;
			e.mEventId = eventId;
			e.mContext = context;

			mQueuedSessionEvents.fPushBack( e );
			return;
		}

		log_output(0, "Sending Session event " << fEventIdToString( eventId ).fCStr( ) << " " << context << std::endl);

		tGameApp & gameApp = tGameApp::fInstance( );
		if( gameApp.mCurrentLevel && !gameApp.mCurrentLevel->fRootMenu( ).fIsNull( ) )
		{
			// Any time we send a session event something serious might be happening
			// in which case we need to process input regardless of how much we've buffered

			switch( eventId )
			{
			case ApplicationEvent::cOnPlayerNoLive: // fall-through
			case ApplicationEvent::cOnPlayerYesLive: // fall-through
			case ApplicationEvent::cOnPlayerSignOut: // fall-through
			case ApplicationEvent::cOnPlayerSignIn: // fall-through
			case ApplicationEvent::cOnPlayerLoseInput: // fall-through
			case ApplicationEvent::cSessionInviteAccepted: // fall-through
			case ApplicationEvent::cSessionInviteRejected: // fall-through
			case ApplicationEvent::cSessionInviteNeedFullVer: // fall-through
			case ApplicationEvent::cProflieStorageDeviceRemoved: // fall-through
				gameApp.mCurrentLevel->fRootMenu( ).fHandleCanvasEvent( 
					Logic::tEvent( eventId, NEW Logic::tIntEventContext( context ) ) );
				break;
			default:
				gameApp.mCurrentLevel->fRootMenu( ).fHandleCanvasEvent( 
					Logic::tEvent( eventId, NEW Logic::tBoolEventContext( context ? true : false ) ) );
				break;
			}
		}
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fHandleStartMapMsg( 
		const Net::tPeer & peer, u32 channelId, const byte * data, u32 dataLength )
	{
		Net::tStartMap startMap;
		if( !fDecodeMsg( startMap, data, dataLength ) )
		{
			fLogMsgError( peer, channelId, "Too small for StartMap" );
			return;
		}

		if( mGameSessionNetwork.fIsHost( ) )
		{
			fLogMsgError( peer, channelId, "StartMap received on host session" );
			return;
		}

		mObjectiveSeed = startMap.mObjectiveSeed;
	
		switch( startMap.mType )
		{
		case Net::tStartMap::cNormal:
			fLoadMapForNetGame( startMap.mLoadLevelInfo );
			break;

		case Net::tStartMap::cRestart:
			fReloadMapForNetGame( );
			break;

		case Net::tStartMap::cNext:
			fLoadNextMapForNetGame( );
			break;
		}
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fHandleLoadFromSaveMsg( 
		const Net::tPeer& peer, u32 channelId, const byte * data, u32 dataLength )
	{
		fResetInput( );

		Net::tLoadFromSave loadFromSave;
		if( !fDecodeMsg( loadFromSave, data, dataLength ) )
		{
			fLogMsgError( peer, channelId, "Too small for tLoadFromSave" );
			return;
		}

		if( mGameSessionNetwork.fIsHost( ) )
		{
			fLogMsgError( peer, channelId, "LoadFromSave received on host session" );
			return;
		}

		mObjectiveSeed = loadFromSave.mObjectiveSeed;

		fLoadFromSave( 
			loadFromSave.mSaveGame.fBegin( ), 
			loadFromSave.mSaveGame.fCount( ) );
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fHandleReadyForInputMsg( 
		const Net::tPeer & peer, u32 channelId, const byte * data, u32 dataLength )
	{
		Net::tReadyForInput ready;
		if( !fDecodeMsg( ready, data, dataLength ) )
		{
			fLogMsgError( peer, channelId, "Too small for ReadyForInput" );
			return;
		}

		//if( !mGameSessionNetwork.fIsRunning( ) )
		//{
		//	fLogMsgError( peer, channelId, "ReadyForInput received with invalid session" );
		//	return;
		//}
		
		fMarkReadyForInput( ready.mUserId );
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fHandlePlayerInfo( 
		const Net::tPeer & peer, u32 channelId, const byte * data, u32 dataLength )
	{
		Net::tPlayerInfo info;
		if( !fDecodeMsg( info, data, dataLength ) )
		{
			fLogMsgError( peer, channelId, "Too small for PlayerInfo" );
			return;
		}

		tClientData & clientData = mClientData.fFindOrAdd( info.mUserId );

		tGameArchiveLoad archive( info.mProfile.fBegin( ), info.mProfile.fTotalSizeOf( ) );
		clientData.mProfile->fSaveLoad( archive );

		// Apply profiles to players as they come in
		fApplyProfiles( );

		// If we're trying to load a level and the session is runnin
		if( mWaitingForProfiles && mGameSessionNetwork.fIsRunning( ) && fHasAllProfiles( ) )
		{
			mWaitingForProfiles = false;
			tGameApp::fInstance( ).fLoadLevelNowThatSessionIsReady( );
		}
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fHandleInputMsg( 
		const Net::tPeer & peer, u32 channelId, const byte * data, u32 dataLength )
	{
		Net::tInputFrame inputFrame;
		if( !fDecodeMsg( inputFrame, data, dataLength ) )
		{
			fLogMsgError( peer, channelId, "Too small for InputFrame" );
			return;
		}

		//if( !mGameSessionNetwork.fIsRunning( ) )
		//{
		//	fLogMsgError( peer, channelId, "Input msg received with invalid session state" );
		//	return;
		//}

		// TODO: test for erroneous input packets ( bad sequence, etc. )
		fPushInputFrame( inputFrame );
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fHandleSyncCheckMsg( 
		const Net::tPeer & peer, u32 channelId, const byte * data, u32 dataLength )
	{
		sigassert( channelId == cGameChannelSync && "Sync message received on the wrong channel" );

		Net::tSyncCheck * syncCheck = NEW Net::tSyncCheck( );
		if( !fDecodeMsg( *syncCheck, data, dataLength ) )
		{
			delete syncCheck; 
			fLogMsgError( peer, channelId, "Couldn't parse sync message" );
			return;
		}

		//if( !mGameSessionNetwork.fIsRunning( ) )
		//{
		//	delete syncCheck;
		//	fLogMsgError( peer, channelId, "Sync frame msg received with invalid session state" );
		//	return;
		//}

		tGameApp & gameApp = tGameApp::fInstance( );
		const u32 playerCount = gameApp.mPlayers.fCount( );
		for( u32 p = 0; p < playerCount; ++p )
		{
			const tPlayerPtr & player = gameApp.mPlayers[ p ];

			// Find the remove player - assumes only one and is this peer
			if( player->fUser( )->fIsLocal( ) )
				continue;

			// If this use has it's input ready then this is a valid sync
			// packet, otherwise it's from last session
			if( fFindClientInput( player->fUser( )->fPlatformId( ) ) )
			{
				//log_line( 0, 
				//	"Received SyncCheck - Hash: " << syncCheck.mHash << 
				//	" BufferSize: " << syncCheck.mBuffer.fCount( ) );
				fPushSyncFrame( syncCheck, false );
				syncCheck = NULL;
				
				break;
			}
		}

		// This is an old packet and needs to be deleted
		if( syncCheck )
			delete syncCheck;
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fHandleDesyncMsg( 
		const Net::tPeer & peer, u32 channelId, const byte * data, u32 dataLength )
	{
		fResetInput( );

		mNetUI->fShowDesyncUI( );
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fHandleLobbyMenuStateChange( 
		const Net::tPeer & peer, u32 channelId, const byte * data, u32 dataLength )
	{
		Net::tLevelSelectStatus levelSelectStatus;
		if( !fDecodeMsg( levelSelectStatus, data, dataLength ) )
		{
			fLogMsgError( peer, channelId, "Couldn't parse LevelSelectStatus message" );
			return;
		}

		tGameApp& gameApp = tGameApp::fInstance( );
		if( gameApp.fCurrentLevel( ) && !gameApp.fCurrentLevel( )->fRootMenu( ).fIsNull( ) )
		{
			gameApp.fCurrentLevel( )->fRootMenu( ).fHandleCanvasEvent( 
				Logic::tEvent( ApplicationEvent::cOnMenuStateChange, NEW Logic::tObjectEventContext( &levelSelectStatus ) ) );
		}
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fHandleLobbyPlayerStateChange( 
		const Net::tPeer & peer, u32 channelId, const byte * data, u32 dataLength )
	{
		Net::tClientStateChange clientStatus;
		if( !fDecodeMsg( clientStatus, data, dataLength ) )
		{
			fLogMsgError( peer, channelId, "Couldn't parse PlayerStateChange message" );
			return;
		}

		tGameApp & gameApp = tGameApp::fInstance( );
		if( gameApp.mCurrentLevel && !gameApp.mCurrentLevel->fRootMenu( ).fIsNull( ) )
		{
			gameApp.mCurrentLevel->fRootMenu( ).fHandleCanvasEvent( 
				Logic::tEvent( ApplicationEvent::cOnClientStateChange, NEW Logic::tObjectEventContext( &clientStatus ) ) );
		}
	}

	//------------------------------------------------------------------------------
	u32 tGameAppSession::fGetLevelLeaderboardId( u32 mapType, u32 levelIndex, u32 difficulty ) const
	{
		switch( mapType )
		{
		case GameFlags::cMAP_TYPE_CAMPAIGN:
			sigassert( levelIndex < array_length( GameSession::cLeaderboardLevels ) );
			sigassert( difficulty < GameFlags::cDIFFICULTY_COUNT );
			return GameSession::cLeaderboardLevels[ levelIndex ][ difficulty ];

		case GameFlags::cMAP_TYPE_SURVIVAL:
			sigassert( levelIndex < array_length( GameSession::cLeaderboardSurvival ) );
			sigassert( difficulty < GameFlags::cCHALLENGE_MODE_COUNT );
			return GameSession::cLeaderboardSurvival[ levelIndex ][ difficulty ];

		case GameFlags::cMAP_TYPE_MINIGAME:
			sigassert( levelIndex < array_length( GameSession::cLeaderboardMiniGame ) );
			return GameSession::cLeaderboardMiniGame[ levelIndex ];

		case GameFlags::cMAP_TYPE_HEADTOHEAD:
			sigassert( levelIndex < array_length( GameSession::cLeaderboardH2H ) );
			return GameSession::cLeaderboardH2H[ levelIndex ];

		default:
			return ~0;
		}
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fWriteLossForOtherPlayer( )
	{
		const tLevelLoadInfo& info = tGameApp::fInstance( ).fCurrentLevelLoadInfo( );

		// only do this for net, versus, quick match.
		if( !info.mGameMode.fIsVersus( ) || !info.mGameMode.fIsNet( )/* || !mIsQuickMatch*/ )
			return;

		tPlayer* player = tGameApp::fInstance( ).fOtherPlayer( tGameApp::fInstance( ).fFrontEndPlayer( ) );
		if( !player )
			return;

		tGrowableArray<tGameSessionViewProperties> writes;
		tScoreProps props;

		props.fPushBack( tGrowableArray<tUserProperty>() );
		tGrowableArray<tUserProperty>& back = props.fBack( );
		back.fPushBack( tUserProperty( GameSession::cPropertyWinLose ) );
		back.fPushBack( tUserProperty( GameSession::cPropertyWins ) );
		back.fPushBack( tUserProperty( GameSession::cPropertyLosses ) );


		back[ 0 ].mData.fSet( ( s64 ) ( 1ull ) );
		back[ 1 ].mData.fSet( ( s64 ) ( 0ull ) );
		back[ 2 ].mData.fSet( ( s64 ) ( 1ull ) );

		writes.fPushBack( tGameSessionViewProperties( ) );
		writes.fBack( ).mViewId = GameSession::cLeaderboardH2HTotals;
		writes.fBack( ).mNumProperties = back.fCount( );
		writes.fBack( ).mProperties = back.fBegin( );

		writes.fPushBack( tGameSessionViewProperties( ) );
		writes.fBack( ).mViewId = GameSession::cLeaderboardH2H[ tGameApp::fInstance( ).fCurrentLevelLoadInfo( ).mLevelIndex ];
		writes.fBack( ).mNumProperties = back.fCount( );
		writes.fBack( ).mProperties = back.fBegin( );

		fWriteStatsToRightPlace( player, writes.fCount( ), writes.fBegin( ) );

		log_line( 0, "Writing loss for dropped player" );
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fSendClientStateChange( Net::tClientStateChange& clientStatus )
	{
		fSendMsgToPeers( mGameSessionNetwork, clientStatus, cGameChannelDefault, true );
	}

	//------------------------------------------------------------------------------
	void tGameAppSession::fSendLevelSelectStatus( Net::tLevelSelectStatus& levelStatus )
	{
		fSendMsgToPeers( mGameSessionNetwork, levelStatus, cGameChannelDefault, true );
	}

	u32 tGameAppSession::fMiniGameScoreId( )
	{ 
		if( tGameApp::fInstance( ).fCurrentLevel( ) )
		{
			sigassert( tGameApp::fInstance( ).fCurrentLevel( )->fLevelNumber( ) < cMAX_LEVEL_COUNT );
			return GameSession::cLevelLeaderBoardScoreColumnID[ GameFlags::cMAP_TYPE_MINIGAME ][ tGameApp::fInstance( ).fCurrentLevel( )->fLevelNumber( ) ].mScore; 
		}
		else
			return GameSession::cLevelLeaderBoardScoreColumnID[ GameFlags::cMAP_TYPE_MINIGAME ][ 0 ].mScore;
	}

	tPlayer* tGameAppSession::fGetHostPlayer( ) const
	{
		if( fIsHost( ) )
			return tGameApp::fInstance( ).fFrontEndPlayer( );
		else
			return tGameApp::fInstance( ).fSecondaryPlayer( );
	}

	tPlayer* tGameAppSession::fGetClientPlayer( ) const
	{
		if( !fIsHost( ) )
			return tGameApp::fInstance( ).fFrontEndPlayer( );
		else
			return tGameApp::fInstance( ).fSecondaryPlayer( );
	}

}

namespace Sig
{
	namespace
	{
		static u32 fMiniGameLeaderBoard( u32 levelIndex )
		{
			sigassert( levelIndex < array_length( GameSession::cLeaderboardMiniGame ) );
			return GameSession::cLeaderboardMiniGame[ levelIndex ];
		}

		static u32 fCurrentMiniGameLeaderBoard( )
		{
			u32 levelIndex = tGameApp::fInstance( ).fCurrentLevelLoadInfo( ).mLevelIndex;
			return fMiniGameLeaderBoard( levelIndex );
		}

		static u32 fMiniGameProperty( )
		{
			return GameSession::cPropertyScore;
		}
	}

	void tGameAppSession::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tGameAppSession, Sqrat::NoConstructor> classDesc( vm.fSq( ) );

		// bind "global"ish stuff here, like data tables
		classDesc
			.Prop(_SC("IsHost"),							&tGameAppSession::fIsHost)
			.Prop(_SC("LocalUserCount"),					&tGameAppSession::fLocalUserCount)
			.Prop(_SC("RemoteUserCount"),					&tGameAppSession::fRemoteUserCount)
			.Prop(_SC("GameMode"),							&tGameAppSession::fGameMode)
			.Prop(_SC("GameType"),							&tGameAppSession::fGameType)
			.Prop(_SC("IsOnline"),							&tGameAppSession::fIsOnline)
			.Prop(_SC("IsInactive"),						&tGameAppSession::fIsInactive)
			.Func(_SC("BeginGameSearch"),					&tGameAppSession::fBeginGameSearch)
			.Func(_SC("FirstJoinableSession"),				&tGameAppSession::fFirstJoinableSession)
			.Func(_SC("HostGame"),							&tGameAppSession::fHostOnlineGame)
			.Func(_SC("JoinRemoteSessionFromSearchIndex"),	&tGameAppSession::fJoinRemoteSessionFromSearchIndex)
			.Func(_SC("CancelGameSearch"),					&tGameAppSession::fCancelGameSearch)
			.Func(_SC("CancelSession"),						&tGameAppSession::fCancelSession)
			.Func(_SC("StartQuickMatchMap"),				&tGameAppSession::fStartQuickMatchMap)
			.Func(_SC("RandomQuickMatchLevelIndex"),		&tGameAppSession::fRandomQuickMatchLevelIndex)
			.Func(_SC("StartMap"),							&tGameAppSession::fStartMap)
			.Func(_SC("RestartMap"),						&tGameAppSession::fRestartMap)
			.Func(_SC("StartNextMap"),						&tGameAppSession::fStartNextMap)
			.Func(_SC("LocalUser"),							&tGameAppSession::fLocalUser)
			.Func(_SC("RemoteUser"),						&tGameAppSession::fRemoteUser)
			.Func(_SC("WriteStat"),							&tGameAppSession::fWriteStatFromScript)
			.Func(_SC("FlushStats"),						&tGameAppSession::fFlushStats)
			.Func(_SC("AddLocalUserToSession"),				&tGameAppSession::fAddLocalUserToSession)
			.Func(_SC("AddLocalZombieToSession"),			&tGameAppSession::fAddLocalZombieToSession)
			.Func(_SC("RemoveLocalUserFromSession"),		&tGameAppSession::fRemoveLocalUserFromSession)
			.StaticFunc(_SC("MiniGameLeaderBoard"),			&fMiniGameLeaderBoard)
			.StaticFunc(_SC("CurrentMiniGameLeaderBoard"),	&fCurrentMiniGameLeaderBoard)
			.StaticFunc(_SC("MiniGameProperty"),			&fMiniGameProperty)
			.Func(_SC("GetLevelLeaderboardId"),				&tGameAppSession::fGetLevelLeaderboardId)
			.Func(_SC("SendClientStateChange"),				&tGameAppSession::fSendClientStateChange)
			.Func(_SC("SendLevelSelectStatus"),				&tGameAppSession::fSendLevelSelectStatus)
			.Func(_SC("SetFrontEndLoadBehavior"),			&tGameAppSession::fSetFrontEndLoadBehavior)
			.Prop(_SC("LoadingLevel"),						&tGameAppSession::fLoadingLevel)
			.Var(_SC("IsQuickMatch"),						&tGameAppSession::mIsQuickMatch)
			.Prop(_SC("LEADERBOARD_MINIGAME_SCORE_ID"),		&tGameAppSession::fMiniGameScoreId)
			.Prop(_SC("HostPlayer"),						&tGameAppSession::fGetHostPlayer)
			.Prop(_SC("ClientPlayer"),						&tGameAppSession::fGetClientPlayer)
			.Func(_SC("WriteLossForOtherPlayer"),			&tGameAppSession::fWriteLossForOtherPlayer)
			;

		vm.fRootTable( ).Bind(_SC("GameAppSession"), classDesc );

		vm.fConstTable( ).Const( "CONTEXT_GAME_TYPE_STANDARD",	( int )tUser::cUserContextGameTypeStandard );
		vm.fConstTable( ).Const( "CONTEXT_GAME_TYPE_RANKED",	( int )tUser::cUserContextGameTypeRanked );

		vm.fConstTable( ).Const( "CONTEXT_GAME_MODE_CAMPAIGN",	( int )GameSession::cContextGameModeCampaign );
		vm.fConstTable( ).Const( "CONTEXT_GAME_MODE_SURVIVAL",	( int )GameSession::cContextGameModeSurvival );
		vm.fConstTable( ).Const( "CONTEXT_GAME_MODE_MINIGAME",	( int )GameSession::cContextGameModeMinigame );
		vm.fConstTable( ).Const( "CONTEXT_GAME_MODE_VERSUS",	( int )GameSession::cContextGameModeVersus );

		vm.fConstTable( ).Const( "LEADERBOARD_ARCADE",			( int )GameSession::cLeaderboardArcade );
		vm.fConstTable( ).Const( "LEADERBOARD_H2H_TOTALS", ( int )GameSession::cLeaderboardH2HTotals );
		vm.fConstTable( ).Const( "LEADERBOARD_TRIAL_MINIGAME_1", ( int )GameSession::cLeaderboardTrialMiniGame[ 0 ] );
		vm.fConstTable( ).Const( "LEADERBOARD_TRIAL_MINIGAME_2", ( int )GameSession::cLeaderboardTrialMiniGame[ 1 ] );

		vm.fConstTable( ).Const( "FRONTEND_LOAD_BEHAVIOR_DESTROY_SESSION", (int)cFrontEndLoadBehaviorDestroySession );
		vm.fConstTable( ).Const( "FRONTEND_LOAD_BEHAVIOR_END_SESSION", (int)cFrontEndLoadBehaviorEndSession );

	}
}
