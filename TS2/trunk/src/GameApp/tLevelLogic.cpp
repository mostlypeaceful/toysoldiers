#include "GameAppPch.hpp"
#include "tLevelLogic.hpp"
#include "tGameApp.hpp"
#include "tWaypointLogic.hpp"
#include "tSceneRefEntity.hpp"
#include "tTransitionCamera.hpp"
#include "tUseUnitCamera.hpp"
#include "tTurretLogic.hpp"
#include "tRtsCamera.hpp"
#include "tSaveGame.hpp"
#include "tSaveGameStorage.hpp"
#include "tGoalBoxLogic.hpp"
#include "tBuildSiteLogic.hpp"
#include "tRationTicketUI.hpp"
#include "tGameEffects.hpp"
#include "GameSession.hpp"

#include "Wwise_IDs.h"

// Extra stuff
#include "tCharacterLogic.hpp"

using namespace Sig::Math;

namespace Sig
{
#ifdef sig_devmenu
	void fAAACheatsWin( tDevCallback::tArgs& args )
	{
		tGameApp& gameApp = tGameApp::fInstance( );
		tLevelLogic* level = gameApp.fCurrentLevel( );

		if( !level )
			return;
		tPlayer* player = gameApp.fGetPlayer( gameApp.fWhichPlayer( args.mUser ) );
		if( !player )
			return;

		level->fSetVictoriousPlayer( player );

		Sqrat::Function f( level->fOwnerEntity( )->fScriptLogicObject( ), "GameEnded" );
		if( !f.IsNull( ) )
			f.Execute( );
	}
	void fAAACheatsLose( tDevCallback::tArgs& args )
	{
		tGameApp& gameApp = tGameApp::fInstance( );
		tLevelLogic* level = gameApp.fCurrentLevel( );

		if( !level )
			return;
		tPlayer* player = gameApp.fGetPlayer( gameApp.fWhichPlayer( args.mUser ) );
		if( !player )
			return;

		level->fSetDefeatedPlayer( player );

		Sqrat::Function f( level->fOwnerEntity( )->fScriptLogicObject( ), "GameEnded" );
		if( !f.IsNull( ) )
			f.Execute( );
	}
#endif //sig_devmenu
	devcb( AAACheats_Win, "Auto-victory", make_delegate_cfn( tDevCallback::tFunction, fAAACheatsWin ) );
	devcb( AAACheats_Lose, "Auto-defeat", make_delegate_cfn( tDevCallback::tFunction, fAAACheatsLose ) );
	devvar( bool, AAACheats_Tutorial_AllowSkip, false );

	devvar( bool, Debug_Paths_Render, false );
	devrgba( Debug_Paths_SphereColor, tVec4f( 1.00f, 0.54f, 0.24f, 0.1f ) );
	devrgba( Debug_Paths_LineColor, tVec4f( 0.24f, 0.70f, 1.00f, 0.5f ) );
	devrgba( Debug_Paths_InaccessibleColor, tVec4f( 1.0f, 0.1f, 0.1f, 0.1f ) );

	devvar( f32, Debug_Audio_NeutralSwitch, 60.f );

	devvar( f32, Gameplay_Waves_EndGameStopTimer, 60.f );
	devvar( f32, Gameplay_Audio_NukeStateDuration, 5.f );
	devvar( f32, Renderer_PostEffects_Nuke_ExposureMax, 1.1f );

	tLevelLogic::tLevelLogic( )
		: mMaxLevelDimension( 1.f )
		, mLevelProgression( cLevelProgressionPlaying )
		, mMatchEnded( false )
		, mSlowMoUnits( false )
		, mWaypointsProcessed( false )
		, mGroundHeight( 0.f )
		, mWaterLevel( 0.f )
		, mOnlyPlaceThisUnit( ~0 )
		, mOnlyPlaceSmallUnits( false )
		, mDisableSell( false )
		, mDisableRepair( false )
		, mDisableUpgrade( false )
		, mDisableUse( false )
		, mTurretOptionsChanged( 0 )
		, mFreeUpgrades( false )
		, mHideKillHoverText( false )
		, mDisableBarrage( false )
		, mDisableVehicleRespawn( false )
		, mEnablePlatformLocking( false )
		, mDisableLaunchArrows( false )
		, mLockPlaceMenuUpUntilBuild( false )
		, mDisableRewind( false )
		, mAllowCoopTurrets( false )
		, mDisablePlaceMenu( false )
		, mDisableVehicleInput( false )
		, mDisableOvercharge( false )
		, mForceWeaponOvercharge( false )
		, mSaveFirstWave( false )
		, mDisableRandomPickups( false )
		, mContinuousCombo( false )
		, mDisablePropMoney( false )
		, mDisableOverkills( false )
		, mUseMinigameScoring( false )
		, mDisableQuickSwitch( false )
		, mIsDisplayCase( false )
		, mLightUpOnlyTutorialPlatforms( false )
		, mAlwaysShowUnitControls( false )
		, mDisableBonusHoverText( false )
		, mDisableComboText( false )
		, mAllowSpeedBonus( false )
		, mDisableRTSRings( false )
		, mAllyLaunchArrows( false )
		, mPlatformPrice( 20.f )
		, mSurvivalLevelTime( 0.f )
		, mUnitTimeMultiplier( 1.f )
		, mMapType( ~0 )
		, mLevelNumber( ~0 )
		, mPreviousGlow( ~0 )
		, mDlcNumber( ~0 )
		, mExtraMode( false )
		, mNukeMixNeutralTimer( -1.f )
		, mNeutralTimer( -1.f )
		, mNeutralTimerTime( -1.f )
		, mTutorialSuspended( false )
		, mMiniGameTime( 0.0f )
		, mMiniGameCurrentScore( 0.0f )
		, mCoOpAssists( 0 )
		, mRewindCount( 0 )
		, mStopWavesTimer( -1.f )
		, mBackDropCamera( NULL )
		, mDisplayCaseCountry( GameFlags::cCOUNTRY_USA )
		, mWaitOneFrameToSwitchCountry( 1 ) // needs to be one so it's decremented to zero immediately
		, mCameraAngle( 0.f )
		, mWorldAngleRate( 0.f )
		, mWorldAngle( 0 )
		, mOriginalDir( tVec3f::cZAxis )
		, mPivotToSpawn( tMat3f::cIdentity )
	{
		// We might only need this for SP
		if( tGameApp::fInstance( ).fHasWaveListTable( ) )
			mWaveManager.fReset( NEW tWaveManager( ) );

		if( tGameApp::fInstance( ).fGameMode( ).fIsVersus( ) )
		{
			mVersusWaveManager.fReset( NEW tVersusWaveManager( ) );
		}

		const tLevelLoadInfo& info = tGameApp::fInstance( ).fCurrentLevelLoadInfo( );
		mLevelNumber = info.mLevelIndex;
		mMapType = info.mMapType;
		mDlcNumber = info.mDlcNumber;
		
		sync_event_v_c( mLevelNumber, tSync::cSCLevel );
		sync_event_v_c( mMapType, tSync::cSCLevel );
		sync_event_v_c( mDlcNumber, tSync::cSCLevel );
		
		mScreenSpaceHealthBarList.fReset( 
			NEW Gui::tScreenSpaceHealthBarList( 
				tGameApp::fInstance( ).fGlobalScriptResource( tGameApp::cGlobalScriptScreenSpaceHealthBarList ), 
				tGameApp::fInstance( ).fGetPlayer( 0 )->fUser( ) ) );

		if( mMapType == GameFlags::cMAP_TYPE_MINIGAME )
			mAllowSpeedBonus = true;
	}
	tLevelLogic::~tLevelLogic( )
	{
		mRootMenu.fRemoveFromParent( );
	}
	void tLevelLogic::fOnSpawn( )
	{
		fOnPause( false );

		fComputeLevelBounds( );

		tLogic::fOnSpawn( );

		tGoalDriven::fOnSpawn( this );

		Sqrat::Function f( fOwnerEntity( )->fScriptLogicObject( ), "SetAtmosphere" );
		if( !f.IsNull( ) )
			f.Execute( );

		const tLevelLoadInfo& info = tGameApp::fInstance( ).fCurrentLevelLoadInfo( );
		const tDynamicArray<tPlayerPtr>& players = tGameApp::fInstance( ).fPlayers( );
		for( u32 i = 0; i < players.fCount( ); ++i )
		{
			players[ i ]->fLevelOnSpawn( );
			players[ i ]->fSetInGameMoney( info.mMoney );
		}
	}
	void tLevelLogic::fOnDelete( )
	{
		Logic::tGoalDriven::fOnDelete( this );

		mSaveableEntsFromLevel.fSetCount( 0 );
		mSaveableEntsDynamic.fSetCount( 0 );
		mCameraBoxes.fFill( tShapeEntityPtr( ) );
		mCameraPoints.fSetCount( 0 );
		mCameraBlockers.fSetCount( 0 );
		mCameraGrounds.fSetCount( 0 );
		mBuildSitesSmall.fSetCount( 0 );
		mBuildSitesLarge.fSetCount( 0 );
		mBuildSiteRootsSmall.fSetCount( 0 );
		mBuildSiteRootsLarge.fSetCount( 0 );
		mPathStarts.fSetCount( 0 );
		mTrenchStarts.fSetCount( 0 );
		mTrenchLengths.fSetCount( 0 );
		mExitGenStarts.fSetCount( 0 );
		mContextPathStarts.fSetCount( 0 );
		mFlightPathStarts.fSetCount( 0 );
		mNamedWaypoints.fSetCount( 0 );
		mGoalBoxes.fSetCount( 0 );
		mGenerators.fSetCount( 0 );
		mUseableUnits.fSetCount( 0 );
		mSpecialLevelObjects.fSetCount( 0 );

		mWaveManager.fRelease( );
		mVersusWaveManager.fRelease( );		
		mRationTicketUI.fRelease( );

		mBackDropCamera = NULL;

		tLogic::fOnDelete( );
	}
	void tLevelLogic::fOnPause( b32 paused )
	{
		if( paused )
		{
			fRunListRemove( cRunListActST );
			//fRunListRemove( cRunListThinkST ); // THINK RUNS ALL THE TIME EVEN WHEN THE GAME IS PAUSED
			fRunListRemove( cRunListCoRenderMT );
		}
		else
		{
			// add self to run lists
			fRunListInsert( cRunListActST );
			fRunListInsert( cRunListThinkST );
			fRunListInsert( cRunListCoRenderMT );
		}
	}
	void tLevelLogic::fActST( f32 dt )
	{
		tLogic::fActST( dt );

		if( mWaveManager )
			mWaveManager->fUpdate( dt );

		if( mVersusWaveManager )
			mVersusWaveManager->fUpdate( dt );

		mSurvivalLevelTime += dt;

		if( mNeutralTimer > 0.f )
		{
			mNeutralTimer -= dt;
			if( mNeutralTimer <= 0.f )
				fSetCurrentLevelProgression( cLevelProgressionPlaying );
		}

		if( mNukeMixNeutralTimer > 0.f )
		{
			f32 extraExposureMult = 0.f;

			mNukeMixNeutralTimer -= dt;
			if( mNukeMixNeutralTimer <= 0.f )
			{
				if( !tGameApp::fInstance( ).fGameMode( ).fIsFrontEnd( ) )
					tGameApp::fInstance( ).fSoundSystem( )->fSetState( AK::STATES::GLOBAL_MIX_STATES::GROUP, AK::STATES::GLOBAL_MIX_STATES::STATE::NEUTRAL );
			}
			else
			{
				// TODO make this curve fancier and maybe make it taper by distance to player.
				f32 x = mNukeMixNeutralTimer / Gameplay_Audio_NukeStateDuration;
				x = x;	// rgaule  will be working here shortly!
				extraExposureMult = fLerp( 0.f, (f32)Renderer_PostEffects_Nuke_ExposureMax, x );
			}

			tDynamicArray<tPlayerPtr>& players = tGameApp::fInstance( ).fPlayers( );
			for( u32 i = 0; i < players.fCount( ); ++i )
				tGameApp::fInstance( ).fPostEffectsManager( )->fSetGlobalExposureMult( players[ i ]->fUser( )->fViewportIndex( ), 1.f + extraExposureMult );
		}

		if( mTurretOptionsChanged > 0 )
			--mTurretOptionsChanged;

		Logic::tGoalDriven::fActST( this, dt );
	}
	void tLevelLogic::fThinkST( f32 dt )
	{
		// THINK RUNS ALL THE TIME EVEN WHEN THE GAME IS PAUSED
		if( Debug_Paths_Render )
		{
			for( u32 i = 0; i < mPathStarts.fCount( ); ++i )
				fRenderPath( mPathStarts[ i ].fGetRawPtr( ) );
			for( u32 i = 0; i < mExitGenStarts.fCount( ); ++i )
				fRenderPath( mExitGenStarts[ i ].fGetRawPtr( ) );
			for( u32 i = 0; i < mTrenchStarts.fCount( ); ++i )
				fRenderPath( mTrenchStarts[ i ].fGetRawPtr( ) );
		}

		if( mBackDropCamera )
		{
			f32 desiredWorldAngle = mWorldAngleRate * mWorldAngle;

			// rectify the value so we end up going the shortest way.
			f32 shortestDelta = fShortestWayAround( mAngleDamper.fValue( ), desiredWorldAngle );
			mAngleDamper.fSetValue( desiredWorldAngle - shortestDelta );
			mAngleDamper.fStep( desiredWorldAngle, dt );

			if( mBackDropCamera->fIsActive( ) )
			{
				// show case mode
				tPlayer* player = tGameApp::fInstance( ).fFrontEndPlayer( );
				sigassert( player );

				fIncCameraAngle( -player->fAimStick( tUserProfile::cProfileCamera ).x * cPiOver4 * dt );

				const Input::tGamepad& gp = player->fGamepad( );
				b32 pushed = gp.fButtonDown( player->fProfile( )->fSouthPaw( tUserProfile::cProfileCamera ) ? Input::tGamepad::cButtonRThumbMinMag : Input::tGamepad::cButtonLThumbMinMag );
				if( pushed )
				{
					f32 moveStick = -player->fMoveStick( tUserProfile::cProfileCamera ).x;
					tVec3f camToOrigin = -mBackDropCamera->fCursorPosition( );
					camToOrigin.fNormalizeSafe( tVec3f::cZAxis );
					tVec3f camDir = mBackDropCamera->fCursorDir( );
					camDir *= moveStick;
					f32 sign = fSign( camToOrigin.fDot( camDir ) );				
					fIncWorldAngle( sign );
				}
			}

			fApplyAngles( );
		}

		if( mStopWavesTimer >= 0.f )
		{
			mStopWavesTimer -= dt;
			if( mStopWavesTimer < 0.f )
			{
				mWaveManager->fStopAllWaves( );
			}
		}

		// need to call this from the level, for networking syncing
		tGameApp::fInstance( ).fStepTimedCallbacks( dt );
	}
	void tLevelLogic::fCoRenderMT( f32 dt )
	{
		tLogic::fCoRenderMT( dt );
	}

