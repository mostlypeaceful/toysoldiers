#include "GameAppPch.hpp"
#include "tGameApp.hpp"
#include "Scripts/tScriptFile.hpp"
#include "tGameSessionStats.hpp"

#include "tScoreUI.hpp"
#include "tMiniMap.hpp"
#include "tComboTimerUI.hpp"
#include "tPowerPoolUI.hpp"
#include "tBatteryMeter.hpp"
#include "tPersonalBestUI.hpp"
#include "tOutOfBoundsIndicator.hpp"
#include "tWorldSpaceFloatingText.hpp"
#include "tScreenSpaceNotification.hpp"
#include "tAchievementBuyNotification.hpp"

// camera
#include "Gfx/tDebugFreeCamera.hpp"
#include "tFrontEndCamera.hpp"
#include "tRtsCamera.hpp"
#include "tShellCamera.hpp"
#include "tTransitionCamera.hpp"
#include "tUseUnitCamera.hpp"
#include "tRtsCursorLogic.hpp"

#include "tTurretLogic.hpp"
#include "tVehicleLogic.hpp"
#include "tLevelLogic.hpp"
#include "tAchievements.hpp"
#include "GameSession.hpp"
#include "tFocusCamera.hpp"
#include "tGoalBoxLogic.hpp"
#include "tSync.hpp"

#include "Audio/tListener.hpp"
#include "Wwise_IDs.h"

#include "xlsp/XLSP.h"
#include "tEncryption.hpp"

// debugging of path destruction
#include "tPathEntity.hpp"
#include "tWaypointLogic.hpp"

#include "Gui/tCanvas.hpp"
#include "tProximityLogic.hpp"
#include "tBuildSiteLogic.hpp"
#include "tWaveLaunchArrowUI.hpp"

// Extra Stuff
#include "tCharacterLogic.hpp"

namespace Sig
{
#ifdef sig_devmenu
	void fAAACheatsAddMoney( tDevCallback::tArgs& args )
	{
		tDynamicArray< tPlayerPtr >& players = tGameApp::fInstance( ).fPlayers( );

		for( u32 i = 0; i < players.fCount( ); ++i )
		{
			if( players[ i ]->fUser( ).fGetRawPtr( ) == args.mUser )
			{
				players[ i ]->fAddInGameMoney( 10000 );
				return;
			}
		}
	}
	void fAAACheatsBarrage( tDevCallback::tArgs& args )
	{
		tDynamicArray< tPlayerPtr >& players = tGameApp::fInstance( ).fPlayers( );

		for( u32 i = 0; i < players.fCount( ); ++i )
		{
			if( players[ i ]->fUser( ).fGetRawPtr( ) == args.mUser )
			{
				players[ i ]->fGiveBarrage( false );
				return;
			}
		}
	}
	void fAAARestrictedBarrage( tDevCallback::tArgs& args, const tStringPtr& name )
	{
		tDynamicArray< tPlayerPtr >& players = tGameApp::fInstance( ).fPlayers( );

		for( u32 i = 0; i < players.fCount( ); ++i )
		{
			if( players[ i ]->fUser( ).fGetRawPtr( ) == args.mUser )
			{
				players[ i ]->fRestrictBarrage( tStringPtr( name ) );
				players[ i ]->fGiveBarrage( false );
				return;
			}
		}
	}
	void fAAACheatsArtilleryBarrage( tDevCallback::tArgs& args )
	{
		tStringPtr name( "BARRAGE_ARTILLERY" );
		fAAARestrictedBarrage( args, name );
	}
	void fAAACheatsRamboBarrage( tDevCallback::tArgs& args )
	{
		tStringPtr name( "BARRAGE_RAMBO" );
		fAAARestrictedBarrage( args, name );
	}
	void fAAACheatsB52Barrage( tDevCallback::tArgs& args )
	{
		tStringPtr name( "BARRAGE_B52" );
		fAAARestrictedBarrage( args, name );
	}
	void fAAACheatsAc130Barrage( tDevCallback::tArgs& args )
	{
		tStringPtr name( "BARRAGE_AC130" );
		fAAARestrictedBarrage( args, name );
	}
	void fAAACheatNukeBarrage( tDevCallback::tArgs& args )
	{
		tStringPtr name( "BARRAGE_NUKE" );
		fAAARestrictedBarrage( args, name );
	}
	void fAAACheatLazerBarrage( tDevCallback::tArgs& args )
	{
		tStringPtr name( "BARRAGE_LAZER" );
		fAAARestrictedBarrage( args, name );
	}
	void fAAACheatNapalmBarrage( tDevCallback::tArgs& args )
	{
		tStringPtr name( "BARRAGE_NAPALM" );
		fAAARestrictedBarrage( args, name );
	}
	void fAAACheatsOverCharge( tDevCallback::tArgs& args )
	{
		tDynamicArray< tPlayerPtr >& players = tGameApp::fInstance( ).fPlayers( );

		for( u32 i = 0; i < players.fCount( ); ++i )
		{
			if( players[ i ]->fUser( ).fGetRawPtr( ) == args.mUser )
			{
				players[ i ]->fGiveOverCharge( );
				return;
			}
		}
	}
	void fAAAKillEveryone( tDevCallback::tArgs& args )
	{
		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		if( level )
		{
			const tGrowableArray<tEntityPtr>& units = level->fAllUnitList( );
			for( u32 i = 0; i < units.fCount( ); ++i )
			{
				tUnitLogic* ul = units[ i ]->fLogicDerived<tUnitLogic>( );
				sigassert( ul );

				if( ul->fTeam( ) != tGameApp::fInstance( ).fFrontEndPlayer( )->fTeam( )
					&& ( ul->fUnitType( ) == GameFlags::cUNIT_TYPE_INFANTRY
					|| ul->fUnitType( ) == GameFlags::cUNIT_TYPE_VEHICLE
					|| ul->fUnitType( ) == GameFlags::cUNIT_TYPE_AIR
					|| ul->fUnitType( ) == GameFlags::cUNIT_TYPE_BOSS ) )
				{
					ul->fDestroy( );
				}
			}
		}			
	}
	void fAAAResetProfile( tDevCallback::tArgs& args )
	{	
		tDynamicArray< tPlayerPtr >& players = tGameApp::fInstance( ).fPlayers( );

		for( u32 i = 0; i < players.fCount( ); ++i )
		{
			if( players[ i ]->fUser( ).fGetRawPtr( ) == args.mUser )
			{
				players[ i ]->fSaveDefaultProfile( );
			}
		}		
	}
	void fAAACompleteProfile( tDevCallback::tArgs& args )
	{	
		tDynamicArray< tPlayerPtr >& players = tGameApp::fInstance( ).fPlayers( );

		for( u32 i = 0; i < players.fCount( ); ++i )
		{
			if( players[ i ]->fUser( ).fGetRawPtr( ) == args.mUser )
			{
				players[ i ]->fSaveFullyCompleteProfile( );
			}
		}		
	}
	void fAAAReseLeaderBoards( tDevCallback::tArgs& args )
	{	
		tLeaderboard::fResetLeaderBoards( );
	}
	void fAAAZeroTickets( tDevCallback::tArgs& args )
	{	
		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		if( level )
		{
			u32 playerIndex = tGameApp::fInstance( ).fWhichPlayer( args.mUser );
			if( playerIndex != ~0 )
				level->fOnTicketsCountChanged( 0, tGameApp::fInstance( ).fPlayers( )[ playerIndex ].fGetRawPtr( ) );
		}
	}
	void AAACheatsUnlockAllLevels( tDevCallback::tArgs& args )
	{
		tDynamicArray< tPlayerPtr >& players = tGameApp::fInstance( ).fPlayers( );

		for( u32 i = 0; i < players.fCount( ); ++i )
		{
			if( players[ i ]->fUser( ).fGetRawPtr( ) == args.mUser )
			{
				tUserProfilePtr profile = players[ i ]->fProfile( );
				if( profile )
				{
					profile->fUnlockAllLevels( );
				}
				break;
			}
		}
	}
	void fAAACheatsCauseDesync( tDevCallback::tArgs& args )
	{
		tDynamicArray< tPlayerPtr >& players = tGameApp::fInstance( ).fPlayers( );

		for( u32 i = 0; i < players.fCount( ); ++i )
		{
			if( players[ i ]->fUser( ).fGetRawPtr( ) == args.mUser && players[ i ]->fUser( )->fIsLocal( ) )
			{
				sync_rand( fInt( ) );
				return;
			}
		}
	}
	void fAAAWriteTestMobileRecords( tDevCallback::tArgs& args )
	{
		tXLSP::fInstance( ).fWriteTestValues( args.mUser );
	}
	void fAAAWriteTestMobileOwnership( tDevCallback::tArgs& args )
	{
		tXLSP::fInstance( ).fWriteTestOwnership( args.mUser );
	}

#endif //sig_devmenu
	devcb( AAACheats_AddMoney, "10000", make_delegate_cfn( tDevCallback::tFunction, fAAACheatsAddMoney ) );
	devcb( AAACheats_Barrages_Get, "Get", make_delegate_cfn( tDevCallback::tFunction, fAAACheatsBarrage ) );
	devcb( AAACheats_Barrages_Rambo, "R", make_delegate_cfn( tDevCallback::tFunction, fAAACheatsRamboBarrage ) );
	devcb( AAACheats_Barrages_B52, "B", make_delegate_cfn( tDevCallback::tFunction, fAAACheatsB52Barrage ) );
	devcb( AAACheats_Barrages_AC130, "A", make_delegate_cfn( tDevCallback::tFunction, fAAACheatsAc130Barrage ) );
	devcb( AAACheats_Barrages_NUKE, "N", make_delegate_cfn( tDevCallback::tFunction, fAAACheatNukeBarrage ) );
	devcb( AAACheats_Barrages_NAPALM, "M", make_delegate_cfn( tDevCallback::tFunction, fAAACheatNapalmBarrage ) );
	devcb( AAACheats_Barrages_LAZER, "L", make_delegate_cfn( tDevCallback::tFunction, fAAACheatLazerBarrage ) );
	devcb( AAACheats_Barrages_Artillery, "N", make_delegate_cfn( tDevCallback::tFunction, fAAACheatsArtilleryBarrage ) );
	devcb( AAACheats_CauseDesync, "Desync", make_delegate_cfn( tDevCallback::tFunction, fAAACheatsCauseDesync ) );
	
	devcb( AAACheats_GetOverCharge, "0", make_delegate_cfn( tDevCallback::tFunction, fAAACheatsOverCharge ) );
	devcb( AAACheats_KillEveryone, "0", make_delegate_cfn( tDevCallback::tFunction, fAAAKillEveryone ) );
	devcb( AAACheats_ResetProfile, "0", make_delegate_cfn( tDevCallback::tFunction, fAAAResetProfile ) );
	devcb( AAACheats_CompleteProfile, "0", make_delegate_cfn( tDevCallback::tFunction, fAAACompleteProfile ) );
	
	devcb( AAACheats_ZeroTickets, "0", make_delegate_cfn( tDevCallback::tFunction, fAAAZeroTickets ) );
	devcb( AAACheats_UnlockAllLevels, "Unlock", make_delegate_cfn( tDevCallback::tFunction, AAACheatsUnlockAllLevels ) );
	devcb( AAACheats_LeaderBoards_Reset, "Reset", make_delegate_cfn( tDevCallback::tFunction, fAAAReseLeaderBoards ) );
	devcb( AAACheats_WriteTestMobileRecords, "Write", make_delegate_cfn( tDevCallback::tFunction, fAAAWriteTestMobileRecords ) );
	devcb( AAACheats_WriteTestMobileOwnership, "Write", make_delegate_cfn( tDevCallback::tFunction, fAAAWriteTestMobileOwnership ) );


	devvar( bool, FrontEnd_UseFilmGrain, true );
	devvar( bool, Renderer_PostEffects_PauseCam_Enable, true );
	devvar( bool, Renderer_PostEffects_GameCam_Enable, true );
	devvar( bool, Renderer_PostEffects_FilmGrain_OverrideOn, false );
	devvar_clamp( f32, Renderer_PostEffects_FilmGrain_FadeInTime, 10.f, 0.f, 30.f, 2 );
	devvar( f32, Renderer_PostEffects_HealthThreshold, 0.4f );
	devvar( f32, Gameplay_Player_ToyBoxAlarmTimeout, 4.0f );
	devvar( f32, Gameplay_Player_ToyBoxAlarmTimeoutShort, 1.0f );
	devvar( f32, Gameplay_Vehicle_TinyBatteryValue, 0.005f );
	devvar( f32, Gameplay_Player_MoreTimeValue, 5.0f );
	devvar( f32, Gameplay_Player_LessTimeValue, 10.0f );
	devvar( f32, Gameplay_BombingRunTimer, 4.0f );
	devvar( bool, Debug_RandyMode, false );
	devvar( f32, Gameplay_Gui_PlayerSpawnText_Raise, 3.333f );	// in world-units, so 3.5m above the soldiers feet to display the kill text popups...

	devvar( Math::tVec4f, Renderer_PostEffects_TiltShiftCam_DOF, Math::tVec4f( 0.986f, 0.256f, 3.181f, 0.5f ) );
	devvar( bool, Debug_EnableMinimap, false );

	//devvar( u32, Debug_StopWritingStats, GameFlags::cSESSION_STATS_TIME_USING_ATTACK_HELICOPTER );

	class tTiltShiftCamera : public tRtsCamera
	{
		define_dynamic_cast( tTiltShiftCamera, tRtsCamera );
	public:
		tTiltShiftCamera( tPlayer& player )
			: tRtsCamera( player, false, true )
		{ 
			mTargetDOF = Renderer_PostEffects_TiltShiftCam_DOF;
		}
		virtual void fOnTick( f32 dt )
		{
			mTargetDOF = Renderer_PostEffects_TiltShiftCam_DOF; //for debugging
			tRtsCamera::fOnTick( dt );
		}
	};

	class tFollowPathCam : public tUseUnitCamera
	{
		define_dynamic_cast( tFollowPathCam, tUseUnitCamera );

		Gfx::tFollowPathCamera* mCamera;
		Gfx::tCameraControllerPtr mCamPtr;

	public:
		tFollowPathCam( tPlayer* player
			, const tPathEntityPtr& p
			, const tStringPtr& name
			, b32 skipable
			, const Gfx::tFollowPathCamera::tOnEndOfPathReached& callback )
			: tUseUnitCamera( *player, NULL, false )
		{
			// tUseUnitCamera stuff
			mTargetDOF = tGameApp::fInstance( ).fPostEffectsManager( )->fDefaultGameCamData( player->fUser( )->fViewportIndex( ) ).mDof;
			mTargetZoom = player->fDefaultTranstionCamZoom( );
			//mBlendType = cBlendPureLerp;
			mOverrideBlendDist = 15.f;

			// Do this for all players
			Gfx::tFollowPathCamera::tControlPointList cps;
			Gfx::tFollowPathCamera::fBuildControlPointList( cps, *player->fRootEntity( ), Math::tVec3f::cZeroVector, *p );
			mCamera 
				= NEW Gfx::tFollowPathCamera( name
				, skipable ? make_delegate_memfn( Gfx::tFollowPathCamera::tSkipButtonHeldFunc, tFollowPathCam, fSkipButtonHeld ) : Gfx::tFollowPathCamera::tSkipButtonHeldFunc( ) // TODO: Convert to tGameController
				, player->fUser( )
				, &player->fCameraStack( )
				, player->fDefaultLens( mTargetZoom )
				, cps
				, callback
				);

			mCamPtr.fReset( mCamera );

			//if( skipable )
			//{
			//	// Add all the gamepads for the other cameras
			//	tDynamicArray< tPlayerPtr > players = tGameApp::fInstance( ).fPlayers( );
			//	for( u32 k = 0; k < players.fCount( ); ++k )
			//		mCamera->fAddGamePad( players[ k ]->fUser( ) );
			//}
		}

		virtual void fOnActivate( b32 active )
		{
			mCamera->fOnActivate( active );
			tUseUnitCamera::fOnActivate( active );
			if( active && tGameApp::fInstance( ).fSpawningCurrentLevel( ) )
				fSkipBlendIn( );
		}

		virtual void fUserBlendIn( f32 dt, Gfx::tTripod& tripod )
		{
			fUserTick( dt, tripod );
		}

		virtual void fUserTick( f32 dt, Gfx::tTripod& tripod )
		{
			tripod = mCamera->fStepSlave( dt );
		}

		b32 fSkipButtonHeld( Gfx::tFollowPathCamera& camera, tUserPtr& user )
		{
			const tPlayer* player = tGameApp::fInstance( ).fGetPlayerByUser( user.fGetRawPtr( ) );
			if( player )
			{
				if( player->fGameController( )->fButtonHeld( tUserProfile::cProfileCamera, GameFlags::cGAME_CONTROLS_MENU_ACCEPT, user->fInputFilterLevel( ) ) ||
					player->fGameController( )->fButtonHeld( tUserProfile::cProfileCamera, GameFlags::cGAME_CONTROLS_MENU_CANCEL, user->fInputFilterLevel( ) ) )
				{
					return true;
				}
			}

			return false;
		}
	};

	static const tStringPtr cB52BarrageName( "BARRAGE_B52" );

	const Math::tVec3f tPlayer::cInUseIndicatorOffset( 0, 5, 0 );
}

namespace Sig
{
	tPlayer::tPlayer( const tUserPtr& user )
		: mState( cStateLoading )
		, mTeam( GameFlags::cTEAM_NONE )
		, mEnemyTeam( GameFlags::cTEAM_NONE )
		, mCountry( GameFlags::cCOUNTRY_DEFAULT )
		, mEnemyCountry( GameFlags::cCOUNTRY_DEFAULT )
		, mDontResetComboOnExit( false )
		, mForceFilmGrainActive( false )
		, mChangeUseTurret( false )
		, mOverChargeActive( false )
		, mWasFailing( false )
		, mLockedInUnit( false )
		, mAllowDPadSwitchWhileLocked( false )
		, mQuickSwitchTurret( false )
		, mFocusShown( false )
		, mFocusCameraPushed( false )
		, mBeatLevelTheFirstTime( false )
		, mIsNotAllowedToSaveStats( false )
		, mFullScreenOverlayActive( false )
		, mUserKicked( false )
		, mCachedProfileInfo( false )
		, mDisableQuickSwitch( false )
		, mDisableTiltShiftCount( 0 )
		, mFocusShowTimer( 0 )
		, mCurrentFocusUnit( NULL )
		, mCurrentUnit( NULL )
		, mHoveredUnit( NULL )
		, mAudioListener( NEW Audio::tListener( ) )
		, mToyBoxAlarmTimer( -1.f )
		, mLastKillValue( 0 )
		, mSkippableSeconds( 0.f )
		, mCurrentWaveChain( 0 )
		, mBombingRunCount( 0 )
		, mBombingRunTimer( -1.f )
		, mBombingRunLastKillPos( Math::tVec3f::cZeroVector )
		, mQuickSwitchesWhileOvercharged( 0 )
		, mFollowPathCamActive( false )
		, mWaitingToLeaveFollowPathCam( false )
		, mStepCameras( true )
		, mFocusItems( 10 )
		, mStarPoints( 0.0f )
		, mSessionAchievementMask( 0 )
		, mSessionAvatarMask( 0 )
		, mProfileDeviceAvailable( false )
		, mShowRemovedOncePerLevel( false )
		, mDoesntWantRewind( false )
		, mStatsEnabled( true )
		, mDeviceSelector( 6 * 1024 * 1024 ) // 6 Mb minimum file size
		, mHasJetPack( false )
		, mTimeSinceUnitExit( 0.0f )
	{
		fSetUser( user );

		tGameController* gc = NEW tGameController( mUser, mProfile );
		mGameController.fReset( gc );

		fConfigureAudio( );
	}
	tPlayer::~tPlayer( )
	{
		mUser->fRemoveGameRef( );
		fRemoveStandAloneFromSceneGraph( );
	}
	void tPlayer::fOnPause( b32 paused )
	{
		sigassert( fProfile( ) );
		mUser->fRawGamepad( ).fRumble( ).fSetRumblePaused( paused || fProfile( )->fSetting( tUserProfile::cSettingsDisableControllerVibe ) );
	}
	void tPlayer::fResetProfile( )
	{
		tUserProfile* newUserProfile = NEW tUserProfile( );
		newUserProfile->fSetRealDefaults( );
		newUserProfile->fSetInversionDefaults( mUser->fYAxisInvertedDefault( ) );
		newUserProfile->fSetSouthpawDefaults( mUser->fSouthpawDefault( ) );

		if( mProfile )
		{
			// copy previous settings we want to preserve.
			newUserProfile->fSetSetting( tUserProfile::cSettingsCameraInversion, mProfile->fInversion( tUserProfile::cProfileCamera ) );
			newUserProfile->fSetSetting( tUserProfile::cSettingsCameraSouthpaw, mProfile->fSouthPaw( tUserProfile::cProfileCamera ) );
			newUserProfile->fSetSetting( tUserProfile::cSettingsTurretsInversion, mProfile->fInversion( tUserProfile::cProfileTurrets ) );
			newUserProfile->fSetSetting( tUserProfile::cSettingsTurretsSouthpaw, mProfile->fSouthPaw( tUserProfile::cProfileTurrets ) );
			newUserProfile->fSetSetting( tUserProfile::cSettingsShellCamInversion, mProfile->fInversion( tUserProfile::cProfileShellCam ) );
			newUserProfile->fSetSetting( tUserProfile::cSettingsShellCamSouthpaw, mProfile->fSouthPaw( tUserProfile::cProfileShellCam ) );
			newUserProfile->fSetSetting( tUserProfile::cSettingsPlanesInversion, mProfile->fInversion( tUserProfile::cProfilePlanes ) );
			newUserProfile->fSetSetting( tUserProfile::cSettingsPlanesSouthpaw, mProfile->fSouthPaw( tUserProfile::cProfilePlanes ) );
			newUserProfile->fSetSetting( tUserProfile::cSettingsVehiclesInversion, mProfile->fInversion( tUserProfile::cProfileVehicles ) );
			newUserProfile->fSetSetting( tUserProfile::cSettingsVehiclesSouthpaw, mProfile->fSouthPaw( tUserProfile::cProfileVehicles ) );
			newUserProfile->fSetSetting( tUserProfile::cSettingsCharacterInversion, mProfile->fInversion( tUserProfile::cProfileCharacters ) );
			newUserProfile->fSetSetting( tUserProfile::cSettingsCharacterSouthpaw, mProfile->fSouthPaw( tUserProfile::cProfileCharacters ) );
			newUserProfile->fSetSetting( tUserProfile::cSettingsDisableControllerVibe, mProfile->fSetting( tUserProfile::cSettingsDisableControllerVibe ) );
			newUserProfile->fSetBrightness( mProfile->fBrightness( ) );
			newUserProfile->fSetMusicVolume( mProfile->fMusicVolume( ) );
			newUserProfile->fSetSfxVolume( mProfile->fSfxVolume( ) );
			newUserProfile->fSetListeningMode( mProfile->fListeningMode( ) );
		}

		mProfile.fReset( newUserProfile );

		mCachedProfileInfo = false;
		mDeviceSelector.fReset( );
		//mGameController->fSetProfile( mProfile );

		fApplyProfileSettings( );
	}
	void tPlayer::fOnLevelLoadBegin( )
	{
		mState = cStateLoading;
		mWasFailing = false;
		fResetGameplayRelatedData( );
		mStats->fFillFriendsStats( );
		fConfigureAudio( );

		const tLevelLoadInfo& info = tGameApp::fInstance( ).fCurrentLevelLoadInfo( );

		if( info.mChallengeMode == GameFlags::cCHALLENGE_MODE_COMMANDO && info.mMapType == GameFlags::cMAP_TYPE_SURVIVAL )
		{
			mHasJetPack = mProfile->fEarnedJetPack( );
			if( !mHasJetPack )
				mJetpackQueryBoard.fRequestMobileOwnership( *mUser );
		}
		else
			mHasJetPack = false;
		
		if( tGameApp::fE3Mode( ) )
			mIsNotAllowedToSaveStats = false;
		else if( fUser( )->fIsGuest( ) )
			mIsNotAllowedToSaveStats = true;
		else if( info.mMapType == GameFlags::cMAP_TYPE_CAMPAIGN )
		{
			if( info.mLevelIndex > mProfile->fGetHighestLevelReached( info.mDlcNumber ) )
			{
				if( !tGameApp::fInstance( ).fLevelLockedByDefault( GameFlags::cMAP_TYPE_CAMPAIGN, info.mLevelIndex ) )
					mIsNotAllowedToSaveStats = false;
				else
					mIsNotAllowedToSaveStats = true;
			}
		}
		else
			mIsNotAllowedToSaveStats = false;
	}
	void tPlayer::fOnLevelUnloadBegin( )
	{
		mState = cStateLoading;
		fClearPersistentEffects( );
		fResetGameplayRelatedData( );
		fShowFocus( false );
		mFocusItems.fReset( );
		mStepCameras = true;
		fReleaseScoreUI( );
		fReleaseFocalPrompt( );
		fReleasePersonalBestUI( );
		fReleaseMiniMap( );
		fReleasePowerPoolUI( );
		fReleaseInUseIndicator( );
		fReleaseScreenSpaceNotifications( );
		fReleaseOutOfBoundsIndicator( );
		fReleasePointCaptureUI( );
		fReleaseAchievementBuyNotification( );
		if( mBarrageController )
		{
			mBarrageController->fOnDelete( );
			mBarrageController.fRelease( );
		}

		if( mWaveLaunchMenu && !mWaveLaunchMenu->fCanvas( ).fIsNull( ) ) 
		{
			// super bomb on fade out.
			mWaveLaunchMenu->fCanvas( ).fClearChildren( );
			mWaveLaunchMenu->fCanvas( ).fDeleteSelf( );
		}
		fHideShowOffensiveWaveMenu( false );

		// reset damage pulse
		const u32 vpId = mUser->fViewport( )->fViewportIndex( );
		tGameApp::fInstance( ).fPostEffectsManager( )->fFilmGrainPulse( vpId, 0.f, 0.f );

		mWasFailing = false;
		tGameApp::fInstance( ).fSoundSystem( )->fGlobalValues( )->fSetGameParam( AK::GAME_PARAMETERS::TOYBOX_COUNT, 0.f );
		fSoundSource( )->fHandleEvent( AK::EVENTS::STOP_NEARFAIL );

		mSelectedUnitChangedCallback = Sqrat::Function( );
		mCurrentUnitChangedCallback = Sqrat::Function( );
		mShowRemovedOncePerLevel = false;
		mDoesntWantRewind = false;
	}
	void tPlayer::fOnLevelUnloadEnd( )
	{
		fClearCameraStack( );

		mBarrageController.fRelease( );
		mFollowPathCam.fRelease( );
		fSetSelectedUnitLogic( NULL );

		fRunListRemove( cRunListCameraST );
		fRunListRemove( cRunListActST );
		fRemoveStandAloneFromSceneGraph( );
	}
	void tPlayer::fLevelOnSpawn( )
	{
		tLevelLogic *ll = tGameApp::fInstance( ).fCurrentLevel( );

		fCreateScoreUI( ); // Can't call this in constructor because the script file hasn't loaded yet
		fInitScoreUI( );
		fCreateInUseIndicator( );
		fCreatePowerPoolUI( );

		mBarrageController.fReset( NEW tBarrageController( this ) );

		// Only create certain ui elements for local players
		if( fUser( )->fIsLocal( ) )
		{
			fCreateFocalPrompt( );
			fCreatePersonalBestUI( );
			fCreateMiniMap( );		
			fInitMiniMap( );
			fCreateScreenSpaceNotifications( );
			fCreateOutOfBoundsIndicator( );
			fCreatePointCaptureUI( );

			if( !tGameApp::fInstance( ).fGameMode( ).fIsFrontEnd( ) )
			{	
				fCreateScreenSpaceNotifications( );
			}
		}

		fInsertStandAloneToSceneGraph( *tGameApp::fInstance( ).fSceneGraph( ) );
		fRunListInsert( cRunListActST );
		fRunListInsert( cRunListCameraST );
		fOnPause( false );
	}
	void tPlayer::fResetCameraAspectRatios( )
	{
		Gfx::tCamera camera = mUser->fViewport( )->fRenderCamera( );
		camera.fSetLens( fDefaultLens( fDefaultZoom( ) ) );
		mUser->fViewport( )->fSetCameras( camera );
	}
	void tPlayer::fOnSimulationBegin( )
	{
		mState = cStatePlaying;
		sync_event_v_c( mState, tSync::cSCPlayer );

		//if they've already brought up the creation menu, we need this:
		// EDIT: Randall Knapp - 10/13/2010
		//       - This crashes if player two is holding down the right trigger in co-op
		//fSetSelectedUnitLogic( mHoveredUnit );

		for( u32 i = 0; i < mCameraStack.fCount( ); ++i )
		{
			tRtsCamera* rtsCam = mCameraStack[ i ].fDynamicCast< tRtsCamera >( );
			if( rtsCam )
				rtsCam->fFindNewBlocker( );
		}

		//tGameApp::fInstance( ).fPostEffectsManager( )->fBeginFilmGrainFadeOut( fUser( )->fViewport( )->fViewportIndex( ), Renderer_PostEffects_FilmGrain_FadeInTime );
		
		//TS1
		//tGameApp& gameApp = tGameApp::fInstance( );
		//const tGamePostEffectManager::tInGameFilmGrainMode filmGrainMode = ( tGamePostEffectManager::tInGameFilmGrainMode )fGameSave( )->fFilmGrainEffect( );
		//if( gameApp.fIsHeadToHeadLocalOrNetwork( ) || filmGrainMode == tGamePostEffectManager::cInGameFilmGrainNone )
		//{
		//	gameApp.fPostEffectMgr( )->fBeginFilmGrainFadeOut( fUser( )->fGetId( ), Game_FilmGrainFadeInTime );
		//	gameApp.fPostEffectMgr( )->mInGameFilmGrainMode = tGamePostEffectManager::cInGameFilmGrainNone;
		//}
		//else
		//	gameApp.fPostEffectMgr( )->mInGameFilmGrainMode = filmGrainMode;
	}