	b32 tLevelLogic::fGatherEntitySaveData( tSaveGameData& data, tSaveGameRewindPreview& preview )
	{
		sigassert( mWaveManager );

		if( mMapType != GameFlags::cMAP_TYPE_HEADTOHEAD )
		{
			tWaveList *currentWave = mWaveManager->fDisplayedWaveList( );

			if( !currentWave )
			{
				log_warning( 0, "Tried to save game when no wave was active?" );
				return false;
			}
		}

		data.mLevelData.mRewindCount = mRewindCount;

		if( mMapType != GameFlags::cMAP_TYPE_HEADTOHEAD )
		{
			data.mWaveLists.fSetCapacity( mWaveManager->fWaveListCount( ) );
			for( u32 i = 0; i < mWaveManager->fWaveListCount( ); ++i )
			{
				tWaveList *wave = mWaveManager->fWaveList( i );
				if( wave->fSaveable( ) )
					data.mWaveLists.fPushBack( tSaveGameData::tWaveID( wave->fName( ), wave->fTotalWaveID( ), wave->fCurrentUIWaveListID( ), wave->fIsActive( ), wave->fCountry( ), wave->fIsLooping( ) ) );
			}
		}

		if( mMapType == GameFlags::cMAP_TYPE_HEADTOHEAD )
		{
			for( u32 i = 0; i < mVersusWaveManager->fWaveListCount( ); ++i )
			{
				tWaveList* waveList = mVersusWaveManager->fWaveList( i );
				const u32 country = waveList->fCountry( );
				for( u32 w = 0; w < waveList->fWaveCount( ); ++w )
				{
					const tStringPtr& descWaveID = mVersusWaveManager->fGetDescWaveID( w, country );
					log_line( 0, "Saving:" << descWaveID.fCStr( ) << "(" << country << ")" );
					data.mOffensiveWaveLists.fPushBack( tSaveGameData::tWaveID( descWaveID, 0, 0, fRoundUp< s32 >( waveList->fTimeRemaining( ) ), country ) );
				}
			}
		}
		else if( mMapType == GameFlags::cMAP_TYPE_SURVIVAL )
		{
			data.mLevelData.mPlatformPrice = fPlatformPrice( );
		}

		//{	
		//	log_line( 0, "Wave status: " );
		//	for( u32 i = 0; i < data.mWaveLists.fCount( ); ++i )
		//	{
		//		log_line( 0, " " << data.mWaveLists[ i ].mWaveName << " active: " << data.mWaveLists[ i ].mActive );
		//	}
		//}

		for( u32 i = 0; i < mSaveableEntsFromLevel.fCount( ); ++i )
		{
			tEntity* ent = mSaveableEntsFromLevel[ i ].fGetRawPtr( );
			tUnitLogic* unitLogic = ent->fLogicDerived< tUnitLogic >( );
			sigassert( unitLogic );

			tRefCounterPtr<tEntitySaveData> entData = unitLogic->fStoreSaveGameData( true, preview );
			sigassert( entData );

			data.mLevelData.mEntsFromLevel.fPushBack( entData );				
		}

		for( u32 i = 0; i < mSaveableEntsDynamic.fCount( ); ++i )
		{
			if( !mSaveableEntsDynamic[ i ]->fSceneGraph( ) )
				continue; // entity is no longer in scene graph, no need to store anything

			tEntity* ent = mSaveableEntsDynamic[ i ].fGetRawPtr( );
			tUnitLogic* unitLogic = ent->fLogicDerived< tUnitLogic >( );
			sigassert( unitLogic );

			tRefCounterPtr<tEntitySaveData> entData = unitLogic->fStoreSaveGameData( true, preview );
			sigassert( entData );

			data.mLevelData.mEntsDynamic.fPushBack( entData );
		}

		return true;
	}
	tSaveGameRewindPreview* tLevelLogic::fGetAllTurretData( )
	{
		mTurretData.fReset( NEW tSaveGameRewindPreview( ) );
		for( u32 i = 0; i < mSaveableEntsFromLevel.fCount( ); ++i )
		{
			tEntity* ent = mSaveableEntsFromLevel[ i ].fGetRawPtr( );
			tUnitLogic* turretLogic = ent->fLogicDerived< tTurretLogic >( );
			if( turretLogic )
				turretLogic->fStoreSaveGameData( true, *mTurretData.fGetRawPtr( ) );
		}
		for( u32 i = 0; i < mSaveableEntsDynamic.fCount( ); ++i )
		{
			if( !mSaveableEntsDynamic[ i ]->fSceneGraph( ) )
				continue;
			tEntity* ent = mSaveableEntsDynamic[ i ].fGetRawPtr( );
			tUnitLogic* turretLogic = ent->fLogicDerived< tTurretLogic >( );
			if( turretLogic )
				turretLogic->fStoreSaveGameData( true, *mTurretData.fGetRawPtr( ) );
		}
		return mTurretData.fGetRawPtr( );
	}
	b32 tLevelLogic::fAllowTutorialDebugSkip( ) const
	{
		return AAACheats_Tutorial_AllowSkip;
	}
	void tLevelLogic::fSetDisableRewind( b32 set ) 
	{ 
		mDisableRewind = set;
	}
	void tLevelLogic::fNukeGloablAudioState( )
	{
		mNukeMixNeutralTimer = Gameplay_Audio_NukeStateDuration;
		if( tGameApp::fInstance( ).fGameMode( ).fIsFrontEnd( ) )
			mNukeMixNeutralTimer *= 0.8f;
		else
			tGameApp::fInstance( ).fSoundSystem( )->fSetState( AK::STATES::GLOBAL_MIX_STATES::GROUP, AK::STATES::GLOBAL_MIX_STATES::STATE::NUKE );
		
	}
	void tLevelLogic::fSetRootMenuFromScript( const Sqrat::Object& rootMenuScriptObj )
	{
		mRootMenuScriptObject = rootMenuScriptObj;
		mRootMenu.fRemoveFromParent( );
		mRootMenu = Gui::tCanvasPtr( rootMenuScriptObj );
		if( !mRootMenu.fIsNull( ) )
			tApplication::fInstance( ).fRootCanvas( ).fAddChild( mRootMenu );
	}
	void tLevelLogic::fRegisterSaveableEntity( tEntity* entity )
	{
		const tSaveGame* saveGame = tGameApp::fInstance( ).fCurrentSaveGame( );

		if( tGameApp::fInstance( ).fSpawningCurrentLevel( ) )
		{
			const u32 entIndex = mSaveableEntsFromLevel.fCount( );

			mSaveableEntsFromLevel.fPushBack( tEntityPtr( entity ) );

			const tEntitySaveData* entSaveData = saveGame ? saveGame->fEntFromLevel( entIndex ) : 0;
			if( entSaveData )
			{
				if( entSaveData->mDeleted )
					entity->fDelete( );
				else
					entSaveData->fRestoreSavedEntity( entity );
			}
		}
		else
		{
			mSaveableEntsDynamic.fPushBack( tEntityPtr( entity ) );
		}
	}
	void tLevelLogic::fUnRegisterSaveableEntity( tEntity* entity )
	{
		if( tGameApp::fInstance( ).fSpawningCurrentLevel( ) )
			mSaveableEntsFromLevel.fFindAndEraseOrdered( entity );
		mSaveableEntsDynamic.fFindAndEraseOrdered( entity );
	}
	void tLevelLogic::fRegisterCameraPoint( tEntity* entity )
	{
		mCameraPoints.fPushBack( tEntityPtr( entity ) );
	}
	void tLevelLogic::fRegisterCameraBox( tEntity* entity )
	{
		tShapeEntity* shape = entity->fDynamicCast< tShapeEntity >( );
		if( shape )
		{
			shape->fAddGameTagsRecursive( GameFlags::cFLAG_DUMMY );
			u32 team = shape->fQueryEnumValue( GameFlags::cENUM_TEAM, ~0 );
			if( team == ~0 )
			{
				log_warning( 0, "Camera box has no team. Assigning to all teams." );

				for( u32 i = 0; i < mCameraBoxes.fCount( ); ++i )
					mCameraBoxes[ i ].fReset( shape );
			}
			else
				mCameraBoxes[ team ].fReset( shape );
		}
		else
		{
			log_warning( 0, "Register camera box not added to shape entity." );
		}
	}
	void tLevelLogic::fRegisterBombDropperBox( tEntity* entity )
	{
		tShapeEntity* shape = entity->fDynamicCast< tShapeEntity >( );
		if( shape )
		{
			shape->fAddGameTagsRecursive( GameFlags::cFLAG_DUMMY );
			u32 team = shape->fQueryEnumValue( GameFlags::cENUM_TEAM, ~0 );
			if( team == ~0 )
			{
				log_warning( 0, "Bomb dropper box has no team. Assigning to all teams." );

				for( u32 i = 0; i < mBombDropperBox.fCount( ); ++i )
					mBombDropperBox[ i ].fReset( shape );
			}
			else
				mBombDropperBox[ team ].fReset( shape );
		}
		else
		{
			log_warning( 0, "Register bomb dropper box not added to shape entity." );
		}
	}	
	void tLevelLogic::fRegisterCameraBlocker( tEntity* entity )
	{
		tShapeEntity* shape = entity->fDynamicCast< tShapeEntity >( );
		if( shape )
		{
			shape->fAddGameTagsRecursive( GameFlags::cFLAG_DUMMY );
			mCameraBlockers.fPushBack( tShapeEntityPtr( shape ) );
		}
	}	
	void tLevelLogic::fRegisterCameraGround( tEntity* entity )
	{
		tShapeEntity* shape = entity->fDynamicCast< tShapeEntity >( );
		if( shape )
		{
			shape->fAddGameTagsRecursive( GameFlags::cFLAG_DUMMY );
			mCameraGrounds.fPushBack( tShapeEntityPtr( shape ) );
		}
	}
	void tLevelLogic::fRegisterPathStart( tEntity* startPoint )
	{
		tPathEntity* pe = startPoint->fDynamicCast<tPathEntity>( );

		if( pe )
			mPathStarts.fPushBack( tPathEntityPtr( pe ) );
	}
	void tLevelLogic::fRegisterTrenchStart( tEntity* startPoint )
	{
		tPathEntity* pe = startPoint->fDynamicCast<tPathEntity>( );

		if( pe )
		{
			mTrenchLengths.fPushBack( -1.f );
			mTrenchStarts.fPushBack( tPathEntityPtr( pe ) );
		}
	}
	f32 tLevelLogic::fTrenchLength( u32 index )
	{
		if( mTrenchLengths[ index ] < 0.0f )
		{
			// compute trench length
			tPathEntity* pe = mTrenchStarts[ index ]->fDynamicCast<tPathEntity>( );
			f32 trenchLength = 0;

			if( pe )
			{
				tVec3f prevPos = pe->fObjectToWorld( ).fGetTranslation( );

				tPathEntity *wayPt = pe;

				while( wayPt )
				{
					const tPathEntity::tConnectionsList& children = wayPt->fNextPathPoints( );

					if( children.fCount( ) )
					{
						tPathEntity* child = children[ 0 ];
						wayPt = child;

						tVec3f newPos = wayPt->fObjectToWorld( ).fGetTranslation( );
						trenchLength += (newPos - prevPos).fLength( );
						prevPos = newPos;
					}
					else
						break;
				}
			}

			mTrenchLengths[ index ] = trenchLength;
		}

		return mTrenchLengths[ index ];
	}
	void tLevelLogic::fRegisterExitGenStart( tEntity* startPoint )
	{
		tPathEntity* pe = startPoint->fDynamicCast<tPathEntity>( );

		if( pe )
			mExitGenStarts.fPushBack( tPathEntityPtr( pe ) );
		else
			log_warning( 0, "You've attached an exit gen script to a non path point object. named: " << startPoint->fName( ) );
	}
	void tLevelLogic::fRegisterContextPathStart( tEntity* startPoint )
	{
		tPathEntity* pe = startPoint->fDynamicCast<tPathEntity>( );

		if( pe )
			mContextPathStarts.fPushBack( tPathEntityPtr( pe ) );
		else
			log_warning( 0, "You've attached a context path script to a non path point object. named: " << startPoint->fName( ) );
	}
	void tLevelLogic::fRegisterFlightPathStart( tEntity* startPoint )
	{
		tPathEntity* pe = startPoint->fDynamicCast<tPathEntity>( );

		if( pe )
			mFlightPathStarts.fPushBack( tPathEntityPtr( pe ) );
		else
			log_warning( 0, "You've attached a flight path script to a non path point object. named: " << startPoint->fName( ) );
	}
	void tLevelLogic::fRegisterNamedWaypoint( tEntity* waypoint )
	{
		tPathEntity* pathEnt = waypoint->fDynamicCast<tPathEntity>( );
		
		if( pathEnt )
			mNamedWaypoints.fPushBack( tPathEntityPtr( pathEnt ) );
		else
			log_warning( 0, "You've attached a way point script to a non path point object. named: " << waypoint->fName( ) );

	}
	tEntity* tLevelLogic::fNamedPathPoint( const tStringPtr& name ) const
	{
		for( u32 i = 0; i < mNamedWaypoints.fCount( ); ++i )
			if( mNamedWaypoints[ i ]->fName( ) == name )
				return mNamedWaypoints[ i ].fGetRawPtr( );

		return NULL;
	}
	void tLevelLogic::fRegisterNamedObject( tEntity* obj )
	{
		sigassert( obj );
		if( obj->fName( ).fExists( ) )
		{
			fRemoveNamedObject( obj->fName( ) );
			mNamedObjects.fPushBack( tEntityPtr( obj ) );
		}
	}
	void tLevelLogic::fRegisterAirSpace( tEntity* obj )
	{
		tShapeEntity* shape = obj->fDynamicCast< tShapeEntity >( );
		if( shape )
		{
			shape->fAddGameTagsRecursive( GameFlags::cFLAG_DUMMY );
			u32 team = shape->fQueryEnumValue( GameFlags::cENUM_TEAM, ~0 );
			if( team == ~0 )
			{
				log_warning( 0, "Air space box has no team. Assigning to all teams." );

				for( u32 i = 0; i < mAirSpace.fCount( ); ++i )
					mAirSpace[ i ].fReset( shape );
			}
			else
				mAirSpace[ team ].fReset( shape );
		}
	}
	b32 tLevelLogic::fRemoveNamedObject( const tStringPtr& name )
	{
		b32 found = false;
		for( u32 i = 0; i < mNamedObjects.fCount( ); ++i )
		{
			if( mNamedObjects[ i ]->fName( ) == name )
			{
				mNamedObjects.fErase( i );
				--i;
				found = true;
			}
		}

		return found;
	}
	tEntity* tLevelLogic::fNamedObject( const tStringPtr& name ) const
	{
		for( u32 i = 0; i < mNamedObjects.fCount( ); ++i )
			if( mNamedObjects[ i ]->fName( ) == name )
				return mNamedObjects[ i ].fGetRawPtr( );

		return NULL;
	}
	void tLevelLogic::fRegisterGoalBox( tEntity* GoalBox )
	{
		tGoalBoxLogic* logic = GoalBox->fLogicDerived<tGoalBoxLogic>( );
		sigassert( logic );
		mGoalBoxes.fPushBack( tEntityPtr( GoalBox ) );
	}
	void tLevelLogic::fAddArtilleryBarrageSpawnPt( tEntity* entity )
	{
		mArtillerySpawnPts.fPushBack( tEntityPtr( entity ) );
	}
	void tLevelLogic::fAddArtilleryBarragePt( tEntity* entity )
	{
		sigassert( entity );
		u32 team = entity->fQueryEnumValue( GameFlags::cENUM_TEAM, GameFlags::cTEAM_NONE );
		sigassert( team != GameFlags::cTEAM_NONE && "Artillery Barrage Pt needs a team enum!" );
		
		mArtilleryPtsByTeam[ team ].fPushBack( tEntityPtr( entity ) );
	}
	tEntity* tLevelLogic::fArtilleryBarrageSpawnPt( const tStringPtr& name )
	{
		for( u32 i = 0; i < mArtillerySpawnPts.fCount( ); ++i )
			if( mArtillerySpawnPts[ i ]->fName( ) == name )
				return mArtillerySpawnPts[ i ].fGetRawPtr( );
		return NULL;
	}
	void tLevelLogic::fAddBarrageDropPt( tEntity* entity )
	{
		sigassert( entity );
		u32 team = entity->fQueryEnumValue( GameFlags::cENUM_TEAM, GameFlags::cTEAM_NONE );
		sigassert( team != GameFlags::cTEAM_NONE && "Barrage Drop Pt needs a team enum!" );

		mBarrageDropPtsByTeam[ team ].fPushBack( tEntityPtr( entity ) );
	}
	tEntity* tLevelLogic::fBarrageDropPt( u32 team, const tVec3f& pos, const tStringPtr& name )
	{
		tGrowableArray<tEntityPtr>& array = mBarrageDropPtsByTeam[ team ];
		return tEntity::fClosestEntityNamed( array, pos, name );
	}
	void tLevelLogic::fRegisterToyBoxAlarm( tEntity* entity )
	{
		sigassert( entity );
		u32 team = entity->fQueryEnumValue( GameFlags::cENUM_TEAM, GameFlags::cTEAM_NONE );
		sigassert( team != GameFlags::cTEAM_NONE && "Toy box alarm needs a team enum!" );

		tProximityLogic* logicP = entity->fLogicDerived<tProximityLogic>( );
		sigassert( logicP && "Toy box alarm must be a tProximityLogic" );

		// the team enum is what the alarm will detect, so assign to the other team
		if( team == GameFlags::cTEAM_RED ) 
			team = GameFlags::cTEAM_BLUE;
		else 
			team = GameFlags::cTEAM_RED;

		mToyBoxAlarmEnts[ team ].fPushBack( tEntityPtr( entity ) );
		mToyBoxAlarms[ team ].fPushBack( logicP );
	}
	const tGrowableArray<tProximityLogic*>& tLevelLogic::fToyBoxAlarms( u32 team )
	{
		return mToyBoxAlarms[ team ];
	}
	void tLevelLogic::fRegisterCameraPathStart( tEntity* entity )
	{
		tPathEntity* pe = entity->fDynamicCast<tPathEntity>( );
		if( pe )
			mCameraPathStarts.fPushBack( tPathEntityPtr( pe ) );
	}
	void tLevelLogic::fRenderPath( const tEntity* startEnt )
	{
#ifdef sig_devmenu
		tPathEntity* pathEnt = startEnt->fDynamicCast< tPathEntity >( );
		if( !pathEnt ) return;

		const tMat3f& xform = pathEnt->fObjectToWorld( );
		const tSpheref sphere( tVec3f( 0.f, 0.f, 0.f ), 1.f );

		tWaypointLogic* waypointLogic = pathEnt->fLogicDerived<tWaypointLogic>( );
		const tVec4f color = (!waypointLogic || waypointLogic->fIsAccessible( )) ? Debug_Paths_SphereColor : Debug_Paths_InaccessibleColor;

		tGameApp::fInstance( ).fSceneGraph( )->fDebugGeometry( ).fRenderOnce( sphere, xform, color );

		const tPathEntity::tConnectionsList& nextPts = pathEnt->fNextPathPoints( );
		for( u32 i = 0; i < nextPts.fCount( ); ++i )
		{
			const tPathEntity* child = nextPts[ i ];
			const tVec3f& childPos = child->fObjectToWorld( ).fGetTranslation( );

			tGameApp::fInstance( ).fSceneGraph( )->fDebugGeometry( ).fRenderOnce( tPair< tVec3f, tVec3f >( xform.fGetTranslation( ), childPos ), Debug_Paths_LineColor );
			fRenderPath( child );
		}

#endif //sig_devmenu
	}
	void tLevelLogic::fRegisterBuildSiteSmall( tEntity* entity )
	{
		fRegisterBuildSite( entity, mBuildSitesSmall, mBuildSiteRootsSmall, GameFlags::cBUILD_SITE_SMALL );
	}
	void tLevelLogic::fRegisterBuildSiteLarge( tEntity* entity )
	{
		fRegisterBuildSite( entity, mBuildSitesLarge, mBuildSiteRootsLarge, GameFlags::cBUILD_SITE_LARGE );
	}
	tBuildSiteLogic* tLevelLogic::fBuildSiteNamed( const tStringPtr& name ) const
	{
		for( u32 i = 0; i < mBuildSiteRootsLarge.fCount( ); ++i )
		{
			if( mBuildSiteRootsLarge[ i ]->fName( ) == name )
			{
				tBuildSiteLogic* logic = mBuildSiteRootsLarge[ i ]->fLogicDerived<tBuildSiteLogic>( );
				if( logic )
					return logic;
			}
		}
		for( u32 i = 0; i < mBuildSiteRootsSmall.fCount( ); ++i )
		{
			if( mBuildSiteRootsSmall[ i ]->fName( ) == name )
			{
				tBuildSiteLogic* logic = mBuildSiteRootsSmall[ i ]->fLogicDerived<tBuildSiteLogic>( );
				if( logic )
					return logic;
			}
		}

		return NULL;
	}
	void tLevelLogic::fRegisterBuildSite( tEntity* entity, tGrowableArray<tShapeEntityPtr>& buildSites, tGrowableArray<tEntityPtr>& buildSiteRoots, GameFlags::tBUILD_SITE buildSiteType )
	{
		buildSiteRoots.fPushBack( tEntityPtr( entity ) );
		for( u32 i = 0; i < entity->fChildCount( ); ++i )
		{
			if( entity->fChild( i )->fQueryEnumValue( GameFlags::cENUM_BUILD_SITE ) != ~0 )
			{
				tShapeEntity* shape = entity->fChild( i )->fDynamicCast< tShapeEntity >( );
				if( shape )
				{
					buildSites.fPushBack( tShapeEntityPtr( shape ) );
					shape->fSetEnumValue( tEntityEnumProperty( GameFlags::cENUM_BUILD_SITE, buildSiteType ) );
					if( tGameApp::fInstance( ).fGameMode( ).fIsSinglePlayerOrCoop( ) )
						shape->fSetEnumValue( tEntityEnumProperty( GameFlags::cENUM_TEAM, tGameApp::fInstance( ).fGetPlayer( 0 )->fTeam( ) ) );
				}
			}
		}
	}
	void tLevelLogic::fRegisterGenerator( tEntity* generator )
	{
		mGenerators.fPushBack( tEntityPtr( generator ) );
	}
	void tLevelLogic::fRegisterSpecialLevelObject( tEntity* entity )
	{
		sigassert( entity );

		const u32 specialLevelObject = entity->fQueryEnumValue( GameFlags::cENUM_SPECIAL_LEVEL_OBJECT, GameFlags::cSPECIAL_LEVEL_OBJECT_COUNT );
		switch( specialLevelObject )
		{
		case GameFlags::cSPECIAL_LEVEL_OBJECT_DEFAULT:
			fOnRegisterSpecialLevelObject( entity );
			break;
		case GameFlags::cSPECIAL_LEVEL_OBJECT_BOSS:
			fOnRegisterBoss( entity );
			tUnitLogic* unitLogic = entity->fLogicDerived<tUnitLogic>( );
			if( unitLogic )
			{
				mScreenSpaceHealthBarList->fAddHealthBar( *unitLogic );
				unitLogic->fSetHasScreenSpaceHealthBar( true );
			}
			break;
		}
	}
	void tLevelLogic::fOnRegisterSpecialLevelObject( tEntity* levelObj )
	{
		mSpecialLevelObjects.fPushBack( tEntityPtr( levelObj ) );

		if( fOwnerEntity( ) )
		{
			const tLogicPtr* logicPtr = fOwnerEntity( )->fLogicPtr( );
			if( !logicPtr )
				return;

			if( logicPtr->fIsCodeOwned( ) )
			{
				// TODO: Maybe implement the following
				//logicPtr->fCodeObject( )->fOnRegisterSpecialLevelObject( levelObj );
			}
			else
			{
				Sqrat::Function f( logicPtr->fScriptObject( ), "OnRegisterSpecialLevelObject" );
				if( !f.IsNull( ) )
					f.Execute( levelObj );
				else
					log_warning( 0, "OnRegisterSpecialLevelObject doesn't exist." );
			}
		}
	}
	void tLevelLogic::fOnRegisterBoss( tEntity* levelObj )
	{
		mSpecialLevelObjects.fPushBack( tEntityPtr( levelObj ) );

		if( fOwnerEntity( ) )
		{
			const tLogicPtr* logicPtr = fOwnerEntity( )->fLogicPtr( );
			if( !logicPtr )
				return;

			if( logicPtr->fIsCodeOwned( ) )
			{
				// TODO: Maybe implement the following
				//logicPtr->fCodeObject( )->fOnRegisterBoss( levelObj );
			}
			else
			{
				Sqrat::Function f( logicPtr->fScriptObject( ), "OnRegisterBoss" );
				if( !f.IsNull( ) )
					f.Execute( levelObj );
				else
					log_warning( 0, "OnRegisterBoss doesn't exist." );
			}
		}
	}
	void tLevelLogic::fRegisterUseableUnit( tEntity* entity )
	{
		mUseableUnits.fPushBack( tUsableUnit( ) );
		mUseableUnits.fBack( ).mEntity.fReset( entity );
		mUseableUnits.fBack( ).mInitialTransform = entity->fObjectToWorld( );
	}
	void tLevelLogic::fUnregisterUseableUnit( tEntity* entity )
	{
		mUseableUnits.fFindAndErase( entity );
	}
	tEntity* tLevelLogic::fUseableUnit( const tStringPtr& name ) const
	{
		for( u32 i = 0; i < mUseableUnits.fCount( ); ++i )
			if( mUseableUnits[ i ].mEntity->fName( ) == name )
				return mUseableUnits[ i ].mEntity.fGetRawPtr( );

		return NULL;
	}
	const tMat3f* tLevelLogic::fUsableUnitInitialTransform( tEntity* ent ) const
	{
		const tUsableUnit* usable = mUseableUnits.fFind( ent );
		if( usable )
			return &usable->mInitialTransform;
		else
			return NULL;
	}
	void tLevelLogic::fRegisterUnit( const tUnitLogic& unitLogic )
	{
		u32 type = unitLogic.fUnitType( );
		tEntityPtr p( unitLogic.fOwnerEntity( ) );
		mUnitsByType[ type ].fPushBack( p );
		if( type != GameFlags::cUNIT_TYPE_NONE )
			mAllUnits.fPushBack( p );

		fRegisterSpecialLevelObject( unitLogic.fOwnerEntity( ) );
	}
	void tLevelLogic::fUnregisterUnit( const tUnitLogic& unitLogic )
	{
		u32 type = unitLogic.fUnitType( );
		mUnitsByType[ type ].fFindAndErase( unitLogic.fOwnerEntity( ) );
		mAllUnits.fFindAndErase( unitLogic.fOwnerEntity( ) );
	}
	void tLevelLogic::fOnSimulationBegin( )
	{
		if( mWaveManager )
			mWaveManager->fShow( true );

		if( mVersusWaveManager )
			mVersusWaveManager->fShow( true );

		// Create Ration Ticket UI
		if( tGameApp::fInstance( ).fGameMode( ).fIsSinglePlayerOrCoop( ) )
		{
			mRationTicketUI.fReset( NEW Gui::tRationTicketUI( ) );
			/*if( tGameApp::fInstance( ).fGameMode( ).fIsSplitScreen( ) )
				tGameApp::fInstance( ).fHudLayer( "viewport1" ).fToCanvasFrame( ).fAddChild( mRationTicketUI->fCanvas( ) );
			else
				tGameApp::fInstance( ).fHudLayer( "alwaysHide" ).fToCanvasFrame( ).fAddChild( mRationTicketUI->fCanvas( ) );*/
			tGameApp::fInstance( ).fHudLayer( "alwaysShow" ).fToCanvasFrame( ).fAddChild( mRationTicketUI->fCanvas( ) );
		}

		// Add boss health bar to screen
		tGameApp::fInstance( ).fHudLayer( "alwaysShow" ).fToCanvasFrame( ).fAddChild( mScreenSpaceHealthBarList->fCanvas( ) );

		// initialize game effects
		tDynamicArray< tPlayerPtr >& gamePlayers = tGameApp::fInstance( ).fPlayers( );
		tGrowableArray< tEffectPlayerPtr >& fxPlayers = tGameEffects::fInstance( ).gPlayers;
		fxPlayers.fSetCount( 0 );

		if( !tGameApp::fInstance( ).fGameMode( ).fIsFrontEnd( ) )
		{
			for( u32 i = 0; i < gamePlayers.fCount( ); ++i )
				fxPlayers.fPushBack( tEffectPlayerPtr( gamePlayers[ i ].fGetRawPtr( ) ) );
		}
	}
	void tLevelLogic::fOnLevelLoadEnd( )
	{
		// Show all the hud layers
		tGameApp::fInstance( ).fHudLayer( "viewport0" ).fCanvas( )->fSetInvisible( false );
		tGameApp::fInstance( ).fHudLayer( "viewport1" ).fCanvas( )->fSetInvisible( false );
		tGameApp::fInstance( ).fHudLayer( "alwaysHide" ).fCanvas( )->fSetInvisible( false );
		tGameApp::fInstance( ).fHudLayer( "alwaysShow" ).fCanvas( )->fSetInvisible( false );

		tGameApp::fInstance( ).fSoundSystem( )->fMasterSource( )->fHandleEvent( tGameApp::fExtraDemoMode( ) ? AK::EVENTS::PLAY_EXTRA : AK::EVENTS::PLAY_AMBIENCE );	

		tGameApp::fInstance( ).fSoundSystem( )->fSetState( AK::STATES::GLOBAL_MIX_STATES::GROUP, tGameApp::fInstance( ).fGameMode( ).fIsFrontEnd( ) ? AK::STATES::GLOBAL_MIX_STATES::STATE::FRONTEND : AK::STATES::GLOBAL_MIX_STATES::STATE::NEUTRAL );
		mNukeMixNeutralTimer = -1.f;
				
		fProcessWaypoints( );

		tGameApp::fInstance( ).fSetSingleScreenControlIndex( 0 );

		const tLevelLoadInfo& info = tGameApp::fInstance( ).fCurrentLevelLoadInfo( );
		if( info.mMapType == GameFlags::cMAP_TYPE_CAMPAIGN
			&& !tGameApp::fInstance( ).fGameMode( ).fIsCoOp( ) )
		{
			u32 wave = 0;
			if( info.mHighestWaveReached != -1 )
				wave = info.mHighestWaveReached;
			
			tPlayer* player = tGameApp::fInstance( ).fFrontEndPlayer( );
			sigassert( player );
			player->fProfile( )->fSetLastPlayedWave( info.mLevelIndex, wave );
		}
	}
	void tLevelLogic::fOnLevelUnloadBegin( )
	{
		Logic::tGoalDriven::fClearGoals( this );

		tGameApp::fInstance( ).fSetSingleScreenControlIndex( 0 );

		if( mScreenSpaceHealthBarList )
		{
			mScreenSpaceHealthBarList->fCanvas( ).fRemoveFromParent( );
			mScreenSpaceHealthBarList->fCanvas( ).fDeleteSelf( );
			mScreenSpaceHealthBarList.fRelease( );
		}

		if( mWaveManager )
			mWaveManager->fOnLevelUnloadBegin( );
		if( mVersusWaveManager )
			mVersusWaveManager->fOnLevelUnloadBegin( );

		// free up any async ui textures
		fSetRootMenuFromScript( Sqrat::Object( ) );
	}
	void tLevelLogic::fPushStandardCameras( )
	{
		tGameApp & gameApp = tGameApp::fInstance( );
		const tDynamicArray< tPlayerPtr > & players = gameApp.fPlayers( );

		const u32 cameraPointCount = fCameraPointCount( );

		const u32 playerCount = players.fCount( );
		for( u32 p = 0; p < playerCount; ++p )
		{
			players[ p ]->fClearCameraStack( );
			if( cameraPointCount )
				players[ p ]->fPushRTSCamera( fCameraPoint( fMin( p, cameraPointCount - 1 ) ) );
			else
				players[ p ]->fPushRTSCamera( NULL );
		}
	}

	void tLevelLogic::fSetPlayerMoney( u32 money )
	{
		log_warning( 0, "DONT CALL THIS ANYMORE: SetPlayerMoney" );
	}

	void tLevelLogic::fSetCurrentLevelProgression( u32 progress )
	{
		mLevelProgression = progress;
		Audio::tSystem* system = tGameApp::fInstance( ).fSoundSystem( ).fGetRawPtr( );

		mNeutralTimer = -1;

		switch( progress )
		{
		case cLevelProgressionIntro:
			mNeutralTimer = mNeutralTimerTime;
			system->fSetState( AK::STATES::MUSIC_CAMPAIGN::GROUP, AK::STATES::MUSIC_CAMPAIGN::STATE::INTRO );
			break;
		case cLevelProgressionPlaying:
			system->fSetState( AK::STATES::MUSIC_CAMPAIGN::GROUP, AK::STATES::MUSIC_CAMPAIGN::STATE::NEUTRAL );
			break;
		case cLevelProgressionMinigame:
			system->fSetState( AK::STATES::MUSIC_CAMPAIGN::GROUP, AK::STATES::MUSIC_CAMPAIGN::STATE::MINIGAME );
			break;
		case cLevelProgressionStageBoss:
			system->fSetState( AK::STATES::MUSIC_CAMPAIGN::GROUP, AK::STATES::MUSIC_CAMPAIGN::STATE::BOSS );
			break;
		case cLevelProgressionStageVictory:
			system->fSetState( AK::STATES::MUSIC_CAMPAIGN::GROUP, AK::STATES::MUSIC_CAMPAIGN::STATE::LEVEL_SUCCESS );
			break;
		case cLevelProgressionStageDefeat:
			system->fSetState( AK::STATES::MUSIC_CAMPAIGN::GROUP, AK::STATES::MUSIC_CAMPAIGN::STATE::LEVEL_FAIL );
			break;
		}
	}