	u32 tPlayer::fListenerIndex( ) const
	{
		return mUser->fLocalHwIndex( );
	}

	void tPlayer::fOnTick( f32 dt )
	{
		tGameApp& gameApp = tGameApp::fInstance( );
		tLevelLogic* level = gameApp.fCurrentLevel( );
		const Gfx::tViewportPtr& viewport = mUser->fViewport( );

		sync_event_v_c( mUser->fPlatformId( ), tSync::cSCPlayer );
		sync_event_v_c( mUser->fInputFilterLevel( ), tSync::cSCPlayer );

		fStepJetpackQuery( );

		switch( mState )
		{
		case cStateLoading:		fOnTickLoading( dt ); break;
		case cStatePlaying:		fOnTickPlaying( dt ); break;
		}

		if( mStatsEnabled )
		{
			sigassert( mStats );
			mStats->fUpdateLeaderBoards( );
		}

		const Math::tMat3f& cameraXform = viewport->fRenderCamera( ).fLocalToWorld( );
		if( fListenerIndex( ) != ~0 )
		{
			mAudioListener->fSetTransform( cameraXform, fListenerIndex( ) );
		}
		f32 height = 0.0f;
		if( level ) 
			height = cameraXform.fGetTranslation( ).y - gameApp.fCurrentLevel( )->fGroundHeight( );
		gameApp.fSoundSystem( )->fMasterSource( )->fSetGameParam( AK::GAME_PARAMETERS::HEIGHT, height );


		// update post effect sequences
		b32 filmGrainOn = false;
		if( mForceFilmGrainActive || 
			gameApp.fPostEffectsManager( ) 
			&& gameApp.fPostEffectsManager( )->fFilmGrainActive( viewport->fViewportIndex( ) ) )
			filmGrainOn = true;


		const tStringPtr pausedEffect = Renderer_PostEffects_PauseCam_Enable ? tGamePostEffectManager::fSeqNamePauseCam( viewport->fViewportIndex( ) ) : tStringPtr( );
		const tStringPtr normalEffect = Renderer_PostEffects_GameCam_Enable ? tGamePostEffectManager::fSeqNameGameCam( viewport->fViewportIndex( ) ) : tStringPtr( );
		const tStringPtr filmEffect = tGamePostEffectManager::fSeqNameFilmGrain( viewport->fViewportIndex( ) );

		b32 usePause = gameApp.fSceneGraph( )->fIsPaused( );
		if( usePause && fCameraStackCount( ) && fCameraStackTop( )->fDynamicCast<tTiltShiftCamera>( ) )
			usePause = false;

		const tStringPtr defaultEffect = usePause ? pausedEffect : normalEffect;
		const b32 filmGrainOverride = filmGrainOn || Renderer_PostEffects_FilmGrain_OverrideOn || ( FrontEnd_UseFilmGrain && gameApp.fGameMode( ).fIsFrontEnd( ) );
		const tStringPtr overrideEffect = filmGrainOverride ? filmEffect : defaultEffect;

		viewport->fSetPostEffectSequence( overrideEffect );

		mTimeSinceUnitExit += dt;

		fDeviceSelectorLogic( );
	}
	void tPlayer::fCameraST( f32 dt ) 
	{ 
		fStepCameraStack( dt );
	}
	void tPlayer::fSetUser( const tUserPtr & user )
	{
		sigassert( user && "Player's require a valid user at all times" );

		mStatsEnabled = true;
		mUserKicked = false;

		if( mUser ) //this is not set during constructor
			mUser->fRemoveGameRef( );

#ifdef sig_logging
		std::stringstream ss;
		ss << "PLAYER(" << this << ") changed user from ";
		if( mUser )
			ss << "hw" << mUser->fLocalHwIndex( ) << " - " << mUser.fGetRawPtr();
		else
			ss << "NULL";
		ss << " to hw" << user->fLocalHwIndex( ) << " - " << user.fGetRawPtr() << std::endl;
		log_output(0, ss.str());
#endif//sig_logging
		
		mUser = user;
		mUser->fAddGameRef( );

		fRefreshUserIds( );
		fReadProfile( );
		fResetGameplayRelatedData( );
		mStats->fFillFriendsStats( );
		//mGameController->fSetUser( mUser );
	}
	void tPlayer::fApplyProfileSettings( )
	{
		tGameApp& gameApp = tGameApp::fInstance( );
		sigassert( fProfile( ) );
		tUserProfile& profile = *fProfile( );

		gameApp.fReadLicense( );

		if( this == gameApp.fFrontEndPlayer( ) || !gameApp.fFrontEndPlayer( ) )
		{
			// Set Settings
			gameApp.fSetMusicVolume( profile.fMusicVolume( ) );
			gameApp.fSetSfxVolume( profile.fSfxVolume( ) );
			gameApp.fSetBrightness( profile.fBrightness( ) );
			gameApp.fSetListeningMode( profile.fListeningMode( ) );
			mDeviceSelector.fSetSaveDeviceId( profile.fSaveDeviceId( ) );
		}
	}
	void tPlayer::fKickUser( )
	{
		mUserKicked = true;
		//tGameApp::fInstance( ).fGameAppSession( )->fSendSessionEvent( ApplicationEvent::cOnPlayerSignOut, mUser->fLocalHwIndex( ) );
	}
	void tPlayer::fRefreshUserIds( )
	{
		mUserId = mUser->fPlatformId( );
		mOfflineUserId = mUser->fOfflinePlatformId( );
	}

	std::string tPlayer::fHudLayerName( ) const
	{
		std::stringstream ss;
		ss << "viewport" << fUser( )->fViewportIndex( );
		return ss.str( );
	}

	u32 tPlayer::fPlayerIndex( ) const
	{
		return tGameApp::fInstance( ).fWhichPlayer( mUser.fGetRawPtr( ) );
	}

	void tPlayer::fOnTickLoading( f32 dt )
	{
		if( !tGameApp::fInstance( ).fSceneGraph( )->fIsPaused( ) )
			fTransitionUseTurrets( );
	}

	namespace
	{
		b32 fAllowDpadSwitch( tUnitLogic* unit )
		{
			sigassert( unit );
			return (unit->fLogicType( ) == GameFlags::cLOGIC_TYPE_TURRET && unit->fStaticCast<tTurretLogic>( )->fOnVehicle( ) );
		}
	}

	void tPlayer::fOnTickPlaying( f32 dt )
	{
		fDebugOnlyToggleFreeCam( );

		mGameController->fOnTick( dt );

		tGameApp& app = tGameApp::fInstance( );

		if( !app.fSceneGraph( )->fIsPaused( ) )
		{
			if( fInUseMode( ) && (!mLockedInUnit || mAllowDPadSwitchWhileLocked || fAllowDpadSwitch( mCurrentUnit )) && !mDisableQuickSwitch )
			{
				tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevelDemand( );
				if( !level->fDisableQuickSwitch( ) && (!app.fSingleScreenCoopEnabled( ) || app.fSingleScreenControlPlayer( ) == this) )
					fProcessQuickSwitch( );
			}

#if defined( sig_devmenu )
			// TESTING
			// TODO Remove
			if( Debug_RandyMode && mScoreUI )
			{
				// A, B, X, Y, LShoulder, RShoulder
				const u32 buttonCount = 6;
				const u32 buttons[ buttonCount ] = { GameFlags::cGAME_CONTROLS_SELECT, GameFlags::cGAME_CONTROLS_CANCEL, GameFlags::cGAME_CONTROLS_HANDBRAKE, GameFlags::cGAME_CONTROLS_ENTER_EXIT_UNIT, 
					GameFlags::cGAME_CONTROLS_ARTILLERY_ROTATE_LEFT, GameFlags::cGAME_CONTROLS_ARTILLERY_ROTATE_RIGHT };

				for( u32 i = 0; i < buttonCount; ++i )
				{
					if( fGameController( )->fButtonHeld( tUserProfile::cProfileCamera, GameFlags::cGAME_CONTROLS_MENU_REWIND ) && fGameController( )->fButtonDown( tUserProfile::cProfileCamera, buttons[ i ] ) )
						mScoreUI->Test( i, buttons[ i ], this, false );
					if( fGameController( )->fButtonHeld( tUserProfile::cProfileCamera, GameFlags::cGAME_CONTROLS_MENU_REWIND ) && fGameController( )->fButtonHeld( tUserProfile::cProfileCamera, buttons[ i ] ) )
						mScoreUI->Test( i, buttons[ i ], this, true );
				}
			}
#endif
			sigassert( mStats );
			mStats->fUpdate( dt );

			fTransitionUseTurrets( );
			fUseBarrageCheck( dt );
			fUseOverChargeCheck( dt );
			fUpdateWaveLaunchMenu( );

			if( mCurrentUnit )
			{
				// Update battery meter
				tVehicleLogic* vehicle = mCurrentUnit->fOwnerEntity( )->fLogicDerived< tVehicleLogic >( );
				if( vehicle && mBatteryMeter )
					mBatteryMeter->fSet( vehicle->fPowerLevel( ) );
			}			

			if( mMiniMap )
				mMiniMap->fUpdate( );

			fStepFocus( dt );
			fStepPersistentEffects( dt );

			if( !tGameApp::fInstance( ).fGameMode( ).fIsFrontEnd( ) )
			{	
				// TODO POWERPOOL/COMBOS
				//// combo timers
				//for( u32 i = 0; i < mComboTimers.fCount( ); ++i )
				//	mComboTimers[ i ]->fStep( mStats->fComboGroups( )[ i ], dt );

				if( mPowerPool )
				{
					f32 comboMeter = mStats->fComboMeterValue( );
					if( comboMeter > 1.f )
						comboMeter = 1.f + mStats->fBarrageMeterPercent( );

					mPowerPool->fStep( comboMeter, mStats->fComboMeterTimerPercentage( ), dt );
				}
			}

			fCheckToyBoxAlarm( dt );

			fStepTutorial( dt );

			fStats( ).fIncQuick( GameFlags::cSESSION_STATS_TOTAL_TIME, dt );
			if( mCurrentUnit )
			{
				fStats( ).fIncQuick( GameFlags::cSESSION_STATS_TIME_IN_UNITS, dt );
				if( mCurrentUnit->fDynamicCast<tVehicleLogic>( ) )
					fStats( ).fIncQuick( GameFlags::cSESSION_STATS_TIME_IN_VEHICLES, dt );
			}

			if( mBombingRunTimer > 0.f )
			{
				mBombingRunTimer -= dt;
				if( mBombingRunTimer < 0.f )
				{
					mBombingRunValues.fPushBack( mBombingRunCount );
					u32 value = mBombingRunCount * tGameSessionStats::fBonusValue( GameFlags::cSESSION_STATS_BOMBING_RUN );

					tLocalizedString moneyLoc = tLocalizedString::fFromCString( " " );
					moneyLoc.fJoinWithCString( StringUtil::fToString( value ).c_str( ) );

					fSpawnBonusTextWithLocSuffix( GameFlags::cSESSION_STATS_BOMBING_RUN, moneyLoc, mBombingRunLastKillPos );
					tGameApp::fInstance( ).fCurrentLevelDemand( )->fExtraMiniGamePoints( (f32)value );

					mStats->fMaxStat( GameFlags::cSESSION_STATS_BOMBING_RUN, (f32)value );
					mBombingRunCount = 0;
				}
			}	

			if( !tGameApp::fInstance( ).fIsFullVersion( ) )
			{
				if( tGameApp::fInstance( ).fCurrentLevelLoadInfo( ).mMapType == GameFlags::cMAP_TYPE_SURVIVAL
					&& !mProfile->fIncrementSurvivalTimer( dt ) )
				{
					if( fStats( ).fStat( GameFlags::cSESSION_STATS_HIGHEST_SURVIVAL_ROUND ) > 1 )
					{
						tGameApp::fInstance( ).fAskPlayerToBuyGame( );
					}
				}
			}
		}
	}
	void tPlayer::fStepJetpackQuery( )
	{
		if( !mJetpackQueryBoard.fIdle( ) && mJetpackQueryBoard.fAdvanceRead( ) )
		{
			mHasJetPack = mJetpackQueryBoard.fHasOwnerShip( );
		}
	}
	void tPlayer::fProcessQuickSwitch( )
	{
		sigassert( mCurrentUnit );

		// quick jump to turrets with dpad
		tEntity* turret = NULL;

		//only select units with this logic ancestor
		tEntity* owner = NULL;

		if( mCurrentUnit->fLogicType( ) != GameFlags::cLOGIC_TYPE_TURRET )
			return;

		if( fGameController( )->fButtonDown( tUserProfile::cProfileTurrets, GameFlags::cGAME_CONTROLS_UNIT_QUICKSWITCH_FORWARD ) || fGameController( )->fButtonDown( tUserProfile::cProfileTurrets, GameFlags::cGAME_CONTROLS_UNIT_QUICKSWITCH_BEHIND ) ||
			fGameController( )->fButtonDown( tUserProfile::cProfileTurrets, GameFlags::cGAME_CONTROLS_UNIT_QUICKSWITCH_LEFT ) || fGameController( )->fButtonDown( tUserProfile::cProfileTurrets, GameFlags::cGAME_CONTROLS_UNIT_QUICKSWITCH_RIGHT ) )
		{
			tVehicleLogic* ownerVehicle = mCurrentUnit->fOwnerEntity( )->fFirstAncestorWithLogicOfType<tVehicleLogic>( );
			if( ownerVehicle )
				owner = ownerVehicle->fOwnerEntity( );

			Math::tVec3f dpadInput( 0 );

			if( fGameController( )->fButtonDown( tUserProfile::cProfileTurrets, GameFlags::cGAME_CONTROLS_UNIT_QUICKSWITCH_FORWARD ) )	dpadInput.z += 1.f;
			if( fGameController( )->fButtonDown( tUserProfile::cProfileTurrets, GameFlags::cGAME_CONTROLS_UNIT_QUICKSWITCH_BEHIND ) )	dpadInput.z -= 1.f;
			if( fGameController( )->fButtonDown( tUserProfile::cProfileTurrets, GameFlags::cGAME_CONTROLS_UNIT_QUICKSWITCH_LEFT ) )	dpadInput.x += 1.f;
			if( fGameController( )->fButtonDown( tUserProfile::cProfileTurrets, GameFlags::cGAME_CONTROLS_UNIT_QUICKSWITCH_RIGHT ) )	dpadInput.x -= 1.f;

			turret = fFindTurretInDirection( dpadInput, owner );

			if( turret )
			{
				tTurretLogic* dest = turret->fLogicDerived<tTurretLogic>( );

				if( tGameApp::fInstance( ).fSingleScreenCoopEnabled( ) )
				{
					tPlayer* op = tGameApp::fInstance( ).fOtherPlayer( this );
					if( op->fCurrentUnit( ) == mCurrentUnit )
						op->fQuickSwitchToUnitImp( dest );
				}

				fQuickSwitchToUnitImp( dest );
			}
		}
	}
	void tPlayer::fQuickSwitchToUnitImp( tTurretLogic* logic )
	{
		if( logic )
		{
			tTurretLogic* previousTurret = mCurrentUnit->fDynamicCast<tTurretLogic>( );
			mQuickSwitchTurret = (previousTurret && previousTurret->fQuickSwitchCamera( ));	

			b32 allowDPad = mAllowDPadSwitchWhileLocked;
			b32 lockWhenFinished = mLockedInUnit;

			fSetDontResetComboOnExit( true );
			fLockInUnitDirect( logic );

			if( !lockWhenFinished )
				fUnlockFromUnit( false );

			mAllowDPadSwitchWhileLocked = allowDPad;

			if( mOverChargeActive )
			{
				++mQuickSwitchesWhileOvercharged;
				fStats( ).fMaxStat( GameFlags::cSESSION_STATS_MOST_HOTSWAPS_IN_A_SINGLE_TURBOCHARGE, (f32)mQuickSwitchesWhileOvercharged );
			}
		}
	}
	namespace
	{
		b32 fContainsEnemy( tProximityLogic* prox, u32 team )
		{
			b32 inZone = false;

			if( prox->fEntityList( ).fCount( ) > 0 )
			{
				for( u32 i = 0; i< prox->fEntityList( ).fCount( ); ++i )
				{
					tUnitLogic* ul = prox->fEntityList( )[ i ]->fLogicDerived<tUnitLogic>( );
					if( ul && ul->fTeam( ) == team )
					{
						inZone = true;

						if( !ul->fInAlarmZone( ) )
						{
							ul->fSetInAlarmZone( true );

							tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
							if( level )
							{
								tTutorialEvent event( GameFlags::cTUTORIAL_EVENT_UNIT_IN_ALARM_ZONE );
								event.mEventUnitType = ul->fUnitType( );
								event.mEntity = ul->fOwnerEntity( );
								level->fHandleTutorialEvent( event );
							}
						}
					}						
				}
			}

			return inZone;
		}
	}
	void tPlayer::fCheckToyBoxAlarm( f32 dt )
	{
		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		if( !level || !mScoreUI )
			return;

		mToyBoxAlarmTimer -= dt;
		if( mToyBoxAlarmTimer <= 0.f )
		{
			mToyBoxAlarmTimer = Gameplay_Player_ToyBoxAlarmTimeoutShort;

			const tGrowableArray<tProximityLogic*>& alarmLogics = level->fToyBoxAlarms( fTeam( ) );
			for( u32 a = 0; a < alarmLogics.fCount( ); ++a )
			{
				if( fContainsEnemy( alarmLogics[ a ], fEnemyTeam( ) ) )
				{
					if( fUser( )->fIsLocal( ) )
					{
						mScoreUI->fProtectToyBox( );
					}
					mToyBoxAlarmTimer = Gameplay_Player_ToyBoxAlarmTimeout;
				}
			}
		}
	}
	void tPlayer::fStepTutorial( f32 dt )
	{
		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );

		if( level && tGameApp::fInstance( ).fSingleScreenControlPlayer( ) == this )
		{
			// process all input as tutorial event
			tTutorialEvent event( GameFlags::cTUTORIAL_EVENT_INPUT, 0 );
			if( mCurrentUnit )
			{
				event.mCurrentUnitID = mCurrentUnit->fUnitID( );
				if( mCurrentUnit->fHasWeapon( 0, 0, 0 ) )
					event.mShellCaming = mCurrentUnit->fWeaponRawPtr( 0, 0, 0 )->fShellCaming( );
			}

			// Check for gamepad input
			//for( u32 i = 0; i < 32; ++i )
			//{
			//	u32 buttonMask = 1 << i;
			//	if( fGamepad( ).fButtonDown( buttonMask ) )
			//	{
			//		event.mEventValue = buttonMask;
			//		level->fHandleTutorialEvent( event );
			//	}
			//}
			for( u32 cp = tUserProfile::cProfileBegin; cp < tUserProfile::cProfileEnd; ++cp )
			{
				event.mControlProfile = cp;
				for( u32 i = 0; i < GameFlags::cGAME_CONTROLS_COUNT; ++i )
				{
					if( fGameController( )->fButtonDown( cp, i ) )
					{
						event.mEventValue = i;
						level->fHandleTutorialEvent( event );
					}
				}
			}
		}
	}
	void tPlayer::fStepCameraStack( f32 dt )
	{
		if( mStepCameras )
		{
			mCameraStack.fStep( dt );
			if( !mFollowPathCamActive )
			{
				// If we lost the followpathcam somehow, make sure we're no longer waiting to leave it.
				mWaitingToLeaveFollowPathCam = false;
				mFollowPathCam.fRelease( );
			}
			// Check if we're waiting for input to leave the followpathcam.
			if( mWaitingToLeaveFollowPathCam )
			{
				// Any input this frame?
				if( fGameController( )->fIsActive( ) )
				{
					// Switch away from the followpathcam now.
					mWaitingToLeaveFollowPathCam = false;
					mFollowPathCamActive = false;

					if( mCameraStack.fPopCamerasOfType<tFollowPathCam>( ) )
						fSetDisableTiltShift( false );
				}
			}
		}
	}
	void tPlayer::fClearCameraStack( )
	{
		mCameraStack.fClear( );
	}
	void tPlayer::fPushCamera( const Gfx::tCameraControllerPtr& camera )
	{
		// push before follow cams, so they're uninterrupted
		mCameraStack.fPushCamera( camera, mCameraStack.fIndexOfType<tFollowPathCam>( ) );
	}
	void tPlayer::fPopCamera( )
	{
		if( mCameraStack.fCount( ) == 1 )
			log_warning( 0, "Popping last camera from the stack." );
		mCameraStack.fPopCamera( );
	}
	void tPlayer::fPushFreeCamera( tEntity* startPoint )
	{
		Gfx::tTripod cameraTripod;

		if( startPoint )
			cameraTripod = Gfx::tTripod( startPoint->fObjectToWorld( ) );
		else
		{
			if( !mCameraStack.fIsEmpty( ) )
				cameraTripod = mUser->fViewport( )->fLogicCamera( ).fGetTripod( );
			else
			{
				Math::tMat3f cameraXform = Math::tMat3f::cIdentity;
				cameraXform.fSetTranslation( Math::tVec3f( 0.f, 15.f, 120.f ) );
				cameraXform.fOrientZAxis( -cameraXform.fGetTranslation( ).fNormalize( ) );
				cameraTripod = Gfx::tTripod( cameraXform );
			}
		}

		fSetDefaultCameraParameters( cameraTripod );
		fPushCamera( Gfx::tCameraControllerPtr( NEW Gfx::tDebugFreeCamera( mUser ) ) );
	}
	void tPlayer::fPushFrontEndCamera( tEntity* startPoint )
	{
		Math::tMat3f cameraXform = Math::tMat3f::cIdentity;
		if( startPoint )
			cameraXform = startPoint->fObjectToWorld( );
		else
		{
			cameraXform.fSetTranslation( Math::tVec3f( 0.f, 15.f, 120.f ) );
			cameraXform.fOrientZAxis( -cameraXform.fGetTranslation( ).fNormalize( ) );
		}

		fSetDefaultCameraParameters( Gfx::tTripod( cameraXform ) );
		fPushCamera( Gfx::tCameraControllerPtr( NEW tFrontEndCamera( *this ) ) );
	}
	void tPlayer::fPushRTSCamera( tEntity* startPoint )
	{
		Gfx::tTripod tripod;

		if( tGameApp::fInstance( ).fCurrentSaveGame( ) )
		{
			tripod = tGameApp::fInstance( ).fCurrentSaveGame( )->mPlayerData[ fPlayerIndex( ) ].mCamera;
		}
		else
		{			
			Math::tMat3f cameraXform = Math::tMat3f::cIdentity;
			if( startPoint )
				cameraXform = startPoint->fObjectToWorld( );
			else
			{
				cameraXform = fUser( )->fViewport( )->fRenderCamera( ).fLocalToWorld( );

				/*cameraXform.fSetTranslation( Math::tVec3f( 0.f, 15.f, 120.f ) );
				cameraXform.fOrientZAxis( -cameraXform.fGetTranslation( ).fNormalize( ) );*/
			}

			tripod = Gfx::tTripod( cameraXform );
		}

		fSetDefaultCameraParameters( tripod );
		fPushCamera( Gfx::tCameraControllerPtr( NEW tRtsCamera( *this ) ) );
	}
	void tPlayer::fPushTransitionCamera( tEntity* endPoint )
	{
		Math::tMat3f startXform = Math::tMat3f::cIdentity;
		Math::tMat3f endXform = Math::tMat3f::cIdentity;

		if( !mCameraStack.fIsEmpty( ) )
		{
			const Gfx::tTripod& tripod = mCameraStack.fTop( )->fViewport( )->fLogicCamera( ).fGetTripod( );
			startXform.fSetTranslation( tripod.mEye );
			startXform.fOrientZAxis( ( tripod.mLookAt - tripod.mEye ).fNormalize( ) );
		}

		if( endPoint )
			endXform = endPoint->fObjectToWorld( );
		else
		{
			endXform.fSetTranslation( Math::tVec3f( 0.f, 15.f, 120.f ) );
			endXform.fOrientZAxis( -endXform.fGetTranslation( ).fNormalize( ) );
		}

		fPushCamera( Gfx::tCameraControllerPtr( NEW tTransitionCamera( *this, startXform, endXform, 5.f ) ) );
	}

	void tPlayer::fReadProfile( )
	{
		if( !mUser->fIsLocal( ) )
			return;

		if( !fActuallyReadProfile( ) )
		{
			if( mUser->fSignedIn( ) && !mUser->fCanWriteToProfile( ) )
			{
				fKickUser( );
				tGameApp::fInstance( ).fShowErrorDialog( tGameApp::cCouldntSaveProfile, mUser.fGetRawPtr( ) );
			}
			else
			{
				//// this is what we were missing in the base game :(
				//tGameApp::fInstance( ).fShowErrorDialog( tGameApp::cWrongProfileVersion, mUser.fGetRawPtr( ) );
				//fKickUser( );

				fSaveDefaultProfile( );
			}
		}

		fApplyProfileSettings( );
	}

	namespace
	{
		//// fUpgrade would be responsible for descrypting previous data, and promting it.
		////  Decryption will be skipped after uprading
		//b32 fUpgradeProfile( tUserProfileHeader* header, byte* data )
		//{
		//	if( header->mVersion == tUserProfile::cCertVersion )
		//	{
		//		log_line( 0, "Promoting profile: " << header->mVersion << " to " << tUserProfile::cVersion );
		//		return true; //nothing to do already good to go :)
		//	}

		//	return false;
		//}

		//b32 fFixProfile( tUserProfile* profile, tUserProfileHeader* oldHeader, byte* oldDta )
		//{
		//	if( oldHeader->mVersion != tUserProfile::cVersion )
		//	{
		//		// we need to reset any rewind data. the old stuff was not encrypted and the new stuff is.
		//		profile->fInvalidateLastRewind( );
		//		return true;
		//	}

		//	return false;
		//}
	}

	b32 tPlayer::fActuallyReadProfile( )
	{
		mCachedProfileInfo = false;
		mProfile.fReset( NEW tUserProfile( ) );
		//mGameController->fSetProfile( mProfile );

		const u32 cHeaderSize = sizeof( tUserProfileHeader );
		const u32 cPresumedProfileSize = 3 * (u32)cSettingsMaxSize; // 3 settings at 1000 bytes each
		const u32 cDataSize = cPresumedProfileSize;
		tDynamicArray<byte> data;
		data.fResize( cPresumedProfileSize );

		if( !mUser->fReadProfile( data.fBegin( ), cDataSize ) ) 
			return false;

		tUserProfileHeader* header = reinterpret_cast< tUserProfileHeader* >( data.fBegin( ) );
		if( !header || !header->fValid( ) )
			return false;

		//b32 decypt = false; //disabling encryption due to complications with the base game destroying data.
		//b32 hadtoUpgrade = false;

		if( header->mVersion != tUserProfile::cVersion )
		{
			log_warning( 0, "Stored profile is of wrong version." );
			return false;

			//if( header->mVersion > tUserProfile::cVersion || !fUpgradeProfile( header, data.fBegin( ) ) )
			//{
			//	log_warning( 0, "Stored profile is of wrong version." );
			//	return false;
			//}
			//else
			//{
			//	hadtoUpgrade = true;
			//	decypt = false;
			//}
		}

		mAchievementsReader.fReset( NEW tAchievementsReader );
		mAchievementsReader->fSelectUser( mUser->fLocalHwIndex( ) );
		mAchievementsReader->fRead( 0, GameFlags::cACHIEVEMENTS_COUNT, tAchievementsReader::cDetailsLabel | tAchievementsReader::cDetailsDescription );

		tGameArchiveLoad archive( data.fBegin( ) + cHeaderSize, header->mSize );
		//if( decypt && !archive.fDecrypt( ) )
		//{
		//	log_warning( 0, "Could not decrypt profile." );
		//	return false;
		//}

		archive.fSaveLoad( *mProfile );

		// Moved to tPlayer::fTick( ) because we cannot block wait on PC
		/*
		// Wait on the achievements if we're still parsing
		if( achievements.fState( ) == tAchievementsReader::cStateReading )
			achievements.fWait( );

		if( achievements.fState( ) != tAchievementsReader::cStateSuccess )
		{
			log_warning( 0, "Could not read acheivements!" );
			return false;
		}

		//read achievement status
		u32 achievementMask = 0;
		for( u32 i = 0; i < GameFlags::cACHIEVEMENTS_COUNT; ++i )
		{
			if( achievements.fIsAwarded( GameSession::cAchievementIds[ i ] ) )
				achievementMask = fSetBits( achievementMask, (1<<i) );

			achievements.fGetData( GameSession::cAchievementIds[ i ], mAchievementData[ i ] );
		}
		
		mProfile->fSetAchievementMask( achievementMask );

		//if( hadtoUpgrade )
		//{
		//	fFixProfile( mProfile.fGetRawPtr( ), header, data.fBegin( ) );
		//}
		*/
		return true;
	}

	const tAchievementData& tPlayer::fGetAchievementData( u32 index )
	{
		sigassert( index >= 0 );
		sigassert( index < GameFlags::cACHIEVEMENTS_COUNT );
		sigassert( index < mAchievementData.fCount( ) );

		return mAchievementData[ index ];
	}

	void tPlayer::fTick( )
	{
		if( mAchievementsReader )
		{
			if( mAchievementsReader->fAdvanceRead( ) )
			{
				if( mAchievementsReader->fState( ) != tAchievementsReader::cStateSuccess )
				{
					log_warning( 0, "Could not read achievements!" );
				}
				else
				{
					//read achievement status
					u32 achievementMask = 0;
					for( u32 i = 0; i < GameFlags::cACHIEVEMENTS_COUNT; ++i )
					{
						if( mAchievementsReader->fIsAwarded( GameSession::cAchievementIds[ i ] ) )
							achievementMask = fSetBits( achievementMask, (1<<i) );

						mAchievementsReader->fGetData( GameSession::cAchievementIds[ i ], mAchievementData[ i ] );
					}
		
					mProfile->fSetAchievementMask( achievementMask );
				}
				mAchievementsReader.fRelease( );
			}
		}
	}

	namespace
	{

		class tProfileSaveInstance : public Gui::tSaveUI::tSaveInstance
		{	
		private:
			tPlayerPtr mPlayer;
			tGrowableArray<byte> mData;
			Threads::tThread mThread;	

		public:
			tProfileSaveInstance( const tPlayerPtr& player, tGrowableArray<byte>& data )
				: mPlayer( player )
			{
				data.fDisown( mData );
			}

			virtual void fBegin( )
			{
				mStarted = true;
				mThread.fStart( &tProfileSaveInstance::fThreadMain, "tProfileSaveInstance::ThreadedMain", this );	
			}

			virtual b32 fFinished( )
			{
				return !mThread.fRunning( );
			}	

			static u32 thread_call fThreadMain( void* threadParam )
			{
				tProfileSaveInstance* me = static_cast<tProfileSaveInstance*>( threadParam );

				u32 retValue = 1;

				const u32 cHeaderSize = sizeof( tUserProfileHeader );
				tUserProfileHeader header;

				header.mVersion = tUserProfile::cVersion;
				header.mSize = me->mData.fCount( );

				tDynamicArray<byte> data;
				data.fResize( header.mSize + cHeaderSize );

				memcpy( data.fBegin( ), (byte*)&header, cHeaderSize );
				memcpy( data.fBegin( ) + cHeaderSize, me->mData.fBegin( ), me->mData.fCount( ) );	

				if( me->mPlayer->fUser( )->fWriteToProfile( data.fBegin( ), data.fCount( ) ) )
					retValue = 0;
				else
					me->mErrored = true;

				return retValue;
			}

			void fPostErrors( )
			{
				if( mErrored )
				{
					mPlayer->fSetProfileDataIsCached( );
					tGameApp::fInstance( ).fShowErrorDialog( tGameApp::cCouldntSaveProfile, mPlayer->fUser( ).fGetRawPtr( ) );
				}
			}

		};

	}

	void tPlayer::fSaveProfile( )
	{
		if( fCanWriteProfile( ) )
		{
			fAwardAvatarAward( GameFlags::cAVATAR_AWARDS_GAMER_PIC_BUY );

			tGameArchiveSave archive;
			archive.fSaveLoad( *mProfile );
			//archive.fEncrypt( ); // no encryption until future tu :(

			
			if( mCachedProfileInfo )
			{
				//write achievements out also

				tGrowableArray<u32> achieve;
				achieve.fSetCapacity( GameFlags::cACHIEVEMENTS_COUNT );
				for( u32 i = 0; i < GameFlags::cACHIEVEMENTS_COUNT; ++i )
					if( mProfile->fAchievementAwarded( i ) )
						achieve.fPushBack( GameSession::cAchievementIds[ i ] );

				if( achieve.fCount( ) )
					tGameApp::fInstance( ).fAddSaveInstance( Gui::tSaveUI::tSaveInstancePtr( NEW tAchievementsWriter( mUser->fLocalHwIndex( ), achieve.fCount( ), achieve.fBegin( ) ) ) );
			}

			// set flag false here because it may get set true during save.
			mCachedProfileInfo = false;
			tGameApp::fInstance( ).fAddSaveInstance( Gui::tSaveUI::tSaveInstancePtr( NEW tProfileSaveInstance( tPlayerPtr( this ), archive.fBuffer( ) ) ) );
		}
		else
			mCachedProfileInfo = true;
	}

	void tPlayer::fSaveDefaultProfile( )
	{
		fResetProfile( );
		fSaveProfile( );
	}

	void tPlayer::fSaveFullyCompleteProfile( )
	{
		mProfile->fCompleteEverything( );
		fSaveProfile( );
	}

	void tPlayer::fSetProfile( tUserProfile * profile )
	{
		mProfile.fReset( profile );
		//mGameController->fSetProfile( mProfile );
	}

	b32 tPlayer::fCanWriteProfile( )
	{
		return ( mUser->fIsLocal( ) && mUser->fSignedIn( ) && tGameApp::fInstance( ).fIsFullVersion( ) && tGameApp::fInstance( ).fHasEverReachedFrontEnd( ) );
	}

	namespace
	{
		b32 fAchievementValidForTrial( u32 index )
		{
			return true; //all valid :)
			//return (index == GameFlags::cACHIEVEMENTS_DEMOLITION_MAN) || (index == GameFlags::cACHIEVEMENTS_SYNERGY);
		}
	}

	void tPlayer::fAwardAchievement( u32 index )
	{
		if( mUser->fIsGuest( ) )
			return;

		if( !tGameApp::fInstance( ).fIsFullVersion( ) )
		{
			if( !fAchievementValidForTrial( index ) )
				return;

			if( fTestBits( mSessionAchievementMask, (1<<index) ) )
				return; //already future awarded
		}

		mSessionAchievementMask = fSetBits( mSessionAchievementMask, (1<<index) );

		log_line( 0, "----- Achievememt: " << GameFlags::fACHIEVEMENTSEnumToString( index ) );

		if( tGameApp::fInstance( ).fIsFullVersion( ) )
		{
			tGameApp::fInstance( ).fAddSaveInstance( 
				Gui::tSaveUI::tSaveInstancePtr( NEW tAchievementsWriter( mUser->fLocalHwIndex( ), 1, &GameSession::cAchievementIds[ index ] ) ) );

			if( !fAchievementAwarded( index ) )
			{
				mProfile->fAwardAchievement( index );
				fAddEarnedItem( tEarnedItemData( tEarnedItemData::cAchievement, index ) );
				fSaveProfile( );
			}
		}
		else
		{
			fCreateAchievementBuyNotification( index );
			fAddEarnedItem( tEarnedItemData( tEarnedItemData::cAchievement, index, true ) );
		}

	}

	void tPlayer::fAwardAchievementDeferred( u32 index )
	{
		if( mUser->fIsGuest( ) )
			return;

		if( !tGameApp::fInstance( ).fIsFullVersion( ) )
		{
			if( !fAchievementValidForTrial( index ) )
				return;

			if( fTestBits( mSessionAchievementMask, (1<<index) ) )
				return; //already future awarded
		}

		mSessionAchievementMask = fSetBits( mSessionAchievementMask, (1<<index) );

		log_line( 0, "----- Deferred Achievememt: " << GameFlags::fACHIEVEMENTSEnumToString( index ) );

		if( !fAchievementAwarded( index ) )
		{
			fAddEarnedItem( tEarnedItemData( tEarnedItemData::cAchievement, index, true ) );
		}	

		if( !tGameApp::fInstance( ).fIsFullVersion( ) )
		{
			fCreateAchievementBuyNotification( index );
		}

	}

	void tPlayer::fAwardAvatarAward( u32 index )
	{
		if( mUser->fIsGuest( ) )
			return;
#if defined( use_avatar_awards )
		if( !tGameApp::fInstance( ).fIsFullVersion( ) )
		{
			if( fTestBits( mSessionAvatarMask, (1<<index) ) )
				return; //already future awarded
		}

		mSessionAvatarMask = fSetBits( mSessionAvatarMask, (1<<index) );

		log_line( 0, "----- Avatar Award: " << GameFlags::fAVATAR_AWARDSEnumToValueString( index ) );

		if( tGameApp::fInstance( ).fIsFullVersion( ) )
		{
			if( index < GameFlags::cAVATAR_AWARDS_DIVIDER )
				mUser->fAwardAvatar( GameSession::cAvatarAwardIds[ index ] );
			else 
				mUser->fAwardGamerPicture( GameSession::cGamerPicsIds[ index ] );

			if( !mProfile->fAvatarAwarded( index ) )
			{
				mProfile->fAwardAvatar( index );
				if( index < GameFlags::cAVATAR_AWARDS_DIVIDER )
					fAddEarnedItem( tEarnedItemData( tEarnedItemData::cAvatarAward, index ) );
				fSaveProfile( );
			}
		}
		else
		{
			if( index < GameFlags::cAVATAR_AWARDS_DIVIDER )
			{
				fAddEarnedItem( tEarnedItemData( tEarnedItemData::cAvatarAward, index, true ) );
				fCreateAvatarAwardBuyNotification( index );
			}
		}
#endif
	}

	void tPlayer::fAwardSessionAwards( )
	{
		// only for the first time the game was purchased
		for( u32 i = 0; i < GameFlags::cACHIEVEMENTS_COUNT; ++i )
			if( fTestBits( mSessionAchievementMask, (1<<i) ) )
				fAwardAchievement( i );

		for( u32 i = 0; i < GameFlags::cAVATAR_AWARDS_COUNT; ++i )
			if( fTestBits( mSessionAvatarMask, (1<<i) ) )
				fAwardAvatarAward( i );
	}

	namespace
	{
#if defined( platform_pcdx )
		devvar( bool, Game_HaveDlcOverride, true );
		devvar( bool, Game_HaveDlc1Napalm, true );
		devvar( bool, Game_HaveDlc2SpecOps, true );
		devvar( bool, Game_HaveTS1Content, false );
#else
		devvar( bool, Game_HaveDlcOverride, false );
		devvar( bool, Game_HaveDlc1Napalm, false );
		devvar( bool, Game_HaveDlc2SpecOps, false );
		devvar( bool, Game_HaveTS1Content, false );
#endif

		b32 fTestDLC( u32 mask, u32 index )
		{
			if( tGameApp::fInstance( ).fIsFullVersion( ) )
				mask = fSetBits( mask, 1u << GameFlags::cDLC_COLD_WAR );

#if defined( sig_devmenu ) || defined( platform_pcdx )
			if( Game_HaveDlcOverride )
			{
				if( Game_HaveDlc1Napalm )
					mask = fSetBits( mask, 1u << GameFlags::cDLC_NAPALM );
				else
					mask = fClearBits( mask, 1u << GameFlags::cDLC_NAPALM );

				if( Game_HaveDlc2SpecOps )
					mask = fSetBits( mask, 1u << GameFlags::cDLC_EVIL_EMPIRE );
				else
					mask = fClearBits( mask, 1u << GameFlags::cDLC_EVIL_EMPIRE );

				if( Game_HaveTS1Content )
				{
					mask = fSetBits( mask, 1u << GameFlags::cDLC_BRITISH );
					mask = fSetBits( mask, 1u << GameFlags::cDLC_GERMAN );
					mask = fSetBits( mask, 1u << GameFlags::cDLC_INVASION );
					mask = fSetBits( mask, 1u << GameFlags::cDLC_KAISER );
				}
				else
				{
					mask = fClearBits( mask, 1u << GameFlags::cDLC_BRITISH );
					mask = fClearBits( mask, 1u << GameFlags::cDLC_GERMAN );
					mask = fClearBits( mask, 1u << GameFlags::cDLC_INVASION );
					mask = fClearBits( mask, 1u << GameFlags::cDLC_KAISER );
				}
			}
#endif
			return mask & ( 1 << index );
		}
	}

	// this means the player ahs the license to play the dlc, and its installed
	b32 tPlayer::fHasDLC( u32 dlcIndex ) const
	{
		tGameApp::fInstance( ).fSetAddonFlags( );

		return fTestDLC( mUser->fAddOnsLicensed( ), dlcIndex );
	}

	// this means the dlc is installed but hte player is not authorized to play it
	b32 tPlayer::fInstalledDLC( u32 dlcIndex ) const
	{
		tGameApp::fInstance( ).fSetAddonFlags( );

		return fTestDLC( mUser->fAddOnsInstalled( ), dlcIndex );
	}

	namespace
	{
		devvar_clamp( f32, Cameras_Defaults_NormalScale, 1.0f, 0.01f, 2.00f, 2 );
		devvar_clamp( f32, Cameras_Defaults_SplitScreenScale, 0.8f, 0.01f, 2.00f, 2 );
		devvar_clamp( f32, Cameras_Defaults_RTSZoom, 1.0f, 0.01f, 2.00f, 2 );
		devvar_clamp( f32, Cameras_Defaults_TransitionZoom, 1.3f, 0.01f, 2.00f, 2 );
	}
	f32 tPlayer::fDefaultZoom( ) const
	{
		return Cameras_Defaults_RTSZoom;
	}
	f32 tPlayer::fDefaultTranstionCamZoom( ) const
	{
		return Cameras_Defaults_TransitionZoom;
	}
	Gfx::tLens tPlayer::fDefaultLens( f32 zoom ) const
	{
		const Gfx::tViewportPtr& viewport = mUser->fViewport( );
		const f32 aspectRatio = mUser->fAspectRatio( );
		const f32 scale = tGameApp::fInstance( ).fGameMode( ).fIsSplitScreen( ) && !tGameApp::fInstance( ).fSingleScreenCoopEnabled( ) ? Cameras_Defaults_SplitScreenScale : Cameras_Defaults_NormalScale;
		return  Gfx::tLens( 1.0f, 8000.f, scale, scale / aspectRatio, Gfx::tLens::cProjectionPersp, zoom );		
	}
	void tPlayer::fSetDefaultCameraParameters( const Gfx::tTripod& cameraTripod )
	{
		const Gfx::tViewportPtr& viewport = mUser->fViewport( );
		viewport->fSetCameras( Gfx::tCamera( fDefaultLens( Cameras_Defaults_RTSZoom ), cameraTripod ) );
	}
	void tPlayer::fDebugOnlyToggleFreeCam( )
	{
#ifdef sig_devmenu
		if( mCameraStack.fIsEmpty( ) )
			return;
		if( mUser->fRawGamepad( ).fButtonHeld( Input::tGamepad::cButtonSelect ) &&
			mUser->fRawGamepad( ).fButtonDown( Input::tGamepad::cButtonDPadDown ) )
		{
			if( mCameraStack.fTop( )->fDynamicCast< Gfx::tDebugFreeCamera >( ) )
				fPopCamera( );
			else
				fPushFreeCamera( 0 );
		}
#endif//sig_devmenu
	}
	void tPlayer::fCreateScreenSpaceNotifications()
	{
		mScreenSpaceNotification.fReset( NEW Gui::tScreenSpaceNotification( tGameApp::fInstance( ).fGlobalScriptResource( tGameApp::cGlobalScriptScreenSpaceNotification ), fUser( ) ) );
		mScreenSpaceNotification->fEnable( false );
	}
	void tPlayer::fReleaseScreenSpaceNotifications()
	{
		if( mScreenSpaceNotification )
		{
			mScreenSpaceNotification->fCanvas( ).fDeleteSelf( );
			mScreenSpaceNotification.fRelease( );
		}
	}
	void tPlayer::fResetGameplayRelatedData( )
	{
		fSetInGameMoney( 0 );
		mStats.fReset( NEW tGameSessionStats( this ) );
		mWaveBonusValues.fSetCount( 0 );
		mSpeedBonusValues.fSetCount( 0 );
		mBombingRunValues.fSetCount( 0 );

		mDontResetComboOnExit = false;
		mChangeUseTurret = false;
		mWasFailing = false;
		mLockedInUnit = false;
		mQuickSwitchTurret = false;
		mBeatLevelTheFirstTime = false;
		mIsNotAllowedToSaveStats = false;
		mFullScreenOverlayActive = false;
		mNewUseUnit.fRelease( );
		mCurrentUnit = NULL;
		mHoveredUnit = NULL;
		mSkippableSeconds = 0.f;
		mCurrentWaveChain = 0;
		mBombingRunCount = 0;
		mBombingRunTimer = 0.f;
		mToyBoxAlarmTimer = -1.f;
		mBombingRunLastKillPos = Math::tVec3f::cZeroVector;
		mStarPoints = 0.0f;
		mForceFilmGrainActive = false;
		mOverChargeActive = false;
		mAllowDPadSwitchWhileLocked = false;
		mDisableQuickSwitch = false;
		mLastKillValue = 0;
		mStepCameras = true;

		mQuickSwitchesWhileOvercharged = 0;
		mDisableTiltShiftCount = 0;

		mLevelScoreAndStats = Sqrat::Object( );
		fShowFocus( false );
		mFocusItems.fReset( );
		mFollowPathCamActive = false;
		mWaitingToLeaveFollowPathCam = false;
		mEarnedItems.fSetCount( 0 );
		fUser( )->fRawGamepad( ).fRumble( ).fClear( );

		// trial shiz
		mSessionAchievementMask = 0;
		mSessionAvatarMask = 0;
		mAudioListener->fClearVolumeStack( );
	}
	u32 tPlayer::fDefaultEnemyTeam( u32 team )
	{
		switch( team )
		{
		case GameFlags::cTEAM_NONE: return GameFlags::cTEAM_NONE;
		case GameFlags::cTEAM_BLUE: return GameFlags::cTEAM_RED;
		case GameFlags::cTEAM_RED: return GameFlags::cTEAM_BLUE;
		default: sigassert( !"invalid team value" ); break;
		}
		return GameFlags::cTEAM_NONE;
	}
	u32	tPlayer::fEnemyCountry( u32 country )
	{
		switch( country )
		{
		case GameFlags::cCOUNTRY_DEFAULT: return GameFlags::cCOUNTRY_DEFAULT;
		case GameFlags::cCOUNTRY_USSR: return GameFlags::cCOUNTRY_USA;
		case GameFlags::cCOUNTRY_USA: return GameFlags::cCOUNTRY_USSR;
		//HACK - added TS1 content - this may also be binary... for multiplayer, not sure how this would work, as it isn't always just the inverse
		case GameFlags::cCOUNTRY_BRITISH: return GameFlags::cCOUNTRY_GERMAN;
		case GameFlags::cCOUNTRY_GERMAN: return GameFlags::cCOUNTRY_BRITISH;
		case GameFlags::cCOUNTRY_FRENCH: return GameFlags::cCOUNTRY_GERMAN;
		default: sigassert( !"invalid country value" ); break;
		}
		return GameFlags::cCOUNTRY_DEFAULT;
	}
	void tPlayer::fSetTeam( u32 team )
	{
		fActuallySetTeam( team );
		fActuallySetCountry( tGameApp::fInstance( ).fDefaultCountryFromTeam( team ) );
	}
	void tPlayer::fSetCountry( u32 country )
	{
		fActuallySetCountry( country );
		fActuallySetTeam( tGameApp::fInstance( ).fDefaultTeamFromCountry( country ) );
	}
	// Separate functions so the other set functions don't call each other back and forth
	void tPlayer::fActuallySetTeam( u32 team )
	{
		mTeam = team;
		mEnemyTeam = fDefaultEnemyTeam( team );
	}
	void tPlayer::fActuallySetCountry( u32 country )
	{
		mCountry = country;
		mEnemyCountry = ( country == tGameApp::fInstance( ).fCurrentLevelLoadInfo( ).mCountry ) ? tGameApp::fInstance( ).fCurrentLevelLoadInfo( ).mCountry2 : tGameApp::fInstance( ).fCurrentLevelLoadInfo( ).mCountry;
		if( mSoundSource )
			mSoundSource->fSetSwitch( tGameApp::cBarrageFactionSwitchGroup, GameFlags::fCOUNTRYEnumToValueString( mCountry ) );
	}
	void tPlayer::fSetInGameMoney( s32 money )
	{
		money = fMax( money, 0 );
		fRawSetMoney( money );

		//not sharing money anymore
		//if( tGameApp::fInstance( ).fGameMode( ).fIsCoOp( ) )
		//{
		//	// Get other player and set their money too
		//	tPlayer* otherPlayer = tGameApp::fInstance( ).fOtherPlayer( this );
		//	if( otherPlayer ) // HACK TODO remove
		//		otherPlayer->fRawSetMoney( money );
		//}
	}
	s32 tPlayer::fTicketsLeft( ) const 
	{ 
		u32 maxTickets = tGameApp::fInstance( ).fMaxTickets( );
		return maxTickets - (s32)mStats->fStat( GameFlags::cSESSION_STATS_ENEMIES_REACHED_GOAL );
	}
	void tPlayer::fSetTicketsLeft( s32 tickets )
	{
		s32 maxTickets = (s32)tGameApp::fInstance( ).fMaxTickets( );
		s32 entered = maxTickets - tickets;
		mStats->fSetStat( GameFlags::cSESSION_STATS_ENEMIES_REACHED_GOAL, (f32)entered );
		if( mScoreUI )
			mScoreUI->fSetScore( tickets, tickets / (f32)maxTickets );
	}
	void tPlayer::fSubtractTickets( s32 tickets )
	{
		mStats->fIncStat( GameFlags::cSESSION_STATS_ENEMIES_REACHED_GOAL, (f32)tickets );
		fOnTicketsChanged( );

		if( tGameApp::fInstance( ).fGameMode( ).fIsCoOp( ) )
		{
			tPlayer* otherPlayer = tGameApp::fInstance( ).fOtherPlayer( this );
			if( otherPlayer )
			{
				otherPlayer->mStats->fIncStat( GameFlags::cSESSION_STATS_ENEMIES_REACHED_GOAL, (f32)tickets );
				otherPlayer->fOnTicketsChanged( );
			}
		}
	}
	void tPlayer::fRawSetMoney( s32 money )
	{
		mInGameMoney = money;
		sync_d_event_v_c( mInGameMoney, tSync::cSCPlayer );

		if( mScoreUI )
			mScoreUI->fSetMoney( mInGameMoney );
	}
	void tPlayer::fAddInGameMoney( s32 moneyToAdd )
	{
		fSetInGameMoney( mInGameMoney + moneyToAdd );
		mStats->fIncStat( GameFlags::cSESSION_STATS_MONEY_EARNED, (f32)moneyToAdd );
	}
	void tPlayer::fSpendInGameMoney( s32 moneyToSpend )
	{
		fSetInGameMoney( mInGameMoney - moneyToSpend );
		mStats->fIncStat( GameFlags::cSESSION_STATS_MONEY_SPENT, (f32)moneyToSpend );
	}
	b32 tPlayer::fAttemptPurchase( s32 cost, b32 force )
	{
		if( !force && cost > mInGameMoney )
		{
			if( mCursorLogic->fDisplay( ).fUI( ) )
				mCursorLogic->fDisplay( ).fUI( )->fSetNoMoney( true, true, cost );
			return false;
		}

		fSpendInGameMoney( cost );

		return true;
	}

	b32 tPlayer::fHasPowerPoolShown( ) const
	{
		if( mPowerPool )
			return mPowerPool->fIsShown( );
		return false;
	}

	void tPlayer::fComboLost( )
	{
		tTutorialEvent e( GameFlags::cTUTORIAL_EVENT_COMBO_LOST );
		tGameApp::fInstance( ).fCurrentLevel( )->fHandleTutorialEvent( e );
		if( mBarrageController )
			mBarrageController->fComboLost( );
		if( mPowerPool )
			mPowerPool->fSetCombo( 0 );
	}
	b32 tPlayer::fOverChargeActive( ) const 
	{
		return mOverChargeActive;
	}
	b32 tPlayer::fOverChargeAvailable( ) const
	{
		sigassert( mStats );
		return mStats->fOverChargeAvailable( mCurrentUnit );
	}
	void tPlayer::fEnemyReachedGoal( tUnitLogic* unit )
	{
		sigassert( unit );
		sigassert( mStats );
		
		{
			const u32 unitID = unit->fUnitID( );
			mStats->fIncStat( GameFlags::cSESSION_STATS_ENEMIES_REACHED_GOAL, 1.f );

			if( unit->fLogicType( ) == GameFlags::cLOGIC_TYPE_CHARACTER )
				mStats->fIncStat( GameFlags::cSESSION_STATS_INFANTRY_REACHED_GOAL, 1.f );
			else
			{
				// some sort of vehicle
				sigassert( unit->fLogicType( ) == GameFlags::cLOGIC_TYPE_VEHICLE );

				if( unit->fUnitType() == GameFlags::cUNIT_TYPE_AIR )
				{
					if( fIsPlane( unitID ) )			mStats->fIncStat( GameFlags::cSESSION_STATS_PLANES_REACHED_TOY_BOX, 1.f );
					else if( fIsCopter( unitID ) )		mStats->fIncStat( GameFlags::cSESSION_STATS_HELICOPTERS_REACHED_TOY_BOX, 1.f );
				}
				else
				{
					if( fIsTank( unitID ) )				mStats->fIncStat( GameFlags::cSESSION_STATS_TANKS_REACHED_TOY_BOX, 1.f );
					else if( fIsATV( unitID ) )			mStats->fIncStat( GameFlags::cSESSION_STATS_ATVS_REACHED_TOY_BOX, 1.f );
					else if( fIsCar( unitID ) )			mStats->fIncStat( GameFlags::cSESSION_STATS_CARS_REACHED_TOY_BOX, 1.f );
					else if( fIsAPC( unitID ) )			mStats->fIncStat( GameFlags::cSESSION_STATS_APCS_REACHED_TOY_BOX, 1.f );
				}

				if( unit->fUnderUserControl( ))
				{
					tPlayer* p = unit->fTeamPlayer( );
					sigassert( p );
					p->mStats->fIncStat( GameFlags::cSESSION_STATS_VEHICLES_DRIVEN_INTO_GOAL, 1.f );
				}	
			}

			if( tGameApp::fInstance( ).fGameMode( ).fIsVersus( ) )
			{
				mStats->fIncStat( GameFlags::cSESSION_STATS_TICKETS_LOST_IN_HEAD_2_HEAD, 1.f );
				tGameApp::fInstance( ).fOtherPlayer( this )->fStats( ).fIncStat( GameFlags::cSESSION_STATS_TICKETS_DEALT_IN_HEAD_2_HEAD, 1.f );
			}
		}

		 fOnTicketsChanged( );
	}

	void tPlayer::fOnTicketsChanged( )
	{
		u32 maxTickets = tGameApp::fInstance( ).fMaxTickets( );
		s32 ticketsLeft = fTicketsLeft( );

		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevelDemand( );
		level->fOnTicketsCountChanged( ticketsLeft, this );

		f32 ticketPercentage = (f32)ticketsLeft / maxTickets;
		ticketPercentage = fMax( ticketPercentage, 0.f );

		const Audio::tSourcePtr& audio = tGameApp::fInstance( ).fSoundSystem( )->fMasterSource( );

		if( ticketPercentage < Renderer_PostEffects_HealthThreshold )
		{
			f32 strength = fApplyDamageEffect( );

			tGameApp::fInstance( ).fSoundSystem( )->fGlobalValues( )->fSetGameParam( AK::GAME_PARAMETERS::TOYBOX_COUNT, strength );
			if( !mWasFailing ) 
			{
				if( mUser->fIsLocal( ) )
					fSoundSource( )->fHandleEvent( AK::EVENTS::PLAY_NEARFAIL );
			}
			mWasFailing = true;
		}

		// Audio
		if( mUser->fIsLocal( ) )
			fSoundSource( )->fHandleEvent( AK::EVENTS::PLAY_UI_ENEMYSCORE );

		// REFERENCE CODE FROM TS1
		//if( teamScore >= warningScore )
		//{
		//	if( teamScore >= maxScore )
		//	{
		//		// fOnGameLost takes the winner.  In a single player game we lose, and in multiplayer its the other player.
		//		fOnGameLost( tGameApp::fGetInstance( ).fIsSinglePlayerLocal( ) ? 0 : otherTeamPlayerEntity );
		//	}
		//	else
		//	{
		//		tGameApp& gameApp = tGameApp::fGetInstance( );
		//		if( !gameApp.fGameOver( ) && gameApp.fPostEffectMgr( ) && otherTeamPlayerEntity )
		//		{
		//			do film grain pulse
		//		}
		//	}
		//}
		// / REFERENCE CODE

		if( mScoreUI )
			mScoreUI->fSetScore( ticketsLeft, (f32)ticketsLeft / (f32)tGameApp::fInstance( ).fMaxTickets( ) );
	}

	f32 tPlayer::fApplyDamageEffect( )
	{
		u32 maxTickets = tGameApp::fInstance( ).fMaxTickets( );
		s32 ticketsLeft = fTicketsLeft( );

		f32 ticketPercentage = (f32)ticketsLeft / maxTickets;
		ticketPercentage = fMax( ticketPercentage, 0.f );

		f32 damageStrength = ticketPercentage / Renderer_PostEffects_HealthThreshold;
		damageStrength = 1.f - damageStrength;

		const u32 vpId = mUser->fViewport( )->fViewportIndex( );
		if( mFullScreenOverlayActive )
			tGameApp::fInstance( ).fPostEffectsManager( )->fClearFilmGrainPulse( vpId );
		else
			tGameApp::fInstance( ).fPostEffectsManager( )->fFilmGrainPulse( vpId, damageStrength, Math::fLerp( 2.f, 16.f, damageStrength ) );

		return damageStrength;
	}

	void tPlayer::fSetFullScreenOverlayActive( b32 active ) 
	{
		mFullScreenOverlayActive = active; 
		fApplyDamageEffect( );
	}

	void tPlayer::fEnemyKilled( const tDamageContext& dc, f32 damageAmount, tUnitLogic& killedLogic, u32 shareIndex, u32 shareCount )
	{
		sigassert( mStats );

		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		sigassert( level );

		b32 barrageKill = dc.fWeaponDesc( ) && dc.fWeaponDesc( )->mBarrageWeapon;
		b32 userInfluenced = dc.fAttackerPlayer( ) == this; //includes regular and all forwarded damage including shooting barrels and barrages
		b32 userUnit = dc.fAttacker( ) == mCurrentUnit && mCurrentUnit;

		if( userUnit && barrageKill && !mBarrageController->fBarrageActive( ) )
			barrageKill = false;
		
		// this is true if the kill should make our combos increment
		const b32 incrementCombos = userInfluenced && !barrageKill;		
		const tComboData combo = mStats->fComputeCombo( *this, killedLogic, incrementCombos );
		const b32 notAProp = combo.mComboExists;
		const b32 comboIncKill = incrementCombos && notAProp;

		if( comboIncKill )
		{
			mStats->fMaxStat( GameFlags::cSESSION_STATS_HIGHEST_COMBO, (f32)combo.mCurrentCombo );
			switch( combo.mCurrentCombo )
			{
			case 5:		fStats( ).fIncStat( GameFlags::cSESSION_STATS_5X_COMBOS, 1.f ); break;
			case 10:	fStats( ).fIncStat( GameFlags::cSESSION_STATS_10X_COMBOS, 1.f ); break;
			case 20:	fStats( ).fIncStat( GameFlags::cSESSION_STATS_20X_COMBOS, 1.f ); break;
			case 40:	fStats( ).fIncStat( GameFlags::cSESSION_STATS_40X_COMBOS, 1.f ); break;
			case 80:	fStats( ).fIncStat( GameFlags::cSESSION_STATS_80X_COMBOS, 1.f ); break;
			case 100:	fStats( ).fIncStat( GameFlags::cSESSION_STATS_100X_COMBOS, 1.f ); break;
			case 150:	fStats( ).fIncStat( GameFlags::cSESSION_STATS_150X_COMBOS, 1.f ); break;
			case 200:	fStats( ).fIncStat( GameFlags::cSESSION_STATS_200X_COMBOS, 1.f ); break;
			case 250:	fStats( ).fIncStat( GameFlags::cSESSION_STATS_250X_COMBOS, 1.f ); break;
			case 300:	fStats( ).fIncStat( GameFlags::cSESSION_STATS_300X_COMBOS, 1.f ); break;
			}

			fSetComboUI( combo.mCurrentCombo );
		}

		u32 destroyValue = killedLogic.fUnitAttributeDestroyValue( );
		if( combo.mSignificantCombo )
			destroyValue *= combo.mCurrentCombo;
		mLastKillValue = destroyValue;

		b32 wasSpeedBonus = false;
		if( dc.fID( ).mSpeedBonus > 0.f )
		{
			u32 base = 0;
			f32 factor = 1.f;

			if( killedLogic.fUnitType( ) == GameFlags::cUNIT_TYPE_TARGET )
			{
				base = mLastKillValue;
				factor = 1.0f + 4.0f * dc.fID( ).mSpeedBonus; //100 to 500% bonus
			}
			else if( fInBounds( killedLogic.fUnitType( ), (u32)GameFlags::cUNIT_TYPE_TURRET, (u32)GameFlags::cUNIT_TYPE_BOSS ) )
			{
				base = mLastKillValue;
				factor = dc.fID( ).mSpeedBonus;
			}

			if( base > 0 )
			{
				mSpeedBonusValues.fPushBack( tSpeedKill( base, factor ) );
				
				f32 value = base * factor;
				tLocalizedString moneyLoc = tLocalizedString::fFromCString( " " );
				moneyLoc.fJoinWithCString( StringUtil::fToString( (u32)value ).c_str( ) );

				fSpawnBonusTextWithLocSuffix( GameFlags::cSESSION_STATS_SPEED_BONUS, moneyLoc, killedLogic.fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( ) );
				tGameApp::fInstance( ).fCurrentLevelDemand( )->fExtraMiniGamePoints( value );

				mStats->fMaxStat( GameFlags::cSESSION_STATS_SPEED_BONUS, (f32)value );

				wasSpeedBonus = true;
			}
		}

		if( userUnit )
		{
			if( killedLogic.fPickup( ) != ~0 )
			{
				killedLogic.fPickedUp( );

				//received pickup
				switch( killedLogic.fPickup( ) )
				{
				case GameFlags::cPICKUPS_BATTERY_1:
					{
						tVehicleLogic* veh = mCurrentUnit->fDynamicCast<tVehicleLogic>( );
						if( veh ) veh->fAddPowerLevel( 0.5f );
					}
					break;
				case GameFlags::cPICKUPS_BATTERY_2:
					{
						tVehicleLogic* veh = mCurrentUnit->fDynamicCast<tVehicleLogic>( );
						if( veh ) veh->fAddPowerLevel( 1.0f );
					}
					break;
				case GameFlags::cPICKUPS_TINY_BATTERY_AMOUNT:
					{
						tVehicleLogic* veh = mCurrentUnit->fDynamicCast<tVehicleLogic>( );
						if( veh ) veh->fAddPowerLevel( Gameplay_Vehicle_TinyBatteryValue );
					}
					break;
				case GameFlags::cPICKUPS_OVERCHARGE:
					fGiveOverCharge( );
					break;
				case GameFlags::cPICKUPS_BARRAGE_ROLL:
					{
						fGiveBarrage( false );
						fStats( ).fIncStat( GameFlags::cSESSION_STATS_BARRAGES_EARNED_FROM_RED_STARS, 1.f );
						level->fHandleTutorialEvent( tTutorialEvent( GameFlags::cTUTORIAL_EVENT_LUCKY_STAR_BARRAGE ) );
					}
					break;
				case GameFlags::cPICKUPS_MORE_TIME:
					{
						level->fAddMiniGameTime( Gameplay_Player_MoreTimeValue );
					}
					break;
				case GameFlags::cPICKUPS_LESS_TIME:
					{
						level->fAddMiniGameTime( -Gameplay_Player_LessTimeValue );
					}
					break;
				}
			}
		}

		if( barrageKill && notAProp )
		{
			const tBarragePtr& barrage = mBarrageController->fCurrentOrLastBarrage( );
			if( barrageKill && !barrage.fIsNull( ) )
			{
				mStats->fIncStat( GameFlags::cSESSION_STATS_KILLS_WITH_BARRAGES, 1.f );
				barrage.fCodeObject( )->fKilledWithBarrageUnit( this, mCurrentUnit, &killedLogic );

				if( barrage.fCodeObject( )->mDevName == cB52BarrageName )
				{
					++mBombingRunCount;
					mBombingRunTimer = Gameplay_BombingRunTimer;
					mBombingRunLastKillPos = killedLogic.fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( );
				}
			}
		}

		if( barrageKill || comboIncKill || !level->fDisablePropMoney( ) )
		{
			f32 moneyMult = 1.f / shareCount;
			fAddInGameMoney( (s32)((f32)destroyValue * moneyMult) );

			b32 wasOverkill = false;
			b32 wasAssist = false;
			b32 wasProne = false;
			if( userInfluenced && notAProp )
				fCheckKillStats( dc, killedLogic, damageAmount, barrageKill, userUnit, wasOverkill, wasAssist, wasProne );

			if( shareIndex == 0 && !level->fHideKillHoverText( ) && destroyValue > 0 )
				fSpawnMoneyText( destroyValue, killedLogic.fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( ) + Math::tVec3f( 0.f, 2.f, 0.f ) );

			tTutorialEvent killEvent( GameFlags::cTUTORIAL_EVENT_UNIT_DESTROYED, killedLogic.fUnitID( ), killedLogic.fUnitID( ), false, killedLogic.fUnitType( ), killedLogic.fOwnerEntity( ), killedLogic.fDestroyedByPlayer( ), dc.fDamageType( ) );
			
			killEvent.mCombo = combo.mCurrentCombo;

			if( userUnit )
				killEvent.mPlayerKillerLogic = dc.fAttacker( );

			if( dc.fWeaponDesc( ) )			killEvent.mWeaponID = dc.fWeaponDesc( )->mWeaponDescName;
			if( wasOverkill )				killEvent.fSetFlag( GameFlags::cKILL_FLAG_OVERKILL );
			if( wasAssist )					killEvent.fSetFlag( GameFlags::cKILL_FLAG_ASSIST );
			if( wasProne )					killEvent.fSetFlag( GameFlags::cKILL_FLAG_PRONE );
			if( barrageKill )				killEvent.fSetFlag( GameFlags::cKILL_FLAG_BARRAGE );
			if( dc.fID( ).mNightVision )	killEvent.fSetFlag( GameFlags::cKILL_FLAG_NIGHT_VISION );
			if( userInfluenced )			killEvent.fSetFlag( GameFlags::cKILL_FLAG_PLAYER_INFLUENCED );

			if( wasSpeedBonus )
			{
				killEvent.fSetFlag( GameFlags::cKILL_FLAG_SPEEDBONUS );
				killEvent.mExtra = dc.fID( ).mSpeedBonus;
			}

			level->fHandleTutorialEvent( killEvent );
		}
	}


	b32 tPlayer::fIsTurret( u32 unitID )		{ return fInBounds( unitID, (u32)GameFlags::cUNIT_ID_TURRET_MG_01, (u32)GameFlags::cUNIT_ID_TURRET_FLAME_03 ); }
	b32 tPlayer::fIsMachineGun( u32 unitID )	{ return fInBounds( unitID, (u32)GameFlags::cUNIT_ID_TURRET_MG_01, (u32)GameFlags::cUNIT_ID_TURRET_MG_03 ); }
	b32 tPlayer::fIsArtillery( u32 unitID )		{ return fInBounds( unitID, (u32)GameFlags::cUNIT_ID_TURRET_ARTY_01, (u32)GameFlags::cUNIT_ID_TURRET_ARTY_03 ); }
	b32 tPlayer::fIsMortar( u32 unitID )		{ return fInBounds( unitID, (u32)GameFlags::cUNIT_ID_TURRET_MORTAR_01, (u32)GameFlags::cUNIT_ID_TURRET_MORTAR_03 ); }
	b32 tPlayer::fIsAntiTank( u32 unitID )		{ return fInBounds( unitID, (u32)GameFlags::cUNIT_ID_TURRET_AT_01, (u32)GameFlags::cUNIT_ID_TURRET_AT_03 ); }
	b32 tPlayer::fIsMakeshift( u32 unitID )		{ return fInBounds( unitID, (u32)GameFlags::cUNIT_ID_TURRET_FLAME_01, (u32)GameFlags::cUNIT_ID_TURRET_FLAME_03 ); }
	b32 tPlayer::fIsAntiAir( u32 unitID )		{ return fInBounds( unitID, (u32)GameFlags::cUNIT_ID_TURRET_AA_01, (u32)GameFlags::cUNIT_ID_TURRET_AA_03 ); }
	b32 tPlayer::fIsBasicInfantry( u32 unitID ) { return fInBounds( unitID, (u32)GameFlags::cUNIT_ID_INFANTRY_BASIC_01, (u32)GameFlags::cUNIT_ID_INFANTRY_COWBOY_01 ) || fInBounds( unitID, (u32)GameFlags::cUNIT_ID_INFANTRY_CAPTAIN_01, (u32)GameFlags::cUNIT_ID_INFANTRY_CAPTAIN_03 ); }
	b32 tPlayer::fIsEliteInfantry( u32 unitID ) { return fInBounds( unitID, (u32)GameFlags::cUNIT_ID_INFANTRY_ELITE_01, (u32)GameFlags::cUNIT_ID_INFANTRY_ELITE_02 ); }

	b32 tPlayer::fIsTransportCopter( u32 unitID )	{ return (unitID == GameFlags::cUNIT_ID_HELO_TRANSPORT_01); }
	b32 tPlayer::fIsGunship( u32 unitID )			{ return (unitID == GameFlags::cUNIT_ID_HELO_TRANSPORT_02); }
	b32 tPlayer::fIsAttackCopter( u32 unitID )		{ return (unitID == GameFlags::cUNIT_ID_HELO_ATTACK_01); }

	b32 tPlayer::fIsFighterPlane( u32 unitID )		{ return (unitID == GameFlags::cUNIT_ID_PLANE_FIGHTER_01); }
	b32 tPlayer::fIsTransportPlane( u32 unitID )	{ return (unitID == GameFlags::cUNIT_ID_PLANE_TRANSPORT_01); }
	b32 tPlayer::fIsBomber( u32 unitID )			{ return (unitID == GameFlags::cUNIT_ID_PLANE_BOMBER_01); }

	b32 tPlayer::fIsIFV( u32 unitID )		{ return (unitID == GameFlags::cUNIT_ID_APC_IFV_01); }
	b32 tPlayer::fIsMedTank( u32 unitID )	{ return (unitID == GameFlags::cUNIT_ID_TANK_MEDIUM_01); }
	b32 tPlayer::fIsHeavyTank( u32 unitID ) { return (unitID == GameFlags::cUNIT_ID_TANK_HEAVY_01); }

	b32 tPlayer::fIsCar( u32 unitID )		{ return (unitID == GameFlags::cUNIT_ID_CAR_01/* || unitID == GameFlags::cUNIT_ID_CAR_01_FULL_PHYSICS*/); }
	b32 tPlayer::fIsATV( u32 unitID )		{ return (unitID == GameFlags::cUNIT_ID_INFANTRY_ATV || unitID == GameFlags::cUNIT_ID_INFANTRY_ATV_FULL_PHYSICS); }
	b32 tPlayer::fIsAPC( u32 unitID )		{ return fInBounds( unitID, (u32)GameFlags::cUNIT_ID_APC_MG_01, (u32)GameFlags::cUNIT_ID_APC_AA_01 ); }

	b32 tPlayer::fIsTank( u32 unitID )		{ return fIsMedTank( unitID ) || fIsHeavyTank( unitID ) || unitID == GameFlags::cUNIT_ID_TANK_ROCKET_01 || unitID == GameFlags::cUNIT_ID_TANK_NUKEPROOF_01; }
	b32 tPlayer::fIsPlane( u32 unitID )		{ return fIsFighterPlane( unitID ) || fIsTransportPlane( unitID ) || fIsBomber( unitID ); }
	b32 tPlayer::fIsCopter( u32 unitID )	{ return fIsAttackCopter( unitID ) || fIsGunship( unitID ) || fIsTransportCopter( unitID ); }