	void tLevelLogic::fComputeLevelBounds( )
	{
		mLevelBounds = tAabbf( tVec3f( -500.f, -100.f, -500.f ), tVec3f( 500.f, 100.f, 500.f ) );

		tSceneRefEntity* level = fOwnerEntity( )->fDynamicCast< tSceneRefEntity >( );
		if( level )
			mLevelBounds = level->fSgResource( )->fCast< tSceneGraphFile >( )->mBounds;
		log_line( 0, "Level Bounds = " << mLevelBounds.mMin << ", " << mLevelBounds.mMax );

		mMaxLevelDimension = mLevelBounds.fComputeDiagonal( ).fLength( );
	}
	void tLevelLogic::fOnWaveListFinished( tWaveList* waveList )
	{
		if( fOwnerEntity( ) )
		{
			const tLogicPtr* logicPtr = fOwnerEntity( )->fLogicPtr( );
			if( !logicPtr )
				return;

			if( logicPtr->fIsCodeOwned( ) )
			{
				// TODO: Maybe implement the following
				//logicPtr->fCodeObject( )->fOnRegisterSpecialLevelObject( levelObj );
			}
			else
			{
				Sqrat::Function f( logicPtr->fScriptObject( ), "OnWaveListFinished" );
				if( !f.IsNull( ) )
					f.Execute( waveList );
				//else
				//	log_warning( 0, "OnWaveListFinished doesn't exist." );
			}
		}
	}
	void tLevelLogic::fOnWaveKilled( tWaveDesc* wave )
	{
		if( fOwnerEntity( ) )
		{
			const tLogicPtr* logicPtr = fOwnerEntity( )->fLogicPtr( );
			if( !logicPtr )
				return;

			if( logicPtr->fIsCodeOwned( ) )
			{
				// TODO: Maybe implement the following
				//logicPtr->fCodeObject( )->fOnRegisterSpecialLevelObject( levelObj );
			}
			else
			{
				Sqrat::Function f( logicPtr->fScriptObject( ), "OnWaveKilled" );
				if( !f.IsNull( ) )
					f.Execute( wave );
				//else
				//	log_warning( 0, "OnWaveKilled doesn't exist." );
			}
		}
	}
	void tLevelLogic::fOnNoWavesOrEnemiesLeft( )
	{
		if( fOwnerEntity( ) )
		{
			const tLogicPtr* logicPtr = fOwnerEntity( )->fLogicPtr( );
			if( !logicPtr )
				return;

			if( logicPtr->fIsCodeOwned( ) )
			{
				// TODO: Maybe implement the following
				//logicPtr->fCodeObject( )->fOnRegisterSpecialLevelObject( levelObj );
			}
			else
			{
				Sqrat::Function f( logicPtr->fScriptObject( ), "OnNoWavesOrEnemiesLeft" );
				if( !f.IsNull( ) )
					f.Execute( );
				else
					log_warning( 0, "OnNoWavesOrEnemiesLeft doesn't exist." );
			}
		}
	}
	void tLevelLogic::fOnPathCameraFinished( Gfx::tFollowPathCamera& camera )
	{
		if( camera.fSkipped( ) )
			tGameApp::fInstance( ).fSoundSystem( )->fMasterSource( )->fHandleEvent( "Play_UI_Select_Forward" );

		fSetCurrentLevelProgression( cLevelProgressionPlaying );

		if( fOwnerEntity( ) )
		{
			const tLogicPtr* logicPtr = fOwnerEntity( )->fLogicPtr( );
			if( logicPtr )
			{
				if( logicPtr->fIsCodeOwned( ) )
				{
					// TODO: Maybe implement the following
					//logicPtr->fCodeObject( )->fOnRegisterSpecialLevelObject( levelObj );
				}
				else
				{
					Sqrat::Function f( logicPtr->fScriptObject( ), "OnPathCameraFinished" );
					if( !f.IsNull( ) )
						f.Execute( camera.fKey( ) );
					else
						log_warning( 0, "OnPathCameraFinished doesn't exist." );
				}
			}
		}
	}

	tWaveList* tLevelLogic::fAddWaveList( const tStringPtr& waveListName )
	{
		if( mWaveManager )
			return mWaveManager->fAddWaveList( waveListName );

		return 0;
	}

	tWaveList* tLevelLogic::fWaveList( u32 i )
	{
		if( mWaveManager )
			return mWaveManager->fWaveList( i );

		return 0;
	}
	u32 tLevelLogic::fWaveListCount( ) const
	{
		if( mWaveManager )
			return mWaveManager->fWaveListCount( );

		return 0;
	}
	void tLevelLogic::fSetUIWaveList( const tStringPtr& listName )
	{
		if( mWaveManager )
			mWaveManager->fSetUIWaveList( listName );
	}

	void tLevelLogic::fShowEnemiesAliveList( b32 show )
	{
		if( mWaveManager )
			mWaveManager->fShowEnemiesAliveList( show );
	}

	void tLevelLogic::fShowWaveListUI( b32 show )
	{
		if( mWaveManager )
		{
			if( !show )
				mWaveManager->fSetUIWaveList( tStringPtr::cNullPtr );
			mWaveManager->fShow( show );
		}
	}

	void tLevelLogic::fProcessWaypoints( )
	{
		Time::tStopWatch stopWatch;
		stopWatch.fStart( );
		//fAttachWaypointLogics( );
		fFindGoalPathEnds( );
		fLinkGeneratorParents( );
		stopWatch.fStop( );

		// camera boxes
		for( u32 i = 0; i < GameFlags::cTEAM_COUNT; ++i )
			if( !mBombDropperBox[ i ] )
				mBombDropperBox[ i ] = mCameraBoxes[ i ];

		log_line( 0, "Processing waypoints time (milliseconds) = " << stopWatch.fGetElapsedMs( ) );
		mWaypointsProcessed = true;
	}
	void tLevelLogic::fAttachWaypointLogics( )
	{
		for( u32 i = 0; i < mPathStarts.fCount( ); ++i )
			fAttachLogicRecursively( mPathStarts[ i ].fGetRawPtr( ) );

		for( u32 i = 0; i < mTrenchStarts.fCount( ); ++i )
			fAttachLogicRecursively( mTrenchStarts[ i ].fGetRawPtr( ) );

		for( u32 i = 0; i < mExitGenStarts.fCount( ); ++i )
			fAttachLogicRecursively( mExitGenStarts[ i ].fGetRawPtr( ) );

		for( u32 i = 0; i < mContextPathStarts.fCount( ); ++i )
			fAttachLogicRecursively( mContextPathStarts[ i ].fGetRawPtr( ) );

		for( u32 i = 0; i < mFlightPathStarts.fCount( ); ++i )
			fAttachLogicRecursively( mFlightPathStarts[ i ].fGetRawPtr( ) );
	}
	void tLevelLogic::fAttachLogicRecursively( tPathEntity* path )
	{
		if( path->fLogic( ) == 0 )
			path->fAcquireLogic( NEW tLogicPtr( NEW tWaypointLogic( ) ) );

		for( u32 i = 0; i < path->fNextPointCount( ); ++i )
			fAttachLogicRecursively( path->fNextPoint( i ) );
	}

	void tLevelLogic::fFindGoalPathEnds( )
	{
		tGrowableArray<tPathEntityPtr> pathEnds;
		mPathEndsInGoal.fSetCount( mPathStarts.fCount( ) );
		mPathHasDropPoints.fSetCount( mPathStarts.fCount( ) );

		for( u32 i = 0; i < mPathStarts.fCount( ); ++i )
		{
			mPathHasDropPoints[ i ] = false;

			b32 shouldEndInBox = !mPathStarts[ i ]->fHasGameTagsAny( GameFlags::cFLAG_DOESNT_END_IN_GOALBOX );
			b32 found = fFindAndRegisterPathEndWithGoalLogic( mPathStarts[ i ].fGetRawPtr( ), mPathHasDropPoints[ i ] );
			
			mPathEndsInGoal[ i ] = found || shouldEndInBox;
			if( !found && shouldEndInBox )
				log_warning( 0, "Goal path ( " << mPathStarts[ i ]->fName( ) << " ) doesn't end in a goal box!" );
		}
	}
	void tLevelLogic::fLinkGeneratorParents( )
	{
		for( u32 i = 0; i < mGenerators.fCount( ); ++i )
		{
			mGenerators[ i ]->fLogicDerivedStaticCast<tGeneratorLogic>( )->fFindAndLinkParent( mGenerators );
		}
	}
	b32 tLevelLogic::fFindAndRegisterPathEndWithGoalLogic( tPathEntity* path, b32& hasDropPoints )
	{
		if( path->fHasGameTagsAny( GameFlags::cFLAG_DROP_CARGO ) )
			hasDropPoints = true;

		if( path->fNextPointCount( ) == 0 )
		{
			for( u32 i = 0; i < mGoalBoxes.fCount( ); ++i )
			{
				tGoalBoxLogic* goalBox = mGoalBoxes[ i ]->fLogicDerived<tGoalBoxLogic>( );
				sigassert( goalBox );
				if( goalBox->fGetGoalZone( )->fContains( path->fObjectToWorld( ).fGetTranslation( ) ) )
				{
					goalBox->fRegisterPathEnd( path );
					// A path shouldn't be inside more than one goal box right?
					return true;
				}
			}

			// If we got this far then the goal path doesn't end in a goal box
			return false;
		}
		else
		{
			for( u32 i = 0; i < path->fNextPointCount( ); ++i )
			{
				if( fFindAndRegisterPathEndWithGoalLogic( path->fNextPoint( i ), hasDropPoints ) )
					return true;
			}

			return false;
		}
	}
	void tLevelLogic::fQuitNetGameEarly( )
	{
		fMatchEnded( true );
	}
	void tLevelLogic::fSetDefeatedPlayer( tPlayer* player )
	{
		mDefeatedPlayer.fReset( player );
		if( tGameApp::fInstance( ).fGameMode( ).fIsVersus( ) )
			mVictoriousPlayer.fReset( tGameApp::fInstance( ).fOtherPlayer( player ) );
		fMatchEnded( );
	}
	void tLevelLogic::fSetVictoriousPlayer( tPlayer* player ) 
	{ 
		mVictoriousPlayer.fReset( player );
		if( tGameApp::fInstance( ).fGameMode( ).fIsVersus( ) )
			mDefeatedPlayer.fReset( tGameApp::fInstance( ).fOtherPlayer( player ) );
		fMatchEnded( ); 
	}
	void tLevelLogic::fMatchEnded( bool endedEarly )
	{
		mMatchEnded = true;
		mStopWavesTimer = Gameplay_Waves_EndGameStopTimer;

		if( endedEarly )
		{
			sync_end( );
			return;
		}

		const b32 coop = tGameApp::fInstance( ).fGameMode( ).fIsCoOp( );
		if( coop && mMapType == GameFlags::cMAP_TYPE_SURVIVAL )
			tGameApp::fInstance( ).fAwardDeferredAchievementToAllPlayers( GameFlags::cACHIEVEMENTS_SYNERGY );

		tGameApp& gameApp = tGameApp::fInstance( );

		const tDynamicArray< tPlayerPtr > & players = gameApp.fPlayers( );
		const u32 playerCount = players.fCount( );

		if( coop )
		{
			// combines scores and shiz
			tPlayer::fLockScoreAndStats( false, players );
		}
		else
		{
			// one at a time
			for( u32 p = 0; p < playerCount; ++p )
			{
				const tPlayerPtr& player = players[ p ];
				tDynamicArray<tPlayerPtr> onePlayerArray;
				onePlayerArray.fResize( 1 );
				onePlayerArray[ 0 ] = player;

				tPlayer::fLockScoreAndStats( false, onePlayerArray );
			}
		}

		tGameApp::fInstance( ).fGameAppSession( )->fWriteLeaderboards( );

		tGameApp::fInstance( ).fWriteCachedLeaderboards( );

		for( u32 p = 0; p < playerCount; ++p )
		{
			const tPlayerPtr& player = players[ p ];
			fCheckAchievements( *player );
			if( player->fUser( )->fIsLocal( ) )
				player->fSaveProfile( );
		}

		sync_end( ); // We dont care about deterministic simulation after this.
	}
	void tLevelLogic::fSetMiniGameScore( u32 score, u32 extraMiniIndex, u32 leaderBoard )
	{
		// meaning, that this is a SetMiniGameScore called from a map that is not a minigame type (i.e. the trial)
		b32 extraMiniGame = ( mMapType != GameFlags::cMAP_TYPE_MINIGAME );
		sigassert( extraMiniGame ^ (extraMiniIndex == -1) );

		tPlayer* player = tGameApp::fInstance( ).fSingleScreenControlPlayer( );
		sigassert( player );

		player->fStats( ).fSetStat( GameFlags::cSESSION_STATS_SCORE, (f32)score );

		u32 realHighScore = score;

		if( extraMiniGame )
		{
			// For trial level, extra minigame scores are written into the associated minigame
			sigassert( extraMiniIndex < 2 );
			tLevelScores* scores = player->fProfile( )->fGetLevelScores( GameFlags::cMAP_TYPE_MINIGAME, extraMiniIndex );
			sigassert( scores );

			scores->fSetHighScore( 0, score ); //difficulty 0
			realHighScore = scores->fGetHighScore( 0 );
		}
		else
		{
			tLevelScores* scores = tGameApp::fInstance( ).fCurrentLevel( )->fLevelScores( player );
			sigassert( scores );

			scores->fSetHighScore( 0, score ); //difficulty 0
			realHighScore = scores->fGetHighScore( 0 );
		}

		tDynamicArray<tPlayerPtr> onePlayerArray;
		onePlayerArray.fResize( 1 );
		onePlayerArray[ 0 ].fReset( player );
		tPlayer::fLockScoreAndStats( extraMiniGame, onePlayerArray );

		// Post to LIVE
		tGameAppSession* session = tGameApp::fInstance( ).fGameAppSession( );

		{
			tGrowableArray<tUserProperty> levelProps;

			u32 lbScore = tGameSessionStats::fStatToLeaderBoard( GameFlags::cSESSION_STATS_MINIGAME_META_STAT, (f32)realHighScore );
			levelProps.fPushBack( tUserProperty( GameSession::cPropertyScore ) );
			levelProps.fBack( ).mData.fSet( ( s64 ) lbScore );

			if( !extraMiniGame )
			{
				tLevelScores* scores = tGameApp::fInstance( ).fCurrentLevel( )->fLevelScores( player );
				sigassert( scores );

				levelProps.fPushBack( tUserProperty( GameSession::cPropertyChallengeProgress ) );
				levelProps.fBack( ).mData.fSet( ( s64 ) scores->fRankProgress( ) );
			}

			session->fWriteStats( player, leaderBoard, levelProps.fCount( ), levelProps.fBegin( ) );
		}

		tGameApp& gameApp = tGameApp::fInstance( );

		// Kick off score writer for stats queued while the session was missing
		// If it fails, forget about the stats
		tGameApp::fInstance( ).fWriteCachedLeaderboards( );

		fCheckAchievements( *player );

		// Not saving profile immediately after mingame. violates tcr
		//if( player->fUser( )->fIsLocal( ) )
		//	player->fSaveProfile( );
	}
	void tLevelLogic::fLevelExited( )
	{
		if( !fVictoryOrDefeat( ) && !fIsDisplayCase( ) )
		{
			tDynamicArray<tPlayerPtr>& players = tGameApp::fInstance( ).fPlayers( );
			for( u32 i = 0; i < players.fCount( ); ++i )
				players[ i ]->fSaveProfile( );
		}
	}
	void tLevelLogic::fCheckAchievements( tPlayer& player )
	{
		sync_event_v_c( mMapType, tSync::cSCLevel );
		sync_event_v_c( mLevelNumber, tSync::cSCLevel );

		if( mMapType == GameFlags::cMAP_TYPE_CAMPAIGN 
			&& tGameApp::fInstance( ).fIsCampaignComplete( 0, &player ) )
			player.fAwardAvatarAward( GameFlags::cAVATAR_AWARDS_GAMER_PIC_BEAT );
	}
	void tLevelLogic::fOnTicketsCountChanged( u32 currentTickets, tPlayer* player )
	{
		if( fOwnerEntity( ) )
		{
			const tLogicPtr* logicPtr = fOwnerEntity( )->fLogicPtr( );
			if( !logicPtr )
				return;

			if( logicPtr->fIsCodeOwned( ) )
			{
				// TODO: Maybe implement the following
				//logicPtr->fCodeObject( )->fOnRegisterSpecialLevelObject( levelObj );
			}
			else
			{
				Sqrat::Function f( logicPtr->fScriptObject( ), "OnTicketsCountChanged" );
				if( !f.IsNull( ) )
					f.Execute( currentTickets, player );
				else
					log_warning( 0, "OnTicketsCountChanged doesn't exist." );
			}
		}
	}

	void tLevelLogic::fWaveListScriptCall( const tStringPtr& function )
	{
		if( fOwnerEntity( ) )
		{
			const tLogicPtr* logicPtr = fOwnerEntity( )->fLogicPtr( );
			if( !logicPtr )
				return;

			if( logicPtr->fIsCodeOwned( ) )
			{
				// TODO: Maybe implement the following
				//logicPtr->fCodeObject( )->fOnRegisterSpecialLevelObject( levelObj );
			}
			else
			{
				Sqrat::Function f( logicPtr->fScriptObject( ), function.fCStr( ) );
				if( !f.IsNull( ) )
					f.Execute( );
				else
					log_warning( 0, function << " doesn't exist." );
			}
		}
	}

	void tLevelLogic::fAddMiniGameTime( f32 time )
	{
		if( fOwnerEntity( ) )
		{
			const tLogicPtr* logicPtr = fOwnerEntity( )->fLogicPtr( );
			if( !logicPtr )
				return;

			if( logicPtr->fIsCodeOwned( ) )
			{
				// TODO: Maybe implement the following
				//logicPtr->fCodeObject( )->fAddMiniGameTime( levelObj );
			}
			else
			{
				Sqrat::Function f( logicPtr->fScriptObject( ), "AddMiniGameTime" );
				if( !f.IsNull( ) )
					f.Execute( time );
				else
					log_warning( 0, "AddMiniGameTime doesn't exist." );
			}
		}
	}

	void tLevelLogic::fHandleTutorialEventScript( u32 event )
	{
		fHandleTutorialEvent( tTutorialEvent( event ) );
	}