#pragma optimize( "", off )
	void tPlayer::fCheckKillStats( const tDamageContext& dc, tUnitLogic& killedLogic, f32 damageAmount, b32 barrageKill, b32 userUnit, b32& outOverkill, b32& outAssist, b32& outProne )
	{
		// assumes the user has killed a significant unit.
		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevelDemand( );

		mStats->fIncStat( GameFlags::cSESSION_STATS_KILLS, 1.f );

		if( mOverChargeActive )
			mStats->fIncStat( GameFlags::cSESSION_STATS_TURBOCHARGE_KILLS, 1.f );

		if( dc.fID( ).mNightVision )
			mStats->fIncStat( GameFlags::cSESSION_STATS_KILLS_IN_NIGHT_VISION, 1.f );

		if( killedLogic.fDestroyedEnemyUnit( ) )
			fIncrementLocationalStat( GameFlags::cSESSION_STATS_PAYBACK, 1.f, killedLogic.fOwnerEntity( ) );

		// close call check
		{
			for( u32 i = 0; i < level->fGoalBoxCount( ); ++i )
			{
				tGoalBoxLogic* goalBox = level->fGoalBox( i )->fLogicDerived< tGoalBoxLogic >( );
				if( goalBox && goalBox->fTeam( ) == fTeam( ) && goalBox->fCheckInBounds( &killedLogic ) )
				{
					fIncrementLocationalStat( GameFlags::cSESSION_STATS_CLOSE_CALL, 1.f, killedLogic.fOwnerEntity( ) );
					break;
				}
			}
		}


		outOverkill = false;
		if( !level->fHideKillHoverText( ) && !level->fDisableOverkills( ) )
		{
			//no over kill while hover text is hidden
			const b32 overKill = (damageAmount >= killedLogic.fUnitAttributeOverkillDamage( ));
			//log_line( 0, "damage: " << damageAmount << " needed: " << killedLogic.fUnitAttributeOverkillDamage( ) );
			if( overKill )
				fIncrementLocationalStat( GameFlags::cSESSION_STATS_OVER_KILL, 1.f, killedLogic.fOwnerEntity( ) );
			outOverkill = overKill;
		}

		const u32 killedID = killedLogic.fUnitID( );
		outProne = false;
		if( killedLogic.fLogicType( ) == GameFlags::cLOGIC_TYPE_CHARACTER )
		{
			if( fIsBasicInfantry( killedID ) )		mStats->fIncStat( GameFlags::cSESSION_STATS_BASIC_INFANTRY_KILLED, 1.f );
			else if( fIsEliteInfantry( killedID ) )	mStats->fIncStat( GameFlags::cSESSION_STATS_ELITE_INFANTRY_KILLED, 1.f );

			tCharacterLogic& charLogic = *killedLogic.fDynamicCast< tCharacterLogic >();
			if( charLogic.fCurrentContextAnimType( ) == GameFlags::cCONTEXT_ANIM_TYPE_BARBED_WIRE )
				outProne = true;
		}
		else if( killedLogic.fLogicType( ) == GameFlags::cLOGIC_TYPE_VEHICLE )
		{
			if( killedLogic.fUnitType( ) == GameFlags::cUNIT_TYPE_AIR )
			{
				if( fIsAttackCopter( killedID ) )			mStats->fIncStat( GameFlags::cSESSION_STATS_ATTACK_COPTERS_DESTROYED, 1.f );
				else if( fIsTransportCopter( killedID ) )	mStats->fIncStat( GameFlags::cSESSION_STATS_TRANSPORT_COPTERS_DESTROYED, 1.f );
				else if( fIsGunship( killedID ) )			mStats->fIncStat( GameFlags::cSESSION_STATS_GUNSHIPS_DESTROYED, 1.f );
				else if( fIsFighterPlane( killedID ) )		mStats->fIncStat( GameFlags::cSESSION_STATS_FIGHTER_PLANES_DESTROYED, 1.f );
				else if( fIsTransportPlane( killedID ) )	mStats->fIncStat( GameFlags::cSESSION_STATS_TRANSPORT_PLANES_DESTROYED, 1.f );
				else if( fIsBomber( killedID ) )			mStats->fIncStat( GameFlags::cSESSION_STATS_BOMBERS_DESTROYED, 1.f );
			}
			else //atv is not unit type vehicle :(
			{
				if( fIsCar( killedID ) )				mStats->fIncStat( GameFlags::cSESSION_STATS_CARS_DESTROYED, 1.f );
				else if( fIsATV( killedID ) )			mStats->fIncStat( GameFlags::cSESSION_STATS_ATVS_DESTROYED, 1.f );
				else if( fIsMedTank( killedID ) )		mStats->fIncStat( GameFlags::cSESSION_STATS_MEDIUM_TANKS_DESTROYED, 1.f );
				else if( fIsHeavyTank( killedID ) )		mStats->fIncStat( GameFlags::cSESSION_STATS_HEAVY_TANKS_DESTROYED, 1.f );
				else if( fIsAPC( killedID ) )			mStats->fIncStat( GameFlags::cSESSION_STATS_APCS_DESTROYED, 1.f );
				else if( fIsIFV( killedID ) )			mStats->fIncStat( GameFlags::cSESSION_STATS_IFVS_DESTROYED, 1.f );
			}
		}
		else if( killedLogic.fLogicType( ) == GameFlags::cLOGIC_TYPE_TURRET )
		{
			if( tGameApp::fInstance( ).fGameMode( ).fIsVersus( ) )
				mStats->fIncStat( GameFlags::cSESSION_STATS_VERSUS_TURRETS_DESTROYED, 1.f );

			if( userUnit && fIsHeavyTank( mCurrentUnit->fUnitID( ) ) && !killedLogic.fHitpointLinkedUnitLogic( ) )
				fAwardAchievement( GameFlags::cACHIEVEMENTS_BRUTE_FORCE );
		}

		outAssist = false;
		if( userUnit )
		{
			const b32 turret = fIsTurret( mCurrentUnit->fUnitID( ) );
			if( turret )
			{
				mStats->fIncStat( GameFlags::cSESSION_STATS_KILLS_WHILE_USING_TURRETS, 1.f );

				// kill a stunned character with a machine gun to get an assist
				if( killedLogic.fLogicType( ) == GameFlags::cLOGIC_TYPE_CHARACTER 
					&& killedLogic.fCurrentPersistentEffect( ) < GameFlags::cPERSISTENT_EFFECT_BEHAVIOR_COUNT
					&& killedLogic.fCurrentPersistentDC( )
					&& killedLogic.fCurrentPersistentDC( )->fID( ).mDesc 
					&& killedLogic.fCurrentPersistentDC( )->fID( ).mDesc->mWeaponType == tGameApp::cWeaponDerivedTypeMortar
					&& fIsMachineGun( mCurrentUnit->fUnitID( ) ) )
				{
					fIncrementLocationalStat( GameFlags::cSESSION_STATS_ASSISTS, 1.f, killedLogic.fOwnerEntity( ) );
					outAssist = true;

					if( tGameApp::fInstance( ).fGameMode( ).fIsCoOp( ) )
					{
						// make sure they were stunned by other player
						const tDamageContext* dc = killedLogic.fCurrentPersistentDC( );
						sigassert( dc );
						if( dc->fAttackerPlayer( ) == tGameApp::fInstance( ).fOtherPlayer( this ) )
							level->fIncCoopAssists( );
					}
				}
			}
		}
	}
	void tPlayer::fIncrementLocationalStat( u32 stat, f32 value, tEntity* pos )
	{
		mStats->fIncStat( stat, value );

		fSpawnBonusText( stat, pos->fObjectToWorld( ).fGetTranslation( ) );
	}
	void tPlayer::fIncrementWaveChain( tEntity* pos, f32 waveValue )
	{
		mStats->fIncStat( GameFlags::cSESSION_STATS_WAVE_BONUS, 1.f );
		++mCurrentWaveChain;
		mStats->fMaxStat( GameFlags::cSESSION_STATS_WAVE_CHAIN, (f32)mCurrentWaveChain );

		mWaveBonusValues.fPushBack( tWaveKill( waveValue, mCurrentWaveChain ) );

		tLocalizedString moneyLoc = tLocalizedString::fFromCString( " " );
		moneyLoc.fJoinWithCString( StringUtil::fToString( (u32)waveValue ).c_str( ) );

		if( mCurrentWaveChain > 1 )
		{
			std::stringstream ss;
			ss << " x" << mCurrentWaveChain;
			moneyLoc.fJoinWithCString( ss.str( ).c_str( ) );
		}

		fSpawnBonusTextWithLocSuffix( GameFlags::cSESSION_STATS_WAVE_BONUS, moneyLoc, pos->fObjectToWorld( ).fGetTranslation( ) );
		tGameApp::fInstance( ).fCurrentLevelDemand( )->fExtraMiniGamePoints( waveValue * mCurrentWaveChain );
	}
	void tPlayer::fResetWaveChain( )
	{
		mCurrentWaveChain = 0;
		if( !tGameApp::fInstance( ).fGameMode( ).fIsFrontEnd( ) )
			log_line( 0, "Wave combo reset" );
	}
	void tPlayer::fIncrementStarShatter( f32 val, tEntity* pos )
	{
		mStats->fIncStat( GameFlags::cSESSION_STATS_STARS_SHATTERED, 1 );
		std::stringstream ss;
		ss << " " << StringUtil::fToString( (u32)val );
		fSpawnBonusTextWithCSuffix( GameFlags::cSESSION_STATS_STARS_SHATTERED, ss.str( ).c_str( ), pos->fObjectToWorld( ).fGetTranslation( ) );
		mStarPoints += val;
	}
	void tPlayer::fBeginCameraPath( const tStringPtr& name, bool skipable )
	{
		const tGrowableArray<tPathEntityPtr>& starts = tGameApp::fInstance( ).fCurrentLevel( )->fCameraPathStarts( );
		for( u32 i = 0; i < starts.fCount( ); ++i )
		{
			if( starts[ i ]->fName( ) == name )
			{
				mFollowPathCam.fReset( NEW tFollowPathCam( this, starts[ i ], name, skipable, make_delegate_memfn( Gfx::tFollowPathCamera::tOnEndOfPathReached, tPlayer, fOnPathCameraFinished ) ) );
				mFollowPathCamActive = true;

				fPushCamera( mFollowPathCam );
				fSetDisableTiltShift( true );

				return;
			}
		}

		log_warning( 0, "Camera path start not found: " << name );
	}
	void tPlayer::fOnPathCameraFinished( Gfx::tFollowPathCamera& camera )
	{
		// this function is called every frame from the tFollowPathCamera once it has reached the end of the path.
		// If we don't deactivate the tFollowPathCamera immediately, it will continue to call this function.
		// So, we guard the level callback so that it's only called once (on the frame the follow path cam reaches the end of it's path)
		// and not while we're waiting for user input but holding the last position of the follow path cam.
		if( !mWaitingToLeaveFollowPathCam )
		{
			tGameApp::fInstance( ).fCurrentLevel( )->fOnPathCameraFinished( camera );

			// Signal we're now waiting for the user input to end the follow cam and switch to the rts cam.
			mWaitingToLeaveFollowPathCam = true;
		}
	}
	void tPlayer::fSetDisableTiltShift( b32 disable )
	{
		if( disable )
			++mDisableTiltShiftCount;
		else if( mDisableTiltShiftCount > 0 )
			--mDisableTiltShiftCount;
	}
	void tPlayer::fPauseComboTimers( b32 pause )
	{
		mStats->fPauseTimers( pause );
	}
	void tPlayer::fAddBarrage( Sqrat::Object& barrage )
	{
		sync_line_c( tSync::cSCPlayer );
		if( mBarrageController )
		{
			tBarragePtr ptr( barrage );
			sigassert( ptr.fCodeObject( ) );
			ptr.fCodeObject( )->mBarrageFactionSwitchValue = GameFlags::fCOUNTRYEnumToValueString( fCountry( ) );
			mBarrageController->fAddBarrage( ptr );
		}
	}
	void tPlayer::fSetCursorLogic( const tRefCounterPtr<tRtsCursorLogic>& logic )
	{
		mCursorLogic = logic;
	}

	void tPlayer::fCloseRadialMenus( )
	{
		if( mCursorLogic )
			mCursorLogic->fResetNavigation( );

		fHideShowOffensiveWaveMenu( false );
	}

	b32	tPlayer::fInUseMode( ) const
	{
		return mCurrentUnit != NULL;
	}
	tEntity* tPlayer::fFindTurretInDirection( const Math::tVec3f& dir, tEntity* owner ) const
	{
		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		if( !level )
			return NULL;

		if( mCameraStack.fIsEmpty( ) )
			return NULL;

		if( !mCurrentUnit )
			return NULL;

		sigassert( mCurrentUnit );
		const tEntity* useTurret = mCurrentUnit->fOwnerEntity( );
		sigassert( useTurret );

		u32 bestIndex = ~0;
		f32 shortestDist = Math::cInfinity;

		const Math::tVec3f pos = useTurret->fObjectToWorld( ).fGetTranslation( );
		const Math::tVec3f compareDir = owner 
			? owner->fObjectToWorld( ).fXformVector( dir ).fNormalize( )  //if we have a vehicle owner move in its space.
			: useTurret->fObjectToWorld( ).fXformVector( dir ).fNormalize( );

		for( u32 i = 0; i < level->fUseableUnitCount( ); ++i )
		{
			tEntity* turret = level->fUseableUnit( i );
			sigassert( turret );

			if( turret == useTurret ) 
				continue;

			if( turret->fHasGameTagsAny( GameFlags::cFLAG_MINIGAME_UNIT ) ) 
				continue;

			tVehicleLogic* ownerVehicle = turret->fFirstAncestorWithLogicOfType<tVehicleLogic>( );

			if( owner )
			{
				//we're on a vehicle owned turret, only switch to turrets owned by the same vehicle
				if( !ownerVehicle || ownerVehicle->fOwnerEntity( ) != owner )
					continue;
			}
			else if( ownerVehicle )
				continue; //dont switch to vehicle owned turrets if we're not on a vehicle owned turret
			
			const tTurretLogic* turretLogic = turret->fLogicDerived< tTurretLogic >( );

			if( !turretLogic ) 
				continue;

			if( turretLogic->fTeam( ) != fTeam( ) || turretLogic->fUnderUserControl( ) ) 
				continue;

			const Math::tVec3f turretPos = turret->fObjectToWorld( ).fGetTranslation( );
			Math::tVec3f toTurret = turretPos - pos;
			f32 dist;

			toTurret.fNormalizeSafe( dist );			
			f32 angleBetween = Math::fAcos( compareDir.fDot( toTurret ) );

			if( angleBetween <= Math::cPiOver4 && dist < shortestDist )
			{
				shortestDist = dist;
				bestIndex = i;
			}
		}

		if( bestIndex != ~0 )
			return level->fUseableUnit( bestIndex );

		return NULL;
	}
	void tPlayer::fTransitionUseTurrets( )
	{
		if( !mChangeUseTurret ) return;

		if( fInUseMode( ) ) return;
		
		tUnitLogic* unitLogic = mNewUseUnit->fLogicDerived< tUnitLogic >( );
		if( unitLogic && !unitLogic->fIsDestroyed( ) )
		{
			unitLogic->fTryToUse( this );
			fSoundSource( )->fHandleEvent( AK::EVENTS::PLAY_CAMERA_MVMT_IN );
		}

		mNewUseUnit.fRelease( );
		mChangeUseTurret = false;

	}
	tUnitLogic* tPlayer::fLockInUnit( const tStringPtr& name )
	{
		tEntity* newUnit = tGameApp::fInstance( ).fCurrentLevel( )->fUseableUnit( name );
		if( !newUnit )
		{
			log_warning( 0, "No usable unit named: " << name );
			return NULL;
		}

		if( !mCurrentUnit || newUnit != mCurrentUnit->fOwnerEntity( ) )
		{
			mChangeUseTurret = true;
			mNewUseUnit.fReset( newUnit );

			fUnlockFromUnit( true );
		}

		mLockedInUnit = true;
		mAllowDPadSwitchWhileLocked = false;

		return newUnit->fLogicDerived<tUnitLogic>( );
	}
	void tPlayer::fLockInUnitDirect( tUnitLogic* unit )
	{
		sigassert( unit );
		if( !mCurrentUnit || unit != mCurrentUnit )
		{
			mChangeUseTurret = true;
			mNewUseUnit.fReset( unit->fOwnerEntity( ) );

			fUnlockFromUnit( true );
		}

		mLockedInUnit = true;
		mAllowDPadSwitchWhileLocked = false;
	}
	void tPlayer::fLockInCurrentUnit( )
	{
		mLockedInUnit = true;
		mAllowDPadSwitchWhileLocked = false;
	}
	void tPlayer::fUnlockFromUnit( b32 kickOut )
	{
		mLockedInUnit = false;
		mAllowDPadSwitchWhileLocked = false;

		if( kickOut )
		{
			for( s32 i = mCameraStack.fCount( ) - 1; i >= 0; --i )
			{				
				tUseUnitCamera* useCam = mCameraStack[ i ].fDynamicCast<tUseUnitCamera>( );
				if( useCam && !useCam->fDynamicCast<tRtsCamera>( ) && !useCam->fDynamicCast<tShellCamera>( ) )
				{
					useCam->fCheckForAndHandleExit( true );
					fSetSelectedUnitLogic( NULL );
					break;
				}
			}
		}
	}
	void tPlayer::fUseBarrageCheck( f32 dt )
	{
		if( mBarrageController )
		{
			if( mBarrageController->fDormant( ) && mStats->fBarrageAvailable( mCurrentUnit ) )
			{
				fStats( ).fIncStat( GameFlags::cSESSION_STATS_BARRAGES_EARNED_FROM_TURBOCHARGE, 1.f );

				if( mPowerPool )
					mPowerPool->fShowBarrage( true );

				fGiveBarrage( false );
			}

			mBarrageController->fStepBarrage( dt );
		}
	}
	void tPlayer::fUseOverChargeCheck( f32 dt )
	{
		if( !mOverChargeActive && mStats && mCurrentUnit
			&& mCurrentUnit->fCreationType( ) != tUnitLogic::cCreationTypeGhost
			&& mCurrentUnit->fUnderUserControl( )
			&& fOverChargeAvailable( ) )
		{
			mOverChargeActive = true;

			if( mPowerPool )
				mPowerPool->fSetOverChargeActive( true );

			mStats->fIncStat( GameFlags::cSESSION_STATS_OVERCHARGE, 1.f );
			const u32 unitID = mCurrentUnit->fUnitID( );
			if( fIsMachineGun( unitID ) )		mStats->fIncStat( GameFlags::cSESSION_STATS_TURBOCHARGES_WITH_MACHINEGUNS, 1.f );
			else if( fIsArtillery( unitID ) )	mStats->fIncStat( GameFlags::cSESSION_STATS_TURBOCHARGES_WITH_HOWITZERS, 1.f );
			else if( fIsMortar( unitID ) )		mStats->fIncStat( GameFlags::cSESSION_STATS_TURBOCHARGES_WITH_MORTARS, 1.f );
			else if( fIsAntiTank( unitID ) )	mStats->fIncStat( GameFlags::cSESSION_STATS_TURBOCHARGES_WITH_ANTI_TANK, 1.f );
			else if( fIsMakeshift( unitID ) )	mStats->fIncStat( GameFlags::cSESSION_STATS_TURBOCHARGES_WITH_MAKESHIFT, 1.f );
			else if( fIsAntiAir( unitID ) )		mStats->fIncStat( GameFlags::cSESSION_STATS_TURBOCHARGES_WITH_ANTI_AIR, 1.f );

			tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
			if( level ) level->fHandleTutorialEvent( tTutorialEvent( GameFlags::cTUTORIAL_EVENT_OVERCHARGE ) );

			mCurrentUnit->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_CANCEL_RELOAD ) );
		}

		if( mOverChargeActive && !fOverChargeAvailable( ) )
		{
			if( mPowerPool )
				mPowerPool->fSetOverChargeActive( false );
			mOverChargeActive = false;
			mQuickSwitchesWhileOvercharged = 0; // only counts for a single overcharge.
		}

		if( mOverChargeActive )
			fStats( ).fIncQuick( GameFlags::cSESSION_STATS_TIME_SPENT_IN_TURBOCHARGE, dt );
	}
	void tPlayer::fCreateScoreUI( )
	{
		tGameApp & gameApp = tGameApp::fInstance( );

		if( !gameApp.fHasWaveListTable( ) )
			return;

		if( gameApp.fGameMode( ).fIsFrontEnd( ) )
			return;

		if( !fIsActive( ) )
			return;

		if( tGameApp::fExtraDemoMode( ) )
			return;

		mScoreUI.fReset( NEW Gui::tScoreUI( gameApp.fGlobalScriptResource( tGameApp::cGlobalScriptScoreUI ), tPlayerPtr( this ) ) );

		gameApp.fRootHudCanvas( ).fToCanvasFrame( ).fSetIgnoreBoundsChanged( true );
		if( gameApp.fGameMode( ).fIsNet( ) && fUser( )->fIsViewportVirtual( ) )
			gameApp.fHudLayer( gameApp.fOtherPlayer( this )->fHudLayerName( ) ).fToCanvasFrame( ).fAddChild( mScoreUI->fCanvas( ) );
		else
			gameApp.fHudLayer( fHudLayerName( ) ).fToCanvasFrame( ).fAddChild( mScoreUI->fCanvas( ) );
	}
	void tPlayer::fReleaseScoreUI( )
	{
		if( mScoreUI )
		{
			mScoreUI->fCanvas( ).fDeleteSelf( );
			mScoreUI.fRelease( );
		}
	}
	void tPlayer::fInitScoreUI( )
	{
		if( mScoreUI )
		{
			mScoreUI->fShow( true );
			mScoreUI->fSetScore( tGameApp::fInstance( ).fMaxTickets( ), 1.0f );
			mScoreUI->fSetMoney( mInGameMoney );
		}
	}
	void tPlayer::fShowScoreUI( b32 scoreVisible )
	{
		if( mScoreUI )
		{
			mScoreUI->fShow( scoreVisible );
		}
	}
	void tPlayer::fShowMoney( b32 show ) 
	{ 
		if( mScoreUI ) 
		{
			mScoreUI->fShowMoney( show );
		}
	}
	void tPlayer::fShowTickets( b32 show ) 
	{ 
		if( mScoreUI )
			mScoreUI->fShowTickets( show ); 
	}
	void tPlayer::fShowBarrageIndicator( b32 show ) 
	{ 
		if( mBarrageController )
			mBarrageController->fShowUI( show );
	}
	void tPlayer::fCreateFocalPrompt( )
	{
		mFocalPrompt.fReset( NEW Gui::tFocalPrompt( tGameApp::fInstance( ).fGlobalScriptResource( tGameApp::cGlobalScriptFocalPrompt ), *fUser( ) ) );
	}
	void tPlayer::fReleaseFocalPrompt( )
	{
		if( mFocalPrompt )
		{
			mFocalPrompt->fCanvas( ).fDeleteSelf( );
			mFocalPrompt.fRelease( );
		}
	}

	void tPlayer::fCreatePersonalBestUI()
	{
		mPersonalBestUI.fReset( NEW Gui::tPersonalBestUI( tPlayerPtr( this ) ) );
	}

	void tPlayer::fReleasePersonalBestUI()
	{
		if( mPersonalBestUI )
		{
			mPersonalBestUI->fCanvas( ).fDeleteSelf( );
			mPersonalBestUI.fRelease( );
		}
	}

	void tPlayer::fCreateMiniMap( )
	{
		if( !tGameApp::fInstance( ).fGameMode( ).fIsFrontEnd( ) )
		{
			if( Debug_EnableMinimap )
			{
				mMiniMap.fReset( NEW Gui::tMiniMap( tGameApp::fInstance( ).fGlobalScriptResource( tGameApp::cGlobalScriptMiniMap ), fUser( ) ) );
				tGameApp::fInstance( ).fHudLayer( fHudLayerName( ) ).fToCanvasFrame( ).fAddChild( mMiniMap->fCanvas( ) );
			}
		}
	}
	void tPlayer::fReleaseMiniMap( )
	{
		if( mMiniMap )
		{
			mMiniMap->fCanvas( ).fDeleteSelf( );
			mMiniMap.fRelease( );
		}
	}
	void tPlayer::fInitMiniMap( )
	{
		if( mMiniMap )
		{
			mMiniMap->fInitialize( this );
			mMiniMap->fFadeIn( );
		}
	}
	void tPlayer::fShowMiniMap( b32 show )
	{
		if( mMiniMap )
		{
			mMiniMap->fCanvas( ).fCanvas( )->fSetInvisible( !show );
			if( show )
			{
				mMiniMap->fCanvas( ).fCanvas( )->fSetAlpha( 0 );
				mMiniMap->fFadeIn( );
			}
		}
	}
	void tPlayer::fCreatePowerPoolUI( )
	{
		if( !tGameApp::fInstance( ).fGameMode( ).fIsFrontEnd( ) )
		{
			std::string hudName = fHudLayerName( );
			mPowerPool.fReset( NEW Gui::tPowerPoolUI( tGameApp::fInstance( ).fGlobalScriptResource( tGameApp::cGlobalScriptPowerPoolUI ), fUser( ) ) );
			tGameApp::fInstance( ).fHudLayer( hudName ).fToCanvasFrame( ).fAddChild( mPowerPool->fCanvas( ) );

			mBatteryMeter.fReset( NEW Gui::tBatteryMeter( tGameApp::fInstance( ).fGlobalScriptResource( tGameApp::cGlobalScriptBatteryMeter ), fUser( ) ) );
			tGameApp::fInstance( ).fHudLayer( hudName ).fToCanvasFrame( ).fAddChild( mBatteryMeter->fCanvas( ) );

			if( !fUser( )->fIsLocal( ) )
			{
				mPowerPool->fCanvas( ).fCanvas( )->fSetInvisible( true );
				mBatteryMeter->fCanvas( ).fCanvas( )->fSetInvisible( true );
			}
		}
	}
	void tPlayer::fReleasePowerPoolUI( )
	{
		if( mPowerPool )
		{
			mPowerPool->fCanvas( ).fDeleteSelf( );
			mPowerPool.fRelease( );
		}
		if( mBatteryMeter )
		{
			mBatteryMeter->fCanvas( ).fDeleteSelf( );
			mBatteryMeter.fRelease( );
		}
	}

	void tPlayer::fShowOverchargeMeter( b32 show )
	{
		if( mPowerPool )
		{
			mPowerPool->fCanvas( ).fCanvas( )->fSetInvisible( !show );
			if( show && mCurrentUnit )
			{
				mPowerPool->fCanvas( ).fCanvas( )->fSetAlpha( 0 );
				mPowerPool->fShow( true );
			}
		}
	}

	void tPlayer::fSpawnMoneyText( u32 amount, const Math::tVec3f& pos )
	{
		std::string amountStr = StringUtil::fToString( amount );
		
		if( !tGameApp::fInstance( ).fCurrentLevel( )->fUseMinigameScoring( ) )
			amountStr = tLocalizedString::fConstructMoneyString( amountStr.c_str( ) ).fToCString( );

		fSpawnText( 
			amountStr.c_str( ), pos, 
			tGameApp::fInstance( ).cColorCleanWhite, 0.45f, 
			sync_rand( fFloatInRange( 0.9f, 1.15f ) ) ); // clean white
	}

	void tPlayer::fCreateOutOfBoundsIndicator( )
	{
		if( !tGameApp::fInstance( ).fGameMode( ).fIsFrontEnd( ) )
		{
			mOutOfBoundsIndicator.fReset( NEW Gui::tOutOfBoundsIndicator( tGameApp::fInstance( ).fGlobalScriptResource( tGameApp::cGlobalScriptOutOfBoundsIndicator ), fUser( ) ) );
		}
	}

	void tPlayer::fReleaseOutOfBoundsIndicator( )
	{
		if( mOutOfBoundsIndicator )
		{
			mOutOfBoundsIndicator->fCanvas( ).fDeleteSelf( );
			mOutOfBoundsIndicator.fRelease( );
		}
	}

	void tPlayer::fCreatePointCaptureUI( )
	{
		if( !tGameApp::fInstance( ).fGameMode( ).fIsFrontEnd( ) && tGameApp::fInstance( ).fGameMode( ).fIsVersus( ) )
			mPointCaptureUI.fReset( NEW Gui::tPointCaptureUI( tGameApp::fInstance( ).fGlobalScriptResource( tGameApp::cGlobalScriptPointCaptureUI ), fUser( ) ) );
	}

	void tPlayer::fReleasePointCaptureUI( )
	{
		if( mPointCaptureUI )
		{
			mPointCaptureUI->fCanvas( ).fDeleteSelf( );
			mPointCaptureUI.fRelease( );
		}
	}

	void tPlayer::fCreateAchievementBuyNotification( u32 index )
	{
		if( mAchievementBuyNotification )
			mAchievementBuyNotification->fShow( false );
		mAchievementBuyNotification.fReset( NEW Gui::tAchievementBuyNotification( tGameApp::fInstance( ).fGlobalScriptResource( tGameApp::cGlobalScriptAchievementBuyNotification ), tPlayerPtr( this ), index ) );
		mAchievementBuyNotification->fShow( true );
	}

	void tPlayer::fCreateAvatarAwardBuyNotification( u32 index )
	{
#if defined( use_avatar_award )
		if( mAchievementBuyNotification )
			mAchievementBuyNotification->fShow( false );
		mAchievementBuyNotification.fReset( NEW Gui::tAchievementBuyNotification( tGameApp::fInstance( ).fGlobalScriptResource( tGameApp::cGlobalScriptAvatarAwardBuyNotification ), tPlayerPtr( this ), index ) );
		mAchievementBuyNotification->fShow( true );
#endif
	}

	void tPlayer::fReleaseAchievementBuyNotification( )
	{
		if( mAchievementBuyNotification )
		{
			mAchievementBuyNotification->fShow( false );
			mAchievementBuyNotification.fRelease( );
		}
	}

	Gui::tWorldSpaceFloatingText* tPlayer::fSpawnText( const tLocalizedString& locText, const Math::tVec3f& pos, const Math::tVec4f& tint, f32 scale, f32 timeToLive, b32 flyToScreenCorner )
	{
		if( !tGameApp::fInstance( ).fGameMode( ).fIsFrontEnd( ) )
		{
			tUserArray users;
			users.fPushBack( fUser( ) );

			/*if( tGameApp::fInstance( ).fGameMode( ).fIsCoOp( ) )
				users.fPushBack( tGameApp::fInstance( ).fOtherPlayer( this )->fUser( ) );*/

			if( mScreenSpaceNotification )
				mScreenSpaceNotification->fSpawnText( locText, tint );

			Gui::tWorldSpaceFloatingTextPtr floatingText( NEW Gui::tWorldSpaceFloatingText( users, tGameApp::cFontFancyLarge ) );
			floatingText->fSpawn( *tGameApp::fInstance( ).fCurrentLevel( )->fOwnerEntity( ) );
			floatingText->fSetText( locText );
			floatingText->fMoveTo( pos + ( Math::tVec3f::cYAxis * Gameplay_Gui_PlayerSpawnText_Raise ) );
			floatingText->fSetScale( scale );
			floatingText->fSetTint( tint );
			floatingText->fSetTimeToLive( timeToLive );
			if( flyToScreenCorner )
				floatingText->fMorphIntoFlyingText( );

			return floatingText.fGetRawPtr( );
		}
		else
			return 0;
	}

	Gui::tWorldSpaceFloatingText* tPlayer::fSpawnText( const char* text, const Math::tVec3f& pos, const Math::tVec4f& tint, f32 scale, f32 timeToLive, b32 flyToScreenCorner )
	{
		return fSpawnText( tLocalizedString::fFromCString( text ), pos, tint, scale, timeToLive, flyToScreenCorner );
	}
	Gui::tWorldSpaceFloatingText* tPlayer::fSpawnLocText( tLocalizedString text, const Math::tVec3f& pos, const Math::tVec4f& tint, f32 scale )
	{
		return fSpawnText( text, pos, tint, scale );
	}
	void tPlayer::fSpawnBonusText( u32 statID, const Math::tVec3f& pos )
	{
		fSpawnBonusTextWithCSuffix( statID, NULL, pos );
	}

	void tPlayer::fSpawnBonusTextWithCSuffix( u32 statID, const char* suffix, const Math::tVec3f& pos )
	{
		// Get Loc String
		tLocalizedString locName = tGameSessionStats::fBonusDisplayLocString( statID ).fClone( );
		if( !locName.fNull( ) && suffix )
			locName.fJoinWithCString( suffix );

		fCreateBonusText( locName, pos, statID );
	}

	void tPlayer::fSpawnBonusTextWithLocSuffix( u32 statID, const tLocalizedString& suffix, const Math::tVec3f& pos )
	{
		// Get Loc String
		tLocalizedString locName = tGameSessionStats::fBonusDisplayLocString( statID ).fClone( );
		if( !locName.fNull( ) && !suffix.fNull( ) )
			locName.fJoinWithLocString( suffix );

		fCreateBonusText( locName, pos, statID );
	}

	void tPlayer::fCreateBonusText( const tLocalizedString& locName, const Math::tVec3f& pos, u32 statID )
	{
		if( !locName.fNull( ) && !tGameApp::fInstance( ).fCurrentLevel( )->fDisableBonusHoverText( ) )
		{
			const Math::tVec4f color = tGameSessionStats::fDisplayColor( statID );
			const u32 priority = tGameSessionStats::fPriority( statID );
			Gui::tWorldSpaceFloatingText* text = fSpawnText( locName, pos, color, 0.6f, 1.3f );
			if( text )
				text->fSetZOffset( -0.001f + priority * 0.0001f );
		}
	}

	void tPlayer::fSetSelectedUnitLogic( tUnitLogic *unitLogic )
	{
		tUnitLogic* last = mHoveredUnit;
		tEntityPtr keepAround = mHoverUnitEnt;

		mHoveredUnit = unitLogic;
		mHoverUnitEnt.fReset( mHoveredUnit ? mHoveredUnit->fOwnerEntity( ) : NULL );

		if( mHoveredUnit != last )
		{
			if( last )
			{
				last->fOnRtsCursorHoverBeginEnd( false );
				tTurretLogic* tl = last->fDynamicCast<tTurretLogic>( );
				if( tl )
				{
					if( tl->fHasWeapon(0,0,0) )
						tl->fWeaponRawPtr( 0,0,0 )->fEndRendering( );
				}
			}
			if( mHoveredUnit )
			{
				mHoveredUnit->fOnRtsCursorHoverBeginEnd( true );
				tTurretLogic* tl = mHoveredUnit->fDynamicCast<tTurretLogic>( );
				if( tl )
				{
					if( tl->fHasWeapon(0,0,0) )
						tl->fWeaponRawPtr( 0,0,0 )->fBeginRendering( this );
				}
			}

			if( !mSelectedUnitChangedCallback.IsNull( ) )
				mSelectedUnitChangedCallback.Execute( unitLogic );
		}
	}

	void tPlayer::fSetCurrentUnitLogic( tUnitLogic *unitLogic )
	{
		// dont let the user commit two actions as once
		mDisableQuickSwitch = true;

		if( unitLogic == NULL )
		{
			mTimeSinceUnitExit = 0.0f;
		}

		u32 previousUnitID = ~0;
		
		if( mCurrentUnit )
		{
			previousUnitID = mCurrentUnit->fUnitID( );
			mCurrentUnit->fAudioSource( )->fSetGameParam( tGameApp::cPlayerControlRTPC, 0.f );
		}

		mCurrentUnit = unitLogic;
		if( !unitLogic && mLockedInUnit )
			mLockedInUnit = false;

		if( !mCurrentUnitChangedCallback.IsNull( ) )
		{
			mCurrentUnitChangedCallback.Execute( unitLogic );
		}

		if( mCurrentUnit )
		{
			sync_event_v_c( mCurrentUnit->fOwnerEntity( )->fGuid( ), tSync::cSCPlayer );

			if( mCurrentUnit->fAudioSource( ) )
				mCurrentUnit->fAudioSource( )->fSetGameParam( tGameApp::cPlayerControlRTPC, 1.f );

			fShowInUseIndicator( true, mCurrentUnit );

			if( mCurrentUnit->fUnitType( ) == GameFlags::cUNIT_TYPE_TURRET ) 
			{		
				tTurretLogic* tl = mCurrentUnit->fDynamicCast<tTurretLogic>( );
				if( tl->fOnVehicle( ) )
					mAllowDPadSwitchWhileLocked = true;

				mStats->fPauseTimers( false );
				f32 duration = unitLogic->fUnitAttributeOverChargeDuration( );
				if( duration > 0 )
				{
					const f32 overChargeCost = unitLogic->fUnitAttributeOverChargeComboCost( );
					mStats->fSetOverChargeComboCost( overChargeCost );

					if( mPowerPool )
						mPowerPool->fShow( true );
				}
				else 
				{
					mStats->fSetOverChargeComboCost( 0 );
					if( mPowerPool )
						mPowerPool->fShow( false );
				}
			}
			else if( mBatteryMeter && mCurrentUnit->fDynamicCast<tVehicleLogic>( ) && !mCurrentUnit->fExtraMode( ) )
			{
				mBatteryMeter->fFadeIn( );
			}
		}
		else
		{
			if( !mDontResetComboOnExit )
				mStats->fResetComboMeter( );
			mDontResetComboOnExit = false;

			if( mPowerPool ) 
				mPowerPool->fShow( false );
			if( mBatteryMeter )
				mBatteryMeter->fFadeOut( );
			if( mScreenSpaceNotification )
				mScreenSpaceNotification->fEnable( false );
			fShowInUseIndicator( false, NULL );
		}

		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		if( level ) level->fHandleTutorialEvent( tTutorialEvent( GameFlags::cTUTORIAL_EVENT_USE_UNIT, (unitLogic != NULL), unitLogic ? unitLogic->fUnitID( ) : previousUnitID ) );
	}