	void tLevelLogic::fHandleTutorialEvent( const tTutorialEvent& event )
	{
		profile( cProfilePerfTutorialEvents );
		tTutorialEvent* newEvent = NEW tTutorialEvent( );
		*newEvent = event;
		fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_TUTORIAL_EVENT, newEvent ) );

		//log_line( 0, "Tutorial Event: " << GameFlags::fTUTORIAL_EVENTEnumToValueString( event.mEventID ) );
	}

	void tLevelLogic::fSuspendTutorial( b32 suspend )
	{
		if( suspend )
		{
			if( !mTutorialSuspended )
			{
				fHandleTutorialEvent( tTutorialEvent( GameFlags::cTUTORIAL_EVENT_SUSPEND, 1 ) );
				mTutorialSuspended = true;
			}
		}
		else
		{
			if( mTutorialSuspended )
			{
				fHandleTutorialEvent( tTutorialEvent( GameFlags::cTUTORIAL_EVENT_SUSPEND, 0 ) );
				mTutorialSuspended = false;
			}
		}
	}

	b32 tLevelLogic::fHandleLogicEvent( const Logic::tEvent& e )
	{
		switch( e.fEventId( ) )
		{
		case GameFlags::cEVENT_GAME_EFFECT:
			{
				const tEffectLogicEventContext* context = e.fContext<tEffectLogicEventContext>( );
				if( context )
				{
					if( context->mAudio && context->mArgs.mSurfaceType != ~0 )
					{
						context->mAudio->fSetSwitch( tGameApp::cSurfaceTypeSwitchGroup, GameFlags::fSURFACE_TYPEEnumToValueString( context->mArgs.mSurfaceType ) );
					}
				}
				return true;
			}
			break;
		}

		const tLogicPtr* logicPtr = fOwnerEntity( )->fLogicPtr( );
		if( logicPtr )
		{
			Sqrat::Function f( logicPtr->fScriptObject( ), "HandleLogicEventScript" );
			if( !f.IsNull( ) )
				f.Execute( e );
			else
				log_warning( 0, "HandleLogicEventScript doesn't exist." );
		}

		return Logic::tGoalDriven::fHandleLogicEvent( this, e );
	}

	f32 tLevelLogic::fQueryCurrentUnitTimeScale( tUnitLogic& unitLogic ) const
	{
		return (!unitLogic.fUnderUserControl( )) ? mUnitTimeMultiplier : 1.f;
	}

	void tLevelLogic::fAddOffensiveWave( const tOffensiveWaveDesc& waveDesc )
	{
		if( !tGameApp::fInstance( ).fGameMode( ).fIsVersus( ) )
		{
			log_warning( 0, "Don't call this method (tLevelLogic::fAddOffensiveWave) when it's not Versus mode!" );
			return;
		}

		mVersusWaveManager->fAddOffensiveWave( waveDesc );
	}

	b32 tLevelLogic::fLaunchOffensiveWave( tOffensiveWaveDesc& waveDesc, tPlayer* player )
	{
		if( !tGameApp::fInstance( ).fGameMode( ).fIsVersus( ) )
		{
			log_warning( 0, "Don't call this method (tLevelLogic::fLaunchOffensiveWave) when it's not Versus mode!" );
			return false;
		}

		return mVersusWaveManager->fLaunchOffensiveWave( waveDesc, player );
	}

	void tLevelLogic::fPlatformPurchased( tEntity* platform )
	{
		if( fOwnerEntity( ) )
		{
			const tLogicPtr* logicPtr = fOwnerEntity( )->fLogicPtr( );
			if( !logicPtr )
				return;

			if( logicPtr->fIsCodeOwned( ) )
			{
				// TODO: Maybe implement the following
				//logicPtr->fCodeObject( )->fOnRegisterSpecialLevelObject( levelObj );
			}
			else
			{
				Sqrat::Function f( logicPtr->fScriptObject( ), "PlatformPurchased" );
				if( !f.IsNull( ) ) f.Execute( platform );
				else
					log_warning( 0, "PlatformPurchased" << " doesn't exist." );
			}
		}
	}

	void tLevelLogic::fExtraMiniGamePoints( f32 points )
	{
		if( fOwnerEntity( ) )
		{
			const tLogicPtr* logicPtr = fOwnerEntity( )->fLogicPtr( );
			if( !logicPtr )
				return;

			if( logicPtr->fIsCodeOwned( ) )
			{
				// TODO: Maybe implement the following
				//logicPtr->fCodeObject( )->fOnRegisterSpecialLevelObject( levelObj );
			}
			else
			{
				Sqrat::Function f( logicPtr->fScriptObject( ), "ExtraMiniGamePoints" );
				if( !f.IsNull( ) ) f.Execute( points );
				else
					log_warning( 0, "ExtraMiniGamePoints doesn't exist." );
			}
		}
	}

	void tLevelLogic::fUpdatePlatformGlow( u32 size )
	{
		if( size != mPreviousGlow && tGameApp::fInstance( ).fGameMode( ).fIsSinglePlayerOrCoop( ) )
		{
			b32 smallGlow = ( size == GameFlags::cBUILD_SITE_SMALL );
			b32 largeGlow = ( size == GameFlags::cBUILD_SITE_SMALL || size == GameFlags::cBUILD_SITE_LARGE );

			for( u32 i = 0; i < mBuildSiteRootsSmall.fCount( ); ++i )
			{
				b32 show = !mLightUpOnlyTutorialPlatforms || mBuildSiteRootsSmall[ i ]->fQueryEnumValue( GameFlags::cENUM_PLATFORM_TYPE ) == GameFlags::cPLATFORM_TYPE_TUTORIAL;

				tBuildSiteLogic* logic = mBuildSiteRootsSmall[ i ]->fLogicDerived<tBuildSiteLogic>( );
				sigassert( logic );
				logic->fSetSelectionOverride( show && smallGlow && !logic->fIsOccupied( ) );
			}

			for( u32 i = 0; i < mBuildSiteRootsLarge.fCount( ); ++i )
			{
				b32 show = !mLightUpOnlyTutorialPlatforms || mBuildSiteRootsLarge[ i ]->fQueryEnumValue( GameFlags::cENUM_PLATFORM_TYPE ) == GameFlags::cPLATFORM_TYPE_TUTORIAL;

				tBuildSiteLogic* logic = mBuildSiteRootsLarge[ i ]->fLogicDerived<tBuildSiteLogic>( );
				sigassert( logic );
				logic->fSetSelectionOverride( show && largeGlow && !logic->fIsOccupied( ) );
			}

			mPreviousGlow = size;
		}
	}

	tEntity* tLevelLogic::fPlayerSpawn( const tStringPtr& name ) const
	{
		for( u32 i = 0; i < mPlayerSpawn.fCount( ); ++i )
			if( mPlayerSpawn[ i ]->fName( ) == name )
				return mPlayerSpawn[ i ].fGetRawPtr( );

		return NULL;
	}

	namespace
	{
		enum tFocusTableColumns
		{
			cFocusColumnLocKey,
			cFocusColumnLocKeyEnemy,
			cFocusColumnPromptDelay,
			cFocusColumnPromptDuration,
			cFocusColumnBlendDist
		};

		static const tStringPtr cSpawnPt( "spawnPt" );
		static const tStringPtr cPivotPt( "pivotPt" );
		static const tStringPtr cBackDrop( "backDrop" );
		static const tStringPtr cUSAFlag( "usaFLAG" );
		static const tStringPtr cUSSRFlag( "ussrFLAG" );

		u32 fTurretCount( )
		{
			u32 start = GameFlags::cUNIT_ID_TURRET_MG_01;
			u32 end = GameFlags::cUNIT_ID_TURRET_FLAME_03;
			u32 count = end - start + 1;
			return count;
		}
	}

	tMat3f tLevelLogic::fMakeTurretMatrix( u32 index ) const
	{
		tMat3f rotate( tQuatf( tAxisAnglef( tVec3f::cYAxis, mWorldAngleRate * index ) ) );
		return rotate * mPivotToSpawn * tMat3f( tQuatf( tAxisAnglef( tVec3f::cYAxis, cPi ) ) );
	}

	void tLevelLogic::fGamePurchased( )
	{
		if( tGameApp::fInstance( ).fIsFullVersion( ) )
		{
			Logic::tEvent e = Logic::tEvent( ApplicationEvent::cOnUpgradeToFullVersion );
			fRootMenu( ).fHandleCanvasEvent( e );
			tGameApp::fInstance( ).fRootHudCanvas( ).fHandleCanvasEvent( e );
		}
	}

	void tLevelLogic::fSpawnShowcaseTurrets( )
	{
		for( u32 i = 0; i < mTurrets.fCount( ); ++i )
		{
			mTurrets[ i ]->fOwnerEntity( )->fDelete( );
			mTurretTargets[ i ]->fDelete( );
		}

		mTurrets.fSetCount( 0 );
		mTurretTargets.fSetCount( 0 );

		tEntity* spawnPt = fNamedObject( cSpawnPt );
		sigassert( spawnPt && "Need pt named: spawnPt" );
		tEntity* pivotPt = fNamedObject( cPivotPt );
		sigassert( pivotPt && "Need pt named: pivotPt" );
		mBackDrop.fReset( fNamedObject( cBackDrop ) );
		sigassert( mBackDrop && "Need backdrop named: backDrop" );
		mUSAFlag.fReset( fNamedObject( cUSAFlag ) );
		sigassert( mUSAFlag && "Need backdrop named: usaFLAG" );
		mUSSRFlag.fReset( fNamedObject( cUSSRFlag ) );
		sigassert( mUSSRFlag && "Need backdrop named: ussrFLAG" );

		mPivotToSpawn = pivotPt->fWorldToObject( ) * spawnPt->fObjectToWorld( );

		u32 start = GameFlags::cUNIT_ID_TURRET_MG_01;
		u32 count = fTurretCount( );
		mWorldAngleRate = c2Pi / count;

		mTurrets.fSetCount( count );
		mTurretTargets.fSetCount( count );

		for( u32 i = 0; i < count; ++i )
		{
			tEntity* ent = pivotPt->fSpawnChild( tGameApp::fInstance( ).fUnitResourcePath( (GameFlags::tUNIT_ID)(start + i), mDisplayCaseCountry ) );

			tMat3f xform = fMakeTurretMatrix( i );
			xform = xform * tMat3f( tQuatf( tAxisAnglef( tVec3f::cYAxis, mCameraAngle ) ) );

			ent->fSetParentRelativeXform( xform );
			ent->fAddGameTags( GameFlags::cFLAG_SELECTABLE );

			mTurretTargets[ i ].fReset( NEW tEntity( ) );
			mTurretTargets[ i ]->fSpawn( *pivotPt );

			xform.fTranslateLocal( tVec3f( 0,0,100 ) );
			mTurretTargets[ i ]->fSetParentRelativeXform( xform );

			tTurretLogic* turret = ent->fLogicDerived<tTurretLogic>( );
			sigassert( turret );
			mTurrets[ i ] = turret;

			turret->fSetIdleTarget( mTurretTargets[ i ].fGetRawPtr( ) );
			turret->fSetTakesDamage( false );
			ent->fRemoveGameTagsRecursive( GameFlags::cFLAG_GROUND );
		}

		mBackDropCamera = tGameApp::fInstance( ).fFrontEndPlayer( )->fCameraStack( ).fFindCameraOfType<tRtsCamera>( );
		sigassert( mBackDropCamera );

		mOriginalDir = spawnPt->fObjectToWorld( ).fZAxis( );
		fApplyAngles( );
	}

	void tLevelLogic::fToggleDisplayCaseCountry( )
	{
		if( mDisplayCaseCountry == GameFlags::cCOUNTRY_USA )
			mDisplayCaseCountry = GameFlags::cCOUNTRY_USSR;
		else
			mDisplayCaseCountry = GameFlags::cCOUNTRY_USA;

		// units dont change so dont change the player
		//mBackDropCamera->fPlayer( ).fSetCountry( mDisplayCaseCountry );
		mWaitOneFrameToSwitchCountry = 2;
		fSpawnShowcaseTurrets( );
	}

	void tLevelLogic::fIncCameraAngle( f32 angle )
	{
		mCameraAngle += angle;
	}

	void tLevelLogic::fIncWorldAngle( f32 angle )
	{
		mWorldAngle += (s32)angle;
		mWorldAngle = fModulus( mWorldAngle, (s32)fTurretCount( ) );
	}

	void tLevelLogic::fApplyAngles( )
	{
		f32 worldAngle = mAngleDamper.fValue( );
		tMat3f rot( tQuatf( tAxisAnglef( tVec3f::cYAxis, -worldAngle ) ) );
		tQuatf rot2( tAxisAnglef( tVec3f::cYAxis, /*mCameraAngle */- worldAngle ) );

		tQuatf camRot( tAxisAnglef( tVec3f::cYAxis, mCameraAngle ) );

		mBackDropCamera->fLockPosition( rot.fXformPoint( mPivotToSpawn.fGetTranslation( ) ), rot2.fRotate( mOriginalDir ) );
		sigassert( mBackDrop );
		mBackDrop->fMoveTo( rot );

		u32 count = fTurretCount( );
		for( u32 i = 0; i < count; ++i )
		{
			tMat3f xform = fMakeTurretMatrix( i );	
			xform.fTranslateLocal( camRot.fRotate( tVec3f( 0,0,100 ) ) );
			mTurretTargets[ i ]->fSetParentRelativeXform( xform );
		}

		if( mWaitOneFrameToSwitchCountry != ~0 && --mWaitOneFrameToSwitchCountry == 0 )
		{
			mWaitOneFrameToSwitchCountry = ~0;

			Gfx::tRenderableEntity::fSetDisabled( *mUSAFlag, mDisplayCaseCountry == GameFlags::cCOUNTRY_USSR );
			Gfx::tRenderableEntity::fSetDisabled( *mUSSRFlag, mDisplayCaseCountry == GameFlags::cCOUNTRY_USA );
		}
	}

	b32 tLevelLogic::fOverDisplayCaseTurret( ) const
	{
		f32 error = fAbs( fShortestWayAround( mAngleDamper.fValue( ), mWorldAngleRate * mWorldAngle ) );
		f32 thresh = fToRadians( 5.f );
		return (error < thresh);
	}

	b32 tLevelLogic::fCanExitDisplayCase( )
	{
		tPlayer* p = tGameApp::fInstance( ).fFrontEndPlayer( );
		tRtsCamera* camera = p->fCameraStack( ).fFindCameraOfType<tRtsCamera>( );
		return camera->fIsActive( ) && camera->fHasBlendedIn( );
	}

	void tLevelLogic::fIncRankProgress( tPlayer* player, s32 val )
	{
		sigassert( player );
		if( player->fIsNotAllowedToSaveStats( ) )
			return;

		tUserProfile* profile = player->fProfile( );
		sigassert( profile );

		tLevelScores* scores = profile->fGetLevelScores( mMapType, mLevelNumber );
		sigassert( scores );

		s32 earnedRank = scores->fIncRankProgress( val );

		// Earned a Rank
		if( earnedRank > -1 )
		{
			if( fRationTicketUI( ) )
				fRationTicketUI( )->fAwardNewRank( earnedRank, player->fUser( ).fGetRawPtr( ) );
			player->fAddEarnedItem( tEarnedItemData( tEarnedItemData::cRank, earnedRank ) );

			player->fStats( ).fIncStat( GameFlags::cSESSION_STATS_TOTAL_CHALLENGES_EARNED, 1.f );

			// Check reached 2nd rank in all base minigames
			if( mMapType == GameFlags::cMAP_TYPE_MINIGAME && earnedRank >= 1 ) //2nd rank
			{
				b32 achieved = true;

				u32 rowCount = tGameApp::fInstance( ).fNumLevelsInTable( GameFlags::cMAP_TYPE_MINIGAME );
				for( u32 i = 0; i < rowCount; ++i )
				{
					if( tGameApp::fInstance( ).fLevelDLCInTable( GameFlags::cMAP_TYPE_MINIGAME, i ) == 0 )
					{
						tLevelScores* miniScores = profile->fGetLevelScores( GameFlags::cMAP_TYPE_MINIGAME, i );
						sigassert( miniScores );
						if( miniScores->fRankCount( ) < 1 )
							achieved = false;						
					}
				}

				if( achieved )
					player->fAwardAchievement( GameFlags::cACHIEVEMENTS_PERSEVERANCE );
			}
		}
	}

	void tLevelLogic::fFocusTarget( tEntity* entity, const tStringPtr& key )
	{
		sigassert( entity );

		const u32 country = entity->fQueryEnumValue( GameFlags::cENUM_COUNTRY );

		const tStringHashDataTable& dt = tGameApp::fInstance( ).fFocusItemsTable( );
		
		u32 index = dt.fRowIndex( key );
		if( index != ~0 )
		{
			tStringPtr locKey  = dt.fIndexByRowCol<tStringPtr>( index, cFocusColumnLocKey );
			tStringPtr locKeyEnemy  = dt.fIndexByRowCol<tStringPtr>( index, cFocusColumnLocKeyEnemy );
			f32 promptDelay    = dt.fIndexByRowCol<f32>( index, cFocusColumnPromptDelay );
			f32 promptDuration = dt.fIndexByRowCol<f32>( index, cFocusColumnPromptDuration );
			f32 blendDist	   = dt.fIndexByRowCol<f32>( index, cFocusColumnBlendDist );

			tFocusItem item( entity, tStringPtr::cNullPtr, promptDelay, promptDuration, blendDist );

			const tDynamicArray<tPlayerPtr>& players = tGameApp::fInstance( ).fPlayers( );
			for( u32 i = 0; i < players.fCount( ); ++i )
			{
				if( country == players[ i ]->fCountry( ) )
					item.mMessage = locKey;
				else
					item.mMessage = locKeyEnemy;

				players[ i ]->fFocusTarget( item );
			}
		}
	}

	void tLevelLogic::fAwardDecoration( u32 index )
	{
		tGameApp & gameApp = tGameApp::fInstance( );

		const u32 playerCount = gameApp.fPlayerCount( );
		for( u32 p = 0; p < playerCount; ++p ) // for now
		{
			tPlayer* player = gameApp.fGetPlayer( p );
			sigassert( player );

			tLevelScores* scores = fLevelScores( player );
			sigassert( scores );

			if( player->fIsNotAllowedToSaveStats( ) )
				continue;

			//index is level specific. expect 0 or 1 for first and second ration ticket from ration ticket doc.
			if( !scores->fIsGoalComplete( index ) )
			{
				scores->fSetGoalComplete( index, true );

				if( fRationTicketUI( ) )
					fRationTicketUI( )->fAwardRationTicket( index, player->fUser( ).fGetRawPtr( ) );

				player->fAddEarnedItem( tEarnedItemData( tEarnedItemData::cDecoration, index ) );

				player->fStats( ).fIncStat( GameFlags::cSESSION_STATS_DECORATIONS_ACQUIRED, 1.f );

				//calculate total number of decorations
				u32 totalDecorations = 0;
				const u32 numLevels = gameApp.fNumLevelsInTable( GameFlags::cMAP_TYPE_CAMPAIGN );
				for( u32 i = 0; i < numLevels; ++i )
				{
					tLevelLoadInfo info = gameApp.fLevelLoadInfo( GameFlags::cMAP_TYPE_CAMPAIGN, i );
					if( info.mDlcNumber == fDlcNumber( ) )
					{
						sigassert( player->fProfile( ) );
						tLevelScores* scores = player->fProfile( )->fGetLevelScores( GameFlags::cMAP_TYPE_CAMPAIGN, info.mLevelIndex );
						if( scores )
						{
							if( scores->fIsGoalComplete( 0 ) )
								++totalDecorations;
							if( scores->fIsGoalComplete( 1 ) )
								++totalDecorations;
						}
					}
				}
				
				if( !player->fAchievementAwarded( GameFlags::cACHIEVEMENTS_WITH_DISTINCTION ) )
				{
					if( totalDecorations >= 11 )
						player->fAwardAchievement( GameFlags::cACHIEVEMENTS_WITH_DISTINCTION );
				}

				if( !player->fAchievementAwarded( GameFlags::cACHIEVEMENTS_HIGHLY_DECORATED ) )
				{
					if( totalDecorations >= 22 )
						player->fAwardAchievement( GameFlags::cACHIEVEMENTS_HIGHLY_DECORATED );
				}
			}
		}
	}

	void tLevelLogic::fFailDecoration( u32 index )
	{
		tGameApp & gameApp = tGameApp::fInstance( );

		const u32 playerCount = gameApp.fPlayerCount( );
		for( u32 p = 0; p < playerCount; ++p ) // for now
		{
			tPlayer* player = gameApp.fGetPlayer( p );
			sigassert( player );

			tLevelScores* scores = fLevelScores( player );
			sigassert( scores );

			if( player->fIsNotAllowedToSaveStats( ) )
				continue;

			if( fDecorationActive( index ) )
			{
				mRationTickets[ p ][ index ].mFailed = 1;
				log_line( 0, " Ration ticket failed: " << index );
				// Edit for DR: Don't show failed status
				//if( fRationTicketUI( ) )
					//fRationTicketUI( )->fFailRationTicket( index, player->fUser( ).fGetRawPtr( ) );
			}
		}
	}

	b32 tLevelLogic::fDecorationActive( u32 index )
	{
		b32 isActive = false;
		tGameApp & gameApp = tGameApp::fInstance( );

		const u32 playerCount = gameApp.fPlayerCount( );
		for( u32 p = 0; p < playerCount; ++p ) // for now
		{
			tPlayer* player = gameApp.fGetPlayer( p );
			sigassert( player );

			tLevelScores* scores = fLevelScores( player );
			sigassert( scores );

			if( player->fIsNotAllowedToSaveStats( ) )
				continue;

			isActive = isActive || ( !mRationTickets[ p ][ index ].mFailed && !scores->fIsGoalComplete( index ) );
		}

		return isActive;
	}

	b32 tLevelLogic::fDecorationHasProgress( u32 index, tUser* user ) const
	{
		sigassert( user );
		u32 playerIndex = tGameApp::fInstance( ).fWhichPlayer( user );
		sigassert( playerIndex != ~0 );

		return mRationTickets[ playerIndex ][ index ].mHasProgress;
	}

	void tLevelLogic::fDecorationProgress( u32 index, f32 progress, f32 max )
	{
		tGameApp & gameApp = tGameApp::fInstance( );

		const u32 playerCount = gameApp.fPlayerCount( );
		for( u32 p = 0; p < playerCount; ++p ) // for now
		{
			tPlayer* player = gameApp.fGetPlayer( p );
			sigassert( player );

			tLevelScores* scores = fLevelScores( player );
			sigassert( scores );

			if( player->fIsNotAllowedToSaveStats( ) )
				continue;

			if( !scores->fIsGoalComplete( index ) )
			{
				mRationTickets[ p ][ index ].mHasProgress = 1;
				mRationTickets[ p ][ index ].mCurrent = progress;
				mRationTickets[ p ][ index ].mMax = max;
			}

			if( progress > 0 && !scores->fIsGoalComplete( index ) && fRationTicketUI( ) )
			{
				fRationTicketUI( )->fRationTicketProgress( index, progress, max, player->fUser( ).fGetRawPtr( ) );
			}
		}
	}

	Sqrat::Object tLevelLogic::fDecorationStatus( u32 index, tUser* user )
	{
		sigassert( user );
		
		u32 playerIndex = tGameApp::fInstance( ).fWhichPlayer( user );
		sigassert( playerIndex != ~0 );

		tPlayer* player = tGameApp::fInstance( ).fGetPlayer( playerIndex ); // for now
		sigassert( player );

		if( player->fIsNotAllowedToSaveStats( ) )
			return Sqrat::Object( );

		const tRationTicketData& data = mRationTickets[ playerIndex ][ index ];
		Sqrat::Table status;

		tLevelScores* scores = fLevelScores( player );
		sigassert( scores );

		status.SetValue( _SC("failed"), data.mFailed );
		status.SetValue( _SC("complete"), scores->fIsGoalComplete( index ) );
		status.SetValue( _SC("hasProgress"), data.mHasProgress ); 
		if( data.mHasProgress )
		{
			status.SetValue<u32>( _SC("progress"), (u32)data.mCurrent );
			status.SetValue<u32>( _SC("max"), (u32)data.mMax );
		}

		return status;
	}

	void tLevelLogic::fImpactText( const tStringPtr& locID, tPlayer& player )
	{
		if( fOwnerEntity( ) )
		{
			const tLogicPtr* logicPtr = fOwnerEntity( )->fLogicPtr( );
			if( !logicPtr )
				return;

			if( logicPtr->fIsCodeOwned( ) )
			{
				// TODO: Maybe implement the following
				//logicPtr->fCodeObject( )->fOnRegisterSpecialLevelObject( levelObj );
			}
			else
			{
				Sqrat::Function f( logicPtr->fScriptObject( ), "ImpactText" );
				if( !f.IsNull( ) ) f.Execute( locID, &player );
				else
					log_warning( 0, "ImpactText doesn't exist." );
			}
		}
	}

	void tLevelLogic::fIncCoopAssists( )
	{
		++mCoOpAssists;
		if( mCoOpAssists == 10 )
			tGameApp::fInstance( ).fAwardAchievementToAllPlayers( GameFlags::cACHIEVEMENTS_IN_SYNC );
	}

}


namespace Sig
{
	namespace
	{
		tEntity* fRespawnEntity( tEntity* entity )
		{
			sigassert( entity );
			tSceneRefEntity* ent = entity->fDynamicCast<tSceneRefEntity>( );
			if( !ent ) 
				return NULL;

			tEntity* newEnt = ent->fParent( )->fSpawnChildFromProxy( ent->fSgResource( )->fGetPath( ), ent );	

			tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
			level->fRemoveNamedObject( ent->fName( ) );
			level->fRegisterNamedObject( newEnt );

			ent->fDelete( );

			return newEnt;
		}
	}

	void tLevelLogic::fExportScriptInterface( tScriptVm& vm )
	{
		{
			Sqrat::Class<tTutorialEvent, Sqrat::DefaultAllocator<tTutorialEvent> > classDesc( vm.fSq( ) );
			classDesc
				.Var(_SC("EventID"),			&tTutorialEvent::mEventID)
				.Var(_SC("EventValue"),			&tTutorialEvent::mEventValue)
				.Var(_SC("EventUnitType"),		&tTutorialEvent::mEventUnitType)
				.Var(_SC("CurrentUnitID"),		&tTutorialEvent::mCurrentUnitID)
				.Var(_SC("ShellCaming"),		&tTutorialEvent::mShellCaming)
				.Var(_SC("Entity"),				&tTutorialEvent::mEntity)
				.Var(_SC("Player"),				&tTutorialEvent::mPlayer)
				.Var(_SC("PlatformName"),		&tTutorialEvent::mPlatformName)
				.Var(_SC("WeaponID"),			&tTutorialEvent::mWeaponID)
				.Var(_SC("Combo"),				&tTutorialEvent::mCombo)
				.StaticFunc(_SC("Construct"),	&tTutorialEvent::fConstruct)
				.StaticFunc(_SC("Convert"),		&tTutorialEvent::fConvert)
				.Func(_SC("Compare"),			&tTutorialEvent::fCompare)
				.Func(_SC("CheckFlag"),			&tTutorialEvent::fCheckFlag)
				.Var(_SC("SpeedBonus"),			&tTutorialEvent::mExtra)
				.Var(_SC("PlayerKillerLogic"),	&tTutorialEvent::mPlayerKillerLogic)
				;

			vm.fRootTable( ).Bind(_SC("TutorialEvent"), classDesc);
		}
		{
			Sqrat::DerivedClass<tLevelLogic, tLogic, Sqrat::NoCopy<tLevelLogic> > classDesc( vm.fSq( ) );
			classDesc
				.Func(_SC("SetRootMenu"),				&tLevelLogic::fSetRootMenuFromScript)
				.Func(_SC("GetRootMenu"),				&tLevelLogic::fGetRootMenuFromScript)
				.Func(_SC("RegisterCameraPoint"),		&tLevelLogic::fRegisterCameraPoint)
				.Func(_SC("CameraPointCount"),			&tLevelLogic::fCameraPointCount)
				.Func(_SC("GetCameraPoint"),			&tLevelLogic::fCameraPoint)
				.Func(_SC("SetCameraBox"),				&tLevelLogic::fRegisterCameraBox)
				.Func(_SC("RegisterCameraBox"),			&tLevelLogic::fRegisterCameraBox)
				.Func(_SC("RegisterBombDropperBox"),	&tLevelLogic::fRegisterBombDropperBox)
				.Func(_SC("RegisterCameraBlocker"),		&tLevelLogic::fRegisterCameraBlocker)
				.Func(_SC("RegisterCameraGround"),		&tLevelLogic::fRegisterCameraGround)
				.Func(_SC("RegisterBuildSiteSmall"),	&tLevelLogic::fRegisterBuildSiteSmall)
				.Func(_SC("RegisterBuildSiteLarge"),	&tLevelLogic::fRegisterBuildSiteLarge)
				.Func(_SC("BuildSiteNamed"),			&tLevelLogic::fBuildSiteNamed)				
				.Func(_SC("RegisterPathStart"),			&tLevelLogic::fRegisterPathStart)
				.Func(_SC("RegisterTrenchStart"),		&tLevelLogic::fRegisterTrenchStart)
				.Func(_SC("RegisterExitGenStart"),		&tLevelLogic::fRegisterExitGenStart)
				.Func(_SC("RegisterContextPathStart"),	&tLevelLogic::fRegisterContextPathStart)
				.Func(_SC("RegisterFlightPathStart"),	&tLevelLogic::fRegisterFlightPathStart)
				.Func(_SC("RegisterNamedObject"),		&tLevelLogic::fRegisterNamedObject)
				.Func(_SC("RegisterAirSpace"),			&tLevelLogic::fRegisterAirSpace)
				.Func(_SC("NamedObject"),				&tLevelLogic::fNamedObject)
				.Func(_SC("NamedPathPoint"),			&tLevelLogic::fNamedPathPoint)
				.Func(_SC("RegisterGenerator"),			&tLevelLogic::fRegisterGenerator)
				.Func(_SC("RegisterGoalBox"),			&tLevelLogic::fRegisterGoalBox)
				.Func(_SC("PushStandardCameras"),		&tLevelLogic::fPushStandardCameras)
				.Func(_SC("AddWaveList"),				&tLevelLogic::fAddWaveList)
				.Func(_SC("WaveList"),					&tLevelLogic::fWaveList)
				.Func(_SC("WaveListCount"),				&tLevelLogic::fWaveListCount)
				.Func(_SC("SetUIWaveList"),				&tLevelLogic::fSetUIWaveList)
				.Func(_SC("SetPlayerMoney"),			&tLevelLogic::fSetPlayerMoney)
				.Func(_SC("AddArtilleryBarragePt"),		&tLevelLogic::fAddArtilleryBarragePt)
				.Func(_SC("AddArtilleryBarrageSpawnPt"),&tLevelLogic::fAddArtilleryBarrageSpawnPt)
				.Func(_SC("AddBarrageDropPt"),			&tLevelLogic::fAddBarrageDropPt)
				.Func(_SC("RegisterCameraPathStart"),	&tLevelLogic::fRegisterCameraPathStart)
				.Func(_SC("SetCurrentLevelProgression"),&tLevelLogic::fSetCurrentLevelProgression)
				.Prop(_SC("VictoryOrDefeat"),			&tLevelLogic::fVictoryOrDefeat)
				.Func(_SC("SetMiniGameScore"),			&tLevelLogic::fSetMiniGameScore)
				.Prop(_SC("GroundHeight"),				&tLevelLogic::fGroundHeight, &tLevelLogic::fSetGroundHeight)
				.Func(_SC("AddOffensiveWave"),			&tLevelLogic::fAddOffensiveWave)
				.Func(_SC("LaunchOffensiveWave"),		&tLevelLogic::fLaunchOffensiveWave)
				.Func(_SC("RegisterToyBoxAlarm"),		&tLevelLogic::fRegisterToyBoxAlarm)
				.Func(_SC("QuitNetGameEarly"),			&tLevelLogic::fQuitNetGameEarly)
				.Prop(_SC("DefeatedPlayer"),			&tLevelLogic::fDefeatedPlayer, &tLevelLogic::fSetDefeatedPlayer)
				.Prop(_SC("VictoriousPlayer"),			&tLevelLogic::fVictoriousPlayer, &tLevelLogic::fSetVictoriousPlayer)
				.Func(_SC("IsVictorious"),				&tLevelLogic::fIsVictorious)
				.Func(_SC("RegisterPlayerSpawn"),		&tLevelLogic::fRegisterPlayerSpawn)
				.Prop(_SC("LevelNumber"),				&tLevelLogic::fLevelNumber)
				.Prop(_SC("DlcNumber"),					&tLevelLogic::fDlcNumber)
				.Prop(_SC("MapType"),					&tLevelLogic::fMapType)
				.Prop(_SC("ExtraMode"),					&tLevelLogic::fExtraMode)
				.StaticFunc(_SC("RespawnEntity"),		&fRespawnEntity)
				.Func(_SC("FocusTarget"),				&tLevelLogic::fFocusTarget)
				.Func(_SC("DisplayedWaveList"),			&tLevelLogic::fDisplayedWaveList)
				.Func(_SC("CurrentOrLastDisplayedWaveList"), &tLevelLogic::fCurrentOrLastDisplayedWaveList)
				.Prop(_SC("IsTrial"),					&tLevelLogic::fIsTrial)
				.Func(_SC("ShowEnemiesAliveList"),		&tLevelLogic::fShowEnemiesAliveList)
				.Func(_SC("ShowWaveListUI"),			&tLevelLogic::fShowWaveListUI)

				// tutorial stuff
				.Var(_SC("NeutralAudioTimer"),			&tLevelLogic::mNeutralTimerTime)
				.Func(_SC("HandleTutorialEvent"),		&tLevelLogic::fHandleTutorialEventScript)
				.Func(_SC("HandleTutorialEventObj"),	&tLevelLogic::fHandleTutorialEvent)
				.Var(_SC("TutUnitTimeMultiplier"),		&tLevelLogic::mUnitTimeMultiplier)
				.Var(_SC("TutOnlyPlaceThisUnit"),		&tLevelLogic::mOnlyPlaceThisUnit)
				.Var(_SC("TutOnlyPlaceSmallUnits"),		&tLevelLogic::mOnlyPlaceSmallUnits)
				.Prop(_SC("TutDisableSell"),			&tLevelLogic::fDisableSell, &tLevelLogic::fSetDisableSell)
				.Prop(_SC("TutDisableRepair"),			&tLevelLogic::fDisableRepair, &tLevelLogic::fSetDisableRepair)
				.Prop(_SC("TutDisableUpgrade"),			&tLevelLogic::fDisableUpgrade, &tLevelLogic::fSetDisableUpgrade)
				.Prop(_SC("TutDisableUse"),				&tLevelLogic::fDisableUse, &tLevelLogic::fSetDisableUse)		
				.Prop(_SC("TurretMenuChanged"),			&tLevelLogic::fTurretMenuChanged)			
				.Var(_SC("TutFreeUpgrades"),			&tLevelLogic::mFreeUpgrades)
				.Var(_SC("TutHideKillHoverText"),		&tLevelLogic::mHideKillHoverText)
				.Var(_SC("TutDisableBarrage"),			&tLevelLogic::mDisableBarrage)
				.Var(_SC("TutDisableOvercharge"),		&tLevelLogic::mDisableOvercharge)
				.Var(_SC("TutForceWeaponOvercharge"),	&tLevelLogic::mForceWeaponOvercharge)
				.Var(_SC("TutDisableVehicleRespawn"),	&tLevelLogic::mDisableVehicleRespawn)
				.Var(_SC("TutEnablePlatformLocking"),	&tLevelLogic::mEnablePlatformLocking)
				.Var(_SC("TutDisableLaunchArrows"),		&tLevelLogic::mDisableLaunchArrows)				
				.Var(_SC("TutLockPlaceMenuUpUntilBuild"),&tLevelLogic::mLockPlaceMenuUpUntilBuild)
				.Var(_SC("TutDisableRewind"),			&tLevelLogic::mDisableRewind)
				.Var(_SC("TutAllowCoopTurrets"),		&tLevelLogic::mAllowCoopTurrets)
				.Var(_SC("TutDisablePlaceMenu"),		&tLevelLogic::mDisablePlaceMenu)
				.Var(_SC("TutDisableVehicleInput"),		&tLevelLogic::mDisableVehicleInput)
				.Var(_SC("TutSaveFirstWave"),			&tLevelLogic::mSaveFirstWave)
				.Var(_SC("TutDisableRandomPickups"),	&tLevelLogic::mDisableRandomPickups)
				.Var(_SC("TutContinuousCombo"),			&tLevelLogic::mContinuousCombo)		
				.Var(_SC("TutDisablePropMoney"),		&tLevelLogic::mDisablePropMoney)
				.Var(_SC("DisableOverkills"),			&tLevelLogic::mDisableOverkills)
				.Var(_SC("UseMinigameScoring"),			&tLevelLogic::mUseMinigameScoring)
				.Var(_SC("TutDisableQuickSwitch"),		&tLevelLogic::mDisableQuickSwitch)
				.Var(_SC("TutLightUpOnlyTutorialPlatforms"), &tLevelLogic::mLightUpOnlyTutorialPlatforms)
				.Var(_SC("TutAlwaysShowUnitControls"),	&tLevelLogic::mAlwaysShowUnitControls)
				.Var(_SC("TutDisableBonusHoverText"),	&tLevelLogic::mDisableBonusHoverText)
				.Var(_SC("TutDisableComboText"),		&tLevelLogic::mDisableComboText)
				.Var(_SC("TutAllowSpeedBonus"),			&tLevelLogic::mAllowSpeedBonus)
				.Var(_SC("TutDisableRTSRings"),			&tLevelLogic::mDisableRTSRings)
				.Var(_SC("TutAllyLaunchArrows"),		&tLevelLogic::mAllyLaunchArrows)
				

				
				
				
				.Var(_SC("IsDisplayCase"),				&tLevelLogic::mIsDisplayCase)
				.Prop(_SC("CanExitDisplayCase"),		&tLevelLogic::fCanExitDisplayCase)
				
				
				.Prop(_SC("MiniGameTime"),				&tLevelLogic::fMiniGameTime, &tLevelLogic::fSetMiniGameTime)
				.Prop(_SC("MiniGameCurrentScore"),		&tLevelLogic::fMiniGameCurrentScore, &tLevelLogic::fSetMiniGameCurrentScore)
				
				.Func(_SC("AwardRationTicket"),			&tLevelLogic::fAwardDecoration)
				.Func(_SC("FailRationTicket"),			&tLevelLogic::fFailDecoration)
				.Func(_SC("RationTicketProgress"),		&tLevelLogic::fDecorationProgress)
				.Func(_SC("RationTicketHasProgress"),	&tLevelLogic::fDecorationHasProgress)
				.Func(_SC("RationTicketActive"),		&tLevelLogic::fDecorationActive)
				.Func(_SC("RationTicketStatus"),		&tLevelLogic::fDecorationStatus)
				.Var(_SC("PlatformPrice"),				&tLevelLogic::mPlatformPrice)
				.Var(_SC("SurvivalLevelTime"),			&tLevelLogic::mSurvivalLevelTime)
				.Func(_SC("AllowDebugSkip"),			&tLevelLogic::fAllowTutorialDebugSkip)
				.Func(_SC("SpawnShowcaseTurrets"),		&tLevelLogic::fSpawnShowcaseTurrets)
				.Func(_SC("ToggleDisplayCaseCountry"),	&tLevelLogic::fToggleDisplayCaseCountry)
				.Func(_SC("GetAllTurretData"),			&tLevelLogic::fGetAllTurretData)
				.Func(_SC("IncRankProgress"),			&tLevelLogic::fIncRankProgress)
				.Func(_SC("NukeGloablAudioState"),		&tLevelLogic::fNukeGloablAudioState)

				.Prop(_SC("RationTicketUI"),			&tLevelLogic::fRationTicketUIScript)
				
				;

			vm.fRootTable( ).Bind(_SC("LevelLogic"), classDesc);
		}

		vm.fConstTable( ).Const( _SC("LEVEL_PROGRESSION_INTRO"), cLevelProgressionIntro );
		vm.fConstTable( ).Const( _SC("LEVEL_PROGRESSION_PLAYING"), cLevelProgressionPlaying );
		vm.fConstTable( ).Const( _SC("LEVEL_PROGRESSION_MINIGAME"), cLevelProgressionMinigame );
		vm.fConstTable( ).Const( _SC("LEVEL_PROGRESSION_BOSS"), cLevelProgressionStageBoss );
		vm.fConstTable( ).Const( _SC("LEVEL_PROGRESSION_VICTORY"), cLevelProgressionStageVictory );
		vm.fConstTable( ).Const( _SC("LEVEL_PROGRESSION_DEFEAT"), cLevelProgressionStageDefeat );
	}
}