#pragma optimize( "", off )
	namespace { 
		static const tStringPtr cLazerBarrageColumn( "BARRAGE_LAZER" );
		static const tStringPtr cNapalmBarrageColumn( "BARRAGE_NAPALM" );
	}

	b32 tPlayer::fUnitLocked( const tStringPtr& unitIDValueString ) const
	{
		const tLevelLoadInfo& info = tGameApp::fInstance( ).fCurrentLevelLoadInfo( );
		if( info.mMapType != GameFlags::cMAP_TYPE_CAMPAIGN )
			return false; //things only lock in campaign

		const tDataTable& dt = tGameApp::fInstance( ).fDataTable( tGameApp::cDataTableUnitUpgradeProgression ).fIndexTable( 0 );
		const tUserProfile& profile = *fProfile( );

		u32 currentRow = info.mLevelIndex;
		u32 highestRow = fMax( profile.fGetHighestLevelReached( info.mDlcNumber ), currentRow ); // otherwise, when starting from level 1, highest = 0 and current = 1, so it pulls from both

		sync_event_c( "HLR", highestRow, tSync::cSCPlayer );

		if( highestRow >= dt.fRowCount( ) )
		{
			log_warning( 0, "UnitUpgradeProgression table does not contain a row for HighestLevelReached = " << profile.fGetHighestLevelReached( info.mDlcNumber ) );
			return false;
		}
		if( currentRow >= dt.fRowCount( ) )
		{
			log_warning( 0, "UnitUpgradeProgression table does not contain a row for LevelIndex = " << currentRow );
			return false;
		}

		u32 col = dt.fColIndex( unitIDValueString );
		if( col == ~0 )
		{
			log_warning( 0, "UnitUpgradeProgression table does not contain a column for unit ID = " << unitIDValueString );
			return false;
		}

		s32 canUpgradeLevelSpecific = (s32)dt.fIndexByRowCol<f32>( currentRow, col );
		s32 canUpgrade = (s32)dt.fIndexByRowCol<f32>( highestRow, col );

		// HACK HACK HACK HACK OH GOD HACK
		// Have to check the DLC-Specific barrages
		if( col == dt.fColIndex( cLazerBarrageColumn ) )
			canUpgrade = ( (s32)dt.fIndexByRowCol<f32>( profile.fGetHighestLevelReached( GameFlags::cDLC_EVIL_EMPIRE ), col ) );
		if( col == dt.fColIndex( cNapalmBarrageColumn ) )
			canUpgrade = ( (s32)dt.fIndexByRowCol<f32>( profile.fGetHighestLevelReached( GameFlags::cDLC_NAPALM ), col ) );

		if( currentRow == 0 ) // HACK: Trial level hard coded!
			return !canUpgradeLevelSpecific;
		else
			return !( canUpgrade || canUpgradeLevelSpecific );
	}

	//if you beat level 1, call this with levelNumberBeat = 1 to see what units you've unlocked. expected to be called after match ended
	Sqrat::Object tPlayer::fCollectUnlockedUnitIDs( u32 levelNumberBeat ) const
	{
		tGameApp& app = tGameApp::fInstance( );
		const tDataTable& dt = app.fDataTable( tGameApp::cDataTableUnitUpgradeProgression ).fIndexTable( 0 );
		const tUserProfile& profile = *fProfile( );
		const tLevelLoadInfo& info = app.fCurrentLevelLoadInfo( );

		u32 currentRow = levelNumberBeat;
		u32 nextRow = currentRow + 1;

		if( nextRow >= dt.fRowCount( ) )
		{
			log_warning( 0, "UnitUpgradeProgression table does not contain a row: " << nextRow );
			return Sqrat::Object( );
		}

		Sqrat::Table out;

		// If this is the last level in the table, get out!
		if( nextRow >= tGameApp::fInstance( ).fNumLevelsInTable( GameFlags::cMAP_TYPE_CAMPAIGN ) )
			return out;

		// highest level reached will be incremented before htis function gets called
		//  so if they've beat level 1, HighetLevelReached will be 2
		if( mBeatLevelTheFirstTime && nextRow == profile.fGetHighestLevelReached( info.mDlcNumber ) )
		{
			// If the next level is part of another DLC
			const tLevelLoadInfo& nextLevelInfo = tGameApp::fInstance( ).fLevelLoadInfo( GameFlags::cMAP_TYPE_CAMPAIGN, nextRow );
			if( nextLevelInfo.mDlcNumber != info.mDlcNumber )
				return out;

			for( u32 i = 0; i < dt.fColCount( ); ++i )
			{
				b32 wasLocked = (dt.fIndexByRowCol<f32>( currentRow, i ) == 0);
				b32 nowLocked = (dt.fIndexByRowCol<f32>( nextRow, i ) == 0);

				if( wasLocked && !nowLocked )
				{
					const tStringPtr& unitName = dt.fColName( i );
					u32 id = GameFlags::fUNIT_IDValueStringToEnum( unitName );
					out.SetValue( _SC( unitName.fCStr( ) ), id );
				}
			}
		}

		return out;
	}

	Sqrat::Object tPlayer::fGetDataForDisplayCase( u32 unitID )
	{
		tGameApp& app = tGameApp::fInstance( );
		const tDataTable& dt = app.fDataTable( tGameApp::cDataTableUnitUpgradeProgression ).fIndexTable( 0 );
		const tUserProfile& profile = *fProfile( );
		const tLevelLoadInfo& info = app.fCurrentLevelLoadInfo( );

		u32 currentRow = profile.fGetHighestLevelReached( info.mDlcNumber );

		for( u32 i = 0; i < dt.fColCount( ); ++i )
		{
			b32 locked = (dt.fIndexByRowCol<f32>( currentRow, i ) == 0);
			u32 id = GameFlags::fUNIT_IDValueStringToEnum( dt.fColName( i ) );

			if( id == unitID )
			{
				Sqrat::Table data;
				data.SetValue( _SC( "id" ), dt.fColName( i ) );
				data.SetValue( _SC( "locked" ), locked );
				return data;
			}
		}

		return Sqrat::Object( );
	}

	namespace
	{
		u32 fMedal( f32 percent, f32 silver, f32 gold )
		{
			if( percent >= gold )
				return tLevelScores::cGold;
			else if( percent >= silver )
				return tLevelScores::cSilver;
			else
				return tLevelScores::cBronze;
		}

		u32 fOverallMedal( u32 medal0, u32 medal1, u32 medal2, b32 hasRewound )
		{
			u32 total = (medal0 + medal1 + medal2);

			if( total == 3 * tLevelScores::cGold ) 
				total += 1;

			if( total >= 2 && hasRewound )
				total -= 2;

			if( total == 10 )
				return tLevelScores::cPlatinum;
			else if( total >= 8 )
				return tLevelScores::cGold;
			else if( total >= 5 )
				return tLevelScores::cSilver;
			else
				return tLevelScores::cBronze;
		}
	}

	void tPlayer::fConfigureAudio( )
	{
		if( !mSoundSource )
			mSoundSource.fReset( NEW Audio::tSource( "Player" ) );
		
		if( mUser->fIsLocal( ) )
		{
			mSoundSource->fSetEnabled( true );
			mSoundSource->fLockToListener( fListenerIndex( ) );

			u32 mask = (1<<fListenerIndex( ));
			mSoundSource->fSetListenerMask( mask );
		}
		else
		{
			mSoundSource->fSetEnabled( false );
			mSoundSource->fSetListenerMask( 0 );
		}
	}

	void tPlayer::fScriptAudioEvent( const char *event )
	{
		mSoundSource->fHandleEvent( event );
	}

	b32 tPlayer::fVictorious( ) const
	{
		tGameApp& app = tGameApp::fInstance( );
		tLevelLogic* level = app.fCurrentLevel( );
		sigassert( level );

		if( level->fVictoriousPlayer( ) == this )
			return true;

		const b32 coop = app.fGameMode( ).fIsCoOp( );
		if( coop && level->fVictoriousPlayer( ) == app.fOtherPlayer( this ) )
			return true;

		return false;
	}

	namespace
	{
		void fIncMedalStats( const tPlayerPtr& player, u32 medal )
		{
			switch( medal )
			{
			case tLevelScores::cBronze:		player->fStats( ).fIncQuick( GameFlags::cSESSION_STATS_TOTAL_BRONZE_MEDALS, 1.f ); break;
			case tLevelScores::cSilver:		player->fStats( ).fIncQuick( GameFlags::cSESSION_STATS_TOTAL_SILVER_MEDALS, 1.f ); break;
			case tLevelScores::cGold:		player->fStats( ).fIncQuick( GameFlags::cSESSION_STATS_TOTAL_GOLD_MEDALS, 1.f ); break;
			case tLevelScores::cPlatinum:	player->fStats( ).fIncQuick( GameFlags::cSESSION_STATS_TOTAL_PLATINUM_MEDALS, 1.f ); break;
			};
		}
	}
#pragma optimize( "", off )
	void tPlayer::fLockScoreAndStats( b32 extraMiniGame, const tDynamicArray< tPlayerPtr >& players )
	{
		tGameApp& app = tGameApp::fInstance( );
		tLevelLogic* level = app.fCurrentLevel( );
		sigassert( level );
		sigassert( players.fCount( ) > 0 );

		const tLevelLoadInfo& info = app.fCurrentLevelLoadInfo( );
		const u32 levelIndex = info.mLevelIndex;
		const u32 dlc = info.mDlcNumber;
		const b32 realMiniGame = info.mMapType == GameFlags::cMAP_TYPE_MINIGAME;
		const b32 anyMiniGame = realMiniGame || extraMiniGame;
		const b32 versus = info.mMapType == GameFlags::cMAP_TYPE_HEADTOHEAD;
		const b32 coop = app.fGameMode( ).fIsCoOp( );
		b32 localVersusLoser = false;
		f32 totalMoney = 0.f;
		f32 totalScore = 0.f;
		b32 victorious = info.mMapType == GameFlags::cMAP_TYPE_SURVIVAL;
		
		// Build table for script
		tGrowableArray<Sqrat::Table> statTables;
		statTables.fSetCount( players.fCount( ) );

		for( u32 i = 0; i < players.fCount( ); ++i )
		{
			const tPlayerPtr& player = players[ i ];
			sigassert( player->mStats );

			if( player->fVictorious( ) )
				victorious = true;
			else if( info.mMapType == GameFlags::cMAP_TYPE_HEADTOHEAD && player->fUser( )->fIsLocal( ) )
				localVersusLoser = true;

			totalMoney += player->mInGameMoney;
		}

		if( victorious && info.mMapType == GameFlags::cMAP_TYPE_CAMPAIGN && !extraMiniGame )
		{
			// Advance highest level reached
			for( u32 i = 0; i < players.fCount( ); ++i )
			{
				if( players[ i ]->mIsNotAllowedToSaveStats )
					continue;

				u32 previousHighestLevelReached = players[ i ]->mProfile->fGetHighestLevelReached( dlc );
				// dont advanced level reached when an extra level-minigame locks scores
				players[ i ]->mProfile->fSetHighestLevelReached( levelIndex + 1, dlc );
				players[ i ]->mBeatLevelTheFirstTime = ( previousHighestLevelReached != players[ i ]->mProfile->fGetHighestLevelReached( dlc ) );
				players[ i ]->mProfile->fInvalidateLastRewind( );
			}
		}

		//compute score
		// old score
		//const f32 cTimeMultiplier = 600.f;
		//f32 totalScore = totalMoney 
		//				+ (totalMoney * 0.5f * ticketRatio) 
		//				+ cTimeMultiplier * mStats->fStat( GameFlags::cSESSION_STATS_SECONDS_SKIPPED );

		// new score
		u32 maxTickets = app.fMaxTickets( );
		s32 ticketsLeft = fMax( 0, (s32)maxTickets - (s32)players[ 0 ]->mStats->fStat( GameFlags::cSESSION_STATS_ENEMIES_REACHED_GOAL ) );

		sigassert( maxTickets > 0 );
		f32 ticketRatio = ticketsLeft / (f32)maxTickets;
		f32 skipableSeconds = players[ 0 ]->mSkippableSeconds;
		f32 secondsSkipped = 0.f;
		f32 moneyEarned = 0.f;
		f32 moneySpent = 0.f;

		for( u32 i = 0; i < players.fCount( ); ++i )
		{			
			moneyEarned += players[ i ]->mStats->fStat( GameFlags::cSESSION_STATS_MONEY_EARNED );
			moneySpent += players[ i ]->mStats->fStat( GameFlags::cSESSION_STATS_MONEY_SPENT );
			secondsSkipped += players[ i ]->mStats->fStat( GameFlags::cSESSION_STATS_SECONDS_SKIPPED );
		}

		f32 timeScore = fClamp( (skipableSeconds > 0.f) ? (secondsSkipped / skipableSeconds) : 1.f, 0.f, 1.f );
		f32 defenceScore = fClamp( ticketRatio, 0.f, 1.f );
		f32 moneyScore = fClamp( (moneyEarned > 0.0f )? (moneyEarned - moneySpent) / moneyEarned: 0.0f, 0.f, 1.f );

		u32 defenceMedal	= fMedal( defenceScore, 0.6f, 1.f );
		u32 timeMedal		= fMedal( timeScore, 0.4f, 0.70f );
		u32 moneyMedal		= fMedal( moneyScore, 0.5f, 0.7f );
		u32 overallMedal	= fOverallMedal( defenceMedal, timeMedal, moneyMedal, level->fRewindCount( ) > 0 );

		const f32 cDefenseScale = 100000.f;
		const f32 cTimeScale = 50000.f;
		const f32 cMoneyScale = 25000.f;

		f32 defenceSubScore = defenceScore * cDefenseScale;
		f32 timeSubScore = timeScore * cTimeScale;
		f32 moneySubScore = moneyScore * cMoneyScale;

		if( victorious && info.mMapType == GameFlags::cMAP_TYPE_CAMPAIGN && !anyMiniGame )
		{
			totalScore = defenceSubScore + timeSubScore + moneySubScore;

			for( u32 i = 0; i < players.fCount( ); ++i )
			{
				const tPlayerPtr& player = players[ i ];

				fIncMedalStats( player, defenceMedal );
				fIncMedalStats( player, timeMedal );
				fIncMedalStats( player, moneyMedal );
				fIncMedalStats( player, overallMedal );
				player->mStats->fSetStat( GameFlags::cSESSION_STATS_DEFENSE_SUB_SCORE,	defenceSubScore );
				player->mStats->fSetStat( GameFlags::cSESSION_STATS_TIME_SUB_SCORE,		timeSubScore );
				player->mStats->fSetStat( GameFlags::cSESSION_STATS_MONEY_SUB_SCORE,	moneySubScore );
			}
		}

		tGrowableArray<f32> myBonusPoints;
		myBonusPoints.fSetCount( players.fCount( ) );
		myBonusPoints.fFill( 0.f );

		// Bonuses
		for( u32 statID = 0; statID < GameFlags::cSESSION_STATS_COUNT; ++statID )
		{
			if( tGameSessionStats::fIsBonus( statID ) )
			{
				const tStringPtr stringID = tGameSessionStats::fBonusDisplayLocID( statID );
				std::stringstream pointsString;
				pointsString << stringID.fCStr( ) << "_Points";

				f32 bonusCount = 0;
				f32 bonusValue = 0;

				for( u32 i = 0; i < players.fCount( ); ++i )
				{
					const tPlayerPtr& player = players[ i ];

					const f32 count = player->mStats->fStat( statID );
					const f32 pointsFromBonus = tGameSessionStats::fBonusValue( statID ) * count;

					// individual scores
					statTables[ i ].SetValue( stringID.fCStr( ), count );
					statTables[ i ].SetValue( pointsString.str( ).c_str( ), pointsFromBonus );

					if( tGameSessionStats::fIsBonusButNotSpecial( statID ) )
					{
						bonusCount += count;
						bonusValue += pointsFromBonus;
						myBonusPoints[ i ] += pointsFromBonus;
					}
				}

				if( info.mMapType == GameFlags::cMAP_TYPE_CAMPAIGN && coop )
				{
					// only the first players table is looked at in end game screen.
					//  store combined value.
					statTables[ 0 ].SetValue( stringID.fCStr( ), bonusCount );
					statTables[ 0 ].SetValue( pointsString.str( ).c_str( ), bonusValue );
				}
			}
		}

		// Special Bonus points for WaveBonuses
		f32 pointsFromWaveBonus = 0.0f;
		f32 pointsFromWaveChain = 0.0f;
		f32 pointsFromSpeedBonus = 0.0f;
		f32 pointsFromBombRuns = 0.0f;
		f32 starPoints = 0.0f;
		u32 speedBonusCount = 0;
		f32 totalPointsFromBonuses = 0.0f;

		const b32 rainbowed = ( !anyMiniGame && info.mMapType == GameFlags::cMAP_TYPE_CAMPAIGN && fEqual( ticketRatio, 1.f ) );
		f32 rainbowBonus = 0.f;
		if( rainbowed )
			rainbowBonus += 100000.f;

		for( u32 p = 0; p < players.fCount( ); ++p )
		{
			const tPlayerPtr& player = players[ p ];

			f32 myPointsFromWaveBonus = 0.0f;
			f32 myPointsFromWaveChain = 0.0f;
			f32 myPointsFromSpeedBonus = 0.0f;
			f32 myPointsFromBombRuns = 0.0f;
			f32 myStarPoints = 0.0f;
			u32 mySpeedBonusCount = 0;

			for( u32 i = 0; i < player->mWaveBonusValues.fCount( ); ++i )
			{
				const tWaveKill& waveKill = player->mWaveBonusValues[ i ];
				myPointsFromWaveBonus += waveKill.mValue;
				myPointsFromWaveChain += waveKill.mValue * ( fMax( 0.0f, waveKill.mCombo - 1.0f ) );
			}

			for( u32 i = 0; i < player->mSpeedBonusValues.fCount( ); ++i )
			{
				const tSpeedKill& kill = player->mSpeedBonusValues[ i ];
				myPointsFromSpeedBonus += kill.mValue * kill.mPercentage;
			}
			mySpeedBonusCount += player->mSpeedBonusValues.fCount( );

			for( u32 i = 0; i < player->mBombingRunValues.fCount( ); ++i )
			{
				log_line( 0, "Bomb Run Value: " << player->mBombingRunValues[ i ] );

				myPointsFromBombRuns += player->mBombingRunValues[ i ] * tGameSessionStats::fBonusValue( GameFlags::cSESSION_STATS_BOMBING_RUN );
			}

			myStarPoints += player->mStarPoints;

			sigassert( players.fCount( ) );
			f32 myRainbowPoints = rainbowBonus / players.fCount( );

			statTables[ p ].SetValue( "Bonus_WaveBonus_Points", myPointsFromWaveBonus );
			statTables[ p ].SetValue( "Bonus_WaveChain_Points", myPointsFromWaveChain );
			statTables[ p ].SetValue( "Bonus_Speed", mySpeedBonusCount );
			statTables[ p ].SetValue( "Bonus_Speed_Points", myPointsFromSpeedBonus );
			statTables[ p ].SetValue( "Bonus_BombingRun_Points", myPointsFromBombRuns );
			statTables[ p ].SetValue( "Bonus_Stars_Points", myStarPoints );
			statTables[ p ].SetValue( "Bonus_Rainbow_Points", myRainbowPoints );

			myBonusPoints[ p ] += myPointsFromWaveBonus + myPointsFromWaveChain + myPointsFromSpeedBonus + myPointsFromBombRuns + myRainbowPoints + myStarPoints;
			
			statTables[ p ].SetValue( "BonusPoints", myBonusPoints[ p ] );

			//totals
			pointsFromWaveBonus += myPointsFromWaveBonus;
			pointsFromWaveChain += myPointsFromWaveChain;
			pointsFromSpeedBonus += myPointsFromSpeedBonus;
			pointsFromBombRuns += myPointsFromBombRuns;
			starPoints += myStarPoints;
			speedBonusCount += mySpeedBonusCount;
			totalPointsFromBonuses += myBonusPoints[ p ];
		}

		if( info.mMapType == GameFlags::cMAP_TYPE_CAMPAIGN && coop )
		{
			// only the first players table is looked at in end game screen.
			//  store combined value.
			statTables[ 0 ].SetValue( "Bonus_WaveBonus_Points", pointsFromWaveBonus );
			statTables[ 0 ].SetValue( "Bonus_WaveChain_Points", pointsFromWaveChain );
			statTables[ 0 ].SetValue( "Bonus_Speed", speedBonusCount );
			statTables[ 0 ].SetValue( "Bonus_Speed_Points", pointsFromSpeedBonus );
			statTables[ 0 ].SetValue( "Bonus_BombingRun_Points", pointsFromBombRuns );
			statTables[ 0 ].SetValue( "Bonus_Stars_Points", starPoints );
			statTables[ 0 ].SetValue( "Bonus_Rainbow_Points", rainbowBonus );
			statTables[ 0 ].SetValue( "BonusPoints", totalPointsFromBonuses );
		}


		log_line( 0, "Wave Chain: value: " << pointsFromWaveChain << " bonus " << pointsFromWaveBonus );
		log_line( 0, "Speed Run: value: " << pointsFromSpeedBonus );
		log_line( 0, "Bomb Run: value: " << pointsFromBombRuns );

		totalScore += totalPointsFromBonuses;
		totalScore = fMax( totalScore, 0.f );
		totalScore = fRoundDown<f32>( totalScore );

		if( !anyMiniGame )
		{
			for( u32 i = 0; i < players.fCount( ); ++i )
			{
				const tPlayerPtr& player = players[ i ];
				player->mStats->fSetStat( GameFlags::cSESSION_STATS_TOTAL_MONEY, (f32)totalMoney );

				if( info.mMapType != GameFlags::cMAP_TYPE_HEADTOHEAD )
				{
					player->mStats->fSetStat( GameFlags::cSESSION_STATS_SCORE, totalScore );
					player->mStats->fCheckStatBeatFriends( GameFlags::cSESSION_STATS_SCORE, totalScore, 0.f );
				}
			}
		}

		if( info.mMapType == GameFlags::cMAP_TYPE_CAMPAIGN )
		{
			// Campaign Specific scores
			for( u32 i = 0; i < players.fCount( ); ++i )
			{
				statTables[ i ].SetValue( "TicketsLeft", ticketsLeft );
				statTables[ i ].SetValue( "TicketsMax", maxTickets );
				statTables[ i ].SetValue( "DefensePoints", defenceSubScore );
				statTables[ i ].SetValue( "DefenseMedal", defenceMedal );
				statTables[ i ].SetValue( "SecondsSkipped", secondsSkipped );
				statTables[ i ].SetValue( "AggressionPoints", timeSubScore );
				statTables[ i ].SetValue( "AggressionMedal", timeMedal );
				statTables[ i ].SetValue( "ProfitRatio", moneyScore );
				statTables[ i ].SetValue( "ProfitPoints", moneySubScore );
				statTables[ i ].SetValue( "ProfitMedal", moneyMedal );
				statTables[ i ].SetValue( "OverallMedal", overallMedal );
			}
		}
		
		if( info.mMapType == GameFlags::cMAP_TYPE_SURVIVAL )
		{
			tLevelLogic* level = app.fCurrentLevel( );
			sigassert( level );

			u32 survivalRound = 0;

			tWaveList* wavelist = level->fDisplayedWaveList( );
			if( wavelist )
				survivalRound = wavelist->fSurvivalRound( ) + 1;

			f32 seconds = players[ 0 ]->mStats->fStat( GameFlags::cSESSION_STATS_TOTAL_TIME );
			f32 kills = 0.f;
			const f32 cKillValue = 100.f;

			for( u32 i = 0; i < players.fCount( ); ++i )
			{
				f32 myKills = players[ i ]->mStats->fStat( GameFlags::cSESSION_STATS_KILLS );
				kills += myKills;

				statTables[ i ].SetValue( "Kills", myKills );
				statTables[ i ].SetValue( "PointsFromKills", myKills * cKillValue ); // TODO
			}

			f32 secondsPoints = seconds * 1000.0f;
			f32 killsPoints = kills * cKillValue;
			f32 roundPoints = survivalRound * 10000.0f;
			totalScore = secondsPoints + killsPoints + roundPoints + totalPointsFromBonuses;

			// Survival specific scores

			for( u32 i = 0; i < players.fCount( ); ++i )
			{
				statTables[ i ].SetValue( "TotalTime", seconds );
				statTables[ i ].SetValue( "PointsFromTime", secondsPoints ); // TODO
				statTables[ i ].SetValue( "Round", survivalRound );
				statTables[ i ].SetValue( "PointsFromRounds", roundPoints ); // TODO
				players[ i ]->mStats->fSetStat( GameFlags::cSESSION_STATS_SCORE, totalScore );
			}
		}

		// determine a few more stats
		if( versus )
		{
			// only handle us, other player will do his own computation
			for( u32 p = 0; p < players.fCount( ); ++p )
			{
				const tPlayerPtr& player = players[ p ];
				if( victorious ) 
					player->mStats->fIncStat( GameFlags::cSESSION_STATS_VERSUS_WINS, 1.f );
				else
					player->mStats->fIncStat( GameFlags::cSESSION_STATS_VERSUS_LOSSES, 1.f );

				{
					s32 ticketsLeft = fMax( 0, (s32)maxTickets - (s32)player->mStats->fStat( GameFlags::cSESSION_STATS_ENEMIES_REACHED_GOAL ) );
					f32 ticketRatio = ticketsLeft / (f32)maxTickets;
					if( fEqual( ticketRatio, 1.f ) )
						player->mStats->fIncStat( GameFlags::cSESSION_STATS_VERSUS_SHUT_OUTS, 1.f );
				}

				// achievement
				{
					b32 hasPlatform = false;
					for( u32 i = 0; i < level->fBuildSiteRootsSmall( ).fCount( ); ++i )
					{
						tBuildSiteLogic* bs = level->fBuildSiteRootsSmall( )[ i ]->fLogicDerived<tBuildSiteLogic>( );
						sigassert( bs );
						if( bs->fCapturable( ) && bs->fCapturedTeam( ) == player->fTeam( ) )
							hasPlatform = true;
					}
					for( u32 i = 0; i < level->fBuildSiteRootsLarge( ).fCount( ); ++i )
					{
						tBuildSiteLogic* bs = level->fBuildSiteRootsLarge( )[ i ]->fLogicDerived<tBuildSiteLogic>( );
						sigassert( bs );
						if( bs->fCapturable( ) && bs->fCapturedTeam( ) == player->fTeam( ) )
							hasPlatform = true;
					}

					if( hasPlatform )
						player->fAwardAchievementDeferred( GameFlags::cACHIEVEMENTS_KING_OF_THE_HILL );
				}
			}
		}

		b32 writeStats = false;
		b32 writeScore = false;

		if( ( victorious || localVersusLoser ) && !anyMiniGame )
		{
			// campaign, survival, versus
			writeStats = true;
			writeScore = true;

			if( info.mMapType == GameFlags::cMAP_TYPE_CAMPAIGN )
			{
				for( u32 i = 0; i < players.fCount( ); ++i )
				{
					const tPlayerPtr& player = players[ i ];

					//TS1 achievement for completing a level on Elite Difficulty
					if( info.mDifficulty == GameFlags::cDIFFICULTY_ELITE && (player->fCountry( ) == GameFlags::cCOUNTRY_BRITISH || player->fCountry( ) == GameFlags::cCOUNTRY_GERMAN) )
					{
							player->fAwardAchievement( GameFlags::cACHIEVEMENTS_TS1_THROW_A_SEVEN );
					}


					if( overallMedal == tLevelScores::cPlatinum )
					{
					if( info.mDifficulty == GameFlags::cDIFFICULTY_GENERAL )
							player->fAwardAchievement( GameFlags::cACHIEVEMENTS_EFFECTIVE_TACTICIAN );

						player->fAwardAchievement( GameFlags::cACHIEVEMENTS_A_JOB_WELL_DONE );

						if( player->fHasDLC( GameFlags::cDLC_EVIL_EMPIRE ) && info.mDifficulty == GameFlags::cDIFFICULTY_ELITE && coop )
							player->fAwardAchievement( GameFlags::cACHIEVEMENTS_DLC1_BROVIET );

					}

					if( !player->mIsNotAllowedToSaveStats )
					{
						tLevelScores* scores = player->fProfile( )->fGetLevelScores( info.mMapType, levelIndex );
						sigassert( scores );

						scores->fSetMedalProgress( app.fDifficulty( ), tLevelScores::cDefenceMedal, defenceMedal );
						scores->fSetMedalProgress( app.fDifficulty( ), tLevelScores::cTimeMedal, timeMedal );
						scores->fSetMedalProgress( app.fDifficulty( ), tLevelScores::cMoneyMedal, moneyMedal );
						scores->fSetMedalProgress( app.fDifficulty( ), tLevelScores::cOverallMedal, overallMedal );
					}
				}
			}
		}

		if( info.mMapType == GameFlags::cMAP_TYPE_MINIGAME )
		{
			// regular minigame
			writeStats = true;
			writeScore = true;
		}

		if( writeScore )
		{
			u32 scoreIndex = app.fDifficulty( );
			if( info.mMapType == GameFlags::cMAP_TYPE_SURVIVAL )
				scoreIndex = app.fChallengeMode( );
			else if( info.mMapType == GameFlags::cMAP_TYPE_MINIGAME )
				scoreIndex = 0;

			for( u32 i = 0; i < players.fCount( ); ++i )
			{
				const tPlayerPtr& player = players[ i ];
				if( player->mIsNotAllowedToSaveStats )
					continue;

				tLevelScores* scores = player->fProfile( )->fGetLevelScores( info.mMapType, levelIndex );
				sigassert( scores );

				scores->fSetHighScore( scoreIndex, (u32)player->fStats( ).fStat( GameFlags::cSESSION_STATS_SCORE ) );
			}
		}

		if( writeStats )
		{			
			for( u32 b = 0; b < 2; ++b )
			{
				// two stats boards
				for( u32 i = 0; i < players.fCount( ); ++i )
				{
					const tPlayerPtr& player = players[ i ];
					if( player->mIsNotAllowedToSaveStats )
						continue;

					u32 propCount = 0;
					tFixedArray<tUserProperty, GameFlags::cSESSION_STATS_COUNT> props;
					player->fCreateStatPropsAndWriteToProfile( propCount, props, b );

					if( propCount )
						app.fGameAppSession( )->fWriteStats( player.fGetRawPtr( ), (b==0) ? GameSession::cLeaderboardPlayerStats : GameSession::cLeaderboardPlayerStats2, propCount, props.fBegin( ) );
				}
			}
		}

		// logging
		if( writeScore )
		{
			log_line( 0, "Score breakdown: " );
			log_line( 0, " Tickets: Left: " << ticketsLeft << " Max: " << maxTickets << " Defense: " << defenceScore << " Score: " << defenceSubScore << " Medal: " << defenceMedal );
			log_line( 0, " Time: Skipped: " << secondsSkipped << " Max: " << skipableSeconds << " Time: " << timeScore << " Score: " << timeSubScore << " Medal: " << timeMedal );
			log_line( 0, " Money: Earned: " << moneyEarned << " Spent: " << moneySpent << " Money: " << moneyScore << " Score: " << moneySubScore << " Medal: " << moneyMedal );
			log_line( 0, " Bonus: " << totalPointsFromBonuses );
			log_line( 0, "Total: " << totalScore << " Medal: " << overallMedal );
		}

		for( u32 i = 0; i < players.fCount( ); ++i )
		{
			const tPlayerPtr& player = players[ i ];

			statTables[ i ].SetValue( "OverallScore", totalScore );
			player->mLevelScoreAndStats = statTables[ i ];

			if( anyMiniGame )
				player->fResetMiniGameStats( );
			else
			{
				// no more stats, only for non minigame types
				player->mStats->fSetLockIncrements( true );
			}
		}
	}

	void tPlayer::fCreateCachedStatProps( tGrowableArray<tUserProperty>& props, u32 tableIndex )
	{
		for( u32 i = 0; i < GameFlags::cSESSION_STATS_COUNT; ++i )
		{
			if( tableIndex == 0 && i >= GameSession::cLeaderboardPlayerStats2StartID )
				continue;
			if( tableIndex == 1 && i < GameSession::cLeaderboardPlayerStats2StartID )
				continue;
			if( tGameSessionStats::fStatTemp( i ) )
				continue; // no stats expected exist
			if( GameSession::cPopertyPlayerStats[ i ] == ~0 )
				continue;

			if( mProfile->fAllTimeStat( i ) != 0.f )
			{
				u32 lbValue = 0;

				if( i == GameFlags::cSESSION_STATS_SCORE )
				{
					// This score isn't just the aggregate of every session.
					lbValue = (u32)mProfile->fGetTotalLevelScore( );

				}
				else
				{
					lbValue = tGameSessionStats::fStatToLeaderBoard( i, mProfile->fAllTimeStat( i ) );
				}

				tUserProperty prop;
				prop.mId = GameSession::cPopertyPlayerStats[ i ];
				prop.mData.fSet( (s64)lbValue );
				props.fPushBack( prop );
			}
		}
	}

	void tPlayer::fCreateStatPropsAndWriteToProfile( u32& propCount, tFixedArray<tUserProperty, GameFlags::cSESSION_STATS_COUNT>& props, u32 tableIndex )
	{
		for( u32 i = 0; i < GameFlags::cSESSION_STATS_COUNT; ++i )
		{
			if( tableIndex == 0 && i >= GameSession::cLeaderboardPlayerStats2StartID )
				continue;
			if( tableIndex == 1 && i < GameSession::cLeaderboardPlayerStats2StartID )
				continue;

			if( tGameSessionStats::fStatTemp( i ) )
				continue; // no stats expected exist
			if( GameSession::cPopertyPlayerStats[ i ] == ~0 )
				continue;

			if( mStats->fStatWasSet( i ) )
			{
				f32 stat = mStats->fStat( i );

				f32 allTime = 0.f;
				u32 lbValue = 0;
				
				if( i == GameFlags::cSESSION_STATS_SCORE )
				{
					// This score isn't just the aggregate of every session.
					allTime = (f32)mProfile->fGetTotalLevelScore( );
					mProfile->fSetAllTimeStat( i, (f32)mProfile->fGetTotalLevelScore( ) );
					lbValue = (u32)allTime;

				}
				else
				{
					allTime = mProfile->fIncAllTimeStat( i, stat );
					lbValue = tGameSessionStats::fStatToLeaderBoard( i, allTime );
				}

				tUserProperty & prop = props[ propCount++ ];
				prop.mId = GameSession::cPopertyPlayerStats[ i ];
				prop.mData.fSet( (s64)lbValue );

				// DEBUGGING
				//if( tableIndex == 1 )
				//{
				//	log_line( 0, "stat prop: " << GameFlags::fSESSION_STATSEnumToValueString( i ) << " id val: " << prop.mId );	


				////	if( i >= Debug_StopWritingStats )
				////	{
				////		//std::sort( props.fBegin( ), props.fEnd( ) );
				////		//for( u32 i = 0; i < props.fCount( ); ++i )

				////		return;
				////	}
				//}
			}
		}
	}

	void tPlayer::fResetMiniGameStats()
	{
		sigassert( mStats );
		
		// Bonuses
		for( u32 statID = 0; statID < GameFlags::cSESSION_STATS_COUNT; ++statID )
		{
			if( tGameSessionStats::fIsBonus( statID ) )
				mStats->fSetStat( statID, 0 );
		}

		mWaveBonusValues.fSetCount( 0 );
		mCurrentWaveChain = 0;
		mBombingRunCount = 0;
		mBombingRunTimer = 0.f;
		mBombingRunLastKillPos = Math::tVec3f::cZeroVector;
		mStarPoints = 0.0f;
		mSpeedBonusValues.fSetCount( 0 );
		mBombingRunValues.fSetCount( 0 );

		mEarnedItems.fSetCount( 0 );
	}

	tUnitLogic* tPlayer::fBuildTurret( tBuildSiteLogic* site, u32 unitID )
	{
		sigassert( site );

		if( mCurrentUnit && mCurrentUnit->fUnitID( ) == unitID && site->fUnit( ) == mCurrentUnit->fOwnerEntity( ) )
			return mCurrentUnit; //already there

		fUnlockFromUnit( true );

		if( site->fIsOccupied( ) )
		{
			site->fShape( )->fClearChildren( );
		}

		sigassert( !site->fIsOccupied( ) );

		site->fSetReserved( true );
		tEntity* parent = site->fShape( );
		tEntity* gameTurret = parent->fSpawnChild( tGameApp::fInstance( ).fUnitResourcePath( (GameFlags::tUNIT_ID)unitID, (GameFlags::tCOUNTRY)fCountry( ) ) );
		gameTurret->fSetLockedToParent( false );
		gameTurret->fSpawn( *parent );
		gameTurret->fMoveTo( parent->fObjectToWorld( ).fGetTranslation( ) );

		tUnitLogic* unitLogic = gameTurret->fLogicDerived< tUnitLogic >( );
		sigassert( unitLogic );
		gameTurret->fAddGameTags( GameFlags::cFLAG_SELECTABLE );
		unitLogic->fSetCreationType( tUnitLogic::cCreationTypeFromBuildSite );

		mChangeUseTurret = true;
		mNewUseUnit.fReset( gameTurret );
		mLockedInUnit = true;

		return unitLogic;
	}

	void tPlayer::fGiveOverCharge( )
	{
		if( mCurrentUnit && mStats->fComboGroups( ).fCount( ) )
		{
			u32 overchargeCost = tGameApp::fInstance( ).fOverChargeCost( mCurrentUnit->fUnitID( ), mCurrentUnit->fCountry( ) );
			
			u32& combo = mStats->fComboGroups( )[ 0 ].mCurrentCombo;
			combo = fMax( combo, overchargeCost );
			mStats->fComboGroups( )[ 0 ].mComboTimeRemaining = 10.0f;
			mStats->fSetComboMeter( 1.f );
		}
	}

	void tPlayer::fCancelOvercharge( )
	{
		if( mStats->fComboGroups( ).fCount( ) )
		{
			mStats->fComboGroups( )[ 0 ].mCurrentCombo = 1;
			mStats->fComboGroups( )[ 0 ].mComboTimeRemaining = 0.0f;
			mStats->fSetComboMeter( 0.f );
		}
	}

	void tPlayer::fOnMatchEnded( )
	{
		fCloseRadialMenus( );
	}

	void tPlayer::fResetAmmo( )
	{
		if( mCurrentUnit )
			mCurrentUnit->fResetAmmo( );
	}

	void tPlayer::fRestrictBarrage( const tStringPtr& name )
	{
		if( mBarrageController )
			mBarrageController->fRestrictBarrage( name );
	}

	void tPlayer::fGiveBarrage( b32 forTutorial )
	{
		if( mStats->fComboGroups( ).fCount( ) )
		{
			if( forTutorial )
			{
				u32 overchargeCost = (u32)mStats->fOverChargeComboCost( );
				u32& combo = mStats->fComboGroups( )[ 0 ].mCurrentCombo;
				combo = fMax( combo, overchargeCost * 2 );
				mStats->fComboGroups( )[ 0 ].mComboTimeRemaining = 10.0f;
				mStats->fSetComboMeter( 2.f );

				if( mPowerPool )
					mPowerPool->fShowBarrage( true );
			}

			tPlayer* otherPlayer = tGameApp::fInstance( ).fGameMode( ).fIsCoOp( )? tGameApp::fInstance( ).fOtherPlayer( this ) : NULL;

			if( mBarrageController )
				mBarrageController->fGiveBarrage( otherPlayer, false, forTutorial );
		}
	}

	void tPlayer::fSetBarrageSkip( tEntity* target )
	{
		if( mBarrageController )
			mBarrageController->fCurrentBarrage( ).fCodeObject( )->fSetTarget( this, target );
	}

	void tPlayer::fGiveInstantBarrage( const tStringPtr& name, tEntity* target )
	{
		if( mBarrageController )
			mBarrageController->fSkipSpinAndEnter( name, target );
	}

	void tPlayer::fForceUseBarrage( )
	{
		if( mBarrageController && !mBarrageController->fCurrentBarrage( ).fIsNull( ) )
			mBarrageController->fCurrentBarrage( ).fCodeObject( )->fForceUse( this );
	}

	void tPlayer::fSetUniqueBarrages( b32 unique )
	{
		if( mBarrageController )
		{
			if( unique )
				mBarrageController->fBeginNoRepeats( );
			else
				mBarrageController->fEndNoRepeats( );
		}
	}

	void tPlayer::fShowFocus( b32 show )
	{
		mFocusShown = show;
		mFocusShowTimer = mCurrentItem.mDuration;

		if( show )
		{
			if( mFocusCameraPushed )
			{
				fPushFocusCamera( false );
				fPushFocusCamera( true );
			}

			if( mCurrentItem.mEntity )
			{
				tEntity* logicEnt = mCurrentItem.mEntity->fFirstAncestorWithLogic( );
				if( logicEnt )
					mCurrentFocusUnit = logicEnt->fLogicDerived<tUnitLogic>( );
			}
		}
		else
		{
			mCurrentFocusUnit = NULL;
			mCurrentItem.mEntity.fRelease( );
		}

		if( mFocalPrompt )
		{
			if( show )
			{
				mFocalPrompt->fShow( mCurrentItem.mMessage );
				if( mFocusCameraPushed )
					mFocalPrompt->fHide( );
			}
			else
				mFocalPrompt->fHide( );
		}
	}

	void tPlayer::fHideFocus( b32 hide )
	{
		if( mFocalPrompt )
			mFocalPrompt->fHide( hide );
	}

	void tPlayer::fStepFocus( f32 dt )
	{
		if( mFocusShown )
		{
			mFocusShowTimer -= dt;
			if( mFocusShowTimer < 0.f || (mCurrentFocusUnit && mCurrentFocusUnit->fIsDestroyed( )) )
				fShowFocus( false ) ;

		}

		for( u32 i = 0; i < mFocusItems.fNumItems( ); ++i )
		{
			if( mFocusItems[ i ].mEntity && mFocusItems[ i ].mEntity->fSceneGraph( ) )
			{
				mFocusItems[ i ].mDelay -= dt;
				if( mFocusItems[ i ].mDelay < 0.f )
				{				
					//if( !mFocusShown && mCurrentItem.mMessage != mFocusItems[ i ].mMessage )
					//	fShowFocus( false );

					if( !mFocusShown )
					{
						mCurrentItem = mFocusItems[ i ];
						mFocusItems.fGet( );

						fShowFocus( true );
						if( i > 0 )
							--i;
					}
				}
			}
		}

		if( !mFocusCameraPushed )
		{
			if( mFocusShown && fGameController( )->fButtonHeld( tUserProfile::cProfileTurrets, GameFlags::cGAME_CONTROLS_CAMERA_FOCUS_EVENT ) )
			{
				fPushFocusCamera( true );
			}
		}
		else
		{
			mFocusCameraTimer += dt;

			if( !fGameController( )->fButtonHeld( tUserProfile::cProfileTurrets, GameFlags::cGAME_CONTROLS_CAMERA_FOCUS_EVENT ) )
			{
				const f32 cMinCameraTime = 2.f;
				if( mFocusCameraTimer > cMinCameraTime )
				{
					fShowFocus( false );
					fPushFocusCamera( false );
				}
			}
		}
	}

	void tPlayer::fPushFocusCamera( b32 push )
	{
		if( push )
		{
			if( !mFocusCameraPushed && mFocusShown )
			{
				mFocusCameraPushed = true;
				mFocusCameraTimer = 0.f;

				fCameraStack( ).fPopCamerasOfType<tFocusCamera>( );

				sigassert( fCameraStack( ).fCount( ) );
				tRtsCamera* rtsCam = fCameraStack( ).fTop( )->fDynamicCast<tRtsCamera>( );
				if( rtsCam )
					rtsCam->fSetPreventPositionAcquisition( true );

				fCameraStack( ).fPushCamera( Gfx::tCameraControllerPtr( NEW tFocusCamera( *this, mCurrentItem.mEntity, -1, mCurrentItem.mBlendDist ) ) );
				
				fHideFocus( true );
			}
		}
		else
		{
			if( mFocusCameraPushed )
			{
				mFocusCameraPushed = false;
				fCameraStack( ).fPopCamerasOfType<tFocusCamera>( );

				//if( mFocusShown )
				//	fHideFocus( false );
			}
		}
	}

	void tPlayer::fFocusTarget( const tFocusItem& item )
	{
		if( mFocusItems.fNumItems( ) == 0 && !mFocusShown && mBarrageController
			&& (mBarrageController->fCurrentBarrage( ).fIsNull( ) || mBarrageController->fCurrentBarrage( ).fCodeObject( )->mDevName == cB52BarrageName) )
		{
			if( mFocusItems.fNumItems( ) == mFocusItems.fCapacity( ) )
			{
				log_warning( 0, "Too many focus items" );
				mFocusItems.fResize( u32(mFocusItems.fNumItems( ) * 1.5f) );
			}
			
			mFocusItems.fPut( item );
		}
		else
			log_warning( 0, "Only one focus target at a time for demo sanity" );
	}

	void tPlayer::fSpawnIndicatorArrow( tEntity* attachTo, f32 duration )
	{
		sigassert( fUser( ) );
		Gui::tWaveLaunchArrowUI* arrow = NEW Gui::tWaveLaunchArrowUI( tGameApp::fInstance( ).fGlobalScriptResource( tGameApp::cGlobalScriptWaveLaunchArrowUI ), fUser( ), duration );
		arrow->fSpawn( *attachTo );
		arrow->fSetParentRelativeXform( Math::tMat3f::cIdentity );
	}

	b32 tPlayer::fDisableYAccessDueToFocus( ) const
	{
		return mFocusShown || mFocusCameraPushed;
	}

	u32 tPlayer::fCurrentLevelHighScore( ) const
	{
		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		if( !level )
			return 0;

		u32 scoreIndex = 0;
		if( level->fMapType( ) == GameFlags::cMAP_TYPE_SURVIVAL )
			scoreIndex = tGameApp::fInstance( ).fChallengeMode( );
		else if( level->fMapType( ) == GameFlags::cMAP_TYPE_CAMPAIGN )
			scoreIndex = tGameApp::fInstance( ).fDifficulty( );
		return level->fLevelScores( this )->fGetHighScore( scoreIndex );
	}

	tLevelScores* tPlayer::fLevelScores( )
	{
		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		if( !level )
			return NULL;

		return level->fLevelScores( this );
	}

	Sqrat::Object tPlayer::fCurrentUnitScript( ) const 
	{ 
		return mCurrentUnit ? mCurrentUnit->fOwnerEntity( )->fScriptLogicObject( ) : Sqrat::Object( ); 
	}


	void tPlayer::fPushTiltShiftCamera( )
	{
		fPushCamera( Gfx::tCameraControllerPtr( NEW tTiltShiftCamera( *this ) ) );
	}

	void tPlayer::fPopTiltShiftCamera( )
	{
		fCameraStack( ).fPopCamerasOfType<tTiltShiftCamera>( );
	}

	void tPlayer::fSpawnCharacter( tFilePathPtr& path, const tStringPtr& name )
	{
		mCharacterPath = path;
		mLastSpawnPt = name;

		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		sigassert( level );

		tEntity* spawnPt = level->fPlayerSpawn( name );
		sigassert( spawnPt );

		tEntity* ent = level->fRootEntity( )->fSpawnChild( path );
		if( ent )
		{
			ent->fMoveTo( spawnPt->fObjectToWorld( ) );
			tCharacterLogic* logic = ent->fLogicDerived<tCharacterLogic>( );
			if( logic )
				logic->fSetTeamPlayer( this );
			else
				log_warning( 0, "Player does not have character logic!" );
		}
	}

	void tPlayer::fRespawn( )
	{
		fSpawnCharacter( mCharacterPath, mLastSpawnPt );
	}

	void tPlayer::fCreateInUseIndicator( )
	{
		tUserArray users;
		tGameApp& gameApp = tGameApp::fInstance( );

		if( gameApp.fGameMode( ).fIsMultiPlayer( ) )
		{
			for( u32 i = 0; i < gameApp.fPlayers( ).fCount( ); ++i )
			{
				tPlayer* player = gameApp.fPlayers( )[ i ].fGetRawPtr( );
				if( player && player != this && player->fIsActive( ) && !player->fUser( )->fIsViewportVirtual( ) )
					users.fPushBack( player->fUser( ) );
			}

			mInUseIndicator.fReset( NEW Gui::tInUseIndicator( users ) );
			mInUseIndicator->fSpawn( *gameApp.fCurrentLevel( )->fOwnerEntity( ) );
			mInUseIndicator->fHide( );
			mInUseIndicator->fSetIndicator( this, GameFlags::cCOUNTRY_DEFAULT );
			mInUseIndicator->fSetObjectSpaceOffset( cInUseIndicatorOffset );
		}
	}

	void tPlayer::fReleaseInUseIndicator( )
	{
		if( mInUseIndicator )
		{
			mInUseIndicator->fHide( );
			mInUseIndicator.fRelease( );
		}
	}

	void tPlayer::fAddInUseIndicator( tUnitLogic* unit )
	{
		if( mInUseIndicator )
		{
			sigassert( unit );
			mInUseIndicator->fReparentImmediate( *unit->fOwnerEntity( ) );
			mInUseIndicator->fSetParentRelativeXform( Math::tMat3f::cIdentity );
		}
	}

	void tPlayer::fShowInUseIndicator( b32 show, tUnitLogic* unit )
	{
		if( mInUseIndicator )
		{
			if( show && unit ) 
			{
				fAddInUseIndicator( unit );
				unit->fShowInUseIndicator( mInUseIndicator.fGetRawPtr( ) );
			}
			else
				mInUseIndicator->fHide( );
		}
	}

	void tPlayer::fUpdateWaveLaunchMenu( )
	{
		if( tGameApp::fInstance( ).fGameMode( ).fIsVersus( ) )
		{
			if( mWaveLaunchMenu )
			{
				const tGameControllerPtr gc = mWaveLaunchMenu->fGameController( );
				u32 inputFilter = mWaveLaunchMenu->fInputFilterLevel( );
				b32 closeMenu = false;

				// Close the menu if the user presses B (for back) or Y (what they used to open it)
				if( ( ( gc->fMode( ) == tGameController::GamePad ) && gc->fButtonDown( tUserProfile::cProfileCamera, GameFlags::cGAME_CONTROLS_CANCEL, inputFilter ) ) ||
					( ( gc->fMode( ) == tGameController::KeyboardMouse ) && gc->fButtonDown( tUserProfile::cProfileCamera, GameFlags::cGAME_CONTROLS_TRIGGER_SECONDARY, inputFilter ) ) ||
					gc->fButtonDown( tUserProfile::cProfileCamera, GameFlags::cGAME_CONTROLS_LAUNCH_WAVE, inputFilter ) ) // NOTE: comment says Y, code said X.. using X.
					closeMenu = true;
				else
				{

					if(gc->fMode() == tGameController::KeyboardMouse)
					{
						mWaveLaunchMenu->fHighlightByPosition( gc->fUIMousePos( inputFilter ), 20.0f);
					}
					else
					{
						mWaveLaunchMenu->fHighlightByAngle( gc->fMoveStick( tUserProfile::cProfileCamera, inputFilter ).fAngle( ), gc->fMoveStickMagnitude( tUserProfile::cProfileCamera, inputFilter ) );
					}

					if( gc->fButtonDown( tUserProfile::cProfileCamera, GameFlags::cGAME_CONTROLS_SELECT, inputFilter ) && mWaveLaunchMenu->fSelectActiveIcon( ) )
						closeMenu = true;
				}

				if( closeMenu )
					fHideShowOffensiveWaveMenu( false );
			}
			else if( !tGameApp::fInstance( ).fCurrentLevelDemand( )->fVictoryOrDefeat( ) )
			{
				const tGameControllerPtr gc = fGameController( );
				if( gc->fButtonDown( tUserProfile::cProfileCamera, GameFlags::cGAME_CONTROLS_LAUNCH_WAVE ) ) // NOTE: was fFilteredGamepad( 0 )
					fHideShowOffensiveWaveMenu( true );
			}
		}
	}

	void tPlayer::fHideShowOffensiveWaveMenu( b32 show )
	{
		if( show )
		{
			if( !mWaveLaunchMenu )
			{
				mWaveLaunchMenu = Gui::tRadialMenuPtr( NEW Gui::tRadialMenu( tGameApp::fInstance( ).fGlobalScriptResource( tGameApp::cGlobalScriptWaveLaunch ), fUser( ), fGameController( ) ) );
				tGameApp::fInstance( ).fCurrentLevel( )->fVersusWaveManager( )->fSetupOffensiveWaveMenu( mWaveLaunchMenu, this );

				tGameApp::fInstance( ).fRootHudCanvas( ).fToCanvasFrame( ).fAddChild( mWaveLaunchMenu->fCanvas( ) );
				const Math::tRect vpRect = fUser( )->fComputeViewportRect( );
				mWaveLaunchMenu->fCanvas( ).fCodeObject( )->fSetPosition( Math::tVec3f( vpRect.fCenter( ), 0.5f ) );
				mWaveLaunchMenu->fFadeIn( );

				mWaveLaunchMenu->fCanvas( ).fCanvas( )->fSetInvisible( fUser( )->fViewport( )->fIsVirtual( ) );
			}
		}
		else
		{
			if( mWaveLaunchMenu )
			{
				mWaveLaunchMenu->fFadeOut( );
				mWaveLaunchMenu.fRelease( );
			}
		}
	}
	
	tEffectPlayer::tEffectData tPlayer::fGetEffectsData( tEntity* owner ) const 
	{ 
		if( !fCameraStack( ).fCount( ) )
			return tEffectData( );
		else
		{
			Gfx::tCameraController* cam = fCameraStackTop( ).fGetRawPtr( );
			b32 useUnitPos = (mCurrentUnit && cam->fDynamicCast<tShellCamera>( ) == NULL);

			return tEffectData( cam			
				, &( fUser( )->fFilteredGamepad( 0 ) )
				, useUnitPos ? mCurrentUnit->fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( ) : cam->fViewport( )->fLogicCamera( ).fLocalToWorld( ).fGetTranslation( ) 
				, (mCurrentUnit && mCurrentUnit->fOwnerEntity( ) == owner) );
		}
	}

	void tPlayer::fSetComboUI( u32 combo )
	{
		if( mPowerPool )
			mPowerPool->fSetCombo( combo );
	}

	void tPlayer::fBarrageHasEnded( )
	{
		if( mPowerPool )
			mPowerPool->fShowBarrage( false );
	}

	u32 tPlayer::fGoldenArcadeFoundCount( ) const
	{
		u32 count = 0;
		u32 levelCount = tGameApp::fInstance( ).fNumLevelsInTable( GameFlags::cMAP_TYPE_CAMPAIGN );

		for( u32 i = 0; i < levelCount; ++i )
		{
			const tLevelLoadInfo& info = tGameApp::fInstance( ).fLevelLoadInfo( GameFlags::cMAP_TYPE_CAMPAIGN, i );
			if( info.mDlcNumber != GameFlags::cDLC_COLD_WAR )
				continue;

			tLevelScores* scores = mProfile->fGetLevelScores( GameFlags::cMAP_TYPE_CAMPAIGN, i );
			sigassert( scores );
			if( scores->fHasFoundGoldenArcade( ) )
				++count;
		}

		return count;
	}

	b32 tPlayer::fFoundAllGoldenBabushkas() const
	{
		u32 levelCount = tGameApp::fInstance( ).fNumLevelsInTable( GameFlags::cMAP_TYPE_CAMPAIGN );

		for( u32 i = 0; i < levelCount; ++i )
		{
			const tLevelLoadInfo& info = tGameApp::fInstance( ).fLevelLoadInfo( GameFlags::cMAP_TYPE_CAMPAIGN, i );
			if( info.mDlcNumber != GameFlags::cDLC_EVIL_EMPIRE )
				continue;

			tLevelScores* scores = mProfile->fGetLevelScores( GameFlags::cMAP_TYPE_CAMPAIGN, i );
			sigassert( scores );
			if( !scores->fHasFoundAllGoldenBabushkas( ) )
				return false;
		}

		return true;
	}

	b32 tPlayer::fFoundAllGoldenDogTags() const
	{
		u32 levelCount = tGameApp::fInstance( ).fNumLevelsInTable( GameFlags::cMAP_TYPE_CAMPAIGN );

		for( u32 i = 0; i < levelCount; ++i )
		{
			const tLevelLoadInfo& info = tGameApp::fInstance( ).fLevelLoadInfo( GameFlags::cMAP_TYPE_CAMPAIGN, i );
			if( info.mDlcNumber != GameFlags::cDLC_NAPALM )
				continue;

			tLevelScores* scores = mProfile->fGetLevelScores( GameFlags::cMAP_TYPE_CAMPAIGN, i );
			sigassert( scores );
			if( !scores->fHasFoundAllGoldenDogTags( ) )
				return false;
		}

		return true;
	}

	void tPlayer::fAddEarnedItem( const tEarnedItemData& data )
	{
		// dont add duplicates
		mEarnedItems.fFindOrAdd( data );
	}

	Sqrat::Object tPlayer::fEarnedItemTable( ) const
	{
		Sqrat::Table out;
		for( u32 i = 0; i < mEarnedItems.fCount( ); ++i )
		{
			out.SetValue( StringUtil::fToString( i ).c_str( ), mEarnedItems[ i ] );
		}
		return out;
	}

	b32 tPlayer::fDebugRandyMode( )
	{
		return Debug_RandyMode;
	}

	b32 tPlayer::fNeedsToChooseSaveDevice( )
	{
		return mDeviceSelector.fNeedsToChooseSaveDevice( fProfile( )->fSaveDeviceId( ) );
	}

	b32 tPlayer::fDidChooseSaveDeviceId( )
	{
		return mDeviceSelector.fDeviceSelected( );
	}

	void tPlayer::fChooseSaveDeviceId( b32 forceShow )
	{
		mDoesntWantRewind = false;
		mDeviceSelector.fChooseSaveDeviceId( *mUser, forceShow );
	}

	void tPlayer::fOnStorageDeviceChanged( )
	{
		if( mUser->fSignedIn( ) && mUser->fIsLocal( ) )
		{
			if( !mUser->fCanWriteToProfile( ) )
			{
				tGameApp::fInstance( ).fShowErrorDialog( tGameApp::cProfileDeviceRemoved, fUser( ).fGetRawPtr( ) );
			}
			else if( mDeviceSelector.fSaveDeviceId( ) && !mDeviceSelector.fDeviceIsAvailable( ) )
			{
				if( !mShowRemovedOncePerLevel )
				{
					mDeviceSelector.fReset( );
					mShowRemovedOncePerLevel = true;
					tGameApp::fInstance( ).fShowErrorDialog( tGameApp::cRewindListDeviceRemoved, fUser( ).fGetRawPtr( ) );
				}
			}
		}
	}

	void tPlayer::fDeviceSelectorLogic( )
	{
		mDeviceSelector.fTick( );
		if( mDeviceSelector.mSelectionJustCompleted )
		{
			mDeviceSelector.mSelectionJustCompleted = false;
			fOnStorageDeviceSelected( );
		}
	}

	void tPlayer::fOnStorageDeviceSelected( )
	{
		if( !mStorageDeviceSelectionCallback.IsNull( ) )
			mStorageDeviceSelectionCallback.Execute( mDeviceSelector.fDeviceSelected( ) ? true : false );

		if( mDeviceSelector.fDeviceSelected( ) )
		{
			if( fUser( )->fIsLocal( ) && fProfile( ) )
			{
				// user selected a device
				fProfile( )->fSaveDeviceId( ) = mDeviceSelector.fSaveDeviceId( );
				mShowRemovedOncePerLevel = false;
				fSaveProfile( );
			}
		}
		else
		{
			mDeviceSelector.fReset( );
			tGameApp::fInstance( ).fShowErrorDialog( tGameApp::cNoSaveDevice, fUser( ).fGetRawPtr( ) );
			mDoesntWantRewind = true;
		}
	}

	const tGameControllerPtr& tPlayer::fGameController( ) const
	{
		return mGameController;
	}
}

namespace Sig
{
	namespace
	{
		static b32 fIsValid( tPlayer* player )
		{
			return player && player->fIsActive( );
		}

		static Sqrat::Object fCurrentBarrageForScript( tPlayer* player )
		{
			return player->fCurrentBarrage( ).fScriptObject( );
		}

		static b32 fDebugEnableMinimap( )
		{
			return Debug_EnableMinimap;
		}

		static void fShowFirstPlayMessages( tPlayer* player )
		{
			// title update: this happens right after a user presses start., throw up the title update warning message if it has never been shown.
#if defined( platform_xbox360 )
			{
				if( !player->fProfile( )->fSetting( tUserProfile::cSettingShownTUWarning ) )
				{
					tGameApp::fInstance( ).fShowErrorDialog( tGameApp::cTitleUpdateWarning, player->fUser( ).fGetRawPtr( ) );
					player->fProfile( )->fSetSetting( tUserProfile::cSettingShownTUWarning, true );
				}
			}
#endif
		}
	}
	void tPlayer::fExportScriptInterface( tScriptVm& vm )
	{
		{
			Sqrat::Class<tEarnedItemData, Sqrat::DefaultAllocator<tEarnedItemData> > classDesc( vm.fSq( ) );
			classDesc
				.Var(_SC("Type"),		&tEarnedItemData::mType)
				.Var(_SC("Value"),		&tEarnedItemData::mValue)
				.Var(_SC("Deferred"),	&tEarnedItemData::mDeferred)
				;

			vm.fRootTable( ).Bind(_SC("EarnedItemData"), classDesc);

			vm.fConstTable( ).Const( _SC("EARNED_ITEM_TYPE_DECORATION"),	(int)tEarnedItemData::cDecoration );
			vm.fConstTable( ).Const( _SC("EARNED_ITEM_TYPE_RANK"),			(int)tEarnedItemData::cRank );
			vm.fConstTable( ).Const( _SC("EARNED_ITEM_TYPE_ACHIEVEMENT"),	(int)tEarnedItemData::cAchievement );
			vm.fConstTable( ).Const( _SC("EARNED_ITEM_TYPE_AVATAR_AWARD"),	(int)tEarnedItemData::cAvatarAward );
			vm.fConstTable( ).Const( _SC("EARNED_ITEM_TYPE_GOLDEN_ARCADE"),	(int)tEarnedItemData::cGoldenArcade );
			vm.fConstTable( ).Const( _SC("EARNED_ITEM_TYPE_JET_PACK"),		(int)tEarnedItemData::cJetPack );
		}

		Sqrat::Class<tPlayer, Sqrat::NoConstructor> classDesc( vm.fSq( ) );

		classDesc
			.StaticFunc(_SC("IsValid"),			&fIsValid)
			.StaticFunc(_SC("EnemyTeam"),		&tPlayer::fDefaultEnemyTeam)
			.StaticFunc(_SC("EnemyIsApc"),		&tPlayer::fIsAPC)
			.Prop(_SC("User"),					&tPlayer::fUserFromScript)
			.Prop( _SC( "GameController" ),		&tPlayer::fGameControllerFromScript )
			.Prop(_SC("PlayerIndex"),			&tPlayer::fPlayerIndex)
			//.Func(_SC("GetFilteredGamepad"),	&tPlayer::fFilteredGamepad)
			.Func(_SC("ComputeViewportRect"),	&tPlayer::fViewportRect)
			.Func(_SC("ComputeViewportSafeRect"), &tPlayer::fViewportSafeRect)
			.Func(_SC("ClearCameraStack"),		&tPlayer::fClearCameraStack)
			.Func(_SC("PushFreeCamera"),		&tPlayer::fPushFreeCamera)
			.Func(_SC("PushFrontEndCamera"),	&tPlayer::fPushFrontEndCamera)
			.Func(_SC("PushRTSCamera"),			&tPlayer::fPushRTSCamera)
			.Func(_SC("AddBarrage"),			&tPlayer::fAddBarrage)
			.Func(_SC("UnitLocked"),			&tPlayer::fUnitLocked)
			.Prop(_SC("Country"),				&tPlayer::fCountry)
			.Prop(_SC("Team"),					&tPlayer::fTeam)
			.Func(_SC("LockInUnit"),			&tPlayer::fLockInUnit)
			.Func(_SC("LockInUnitDirect"),		&tPlayer::fLockInUnitDirect)
			.Func(_SC("LockInCurrentUnit"),		&tPlayer::fLockInCurrentUnit)
			.Func(_SC("UnlockFromUnit"),		&tPlayer::fUnlockFromUnit)
			.Func(_SC("SpawnCharacter"),		&tPlayer::fSpawnCharacter)
			.Func(_SC("GetUserProfile"),		&tPlayer::fProfileFromScript)
			.Func(_SC("ShowScoreUI"),			&tPlayer::fShowScoreUI)
			.Prop(_SC("CurrentUnit"),			&tPlayer::fCurrentUnitScript)
			.Func(_SC("ShowMoney"),				&tPlayer::fShowMoney)
			.Func(_SC("ShowTickets"),			&tPlayer::fShowTickets)
			.Func(_SC("ShowBarrageIndicator"),	&tPlayer::fShowBarrageIndicator)
			.Func(_SC("ShowOverchargeMeter"),	&tPlayer::fShowOverchargeMeter)
			.Func(_SC("ShowMiniMap"),			&tPlayer::fShowMiniMap)
			.Prop(_SC("Stats"),					&tPlayer::fStatsScript)
			.Func(_SC("GetUnlockedUnitIDs"),	&tPlayer::fCollectUnlockedUnitIDs)
			.Func(_SC("ResetProfile"),			&tPlayer::fResetProfile)
			.Func(_SC("SaveProfile"),			&tPlayer::fSaveProfile)
			.Func(_SC("BuildTurret"),			&tPlayer::fBuildTurret)
			.Func(_SC("BeginCameraPath"),		&tPlayer::fBeginCameraPath)
			.Func(_SC("RestrictBarrage"),		&tPlayer::fRestrictBarrage)
			.Func(_SC("GiveBarrage"),			&tPlayer::fGiveBarrage)
			.Func(_SC("SetBarrageSkip"),		&tPlayer::fSetBarrageSkip)
			.Func(_SC("GiveInstantBarrage"),	&tPlayer::fGiveInstantBarrage)
			.Func(_SC("ForceUseBarrage"),		&tPlayer::fForceUseBarrage)
			.Func(_SC("SetUniqueBarrages"),		&tPlayer::fSetUniqueBarrages)
			
			.Func(_SC("GiveOverCharge"),		&tPlayer::fGiveOverCharge)
			.Func(_SC("CancelOverCharge"),		&tPlayer::fCancelOvercharge)
			.Func(_SC("PushTiltShiftCamera"),	&tPlayer::fPushTiltShiftCamera)
			.Func(_SC("PopTiltShiftCamera"),	&tPlayer::fPopTiltShiftCamera)
			.Func(_SC("GetDataForDisplayCase"),	&tPlayer::fGetDataForDisplayCase)
			.Var(_SC("LastKillValue"),			&tPlayer::mLastKillValue)
			.Prop(_SC("DisableTiltShift"),		&tPlayer::fDisableTiltShift, &tPlayer::fSetDisableTiltShift)
			.Func(_SC("ResetAmmo"),				&tPlayer::fResetAmmo)
			.Var(_SC("LevelScoreAndStats"),		&tPlayer::mLevelScoreAndStats)
			.Prop(_SC("CurrentLevelHighScore"),	&tPlayer::fCurrentLevelHighScore)
			.Func(_SC("SetFullScreenOverlayActive"),&tPlayer::fSetFullScreenOverlayActive)
			.Func< Gui::tWorldSpaceFloatingText* (tPlayer::*)( const char*, const Math::tVec3f&, const Math::tVec4f&, f32, f32, b32 ) >(_SC("SpawnText"), &tPlayer::fSpawnText)
			.Func(_SC("SpawnLocText"),			&tPlayer::fSpawnLocText)
			.Func(_SC("HasDLC"),				&tPlayer::fHasDLC)
			.Func(_SC("InstalledDLC"),			&tPlayer::fInstalledDLC)
			.Func(_SC("AllowDPadSwitch"),		&tPlayer::fAllowDPadSwitch)
			.Func(_SC("ApplyProfileSettings"),	&tPlayer::fApplyProfileSettings)
			.Func(_SC("SpawnIndicatorArrow"),	&tPlayer::fSpawnIndicatorArrow)
			.Prop(_SC("QuickSwitchDisabled"),	&tPlayer::fQuickSwitchDisabled)
			.Var(_SC("SelectedUnitChangedCallback"), &tPlayer::mSelectedUnitChangedCallback)
			.Var( _SC( "CurrentUnitChangedCallback" ), &tPlayer::mCurrentUnitChangedCallback )
			.Prop(_SC("Money"),					&tPlayer::fInGameMoney)
			.Prop(_SC("Tickets"),				&tPlayer::fTicketsLeft)
			.Prop(_SC("BeatLevelTheFirstTime"),	&tPlayer::fBeatLevelTheFirstTime)
			.GlobalFunc(_SC("CurrentBarrage"),	&fCurrentBarrageForScript)
			.Prop(_SC("LevelScores"),			&tPlayer::fLevelScores)
			.Prop(_SC("TotalGoldenArcadeCount"), &tPlayer::fTotalGoldenArcadeCount)
			.Prop(_SC("GoldenArcadeFoundCount"), &tPlayer::fGoldenArcadeFoundCount)
			.Prop(_SC("FoundAllGoldenBabushkas"), &tPlayer::fFoundAllGoldenBabushkas)
			.Prop(_SC("FoundAllGoldenDogTags"), &tPlayer::fFoundAllGoldenDogTags)
			.StaticFunc(_SC("IsMachineGun"),	&tPlayer::fIsMachineGun)
			.StaticFunc(_SC("IsArtillery"),		&tPlayer::fIsArtillery)
			.StaticFunc(_SC("IsMortar"),		&tPlayer::fIsMortar)
			.StaticFunc(_SC("IsAntiTank"),		&tPlayer::fIsAntiTank)
			.StaticFunc(_SC("IsMakeshift"),		&tPlayer::fIsMakeshift)
			.StaticFunc(_SC("IsAntiAir"),		&tPlayer::fIsAntiAir)
			.Func(_SC("AwardAchievement"),		&tPlayer::fAwardAchievement)
			.Func(_SC("AwardAvatarAward"),		&tPlayer::fAwardAvatarAward)
			.Func(_SC("AddEarnedItem"),			&tPlayer::fAddEarnedItem)
			.Prop(_SC("EarnedItems"),			&tPlayer::fEarnedItemTable)
			.Func(_SC("GetAchievementData"),	&tPlayer::fGetAchievementData)
			.Func(_SC("IncrementLocationalStat"),&tPlayer::fIncrementLocationalStat)
			.Func(_SC("IncrementStarShatter"), &tPlayer::fIncrementStarShatter)
			.Prop(_SC("HudLayerName"),			&tPlayer::fHudLayerName)
			.Func(_SC("AudioEvent"),			&tPlayer::fScriptAudioEvent)
			.Prop(_SC("AudioSource"),			&tPlayer::fScriptSoundSource)
			.Func(_SC("SubtractTickets"),		&tPlayer::fSubtractTickets)
			.Func(_SC("ChooseSaveDeviceId"),	&tPlayer::fChooseSaveDeviceId)
			.Prop(_SC("NeedsToChooseSaveDevice"),&tPlayer::fNeedsToChooseSaveDevice)
			.Prop(_SC("DidChooseSaveDeviceId"),	&tPlayer::fDidChooseSaveDeviceId)
			.StaticFunc(_SC("RandyMode"),		&fDebugRandyMode)
			.StaticFunc(_SC("DebugEnableMinimap"), &fDebugEnableMinimap)
			.Prop(_SC("IsNotAllowedToSaveStats"),&tPlayer::fIsNotAllowedToSaveStats)
			.Var(_SC("StorageDeviceSelectionCallback"),&tPlayer::mStorageDeviceSelectionCallback)
			.Var(_SC("DoesntWantRewind"),		&tPlayer::mDoesntWantRewind)
			.Func( _SC("CloseRadialMenus"),		&tPlayer::fCloseRadialMenus)
			.Func( _SC("OnMatchEnded"),			&tPlayer::fOnMatchEnded)
			.GlobalFunc(_SC("ShowFirstPlayMessages"),	&fShowFirstPlayMessages)
			.Prop(_SC("TimeSinceUnitExit"),	&tPlayer::fTimeSinceUnitExit)
			;

		vm.fRootTable( ).Bind( _SC("Player"), classDesc );
	}

}

