#include "GameAppPch.hpp"
#include "tGameApp.hpp"
#include "FileSystem.hpp"
#include "tFilePackageFile.hpp"
#include "tGameLoadAppState.hpp"
#include "Scripts/tScriptFile.hpp"
#include "Gfx/tDefaultAllocators.hpp"
#include "Gfx/tGroundCoverCloud.hpp"
#include "Threads/tThread.hpp"
#include "tSync.hpp"
#include "tVoice.hpp"

// misc gameplay stuff
#include "tPlayer.hpp"
#include "tSaveGame.hpp"
#include "tDecalManager.hpp"
#include "ContextAnim.hpp"
#include "tWaveList.hpp"
#include "tBarrage.hpp"
#include "tBarrageImp.hpp"
#include "tGameEffects.hpp"
#include "tGamePostEffectMgr.hpp"
#include "tLeaderboard.hpp"
#include "GameSession.hpp"
#include "GameNetMessages.hpp"

// game logic types
#include "tAnimGoalLogic.hpp"
#include "tLevelLogic.hpp"
#include "tUnitLogic.hpp"
#include "tTurretLogic.hpp"
#include "tCharacterLogic.hpp"
#include "tUserControllableCharacterLogic.hpp"
#include "tVehicleLogic.hpp"
#include "tVehiclePassengerLogic.hpp"
#include "tWheeledVehicleLogic.hpp"
#include "tAirborneLogic.hpp"
#include "tHoverLogic.hpp"
#include "tUnitPath.hpp"
#include "tExplosionLogic.hpp"
#include "tAreaDamageLogic.hpp"
#include "tRtsCursorLogic.hpp"
#include "tWaypointLogic.hpp"
#include "tGeneratorLogic.hpp"
#include "tGoalBoxLogic.hpp"
#include "tBulletLogic.hpp"
#include "tShellLogic.hpp"
#include "tRocketLogic.hpp"
#include "tBreakableLogic.hpp"
#include "tUberBreakableLogic.hpp"
#include "tLandMineLogic.hpp"
#include "tPowerUpLogic.hpp"
#include "tLightEffectLogic.hpp"
#include "tWeaponStation.hpp"
#include "tBuildSiteLogic.hpp"
#include "tTeleporterLogic.hpp"
#include "tTimedSpawnLogic.hpp"
#include "tTankPalette.hpp"
#include "tProximityLogic.hpp"
#include "tDropLogic.hpp"
#include "tLightningLogic.hpp"
#include "tMobileTurretLogic.hpp"
#include "tDebrisLogic.hpp"
#include "tBatteryCharger.hpp"
#include "tRainLogic.hpp"

// weapons
#include "tWeapon.hpp"

// game ui
#include "tRadialMenu.hpp"
#include "tSinglePlayerWaveList.hpp"
#include "tEnemiesAliveList.hpp"
#include "tScoreUI.hpp"
#include "tComboTimerUI.hpp"
#include "tPowerPoolUI.hpp"
#include "tBatteryMeter.hpp"
#include "tWeaponUI.hpp"
#include "tDialogBox.hpp"
#include "tMiniMap.hpp"
#include "tWaveLaunchArrowUI.hpp"
#include "tScreenSpaceHealthBarList.hpp"
#include "Gui/tScreenSpaceFxSystem.hpp"
#include "tRtsCursorUI.hpp"
#include "Gui/tColoredQuad.hpp"
#include "Gui/tTexturedQuad.hpp"
#include "gui/tCanvas.hpp"
#include "tWorldSpaceFloatingText.hpp"
#include "tScreenSpaceNotification.hpp"
#include "tFocalPrompt.hpp"
#include "tBarrageUI.hpp"
#include "tBombDropOverlay.hpp"
#include "tPersonalBestUI.hpp"
#include "tHoverTimer.hpp"
#include "tOutOfBoundsIndicator.hpp"
#include "tPointCaptureUI.hpp"
#include "tAchievementBuyNotification.hpp"
#include "tNetUI.hpp"
#include "tAnimPackFile.hpp"

// game anim track
#include "tUnitPathAnimTrack.hpp"
#include "tUnitInterpolatePathAnimTrack.hpp"
#include "tUnitContextAlignAnimTrack.hpp"
#include "tTurretOrientAnimTrack.hpp"
#include "tPitchBlendAnimTrack.hpp"
#include "tPitchBlendMuzzleAnimTrack.hpp"
#include "tVehiclePassengerAnimTrack.hpp"
#include "tCharacterMoveAnimTrackFPS.hpp"
#include "tCharacterAimAnimTrack.hpp"
#include "tVelocityBlendAnimTrack.hpp"
#include "tAirborneAnimTrack.hpp"
#include "tTankTreadAnimTrack.hpp"

#include "Wwise_IDs.h"

#include "xlsp/XLSP.h"
#include "tEncryption.hpp"

//just to log stats
#include "tUberBreakableDebris.hpp"

namespace Sig
{
	devvar( bool, Game_StartInFrontEnd, false );
	devvar( u32,  Game_InitialDifficulty, GameFlags::cDIFFICULTY_NORMAL );
	devvar( bool, Game_SplitHorizontal, false );
	devvar( bool, Gameplay_Debris_ApplyTable, false );
	devvar( bool, Gameplay_Tracers_ApplyTable, false );
	devvar( bool, AAACheats_CantLose, false );
	devvar( bool, Debug_SoundSystem_DisableMusic, false );
	devvar_clamp( f32, Debug_Memory_MaxArtistVramAmount, 320.f, 1.f, 512.f, 0 ); //multiple of mVram page size
	devvar( bool, Debug_Memory_MaxArtistVramEnable, true );
	devvar( bool, Perf_Audio_Disable2D, false );
	devvar( bool, Perf_Debris_ShowCount, false );
	devvar( bool, Debug_UI_DisableRewindMenu, false );
	devvar( bool, Debug_UI_DisableGamercardView, false );
	devvar( bool, GameApp_ForceCoOp, false );
	devvar( bool, Debug_DebugMainMenuEnabled, false );
	devvar( bool, Debug_FastBootupScreens, false );
	devvar( bool, Debug_Stats_Snapshot, false );
	devvar( bool, Debug_ForceLoadDLC, false );

	devvar( bool, Game_ExtraMode, false );
	devvar( bool, Game_PAXMode, false );
	devvar( bool, Game_E3Mode, false );

	devvar( f32, Game_Memory_MBGeomText, 1.5f );
	devvar( f32, Game_Memory_MBGeomFullBright, 0.9f );
	devvar( f32, Game_Memory_MBGeomSolidColor, 0.5f );
	devvar( f32, Game_Memory_MBGeomParticle, 2.0f );
	devvar( f32, Game_Memory_MBGeomDecal, 1.5f );
	devvar( f32, Game_Memory_MBIndices, 0.8f );

	devvar( u32, Game_Renderer_Shadows_OneScreen, 2 );
	devvar( u32, Game_Renderer_Shadows_SplitScreen, 0 );
	devvar( bool, Game_Renderer_GroundCover_ForceForever, true );

	devvar( bool, Game_Sync_AllowedCategory_Random, true );
	devvar( bool, Game_Sync_AllowedCategory_Input, true );
	devvar( bool, Game_Sync_AllowedCategory_SceneGraph, true );
	devvar( bool, Game_Sync_AllowedCategory_Physics, true );
	devvar( bool, Game_Sync_AllowedCategory_AI, true );
	devvar( bool, Game_Sync_AllowedCategory_Player, true );
	devvar( bool, Game_Sync_AllowedCategory_NPC, true );
	devvar( bool, Game_Sync_AllowedCategory_Unit, true );
	devvar( bool, Game_Sync_AllowedCategory_Vehicle, true );
	devvar( bool, Game_Sync_AllowedCategory_Projectile, true );
	devvar( bool, Game_Sync_AllowedCategory_Proximity, true );
	devvar( bool, Game_Sync_AllowedCategory_Logic, true );
	devvar( bool, Game_Sync_AllowedCategory_Stats, true );
	devvar( bool, Game_Sync_AllowedCategory_Level, true );
	devvar( bool, Game_Sync_AllowedCategory_User, true );
	devvar( bool, Game_Sync_AllowedCategory_Debris, true );
	devvar( bool, Game_Sync_AllowedCategory_Camera, true );
	devvar( bool, Game_Sync_AllowedCategory_Raycast, true );
	devvar( bool, Game_Sync_AllowedCategory_Thread, true );
	devvar( bool, Game_Sync_AllowedCategory_Spatial, true );
	devvar( bool, Game_Sync_AllowedCategory_Particles, true );

	namespace
	{
		devvar_clamp( f32, Particles_LOD_WackFactorMP, 0.3f, 0.f, 1.f, 2 );
		devvar_clamp( f32, Particles_LOD_WackFactorSplit, 0.50f, 0.f, 1.f, 2 );
		devvar_clamp( f32, Particles_LOD_MinEmissionPct, 0.25f, 0.f, 1.f, 2 );
		devvar_clamp( f32, Particles_LOD_Threshold, 400.f, 1.f, 10000.f, 0 );
		devvar_clamp( f32, Particles_LOD_DropOff, 400.f, 1.f, 10000.f, 0 );

		static f32 fReduceParticlesLocal( const FX::tParticleSystem& psys )
		{
			const u32 totalPartCount = FX::tParticle::fClassPool( ).fNumObjectsAllocd( );
			f32 extraEmissionReduce = fMax( ( f32 )Particles_LOD_MinEmissionPct, 
				1.f - ( fMax( 0.f, totalPartCount - ( f32 )Particles_LOD_Threshold ) / ( f32 )Particles_LOD_DropOff ) );

			//const u32 totalSystemCount = Particles::tParticleSystem2::fTotalParticleSystemCount( );
			//const f32 extraEmissionReduce = 1.f - fMin( 0.75f, fMax( 0.f, totalSystemCount - 64.f ) / 256.f );

			if( tGameApp::fInstance( ).fGameMode( ).fIsSplitScreen( ) )
				extraEmissionReduce *= Particles_LOD_WackFactorSplit;

			return extraEmissionReduce;
		}
		static f32 fReduceParticlesNetworked( const FX::tParticleSystem& psys )
		{
			// THIS FUNCTION NEEDS TO BE DETERMINISTC
			const f32 extraEmissionReduce = Particles_LOD_WackFactorMP; // whack 'em all!
			return extraEmissionReduce;
		}
	}

	namespace
	{

		enum tTracerDataColumn
		{
			cTracerDataColumnTexture,
			cTracerDataColumnTint,
			cTracerDataColumnWidth,
			cTracerDataColumnLifeSpan,
			cTracerDataColumnSpinRate,
			cTracerDataColumnLeadDistance,
			cTracerDataColumnRealTime,
			cTracerDataColumnBeefy,
			cTracerDataColumnAdditive,
			cTracerDataColumnCount
		};
	}

	namespace { static const tFilePathPtr cBootUpGameScriptPath = tFilePathPtr( "gui/scripts/loadscreens/frontend_bootup.nut" ); }
	namespace { static const tFilePathPtr cExtraBootUpGameScriptPath = tFilePathPtr( "Extra/gui/scripts/loadscreens/frontend_bootup.nut" ); }
	namespace { static const tFilePathPtr cDefaultLoadScriptPath = tFilePathPtr( "gui/scripts/loadscreens/default_level_load.nut" ); }
	namespace { static const tFilePathPtr cDefaultSaveScriptPath = tFilePathPtr ( "gui/scripts/asyncui/saveui.nut" ); }


	const tStringPtr tGameApp::cWeaponTypeSwitchGroup( "WEAPON_TYPE" );
	const tStringPtr tGameApp::cPlayerControlRTPC( "PLAYER_CONTROL" );
	const tStringPtr tGameApp::cUnitIDSwitchGroup( "UNIT_ID" );
	const tStringPtr tGameApp::cSurfaceTypeSwitchGroup( "SURFACE_TYPE" );
	const tStringPtr tGameApp::cLevelSwitchGroup( "LEVEL_SELECT" );	
	const tStringPtr tGameApp::cPersonalityTypeSwitchGroup( "Personality_Type" );	
	const tStringPtr tGameApp::cCrewmanSwitchGroup( "Turret_Crew" );	
	const tStringPtr tGameApp::cBarrageFactionSwitchGroup( "Barrage_Faction" );
	
	Math::tVec4f tGameApp::cColorDirtyWhite = Math::tVec4f( 0.9f, 0.8f, 0.6f, 1.0f );
	Math::tVec4f tGameApp::cColorCleanWhite = Math::tVec4f( 1.0f, 1.0f, 1.0f, 1.0f );
	Math::tVec4f tGameApp::cColorLockedGreen = Math::tVec4f( 0.6f, 0.7f, 0.6f, 1.0f );
	Math::tVec4f tGameApp::cColorSuspendedBlue = Math::tVec4f( 0.557f, 0.855f, 1.0f, 1.0f );

	tGameApp* tGameApp::gGameApp=0;
	tGameApp::tGameApp( )
		: mCurrentLevel( 0 )
		, mSpawningCurrentLevel( false )
		, mNextLevelQueued( false )
		, mGamePauseOverride( false )
		, mAskPlayerToBuy( false )
		, mSingleScreenCoop( false )
		, mSingleScreenControl( 0 )
		, mLanguage( GameFlags::cLANGUAGE_ENGLISH )
		, mRegion( GameFlags::cREGION_REST_OF_WORLD )
		, mLocale( GameFlags::cLOCALE_NO_LOCALE )
		, mInMultiplayerLobby( false )
		, mHasEverReachedFrontEnd( false )
		, mLevelNameReadyToRead( false )
		, mLoadingFrontEndBecauseProfileCouldntSave( false )
		, mDeathLimiter( 0 )
		, mDLCMusicPushed( 0 )
		, mDifficultyOverride( ~0 )
		, mSimulationBegan( false )
	{
		gGameApp = this;
		mStartupParams.mRgbaClearColor = Math::tVec4f(0.f,0.f,0.f,1.f);
		mStartupParams.mMemory.mMBGeomText			= Game_Memory_MBGeomText;
		mStartupParams.mMemory.mMBGeomFullBright	= Game_Memory_MBGeomFullBright;
		mStartupParams.mMemory.mMBGeomSolidColor	= Game_Memory_MBGeomSolidColor;
		mStartupParams.mMemory.mMBGeomParticle		= Game_Memory_MBGeomParticle;
		mStartupParams.mMemory.mMBGeomDecal			= Game_Memory_MBGeomDecal;
		mStartupParams.mMemory.mMBIndices			= Game_Memory_MBIndices;

		mTeamOrientation.fFill( Math::tMat3f::cIdentity );
		mTeamOrientationSet.fFill( false );

		FX::tParticleSystem::fSetEmissionReductionFactor( fReduceParticlesLocal );

		tUberBreakableLogic::cGroundFlags = GameFlags::cFLAG_GROUND;
		tUberBreakableLogic::cCollisionFlags = GameFlags::cFLAG_COLLISION | GameFlags::cFLAG_PROXY_COLLISION_SHAPE;
		tUberBreakableLogic::cOwnerEntityFlags = 0;
		tUberBreakableLogic::cDebrisFlags = 0;
		tUberBreakableLogic::cCollideWithShapeSpatialSet = false;
		tUberBreakableLogic::cDebrisMaxV = 1.f;
		tUberBreakableLogic::cAttachPieceLogic = true;
		tUberBreakableLogic::cAcquireProperties = true;
		//tUberBreakableLogic::cGameFlags = GameFlags::cFLAG_HIT_VOLUME;


#if defined( sig_profile ) && defined( sig_devmenu )
		const u32 hwThreadCount = Threads::tThread::fHardwareThreadCount( );
		const u32 coRenderThreadCount = 4;
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfCharacterLogicActST		, tProfiler::tPerfCategory( tStringPtr( "ActST" )		, tStringPtr( "Characters" ), 1 ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfCharacterLogicAnimateMT	, tProfiler::tPerfCategory( tStringPtr( "AnimateMT" )	, tStringPtr( "Characters" ), hwThreadCount ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfCharacterLogicPhysicsMT	, tProfiler::tPerfCategory( tStringPtr( "PhysicsMT" )	, tStringPtr( "Characters" ), hwThreadCount ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfCharacterLogicMoveST		, tProfiler::tPerfCategory( tStringPtr( "MoveST" )		, tStringPtr( "Characters" ), 1 ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfCharacterLogicThinkST		, tProfiler::tPerfCategory( tStringPtr( "ThinkST" )		, tStringPtr( "Characters" ), 1 ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfCharacterLogicCoRenderMT	, tProfiler::tPerfCategory( tStringPtr( "~CoRenderMT" )	, tStringPtr( "Characters" ), coRenderThreadCount, true ) );
		
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfVehicleLogicActST			, tProfiler::tPerfCategory( tStringPtr( "ActST" )		, tStringPtr( "Vehicles" ), 1 ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfVehicleLogicAnimateMT		, tProfiler::tPerfCategory( tStringPtr( "AnimateMT" )	, tStringPtr( "Vehicles" ), hwThreadCount ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfVehicleLogicPhysicsMT		, tProfiler::tPerfCategory( tStringPtr( "PhysicsMT" )	, tStringPtr( "Vehicles" ), hwThreadCount ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfVehicleLogicMoveST		, tProfiler::tPerfCategory( tStringPtr( "MoveST" )		, tStringPtr( "Vehicles" ), 1 ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfVehicleLogicBaseMoveST	, tProfiler::tPerfCategory( tStringPtr( "BaseMoveST" )	, tStringPtr( "Vehicles" ), 1, true ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfVehicleLogicThinkST		, tProfiler::tPerfCategory( tStringPtr( "ThinkST" )		, tStringPtr( "Vehicles" ), 1 ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfVehicleLogicBaseThinkST	, tProfiler::tPerfCategory( tStringPtr( "BaseThinkST" )	, tStringPtr( "Vehicles" ), 1, true ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfVehicleLogicBaseThinkSTWeapons	, tProfiler::tPerfCategory( tStringPtr( "WeaponsThinkST" ), tStringPtr( "Vehicles" ), 1, true ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfVehicleLogicCoRenderMT	, tProfiler::tPerfCategory( tStringPtr( "~CoRenderMT" )	, tStringPtr( "Vehicles" ), coRenderThreadCount, true ) );

		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfVehiclePhysicsKinematics	, tProfiler::tPerfCategory( tStringPtr( "Kinematics" )	, tStringPtr( "Vehicles" ), hwThreadCount, true ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfVehiclePhysicsDynamics	, tProfiler::tPerfCategory( tStringPtr( "Dynamics" )	, tStringPtr( "Vehicles" ), hwThreadCount, true ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfVehiclePhysicsRayCast		, tProfiler::tPerfCategory( tStringPtr( "Raycast" )		, tStringPtr( "Vehicles" ), hwThreadCount, true ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfVehicleLogicCollisionQuery, tProfiler::tPerfCategory( tStringPtr( "Col. Query" )	, tStringPtr( "Vehicles" ), hwThreadCount ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfVehicleLogicCollision		, tProfiler::tPerfCategory( tStringPtr( "Collision" )	, tStringPtr( "Vehicles" ), hwThreadCount ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfVehicleLogicAIST			, tProfiler::tPerfCategory( tStringPtr( "AI" )			, tStringPtr( "Vehicles" ), 1 ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfVehicleLogicUserST		, tProfiler::tPerfCategory( tStringPtr( "User" )		, tStringPtr( "Vehicles" ), 1 ) );

		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfTurretLogicActST			, tProfiler::tPerfCategory( tStringPtr( "ActST" )		, tStringPtr( "Turrets" ), 1 ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfTurretLogicAnimateMT		, tProfiler::tPerfCategory( tStringPtr( "AnimateMT" )	, tStringPtr( "Turrets" ), hwThreadCount ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfTurretLogicPhysicsMT		, tProfiler::tPerfCategory( tStringPtr( "PhysicsMT" )	, tStringPtr( "Turrets" ), hwThreadCount ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfTurretLogicMoveST			, tProfiler::tPerfCategory( tStringPtr( "MoveST" )		, tStringPtr( "Turrets" ), 1 ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfTurretLogicThinkST		, tProfiler::tPerfCategory( tStringPtr( "ThinkST" )		, tStringPtr( "Turrets" ), 1 ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfTurretLogicCoRenderMT		, tProfiler::tPerfCategory( tStringPtr( "~CoRenderMT" )	, tStringPtr( "Turrets" ), coRenderThreadCount, true ) );

		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfProjectileLogicActST			, tProfiler::tPerfCategory( tStringPtr( "ActST" )		, tStringPtr( "Projectiles" ), 1 ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfProjectileLogicAnimateMT		, tProfiler::tPerfCategory( tStringPtr( "AnimateMT" )	, tStringPtr( "Projectiles" ), hwThreadCount ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfProjectileLogicPhysicsMT		, tProfiler::tPerfCategory( tStringPtr( "PhysicsMT" )	, tStringPtr( "Projectiles" ), hwThreadCount ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfProjectileLogicMoveST			, tProfiler::tPerfCategory( tStringPtr( "MoveST" )		, tStringPtr( "Projectiles" ), 1 ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfProjectileLogicThinkST		, tProfiler::tPerfCategory( tStringPtr( "ThinkST" )		, tStringPtr( "Projectiles" ), 1 ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfProjectileLogicCoRenderMT		, tProfiler::tPerfCategory( tStringPtr( "~CoRenderMT" )	, tStringPtr( "Projectiles" ), coRenderThreadCount, true ) );

		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfPassengerAnimateMT		, tProfiler::tPerfCategory( tStringPtr( "AnimateMT" )	, tStringPtr( "Passengers" ), hwThreadCount ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfPassengerMoveST			, tProfiler::tPerfCategory( tStringPtr( "MoveST" )		, tStringPtr( "Passengers" ), 1 ) );
		
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfBreakableAnimateMT		, tProfiler::tPerfCategory( tStringPtr( "AnimateMT" )	, tStringPtr( "Breakables" ), hwThreadCount ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfBreakableMoveST			, tProfiler::tPerfCategory( tStringPtr( "MoveST" )		, tStringPtr( "Breakables" ), 1 ) );

		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfExplosionLogicActST		, tProfiler::tPerfCategory( tStringPtr( "ActST" )		, tStringPtr( "Explosions" ), 1 ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfExplosionLogicThinkST		, tProfiler::tPerfCategory( tStringPtr( "ThinkST" )		, tStringPtr( "Explosions" ), 1 ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfExplosionLogicCoRenderMT	, tProfiler::tPerfCategory( tStringPtr( "~CoRenderMT" )	, tStringPtr( "Explosions" ), coRenderThreadCount, true ) );

		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfProximityLogicActST		, tProfiler::tPerfCategory( tStringPtr( "ActST" )		, tStringPtr( "ProximityLogic" ), 1 ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfProximityLogicCoRenderMT	, tProfiler::tPerfCategory( tStringPtr( "~CoRenderMT" )	, tStringPtr( "ProximityLogic" ), coRenderThreadCount, true ) );

		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfArcQuery		, tProfiler::tPerfCategory( tStringPtr( "SGQuery" )	, tStringPtr( "Arc" ), 1 ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfArcRay		, tProfiler::tPerfCategory( tStringPtr( "Ray" )		, tStringPtr( "Arc" ), 1 ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfArcGfx		, tProfiler::tPerfCategory( tStringPtr( "Gfx" )		, tStringPtr( "Arc" ), 1 ) );

		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfRTSCam		, tProfiler::tPerfCategory( tStringPtr( "RTSCam" )		, tStringPtr( "Misc" ), 1 ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfWeaponUI		, tProfiler::tPerfCategory( tStringPtr( "Weapon UI" )	, tStringPtr( "Misc" ), 1 ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfTintStack		, tProfiler::tPerfCategory( tStringPtr( "Tint Stack" )	, tStringPtr( "Misc" ), 1 ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfSurfaceLookup	, tProfiler::tPerfCategory( tStringPtr( "SurfaceLookup" ), tStringPtr( "Misc" ), 1 ) );
		tProfiler::fInstance( ).fAddPerfCategory( cProfilePerfTutorialEvents	, tProfiler::tPerfCategory( tStringPtr( "TutorialEvents" ), tStringPtr( "Misc" ), 1 ) );

		
#endif//defined( sig_profile ) && defined( sig_devmenu )
	}
	tGameApp::~tGameApp( )
	{
	}

#ifdef sig_devmenu
	devvar( bool, Debug_GameAppDebugInfo_Render, false );
	namespace
	{
		static Gui::tTextPtr gStatsText;
	}
#endif

	void tGameApp::fDrawStats( ) const
	{
#ifdef sig_devmenu
		if( !Debug_GameAppDebugInfo_Render )
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
		statsText << "GameApp Debug" << std::endl;
		for(u32 i = 0; i < mPlayers.fCount( ); ++i)
		{
			statsText << "Player(" << i << ") " << mPlayers[i].fGetRawPtr( );
			if( mPlayers[ i ] )
			{
				tUser* user = mPlayers[ i ]->fUser( ).fGetRawPtr( );
				statsText << " user: " << user;
				if( user )
					statsText << " " << user->fGamerTag( ) << " hw" << user->fLocalHwIndex( );
			}
			statsText << std::endl;
		}

		gStatsText->fBakeBox( 400, statsText.str( ).c_str( ), 0, Gui::tText::cAlignLeft );
		gStatsText->fSetPosition( Math::tVec3f( 400.0f, 40.0f, 0.0f ) ); // Top right
		gStatsText->fSetRgbaTint( Math::tVec4f( 1.f, 1.f, 1.f, 1.f ) );
		fScreen( )->fAddScreenSpaceDrawCall( gStatsText->fDrawCall( ) );
#endif
	}

	void tGameApp::fSetPlayerFromUser( u32 playerIndex, tUser* user )
	{
		if( user )
		{
			if( mPlayers.fCount( ) < playerIndex + 1 )
				mPlayers.fResize( playerIndex + 1 );

			if( mPlayers[ playerIndex ].fNull( ) )
				mPlayers[ playerIndex ].fReset( NEW tPlayer( tUserPtr( user ) ) );
			else
				mPlayers[ playerIndex ]->fSetUser( tUserPtr( user ) );

			if( playerIndex == 1 )
			{
				if( user->fIsLocal( ) )
					fChangeViewportCount( 1, 2, true, NULL );
				else
				{
					tUserArray remoteUsers;
					remoteUsers.fPushBack( tUserPtr( user ) );
					fChangeViewportCount( 1, 1, true, &remoteUsers );
				}
			}

			mGameAppSession.fApplyProfiles( );
		}
		else
		{
			sigassert( playerIndex == 1 && "Tried to remove a player that is not the second player!" );
			if( mPlayers.fCount( ) > 1 && !mPlayers[ 1 ]->fUser( )->fIsLocal( ) )
			{
				mPlayersToRelease.fPushBack( mPlayers[ 1 ] );
				mPlayers[ 1 ]->fUser( )->fSetViewport( mScreen, Gfx::tViewportPtr( ) );
				mPlayers.fResize( 1 );
			}
		}
	}

	//tLevelLoadInfoPtr tGameApp::fLevelLoadInfo( u32 front, u32 index ) const
	tLevelLoadInfo tGameApp::fLevelLoadInfo( u32 front, u32 index ) const
	{
		tLevelLoadInfo levelInfo = fQueryLevelLoadInfoFromTable( (GameFlags::tMAP_TYPE)front, index );
		//return tLevelLoadInfoPtr( levelInfo );
		return levelInfo;
	}

	b32 tGameApp::fAAACantLose( ) const
	{
		return AAACheats_CantLose;
	}
	
	b32 tGameApp::fExtraDemoMode( )
	{
		return Game_ExtraMode;
	}

	b32 tGameApp::fPAXDemoMode( )
	{
		return Game_PAXMode;
	}

	b32 tGameApp::fE3Mode( )
	{
		return Game_E3Mode;
	}

	u32 tGameApp::fMaxTickets( ) const 
	{ 
		return mCurrentLevelLoadInfo.mTickets;
	}

	void tGameApp::fAddToStatText( std::stringstream& statsText )
	{
		statsText << "difficulty = " << GameFlags::fDIFFICULTYEnumToValueString( fDifficulty( ) ).fCStr( ) << std::endl;
		if( Perf_Debris_ShowCount )
			statsText << "debris     = " <<  tUberBreakableDebris::sDebrisCount << std::endl;

		mGameAppSession.fAddToStatText( statsText );
	}

	void tGameApp::fPause( bool pause, Audio::tSource* source )
	{
		pause = (pause || mGamePauseOverride);
		fSceneGraph( )->fPause( pause );


		if( fExtraDemoMode( ) )
			fSoundSystem( )->fMasterSource( )->fHandleEvent( pause ? AK::EVENTS::PAUSE_GAME : AK::EVENTS::RESUME_GAME );
		else
		{
			// always on the master source!
			source = fSoundSystem( )->fMasterSource( ).fGetRawPtr( );

			if( pause )
			{
				source->fHandleEvent( AK::EVENTS::PLAY_UI_PAUSEGAME );
			}
			else
			{
				source->fHandleEvent( AK::EVENTS::STOP_UI_PAUSEGAME );	
			}
		}
	}

	bool tGameApp::fPaused( ) const
	{
		return (fSceneGraph( )->fIsPaused( ) != 0);
	}

	void tGameApp::fSetIngameSimulationState( b32 sim )
	{
		tGameAppBase::fSetIngameSimulationState( sim );
		mGameAppSession.fOnInGameSimulationState( sim );
	}

	void tGameApp::fConfigureApp( tStartupOptions& opts, tPlatformStartupOptions& platformOpts )
	{
		// fill out game-specific application options...
		opts.mGameName = "TS2";
		opts.mUpdateDelay = 0.f;
		opts.mFrameTimeDeltaClamp = 4.f * opts.mFixedTimeStep;
		opts.mResourceLoadingTimeoutMS = 8.0f;

		// fill out platform-specific application options...
		fConfigureAppPlatform( opts, platformOpts );

		// register logging flags
#		define log_flag_register_types
#			include "GameLogFlags.hpp"
#		undef log_flag_register_types
	}
	void tGameApp::fFillOutScreenCreateOpts( Gfx::tScreenCreationOptions& screenCreateOpts )
	{
#ifdef platform_xbox360
		// on xbox, use cascaded shadow maps (> 1 layer)
		screenCreateOpts.mShadowMapLayerCount = 2;
#endif
	}

	//------------------------------------------------------------------------------
	static u32 fAllowedSyncCategories( )
	{
		u32 allowed = ~0;

		if( !Game_Sync_AllowedCategory_Random )
			allowed &= ~tSync::cSCRandom;

		if( !Game_Sync_AllowedCategory_Input )
			allowed &= ~tSync::cSCInput;

		if( !Game_Sync_AllowedCategory_SceneGraph )
			allowed &= ~tSync::cSCSceneGraph;

		if( !Game_Sync_AllowedCategory_Physics )
			allowed &= ~tSync::cSCPhysics;

		if( !Game_Sync_AllowedCategory_AI )
			allowed &= ~tSync::cSCAI;

		if( !Game_Sync_AllowedCategory_Player )
			allowed &= ~tSync::cSCPlayer;

		if( !Game_Sync_AllowedCategory_NPC )
			allowed &= ~tSync::cSCNPC;

		if( !Game_Sync_AllowedCategory_Unit )
			allowed &= ~tSync::cSCUnit;

		if( !Game_Sync_AllowedCategory_Vehicle )
			allowed &= ~tSync::cSCVehicle;

		if( !Game_Sync_AllowedCategory_Projectile )
			allowed &= ~tSync::cSCProjectile;

		if( !Game_Sync_AllowedCategory_Proximity )
			allowed &= ~tSync::cSCProximity;

		if( !Game_Sync_AllowedCategory_Logic )
			allowed &= ~tSync::cSCLogic;

		if( !Game_Sync_AllowedCategory_Stats )
			allowed &= ~tSync::cSCStats;

		if( !Game_Sync_AllowedCategory_Level )
			allowed &= ~tSync::cSCLevel;

		if( !Game_Sync_AllowedCategory_User )
			allowed &= ~tSync::cSCUser;

		if( !Game_Sync_AllowedCategory_Debris )
			allowed &= ~tSync::cSCDebris;

		if( !Game_Sync_AllowedCategory_Camera )
			allowed &= ~tSync::cSCCamera;			

		if( !Game_Sync_AllowedCategory_Raycast )
			allowed &= ~tSync::cSCRaycast;

		if( !Game_Sync_AllowedCategory_Thread )
			allowed &= ~tSync::cSCThread;

		if( !Game_Sync_AllowedCategory_Spatial )
			allowed &= ~tSync::cSCSpatial;

		if( !Game_Sync_AllowedCategory_Particles )
			allowed &= ~tSync::cSCParticles;
		
		return allowed;
	}

	void tGameApp::fStartupApp( )
	{
		sync_init( "TS2", 0 ); // 0 == buildId
		sync_allowed_categories( fAllowedSyncCategories( ) );
		sync_register_thread( true );

		mDLCEnumerator.fEnumerate( );

		tGameAppBase::fStartupApp( );
		
		mGameAppSession.fInitialize( );

		tVoice::fInitialize( 1 ); // Max we have one other player

		tKeyFrameAnimTrack::fSetKeyFrameEventID( GameFlags::cEVENT_ANIMATION );
		
	}
	void tGameApp::fExportGameScriptInterfaces( tScriptVm& vm )
	{
		// technically, this would normally happen later, but it needs to happen right here
		fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "gameflags.nut" ), true );

		tGameAppSession::fExportScriptInterface( vm );
		tGameApp::fExportScriptInterface( vm );
		tPlayer::fExportScriptInterface( vm );
		tGameSessionStats::fExportScriptInterface( vm );
		tUnitLogic::fExportScriptInterface( vm );
		tAnimGoalLogic::fExportScriptInterface( vm );
		tLevelLogic::fExportScriptInterface( vm );
		tTurretLogic::fExportScriptInterface( vm );
		tCharacterLogic::fExportScriptInterface( vm );
		tUserControllableCharacterLogic::fExportScriptInterface( vm );
		tVehicleLogic::fExportScriptInterface( vm );
		tVehiclePassengerLogic::fExportScriptInterface( vm );
		tWheeledVehicleLogic::fExportScriptInterface( vm );
		tAirborneLogic::fExportScriptInterface( vm );
		tHoverLogic::fExportScriptInterface( vm );
		tUnitPath::fExportScriptInterface( vm );
		tExplosionLogic::fExportScriptInterface( vm );
		tRainLogic::fExportScriptInterface( vm );
		tAreaDamageLogic::fExportScriptInterface( vm );
		tRtsCursorLogic::fExportScriptInterface( vm );
		tWaypointLogic::fExportScriptInterface( vm );
		tGeneratorLogic::fExportScriptInterface( vm );
		tProjectileLogic::fExportScriptInterface( vm );
		tBulletLogic::fExportScriptInterface( vm );
		tShellLogic::fExportScriptInterface( vm );
		tRocketLogic::fExportScriptInterface( vm );
		tBreakableLogic::fExportScriptInterface( vm );
		tGoalBoxLogic::fExportScriptInterface( vm );
		tLandMineLogic::fExportScriptInterface( vm );
		tPowerUpLogic::fExportScriptInterface( vm );
		tLightEffectLogic::fExportScriptInterface( vm );
		tBuildSiteLogic::fExportScriptInterface( vm );
		tTeleporterLogic::fExportScriptInterface( vm );
		tTimedSpawnLogic::fExportScriptInterface( vm );
		tTankPalette::fExportScriptInterface( vm );
		tProximityLogic::fExportScriptInterface( vm );
		tDropLogic::fExportScriptInterface( vm );
		tLightningEntity::fExportScriptInterface( vm );
		tMobileTurretLogic::fExportScriptInterface( vm );
		tBatteryCharger::fExportScriptInterface( vm );

		tXLSP::fExportScriptInterface( vm );

		tWaveList::fExportScriptInterface( vm );
		tBarrage::fExportScriptInterface( vm );
		tBarrageImp::fExportScriptInterface( vm );
		tGamePostEffectManager::fExportScriptInterface( vm );
		tOffensiveWaveDesc::fExportScriptInterface( vm );
		tSaveGameRewindPreview::fExportScriptInterface( vm );

		tContextAnimEventContext::fExportScriptInterface( vm );
		tWeapon::fExportScriptInterface( vm );
		tWeaponStation::fExportScriptInterface( vm );

		tLevelLoadInfo::fExportScriptInterface( vm );
		tUserProfile::fExportScriptInterface( vm );

		Gui::tRadialMenu::fExportScriptInterface( vm );
		Gui::tSinglePlayerWaveList::fExportScriptInterface( vm );
		Gui::tVersusWaveList::fExportScriptInterface( vm );
		Gui::tEnemiesAliveList::fExportScriptInterface( vm );
		Gui::tScoreUI::fExportScriptInterface( vm );
		Gui::tBarrageUI::fExportScriptInterface( vm );
		Gui::tBombDropOverlay::fExportScriptInterface( vm );
		Gui::tRationTicketUI::fExportScriptInterface( vm );
		Gui::tComboTimerUI::fExportScriptInterface( vm );
		Gui::tPowerPoolUI::fExportScriptInterface( vm );
		Gui::tBatteryMeter::fExportScriptInterface( vm );
		Gui::tWeaponUI::fExportScriptInterface( vm );
		Gui::tDialogBox::fExportScriptInterface( vm );
		Gui::tMiniMapBase::fExportScriptInterface( vm );
		Gui::tWorldSpaceFloatingText::fExportScriptInterface( vm );
		Gui::tScreenSpaceNotification::fExportScriptInterface( vm );
		Gui::tWaveLaunchArrowUI::fExportScriptInterface( vm );
		Gui::tScreenSpaceHealthBarList::fExportScriptInterface( vm );
		Gui::tScreenSpaceFxSystem::fExportScriptInterface( vm );
		Gui::tRtsCursorUI::fExportScriptInterface( vm );
		Gui::tFocalPrompt::fExportScriptInterface( vm );
		Gui::tPersonalBestUI::fExportScriptInterface( vm );
		Gui::tHoverTimer::fExportScriptInterface( vm );
		Gui::tOutOfBoundsIndicator::fExportScriptInterface( vm );
		Gui::tPointCaptureUI::fExportScriptInterface( vm );
		Gui::tAchievementBuyNotification::fExportScriptInterface( vm );
		Gui::tNetUI::fExportScriptInterface( vm );

		Net::tClientStateChange::fExportScriptInterface( vm );
		Net::tLevelSelectStatus::fExportScriptInterface( vm );

		tUnitPathAnimDesc::fExportScriptInterface( vm );
		tUnitInterpolatePathAnimDesc::fExportScriptInterface( vm );
		tUnitContextAlignDesc::fExportScriptInterface( vm );
		tTurretOrientAnimDesc::fExportScriptInterface( vm );
		tPitchBlendAnimDesc::fExportScriptInterface( vm );
		tPitchBlendMuzzleAnimDesc::fExportScriptInterface( vm );
		tVehiclePassengerAnimDesc::fExportScriptInterface( vm );
		tCharacterMoveFPSAnimDesc::fExportScriptInterface( vm );
		tCharacterAimAnimDesc::fExportScriptInterface( vm );
		tVelocityBlendAnimDesc::fExportScriptInterface( vm );
		tAirborneAnimDesc::fExportScriptInterface( vm );
		tTankTreadAnimDesc::fExportScriptInterface( vm );

		Audio::tSource::fExportScriptInterface( vm );
	}
	void tGameApp::fLoadPermaLoadedGameResources( )
	{
		//{
		//	tResourcePtr bootUpScreenTexture = fResourceDepot( )->fQueryLoadBlock( tResourceId::fMake<Gfx::tTextureFile>( "gui/textures/loadscreens/bootup/first_screen_g.tga" ), this );
		//	Gui::tTexturedQuad bootUpScreenQuad;
		//	bootUpScreenQuad.fSetTexture( bootUpScreenTexture );
		//	bootUpScreenQuad.fCenterPivot( );
		//	bootUpScreenQuad.fSetPosition( 1280/2,720/2,0 );
		//	mScreen->fRender( &bootUpScreenQuad );
		//	bootUpScreenTexture->fUnload( this );
		//}

		log_line( 0, "Pre-loading perma loaded game resources..." );

		// load loc files
		fLoadLocResources( );

		// any blocking stuff up front.
		mBlackBar = Gui::tCanvasPtr( NEW Gui::tTexturedQuad( ) );
		tResourcePtr blackBarTexture = fAddAlwaysLoadedResource( tResourceId::fMake< Gfx::tTextureFile >( "Gui/Textures/Misc/splitscreen_separator_g.png" ), true );
		mBlackBar.fCanvas( )->fDynamicCast< Gui::tTexturedQuad >( )->fSetTexture( blackBarTexture );
		
		// load critical scripts
		mGlobalScripts[ cGlobalScriptAnimatingCanvas ] = fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "Gui/Scripts/Controls/AnimatingCanvas.nut" ) );
		mGlobalScripts[ cGlobalScriptNetUI ] = fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "Gui/Scripts/asyncui/netui.nut" ), true );

		//  boot scripts
		if( fExtraDemoMode( ) )
			fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( cExtraBootUpGameScriptPath ), true );
		else
			fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( cBootUpGameScriptPath ), true );
		fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( cDefaultSaveScriptPath ), true );
		//fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( cDefaultLoadScriptPath ) );

		// load all global data tables, data tables block on load anyways
		mDataTables[ cDataTableLevelResources ] = fAddAlwaysLoadedResource( tResourceId::fMake< tDataTableFile >( Game_ExtraMode ? "DataTables/LevelResourcesExtra.tab" : "DataTables/LevelResources.tab" ) );
		mDataTables[ cDataTableBreakables ] = fAddAlwaysLoadedResource( tResourceId::fMake< tDataTableFile >( "DataTables/Breakables.tab" ), true );
		mDataTables[ cDataTableCombos ] = fAddAlwaysLoadedResource( tResourceId::fMake< tDataTableFile >( "DataTables/Combos.tab" ) );	
		mDataTables[ cDataTableComboEnumValues ] = fAddAlwaysLoadedResource( tResourceId::fMake< tDataTableFile >( "DataTables/ComboEnumValues.tab" ) );	
		mDataTables[ cDataTableContextAnims ] = fAddAlwaysLoadedResource( tResourceId::fMake< tDataTableFile >( "DataTables/ContextAnims.tab" ) );	;	
		mDataTables[ cDataTableCommonWaveList ] = fAddAlwaysLoadedResource( tResourceId::fMake< tDataTableFile >( "DataTables/CommonWaves.tab" ) );	;	
		mDataTables[ cDataTableSessionStats ] = fAddAlwaysLoadedResource( tResourceId::fMake< tDataTableFile >( "DataTables/SessionStats.tab" ) );
		mDataTables[ cDataTableEffectsCombos ] = fAddAlwaysLoadedResource( tResourceId::fMake< tDataTableFile >( "DataTables/EffectsCombos.tab" ) );

		mUnitsShared.fSet( fAddAlwaysLoadedResource( tResourceId::fMake< tDataTableFile >( "DataTables/UnitsShared.tab" ), true ) );		
		sigassert( mUnitsShared.fTableCount( ) == GameFlags::cCOUNTRY_COUNT );
		mUnitsPhysics[ GameFlags::cCOUNTRY_USA ].fSet( fAddAlwaysLoadedResource( tResourceId::fMake< tDataTableFile >( "DataTables/VehiclesUSA.tab" ), true ) );
		mUnitsPhysics[ GameFlags::cCOUNTRY_USSR ].fSet( fAddAlwaysLoadedResource( tResourceId::fMake< tDataTableFile >( "DataTables/VehiclesUSSR.tab" ), true ) );
		mFocusItems.fSet( fAddAlwaysLoadedResource( tResourceId::fMake< tDataTableFile >( "DataTables/FocusItems.tab" ), true ) );
		mTracers.fSet( fAddAlwaysLoadedResource( tResourceId::fMake< tDataTableFile >( "DataTables/Tracers.tab" ), true ) );
		mCargo.fSet( fAddAlwaysLoadedResource( tResourceId::fMake< tDataTableFile >( "DataTables/Cargo.tab" ), true ) );	
		mWeapons.fSet( fAddAlwaysLoadedResource( tResourceId::fMake< tDataTableFile >( "DataTables/Weapons.tab" ), true ) );			
		mCharacterProps.fSet( fAddAlwaysLoadedResource( tResourceId::fMake< tDataTableFile >( "DataTables/CharacterProps.tab" ), true ) );


		fAddAlwaysLoadedResource( tResourceId::fMake< tAnimPackFile >( "anims/characters/red/artillery/artillery.anib" ) );
		fAddAlwaysLoadedResource( tResourceId::fMake< tAnimPackFile >( "anims/characters/blue/artillery/artillery.anib" ) );

		if( mDLCEnumerator.fCountByType( tDLCPackage::cDLCPackage ) || mDLCEnumerator.fCountByType( tDLCPackage::cDLCCompatibilityPackage ) || Debug_ForceLoadDLC )
		{
			fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "dlc_imports.nut" ) );
		}

		// load the achievements from a table since we can't already have them in the game config. STUPID
		tResourcePtr achievementsRes = fAddAlwaysLoadedResource( tResourceId::fMake< tDataTableFile >( "DataTables/Achievements.tab" ), true );
		sigassert( achievementsRes );

		{
			const tDataTable& dt = achievementsRes->fCast<tDataTableFile>( )->fIndexTable( 0 );

			sigassert( dt.fRowCount( ) <= GameFlags::cACHIEVEMENTS_COUNT );
			for( u32 i = 0; i < dt.fRowCount( ); ++i )
			{
				GameSession::cAchievementIds[ i ] = (u32)dt.fIndexByRowCol<f32>( i, 0 );
			}
		}

		//
		tGameEffects::fInstance( ).fLoadResourcesAndValidate( 
			fAddAlwaysLoadedResource( tResourceId::fMake< tDataTableFile >( "DataTables/Effects.tab" ), true ), 
			tGameEnum( &GameFlags::fSURFACE_TYPEValueStringToEnum, &GameFlags::fSURFACE_TYPEEnumToValueString, GameFlags::cSURFACE_TYPE_COUNT ),
			GameFlags::cEVENT_GAME_EFFECT
			);

		// load all global scripts
		mGlobalScripts[ cGlobalScriptPlaceTurret ] = fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "Gui/Scripts/RadialMenus/place_turret.nut" ) );
		mGlobalScripts[ cGlobalScriptTurretOptions ] = fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "Gui/Scripts/RadialMenus/turret_options.nut" ) );
		mGlobalScripts[ cGlobalScriptVehicleOptions ] = fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "Gui/Scripts/RadialMenus/vehicle_options.nut" ) );
		mGlobalScripts[ cGlobalScriptAirborneOptions ] = fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "Gui/Scripts/RadialMenus/vehicle_options.nut" ) );
		mGlobalScripts[ cGlobalScriptInfantryOptions ] = fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "Gui/Scripts/RadialMenus/vehicle_options.nut" ) );
		mGlobalScripts[ cGlobalScriptBuildSiteOptions ] = fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "Gui/Scripts/RadialMenus/build_site_options.nut" ) );
		mGlobalScripts[ cGlobalScriptPurchaseVehicleOptions ] = mResourceDepot->fQuery( tResourceId::fMake< tScriptFile >( "Gui/Scripts/RadialMenus/vehicle_purchase.nut" ) );
		mGlobalScripts[ cGlobalScriptPowerUpOptions ] = fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "Gui/Scripts/RadialMenus/powerup_options.nut" ) );
		mGlobalScripts[ cGlobalScriptWaveLaunch ] = fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "Gui/Scripts/RadialMenus/headtohead_wavelaunch.nut" ) );
		mGlobalScripts[ cGlobalScriptSPWaveList ] = fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "Gui/Scripts/HUD/wavelist_sp.nut" ) );
		mGlobalScripts[ cGlobalScriptVersusWaveList ] = fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "Gui/Scripts/HUD/wavelist_versus.nut" ) );
		mGlobalScripts[ cGlobalScriptEnemiesAliveList ] = fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "Gui/Scripts/HUD/EnemiesAliveList.nut" ) );
		mGlobalScripts[ cGlobalScriptScreenSpaceHealthBarList ] = fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "Gui/Scripts/HUD/ScreenSpaceHealthBarList.nut" ) );
		mGlobalScripts[ cGlobalScriptScoreUI ] = fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "Gui/Scripts/HUD/ScoreUI.nut" ) );
		mGlobalScripts[ cGlobalScriptBarrageIndicator ] = fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "Gui/Scripts/HUD/BarrageIndicator.nut" ) );
		mGlobalScripts[ cGlobalScriptBombDropOverlay ] = fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "Gui/Scripts/HUD/BombDropOverlay.nut" ) );
		mGlobalScripts[ cGlobalScriptRationTicketUI ] = fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "Gui/Scripts/HUD/RationTicketUI.nut" ) );
		mGlobalScripts[ cGlobalScriptPersonalBestUI ] = fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "Gui/Scripts/HUD/PersonalBestUI.nut" ) );
		mGlobalScripts[ cGlobalScriptDialogBox ] = fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "Gui/Scripts/DialogBox/DialogBox.nut" ) );
		mGlobalScripts[ cGlobalScriptMiniMap ] = fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "Gui/Scripts/HUD/MiniMap.nut" ) );
		mGlobalScripts[ cGlobalScriptEnemyHealthBar ] = fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "Gui/Scripts/HUD/EnemyHealthBar.nut" ) );
		mGlobalScripts[ cGlobalScriptInUseIndicator ] = fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "Gui/Scripts/HUD/InUseIndicator.nut" ) );
		mGlobalScripts[ cGlobalScriptTurretUpgradeIndicator ] = fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "Gui/Scripts/HUD/TurretUpgradeIndicator.nut" ) );
		mGlobalScripts[ cGlobalScriptTurretRepairIndicator ] = fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "Gui/Scripts/HUD/TurretRepairIndicator.nut" ) );
		mGlobalScripts[ cGlobalScriptUnitPickupIndicator ] = fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "Gui/Scripts/HUD/UnitPickupIndicator.nut" ) );
		mGlobalScripts[ cGlobalScriptRtsCursorUI ] = fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "Gui/Scripts/HUD/RtsCursorUI.nut" ) );
		mGlobalScripts[ cGlobalScriptRtsHoverText ] = fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "Gui/Scripts/HUD/RtsHoverText.nut" ) );
		mGlobalScripts[ cGlobalScriptWaveLaunchArrowUI ] = fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "Gui/Scripts/HUD/WaveLaunchArrow.nut" ) );
		mGlobalScripts[ cGlobalScriptPowerPoolUI ] = fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "Gui/Scripts/HUD/PowerPoolUI.nut" ) );
		mGlobalScripts[ cGlobalScriptHoverTimer ] = fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "Gui/Scripts/HUD/HoverTimer.nut" ) );
		mGlobalScripts[ cGlobalScriptBatteryMeter ] = fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "Gui/Scripts/HUD/BatteryMeter.nut" ) );
		mGlobalScripts[ cGlobalScriptOutOfBoundsIndicator ] = fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "Gui/Scripts/HUD/OutOfBoundsIndicator.nut" ) );
		mGlobalScripts[ cGlobalScriptPointCaptureUI ] = fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "Gui/Scripts/HUD/PointCaptureUI.nut" ) );
		mGlobalScripts[ cGlobalScriptAchievementBuyNotification ] = fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "Gui/Scripts/HUD/AchievementBuyNotification.nut" ) );
		mGlobalScripts[ cGlobalScriptAvatarAwardBuyNotification ] = fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "Gui/Scripts/HUD/AvatarAwardBuyNotification.nut" ) );
		mGlobalScripts[ cGlobalScriptScreenSpaceNotification ] = fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "Gui/Scripts/HUD/ScreenSpaceNotification.nut" ) );
		mGlobalScripts[ cGlobalScriptFocalPrompt ] = fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "Gui/Scripts/HUD/FocalPrompt.nut" ) );
		mGlobalScripts[ cGlobalScriptWorldSpaceFlyingText ] = fAddAlwaysLoadedResource( tResourceId::fMake< tScriptFile >( "Gui/Scripts/HUD/WorldSpaceFlyingText.nut" ) );
		
		// pre-load misc. game resources (these remain loaded ALL GAME!!!)
		const tFilePathPtr resourceFolders[]=
		{
			tFilePathPtr( "gui/textures/gamepad" ),
			tFilePathPtr( "gui/textures/misc" ),
			tFilePathPtr( "gui/textures/radialmenus" ),
			tFilePathPtr( "gui/textures/weapons" ),
			tFilePathPtr( "gui/textures/pausemenu" ),
			tFilePathPtr( "gui/textures/dialogbox" ),
			tFilePathPtr( "gui/textures/minimap" ),
			tFilePathPtr( "gui/textures/score" ),
			tFilePathPtr( "gui/textures/combos" ),
			tFilePathPtr( "gui/textures/cursor" ),
			tFilePathPtr( "gui/scripts/weapons" ),
			tFilePathPtr( "extra/gui/scripts/weapons" ), // extra stuff
			tFilePathPtr( "effects" ),
			tFilePathPtr( "gameplay/projectiles" ),
		};
		tGrowableArray<tResourceId> resourceIds;
		fGetResourceIDs( resourceIds, resourceFolders, array_length( resourceFolders ) );
		for( u32 i = 0; i < resourceIds.fCount( ); ++i )
		{
			//log_line( 0, resourceIds[ i ].fGetPath( ) );
			fAddAlwaysLoadedResource( resourceIds[ i ] );
		}

		fAddAlwaysLoadedResource( tResourceId::fMake< tSceneGraphFile >( "Effects/Entities/misc/breakablepuff.sigml" ) );
		fAddAlwaysLoadedResource( tResourceId::fMake< tSceneGraphFile >( "Effects/Entities/Projectiles/verrey_light.sigml" ) );

		mDebrisPieces[ cDebrisPiecesDefault ] = fAddAlwaysLoadedResource( tResourceId::fMake< tSceneGraphFile >( "Art/_Placeholder/Gameplay/australian_debris.sigml" ) );
		
		mCannonAimingTexture = fAddAlwaysLoadedResource( tResourceId::fMake< Gfx::tTextureFile >( "Effects/Textures/Particles/arrow_head_i.png" ) );
		mDefaultDecalTexture = fAddAlwaysLoadedResource( tResourceId::fMake< Gfx::tTextureFile >( "Effects/Textures/Decals/scar_01_512_i.png" ) );
		mRtsCursorTexture[ cRtsCursorTextureIDArrow ] = fAddAlwaysLoadedResource( tResourceId::fMake< Gfx::tTextureFile >( "Gui/Textures/Cursor/arrow_g.png" ) );
		mRtsCursorTexture[ cRtsCursorTextureIDRange ] = fAddAlwaysLoadedResource( tResourceId::fMake< Gfx::tTextureFile >( "Gui/Textures/Cursor/range_i.png" ) );

		mUpgradeClockPath[ GameFlags::cBUILD_SITE_SMALL ] = tFilePathPtr( "art/units/bases/blue/base_clock_small.sigml" );
		mUpgradeClockPath[ GameFlags::cBUILD_SITE_LARGE ] = tFilePathPtr( "art/units/bases/blue/base_clock_large.sigml" );
		for( u32 i = 0; i < mUpgradeClockPath.fCount( ); ++i ) fAddAlwaysLoadedResource( tResourceId::fMake< tSceneGraphFile >( mUpgradeClockPath[ i ] ) );

		fAddAlwaysLoadedResource( tResourceId::fMake< tSceneGraphFile >( "Effects/Entities/misc/breakablepuff.sigml" ) );
		mTrajectoryTargetVisual = fAddAlwaysLoadedResource( tResourceId::fMake< tSceneGraphFile >( "Effects/Units/trajectorytarget.sigml" ) );

		fSetupLeaderboardData( );

		// Do some configuration with loaded tables
		fSetupTracers( );
		fSetupDebris( );
		tWeaponDescs::fInstance( ).fSetup( );

		for( u32 t = 0; t < mTracers.fTableCount( ); ++t )
			for( u32 r = 0; r < mTracers.fIndexTable( t ).fTable( ).fRowCount( ); ++r )
			{
				const tFilePathPtr path = mTracers.fIndexTable( t ).fIndexByRowCol<tFilePathPtr>( r, cTracerDataColumnTexture );
				if( path.fExists( ) )
					fAddAlwaysLoadedResource( tResourceId::fMake< Gfx::tTextureFile >( path ) );
			}
		
		for( u32 t = 0; t < mCharacterProps.fTableCount( ); ++t )
			for( u32 r = 0; r < mCharacterProps.fIndexTable( t ).fTable( ).fRowCount( ); ++r )
			{
				const tFilePathPtr path0 = mCharacterProps.fIndexTable( t ).fIndexByRowCol<tFilePathPtr>( r, cCharacterPropsLeftHandResource );
				const tFilePathPtr path1 = mCharacterProps.fIndexTable( t ).fIndexByRowCol<tFilePathPtr>( r, cCharacterPropsRightHandResource );
				const tFilePathPtr path2 = mCharacterProps.fIndexTable( t ).fIndexByRowCol<tFilePathPtr>( r, cCharacterPropsHelmetResource );
				if( path0.fExists( ) )
					fAddAlwaysLoadedResource( tResourceId::fMake< tSceneGraphFile >( path0 ) );
				if( path1.fExists( ) )
					fAddAlwaysLoadedResource( tResourceId::fMake< tSceneGraphFile >( path1 ) );
				if( path2.fExists( ) )
					fAddAlwaysLoadedResource( tResourceId::fMake< tSceneGraphFile >( path2 ) );
			}

		/////////////////////////////////////////////////////////////////////////////////////////////////////
		log_line( 0, "... finished pre-loading perma loaded game resources." );
	}

	void tGameApp::fLoadLocResources( )
	{
		tDynamicArray<tStringPtr> fontNames( cFontCount );
		fontNames[ cFontFixedSmall ]	= tStringPtr( "Font_Fixed_Small" );
		fontNames[ cFontSimpleSmall ]	= tStringPtr( "Font_Simple_Small" );
		fontNames[ cFontSimpleMed ]		= tStringPtr( "Font_Simple_Med" );
		fontNames[ cFontFancyMed ]		= tStringPtr( "Font_Fancy_Med" );
		fontNames[ cFontFancyLarge ]	= tStringPtr( "Font_Fancy_Large" );

		tFilePathPtr locFolder;
		
#ifdef platform_xbox360
		DWORD lang = XGetLanguage( );

		switch( lang )
		{
		case XC_LANGUAGE_ENGLISH:
		case XC_LANGUAGE_POLISH: 
		case XC_LANGUAGE_RUSSIAN:
		case XC_LANGUAGE_PORTUGUESE:
		case XC_LANGUAGE_SCHINESE:
			mLanguage = GameFlags::cLANGUAGE_ENGLISH;
			locFolder = tFilePathPtr( "loc/en" );
			break;
		case XC_LANGUAGE_JAPANESE:
			mLanguage = GameFlags::cLANGUAGE_JAPANESE;
			locFolder = tFilePathPtr( "loc/ja" );
			break;
		case XC_LANGUAGE_GERMAN:
			mLanguage = GameFlags::cLANGUAGE_GERMAN;
			locFolder = tFilePathPtr( "loc/de" );
			break; 
		case XC_LANGUAGE_FRENCH:
			mLanguage = GameFlags::cLANGUAGE_FRENCH;
			locFolder = tFilePathPtr( "loc/fr" );
			break;  
		case XC_LANGUAGE_SPANISH:
			mLanguage = GameFlags::cLANGUAGE_SPANISH;
			locFolder = tFilePathPtr( "loc/es" );
			break; 
		case XC_LANGUAGE_ITALIAN:
			mLanguage = GameFlags::cLANGUAGE_ITALIAN;
			locFolder = tFilePathPtr( "loc/it" );
			break; 
		case XC_LANGUAGE_KOREAN:
			mLanguage = GameFlags::cLANGUAGE_KOREAN;
			locFolder = tFilePathPtr( "loc/ko" );
			break; 
		case XC_LANGUAGE_TCHINESE:
			mLanguage = GameFlags::cLANGUAGE_CHINESE;
			locFolder = tFilePathPtr( "loc/zh" );
			break; 
		};

		DWORD location = XGetLocale( );
		mLocale = location;
		DWORD gameRegion = XGetGameRegion( );

#define USE_GAMEREGION_FOR_REGION
#ifdef USE_GAMEREGION_FOR_REGION
		if( gameRegion == XC_GAME_REGION_NA_ALL )
			mRegion = GameFlags::cREGION_NORTH_AMERICA;
#else
		switch( location )
		{
		case XC_LOCALE_CHINA:
		case XC_LOCALE_HONG_KONG:
		case XC_LOCALE_JAPAN:
		case XC_LOCALE_SINGAPORE:
		case XC_LOCALE_TAIWAN:
			mRegion = GameFlags::cREGION_ASIA;
			break;

		case XC_LOCALE_AUSTRIA:
		case XC_LOCALE_BELGIUM:
		case XC_LOCALE_CZECH_REPUBLIC:
		case XC_LOCALE_DENMARK:
		case XC_LOCALE_FINLAND:
		case XC_LOCALE_FRANCE:
		case XC_LOCALE_GREECE:
		case XC_LOCALE_HUNGARY:
		case XC_LOCALE_IRELAND:
		case XC_LOCALE_ITALY:
		case XC_LOCALE_NETHERLANDS:
		case XC_LOCALE_NEW_ZEALAND:
		case XC_LOCALE_NORWAY:
		case XC_LOCALE_POLAND:
		case XC_LOCALE_PORTUGAL:
		case XC_LOCALE_SLOVAK_REPUBLIC:
		case XC_LOCALE_SPAIN:
		case XC_LOCALE_SWEDEN:
		case XC_LOCALE_SWITZERLAND:
		case XC_LOCALE_GREAT_BRITAIN:
		case XC_LOCALE_RUSSIAN_FEDERATION:
		case XC_LOCALE_SOUTH_AFRICA:
			mRegion = GameFlags::cREGION_EUROPE;
			break;

		case XC_LOCALE_GERMANY:
			mRegion = GameFlags::cREGION_GERMANY;
			break;

		case XC_LOCALE_AUSTRALIA:
			mRegion = GameFlags::cREGION_AUSTRALIA;
			break;

		case XC_LOCALE_KOREA:
			mRegion = GameFlags::cREGION_KOREA;
			break;

		case XC_LOCALE_UNITED_STATES:
		case XC_LOCALE_CANADA:
		case XC_LOCALE_MEXICO:
			mRegion = GameFlags::cREGION_NORTH_AMERICA;
			break;

		case XC_LOCALE_BRAZIL:
		case XC_LOCALE_CHILE:
		case XC_LOCALE_COLOMBIA:
		case XC_LOCALE_INDIA:
		default:
			mRegion = GameFlags::cREGION_REST_OF_WORLD;
			break;
		}

#endif // use game region

#endif
		
		if( !locFolder.fExists( ) )
			locFolder = tFilePathPtr( "loc/en" );

		tGameAppBase::fLoadLocResources( locFolder, fontNames ); 
	}
	void tGameApp::fPreLoadFilePackages( )
	{
		// load base package
		tGameAppBase::fPreLoadFilePackages( );

		mDLCEnumerator.fWaitForFinish( );
		if( mDLCEnumerator.fState( ) == tDLCEnumerator::cStateSuccess )
			fSetAddonFlags( );

		for( u32 i = 0; i < mDLCEnumerator.fPackageCount( ); ++i )
		{
			const tFilePathPtr packagePath = mDLCEnumerator.fPackage( i )->fResPath( );
			log_line( 0, "Looking for res package path: " << packagePath );

			if( FileSystem::fFileExists( packagePath ) )
			{
				tFilePackagePtr package( NEW tFilePackage );
				package->fLoad( this, *fResourceDepot( ), packagePath, false );
				package->fBlockUntilLoaded( );
				if( package->fLoaded( ) )
				{
					mFilePackages.fPushBack( package );
					log_line( 0, "               (((((                               )))))" );
					log_line( 0, "***************(((((    also from '" << packagePath << "'    )))))***************" );
					log_line( 0, "               (((((                               )))))" );
				}
			}
		}
	}
	void tGameApp::fSetupPostEffects( )
	{
		mPostEffectMgr.fReset( new tGamePostEffectManager( mPostEffectsMaterialFile ) );
		mPostEffectMgr->fCreateRenderTargets( *mScreen );
		mScreen->fSetPostEffectMgr( Gfx::tPostEffectManagerPtr( mPostEffectMgr.fGetRawPtr( ) ) );
	}
	void tGameApp::fStartupUsers( )
	{
		tGameAppBase::fStartupUsers( );

		fGatherUserSignInStates( );

		mPlayers.fNewArray( 1 );
		mPlayers[ 0 ].fReset( new tPlayer( mLocalUsers[ mLocalUsers.fFirstSignedInOrValid( ) ] ) );
		mPlayers[ 0 ]->fSetTeam( GameFlags::cTEAM_BLUE ); // HACK until we have proper GUI flow/control
		mPlayers[ 0 ]->fDisableStats( );

		mFrontEndPlayer = mPlayers[ 0 ];

		tXLSP::fInstance( ).fInitialize( );
		tXLSP::fInstance( ).fSetFrontEndPlayer( mFrontEndPlayer->fUser( ) );
		
	}
	void tGameApp::fSetAddonFlags( )
	{
		u32 licenseFlags = 0;
		u32 installedFlags = 0;

		if( Debug_ForceLoadDLC )
			installedFlags = ~0;

		for( u32 i = 0; i < mDLCEnumerator.fPackageCount( ); ++i )
		{
			if( mDLCEnumerator.fPackage( i )->mType == tDLCPackage::cDLCPackage )
			{
				licenseFlags |= ( 1 << ( mDLCEnumerator.fPackage( i )->mId ) );
				installedFlags |= ~0; //our dlcs contain info for all dcls!
			}
			else if( mDLCEnumerator.fPackage( i )->mType == tDLCPackage::cDLCCompatibilityPackage )
				installedFlags |= ~0; //our compatibility packs contain info for all dcls!
		}

		for( u32 i = 0; i < mLocalUsers.fCount( ); ++ i )
			mLocalUsers[ i ]->fSetAddOnsLicensed( licenseFlags );

		for( u32 i = 0; i < mLocalUsers.fCount( ); ++ i )
			mLocalUsers[ i ]->fSetAddOnsInstalled( installedFlags );
	}
	void tGameApp::fPreview( const tFilePathPtr& previewPath )
	{
		// to preview, we need to ensure the user is signed in
		if( previewPath.fExists( ) && !Game_StartInFrontEnd && !fFrontEndPlayer( )->fUser( )->fSignedIn( ) )
		{
			log_warning( 0, "Couldn't run preview because the default controller must be signed in to a profile - kicking back to Front End" );
		}

		b32 startInFrontEnd = Game_StartInFrontEnd || !fFrontEndPlayer( )->fUser( )->fSignedIn( );
		if( startInFrontEnd )
			fStartGame( );
		else
		{
			fLoadLevel( fQueryLevelLoadInfoFromPath( previewPath ) );
			mNextLevelLoadInfo.mPreview = true;
		}
	}
	void tGameApp::fReplay( const tFilePathPtr& replayPath )
	{
		if( Game_StartInFrontEnd )
			fStartGame( );
		else if( !fFrontEndPlayer( )->fUser( )->fSignedIn( ) )
		{
			log_warning( 0, "Couldn't run replay because the default controller must be signed in to a profile - quitting" );
			fQuitAsync( );
		}
		else
			mGameAppSession.fLoadReplay( replayPath );
	}
	void tGameApp::fStartGame( )
	{
		fLoadFrontEnd( true );
	}
	void tGameApp::fTickApp( )
	{
		profile_pix("fTickApp");

		// Clear players released during PreOnTick
		mPlayersToRelease.fSetCount( 0 );

		if( mNextLevelQueued )
		{
			mNextLevelQueued = false;
			mGameAppSession.fOnLoadLevel( false );
		}

		const f32 dt = fGetFrameDeltaTime( );

		if( mPostEffectMgr )
		{
			mPostEffectMgr->mFrontEndMode = fGameMode( ).fIsFrontEnd( );
			mPostEffectMgr->fOnTick( dt, mSceneGraph->fIsPaused( ) );
		}

		//if( mMusicMgr )
		//	mMusicMgr->fOnTick( *fSceneGraph( ), dt );

		//if( mNetServices )
		//	mNetServices->fUpdate( );

		//if( mPlatformServices )
		//	mPlatformServices->fUpdate( );

		tDecalManager::fInstance( ).fUpdate( dt );

		mGameAppSession.fOnAppTick( dt );	
		mCachedScoreWriterSession.fTick( );

		if (mSaveUI)
			fTickSaveUI( dt );

		tGameAppBase::fTickApp( );

		// Mark the sync frame
		mGameAppSession.fSyncFrame( );

		if( mDLCEnumerator.fState( ) == tDLCEnumerator::cStateEnumerating )
		{
			if( mDLCEnumerator.fAdvance( ) && mDLCEnumerator.fState( ) == tDLCEnumerator::cStateSuccess )
				fSetAddonFlags( );
		}

		fDrawStats( );

		tXLSP::fInstance( ).fUpdate( dt );
	}
	void tGameApp::fStepTimedCallbacks( f32 dt )
	{
		sigassert( !fSceneGraph( )->fInMTRunList( ) );

		if( mSimulationBegan )
		{
			for( u32 i = 0; i < mCallbacks.fCount( ); ++i )
			{
				sync_line( );	
				if( (!mCallbacks[ i ].fPausable( ) || !fPaused( ))
					&& mCallbacks[ i ].fStepST( dt ) )
				{
					sync_line( );								
					mCallbacks.fErase( i );
					--i;
				}
			}
		}
	}
	void tGameApp::fGatherUserSignInStates( )
	{
		const u32 userCount = mLocalUsers.fCount( );
		sigassert( userCount == tUser::cMaxLocalUsers );
		for( u32 u = 0; u < userCount; ++u )
		{
			sigassert( mLocalUsers[ u ]->fLocalHwIndex( ) == u );

			mUserSignInStates[ u ].mUserId = mLocalUsers[ u ]->fPlatformId( );
			mUserSignInStates[ u ].mState = mLocalUsers[ u ]->fSignInState( );
		}
	}
	void tGameApp::fOnUserSignInChange( u32 userMask )
	{
		// Reread the license
		fReadLicense( );

		// Store old states
		tFixedArray<tUserSigninInfo, tUser::cMaxLocalUsers> oldStates;
		fMemCpy( oldStates.fBegin( ), mUserSignInStates.fBegin( ), oldStates.fTotalSizeOf( ) );

		// Gather new states
		fGatherUserSignInStates( );

		sigassert( fFrontEndPlayer( ) );
		u32 frontEndPlayerID = fFrontEndPlayer( )->fUser( )->fLocalHwIndex( );
		if( oldStates[ frontEndPlayerID ].mState == tUser::cSignInStateNotSignedIn && mUserSignInStates[ frontEndPlayerID ].mState != tUser::cSignInStateNotSignedIn )
		{
			fFrontEndPlayer( )->fReadProfile( ); //front end may not have been signed in yet. Now that he is, read profile.
		}

		// Inform the app session of the possible changes
		mGameAppSession.fOnUserSignInChange( oldStates.fBegin( ), mUserSignInStates.fBegin( ) );
		mCachedScoreWriterSession.fOnUserSignInChange( oldStates.fBegin( ), mUserSignInStates.fBegin( ) );
	}
	void tGameApp::fOnSystemUiChange( b32 sysUiVisible )
	{
		if( mCurrentLevel && !mCurrentLevel->fRootMenu( ).fIsNull( ) )
		{
			mSysUiEventContext.fReset( NEW Logic::tBoolEventContext( sysUiVisible == 0 ? false: true ) );
			Logic::tEvent e = Logic::tEvent( ApplicationEvent::cOnSystemUiChange, mSysUiEventContext );
			mCurrentLevel->fRootMenu( ).fHandleCanvasEvent( e );
		}
	}

	void tGameApp::fOnGameInviteAccepted( u32 localHwIndex )
	{
		if( fCacheCurrentGameInvite( localHwIndex ) )
		{
			// Test if we're in an acceptable state to accept the invite
			if( !mGameAppSession.fCanAcceptInvite( ) )
			{
				mGameAppSession.fOnGameInviteRejected( localHwIndex );
				mGameInvite.fRelease( );
				return;
			}
			fFireInviteEvent( );
		}
		else
		{
			mGameAppSession.fOnGameInviteRejected( localHwIndex );
		}

		// TODO handle game invite like this:

		// if in front end
		//		if localHwIndex doesn't match default user
		//			change default user to this new hwIndex (i.e., profile is changing too, so any other settings other than just the index)
		//		move default user to lobby
		//		if denied from lobby (error, or lobby is full, etc.)
		//			move to Main Menu (one after Press START screen)
		// else if loading or in game
		//		if loading
		//			wait until loaded into game (whether sp/mp)
		//		prompt default user for destructive action (i.e., your game will end)
		//		if confirmed
		//			quit game session
		//			if localHwIndex doesn't match default user
		//				change default user to this new hwIndex (i.e., profile is changing too, so any other settings other than just the index)
		//			move default user to lobby
		//			if denied from lobby (error, or lobby is full, etc.)
		//				move to Main Menu (one after Press START screen)
		//		else (not confirmed)
		//			dismiss invite, allow default user to continue playing
	}
	void tGameApp::fOnMuteListChanged( )
	{
		mGameAppSession.fOnMuteListChanged( );
	}
	void tGameApp::fOnPartyMembersChanged( ) 
	{
		if( mCurrentLevel && !mCurrentLevel->fRootMenu( ).fIsNull( ) )
			mCurrentLevel->fRootMenu( ).fHandleCanvasEvent( Logic::tEvent( ApplicationEvent::cOnPartyMembersChange ) );
	}
	void tGameApp::fOnInputDevicesChanged( u32 connected )
	{
		for( u32 i = 0; i < mPlayers.fCount( ); ++i )
		{
			const u32 hwIndex = mPlayers[ i ]->fUser( )->fLocalHwIndex( );

			// Connected
			if( connected & ( 1 << hwIndex ) )
				continue;

			fGameAppSession( )->fSendSessionEvent( ApplicationEvent::cOnPlayerLoseInput, hwIndex );
		}
	}
	void tGameApp::fOnStorageDeviceChanged( )
	{
		for( u32 i = 0; i < mPlayers.fCount( ); ++i )
		{
			mPlayers[ i ]->fOnStorageDeviceChanged( );
		}
	}
	void tGameApp::fTickCameras( f32 dt )
	{
		profile_pix("fTickCameras");

		u32 numActivePlayers = 0;
		Math::tVec3f avgLookAt = Math::tVec3f::cZeroVector;
		for( u32 i = 0; i < mPlayers.fCount( ); ++i )
		{
			if( mPlayers[ i ]->fIsActive( ) )
			{
				mPlayers[ i ]->fOnTick( dt );

				// Don't move the shadow map for virtual viewports
				if( !mPlayers[ i ]->fUser( )->fViewport( )->fIsVirtual( ) )
				{
					++numActivePlayers;
					avgLookAt += mPlayers[ i ]->fUser( )->fViewport( )->fRenderCamera( ).fGetTripod( ).mLookAt;
				}
			}
		}

		if( numActivePlayers )
		{
			avgLookAt /= ( f32 )numActivePlayers;
			if( mDefaultLight )
				mDefaultLight->fUpdateShadowMapTarget( avgLookAt );
		}

		mDeathLimiter = 0;
	}

	void tGameApp::fTickCanvas( f32 dt )
	{
		tGameAppBase::fTickCanvas( dt );
		
	}

	void tGameApp::fShutdownApp( )
	{
		sync_deregister_thread( );

		fSetWaveListTable( tResourcePtr( ) );
		for( u32 i = 0; i < mLevelResources.fCount( ); ++i )
			mLevelResources[ i ]->fUnload( this );
		mLevelResources.fSetCount( 0 );
		mPlayers.fResize( 0 );
		mFrontEndPlayer.fRelease( );
		mPostEffectMgr.fRelease( );
		mBlackBar.fDeleteSelf( );
		
		tGameAppBase::fShutdownApp( );
	}

	void tGameApp::fPollInputDevices( )
	{
		mGameAppSession.fPollInputDevices( mLocalUsers );
	}

	void tGameApp::fGetResourceIDs( tGrowableArray<tResourceId>& resourceIds, const tFilePathPtr resourceFolders[], u32 resourceFolderCount )
	{
		fResourceDepot( )->fQueryCommonResourcesByFolder( resourceIds, resourceFolders, resourceFolderCount );
	}
	void tGameApp::fOnLevelLoadBegin( )
	{
		// sound stuff
		{
			fSoundSystem( )->fMasterSource( )->fSetSwitch( cLevelSwitchGroup, mCurrentLevelLoadInfo.mAudioID );
			mSoundSystem->fSetState( AK::STATES::MUSIC_CAMPAIGN::GROUP, AK::STATES::MUSIC_CAMPAIGN::STATE::LOAD_SCREEN );

			if( mCurrentLevelLoadInfo.mDlcNumber == 1 )
			{ 
				mDLCMusicPushed = 1;
				fSoundSystem( )->fMasterSource( )->fHandleEvent( AK::EVENTS::PLAY_MUSIC_MENU_DLC01 );
			}
			else if( mCurrentLevelLoadInfo.mDlcNumber == 2 )
			{ 
				mDLCMusicPushed = 2;
				fSoundSystem( )->fMasterSource( )->fHandleEvent( AK::EVENTS::PLAY_MUSIC_MENU_DLC02 );
			}
		}


		mCurrentLevel = 0;

		mTeamOrientation.fFill( Math::tMat3f::cIdentity );
		mTeamOrientationSet.fFill( false );
		mDifficultyOverride = ~0;

		// Inform the game app session
		mGameAppSession.fOnLevelLoadBegin( );

		for( u32 i = 0; i < mPlayers.fCount( ); ++i )
			if( mPlayers[ i ]->fIsActive( ) )
				mPlayers[ i ]->fOnLevelLoadBegin( );

		fApplyRichPresence( );

		fSceneGraph( )->fClear( ); // for determinism
	}
	void tGameApp::fOnLevelLoadEnd( )
	{
#ifdef sig_devmenu
		if( Debug_Memory_MaxArtistVramEnable )
		{
#ifdef platform_xbox360
			const f32 vramMemUsage = mVramHeap.fNumBytesBeingManaged( )/(1024.f*1024.f); //profile_query_mem( cProfileMemVideo )/(1024.f*1024.f));
			const f32 artistMax = Debug_Memory_MaxArtistVramAmount;
			if( vramMemUsage > artistMax )
			{
				char buff[1024]={0};
				Log::fSprintf( 0, buff, sizeof( buff ), "Exceeded artist-specific VRAM (texture/geometry) allowance (current=%f, max=%f)", vramMemUsage, artistMax );
				Log::fFatalError( buff );
			}
#endif
		}
#endif//sig_devmenu
		if( mCurrentLevel )
			mCurrentLevel->fOnLevelLoadEnd( );

		if( mCurrentLevelLoadInfo.fIsFrontEnd( ) )
		{
			if( !mHasEverReachedFrontEnd && mDLCEnumerator.fFoundCorrupt( ) )
				tGameApp::fInstance( ).fShowErrorDialog( tGameApp::cCorruptDLC, tGameApp::fInstance( ).fFrontEndPlayer( )->fUser( ).fGetRawPtr( ) );
			else if( !mHasEverReachedFrontEnd && mDLCEnumerator.fNeedToLoginForSomeContent( ) )
				tGameApp::fInstance( ).fShowErrorDialog( tGameApp::cUnLicensedDLC, tGameApp::fInstance( ).fFrontEndPlayer( )->fUser( ).fGetRawPtr( ) );

			mHasEverReachedFrontEnd = true;
		}

		mLoadingFrontEndBecauseProfileCouldntSave = false;
		mLevelNameReadyToRead = false;

		if( mCurrentLevelLoadInfo.fIsFrontEnd( ) && mDLCMusicPushed )
		{
			if( mDLCMusicPushed == 1 )
				fSoundSystem( )->fMasterSource( )->fHandleEvent( AK::EVENTS::STOP_MUSIC_MENU_DLC01 );
			else if( mDLCMusicPushed == 2 )
				fSoundSystem( )->fMasterSource( )->fHandleEvent( AK::EVENTS::STOP_MUSIC_MENU_DLC02 );

			mDLCMusicPushed = 0;
		}
	}
	void tGameApp::fOnLevelUnloadBegin( )
	{
		mCurrentLevelLoadInfo = mNextLevelLoadInfo;
		mNextLevelLoadInfo = tLevelLoadInfo( );
		mLevelNameReadyToRead = true;

		if( mCurrentLevelLoadInfo.mGameMode.fIsCoOp( ) && mCurrentLevelLoadInfo.mMapType == GameFlags::cMAP_TYPE_MINIGAME )
			fSetSingleScreenCoop( true );
		else
			fSetSingleScreenCoop( false );

		// Inform the game app that the level is unloading so it should use
		// the mCurrentLevelLoadInfo as the next
		mGameAppSession.fOnLevelUnloadBegin( ); 

		if( mCurrentLevel )
			mCurrentLevel->fOnLevelUnloadBegin( );

		// NOTE! we call fOnLevelUnloadBegin on both players, regardless of "active" or not - this IS intentional
		for( u32 i = 0; i < mPlayers.fCount( ); ++i )
			mPlayers[ i ]->fOnLevelUnloadBegin( );

		fSetWaveListTable( tResourcePtr( ) );
		for( u32 i = 0; i < mLevelResources.fCount( ); ++i )
			mLevelResources[ i ]->fUnload( this );
		mLevelResources.fSetCount( 0 );
		mCurrentLevel = 0;
		mCallbacks.fSetCount( 0 );

		// We wait until level unload begins, means player won't see viewport switch
		// We do this last, so that the level doesn't change and the number of players won't change - Randy
		fConfigureAppForGameMode( mCurrentLevelLoadInfo );
	}
	void tGameApp::fOnLevelUnloadEnd( )
	{
		mSimulationBegan = false;

		// Clear the viewport cameas
		fResetViewportCameras( );

		// NOTE! we call fOnLevelUnloadEnd on both players, regardless of "active" or not - this IS intentional
		for( u32 i = 0; i < mPlayers.fCount( ); ++i )
			mPlayers[ i ]->fOnLevelUnloadEnd( );

		mPostEffectMgr->fResetEffectsData( );

		tGameApp::fInstance( ).fSoundSystem( )->fMasterSource( )->fHandleEvent( AK::EVENTS::STOP_AMBIENCE );
	}
	void tGameApp::fOnSimulationBegin( )
	{
		mSimulationBegan = true;

		// Begin simulation on all players
		const u32 playerCount = mPlayers.fCount( );
		sync_event_v_c( playerCount, tSync::cSCPlayer );
		for( u32 p = 0; p < playerCount; ++p )
			mPlayers[ p ]->fOnSimulationBegin( );

		if( mCurrentLevel )
			mCurrentLevel->fOnSimulationBegin( );
		else
			log_warning( 0, "Simulation beginning without LevelLogic" );

		fSnapshotMemoryConsumption( );
	}
	void tGameApp::fAddLevelResource( const tResourcePtr& levelResource )
	{
		mLevelResources.fFindOrAdd( levelResource );
		levelResource->fLoadDefault( this );
	}
	const tFilePathPtr & tGameApp::fDefaultLoadScriptPath( )
	{
		return cDefaultLoadScriptPath;
	}

	void tLevelLoadInfo::fLog( ) const
	{
		log_line( 0, "Level Load Info: Map = " << mMapPath << ", Load Script = " << mLoadScript << ", Dlc Num = " << mDlcNumber );
	}
	void tLevelLoadInfo::fFillEmptyWithDefaults( )
	{
		sigassert( mMapPath.fExists( ) );
		if( mGameMode.fInvalid( ) )
		{
			mGameMode.fSetState( tGameMode::cStateNormal );

#ifdef sig_devmenu // only for dev builds
			if( StringUtil::fStrStrI( mMapPath.fCStr( ), "HeadToHead" ) )
				mGameMode.fAddVersusFlag( );
#endif//sig_devmenu
		}
		if( mLoadScript.fLength( ) == 0 )
			mLoadScript = cDefaultLoadScriptPath;
		if( mWaveList.fLength( ) == 0 )
		{
			std::string waveListStr = StringUtil::fStripExtension( mMapPath.fCStr( ) );
			waveListStr += std::string( "_" ) + GameFlags::fDIFFICULTYEnumToValueString( mDifficulty ).fCStr( ) + ".tab";
			tFilePathPtr waveList( waveListStr );
			if( tGameApp::fInstance( ).fResourceDepot( )->fResourceExists( tResourceId::fMake<tDataTableFile>( waveList ).fGetPath( ) ) )
				mWaveList = waveList;
			else
			{
				//log_warning( 0, "Could not load wave list: " << waveList << " using default." );
				mWaveList = tFilePathPtr::fSwapExtension( mMapPath, ".tab" );
			}
		}
		if( mCountry == GameFlags::cCOUNTRY_COUNT || mCountry == GameFlags::cCOUNTRY_DEFAULT )
			mCountry = GameFlags::cCOUNTRY_USA;
		if( mMapDisplayName.length( ) == 0 )
			mMapDisplayName.fFromCStr( L"No level name" );
		if( mTickets == ~0 )
			mTickets = mPotentialTickets[ mDifficulty ];
		if( mMoney < 0 )
			mMoney = mPotentialMoneys[ mDifficulty ];
	}
	b32 tLevelLoadInfo::fIsAssetEquivalent( const tLevelLoadInfo& prevLevel ) const
	{
		if( mGameMode == prevLevel.mGameMode &&
			mMapPath == prevLevel.mMapPath &&
			mWaveList == prevLevel.mWaveList )
			return true;
		return false;
	}

	namespace 
	{
		enum tLevelResourceColumns
		{
			cLevelResourceMap,
			cLevelResourceLoadScript,
			cLevelResourceDescription,
			cLevelResourcePreviewImage,
			cLevelResourceAudioID,
			cLevelResourceLocked,
			cLevelResourcePlayerCountry,
			cLevelResourceDlcNumber,
			cLevelResourceAvailableInTrial,
			cLevelResourceCasualTickets,
			cLevelResourceNormalTickets,
			cLevelResourceHardTickets,
			cLevelResourceEliteTickets,
			cLevelResourceGeneralTickets,
			cLevelResourceCasualMoney,
			cLevelResourceNormalMoney,
			cLevelResourceHardMoney,
			cLevelResourceEliteMoney,
			cLevelResourceGeneralMoney,
			cLevelResourceRankDesc,
			cLevelResourceRankThreshold0,
			cLevelResourceRankThreshold1,
			cLevelResourceRankThreshold2,
			cLevelResourceRankThreshold3,
			cLevelResourceColumnCount
		};
	}
	namespace { static const tStringPtr cStringFrontEnd( "FrontEnd" ); }
	namespace { static const tStringPtr cStringCampaign( "Campaign" ); }
	namespace { static const tStringPtr cStringHeadToHead( "HeadToHead" ); }
	namespace { static const tStringPtr cStringSurvival( "Survival" ); }
	namespace { static const tStringPtr cStringMiniGame( "Minigame" ); }
	namespace { static const tStringPtr cStringDevSinglePlayer( "Dev SP & CoOp" ); }

	tLevelLoadInfo tGameApp::fQueryLevelLoadInfoFromTable( u32 mapType, u32 levelIndex ) const
	{
		const tDataTableFile& levelResources = fDataTable( cDataTableLevelResources );
		const tDataTable& table = levelResources.fIndexTable( mapType );
		tLevelLoadInfo o;
		fFillLevelLoadInfo( o, table, mapType, levelIndex );
		return o;
	}
	tLevelLoadInfo tGameApp::fQueryLevelLoadInfoFromPath( const tFilePathPtr& mapPath ) const
	{
		tFilePathPtr correctedPath;

		std::string ext = StringUtil::fGetExtension( mapPath.fCStr( ) );
		if( ext == ".sigb" )
		{
			std::string newPath = StringUtil::fStripExtension( mapPath.fCStr( ) ) + ".sigml";
			correctedPath = tFilePathPtr( newPath.c_str( ) );
		}
		else
			correctedPath = mapPath;

		const tDataTableFile& levelResources = fDataTable( cDataTableLevelResources );
		for( u32 i = 0; i < levelResources.fTableCount( ); ++i )
		{
			const tDataTable& table = levelResources.fIndexTable( i );
			for( u32 j = 0; j < table.fRowCount( ); ++j )
			{
				if( correctedPath == table.fIndexByRowCol<tFilePathPtr>( j, cLevelResourceMap ) )
				{
					tLevelLoadInfo levelLoadInfo;
					GameFlags::tMAP_TYPE mapFront = GameFlags::cMAP_TYPE_COUNT;
					
					if( cStringFrontEnd == table.fName( ) )				mapFront = GameFlags::cMAP_TYPE_FRONTEND;
					else if( cStringCampaign == table.fName( ) )		mapFront = GameFlags::cMAP_TYPE_CAMPAIGN;
					else if( cStringHeadToHead == table.fName( ) )		mapFront = GameFlags::cMAP_TYPE_HEADTOHEAD;
					else if( cStringSurvival == table.fName( ) )		mapFront = GameFlags::cMAP_TYPE_SURVIVAL;
					else if( cStringMiniGame == table.fName( ) )		mapFront = GameFlags::cMAP_TYPE_MINIGAME;					
					else if( cStringDevSinglePlayer == table.fName( ) )	mapFront = GameFlags::cMAP_TYPE_DEVSINGLEPLAYER;

					fFillLevelLoadInfo( levelLoadInfo, table, mapFront, j );
					return levelLoadInfo;
				}
			}
		}

		return tLevelLoadInfo( tFilePathPtr( correctedPath ) );
	}
	void tGameApp::fFillLevelLoadInfo( tLevelLoadInfo& levelLoadInfo, const tDataTable& table, u32 mapType, u32 levelIndex ) const
	{
		levelLoadInfo.mMapType = ( u32 )mapType;
		levelLoadInfo.mLevelIndex = ( u32 )levelIndex;

		switch( mapType )
		{
		case GameFlags::cMAP_TYPE_FRONTEND:
			levelLoadInfo.mGameMode.fSetState( tGameMode::cStateFrontEnd );
			break;
		case GameFlags::cMAP_TYPE_HEADTOHEAD:
			levelLoadInfo.mGameMode.fSetState( tGameMode::cStateNormal, tGameMode::cFlagVersus );
			break;
		default:
			levelLoadInfo.mGameMode.fSetState( tGameMode::cStateNormal );
			break;
		}

		levelLoadInfo.mMapDisplayName		= tGameApp::fInstance( ).fLocString( table.fRowName( levelIndex ) );
		levelLoadInfo.mMapPath				= table.fIndexByRowCol<tFilePathPtr>( levelIndex, cLevelResourceMap );
		levelLoadInfo.mLoadScript			= table.fIndexByRowCol<tFilePathPtr>( levelIndex, cLevelResourceLoadScript );
		levelLoadInfo.mDescriptionLocKey	= table.fIndexByRowCol<tStringPtr>( levelIndex, cLevelResourceDescription );
		levelLoadInfo.mPreviewImage			= table.fIndexByRowCol<tFilePathPtr>( levelIndex, cLevelResourcePreviewImage );
		levelLoadInfo.mAudioID				= table.fIndexByRowCol<tStringPtr>( levelIndex, cLevelResourceAudioID );
		levelLoadInfo.mCountry				= ( GameFlags::tCOUNTRY )GameFlags::fCOUNTRYValueStringToEnum( table.fIndexByRowCol<tStringPtr>( levelIndex, cLevelResourcePlayerCountry ) );
		levelLoadInfo.mDlcNumber			= table.fIndexByRowCol<u32>( levelIndex, cLevelResourceDlcNumber );
		levelLoadInfo.mAvailableInTrial		= table.fIndexByRowCol<u32>( levelIndex, cLevelResourceAvailableInTrial );
		levelLoadInfo.mDifficulty			= Game_InitialDifficulty;
		levelLoadInfo.mRankDescLocKey		= table.fIndexByRowCol<tStringPtr>( levelIndex, cLevelResourceRankDesc );

		sigassert( cLevelResourceRankThreshold3 - cLevelResourceRankThreshold0 == tLevelLoadInfo::cRankCount - 1 );
		for( u32 i = 0; i < tLevelLoadInfo::cRankCount; ++i )
			levelLoadInfo.mRankThresholds[ i ] = table.fIndexByRowCol<u32>( levelIndex, cLevelResourceRankThreshold0 + i );

		sigassert( cLevelResourceGeneralTickets - cLevelResourceCasualTickets == GameFlags::cDIFFICULTY_COUNT-1 );
		for( u32 i = 0; i < GameFlags::cDIFFICULTY_COUNT; ++i )
			levelLoadInfo.mPotentialTickets[ i ] = table.fIndexByRowCol<u32>( levelIndex, cLevelResourceCasualTickets + i );

		sigassert( cLevelResourceGeneralMoney - cLevelResourceCasualMoney == GameFlags::cDIFFICULTY_COUNT-1 );
		for( u32 i = 0; i < GameFlags::cDIFFICULTY_COUNT; ++i )
			levelLoadInfo.mPotentialMoneys[ i ] = table.fIndexByRowCol<u32>( levelIndex, cLevelResourceCasualMoney + i );
	}
	void tGameApp::fSetLevelLoadState( tGameLoadAppState * state )
	{
		sigassert( state && "Cannot set level load state to NULL" );

		if( !state->fCanSpawn( ) )
			mDelayedLoadScreen.fReset( state );

		fNewApplicationState( tApplicationStatePtr( state ) );
	}
	void tGameApp::fLoadLevelDelayed( )
	{
		fSetLevelLoadState( NEW tGameLoadAppState( mNextLevelLoadInfo, true, false  ) );
	}

	void tGameApp::fLoadLevelNowThatSessionIsReady( )
	{
		if( !mDelayedLoadScreen )
			return;

		tGameLoadAppState * appState = static_cast<tGameLoadAppState *>( mDelayedLoadScreen.fGetRawPtr( ) );
		sigassert( appState );
		//appState->fAllowLoad( true );
		appState->fAllowSpawn( true );
		mDelayedLoadScreen.fRelease( );
	}
	void tGameApp::fLoadLevel( const tLevelLoadInfo& levelLoadInfo )
	{
		if( !fGameMode( ).fIsFrontEnd( ) && fCurrentLevel( ) )
		{
			// this may not be the best way to detect this.
			fCurrentLevel( )->fLevelExited( );
		}

		mNextLevelLoadInfo = levelLoadInfo;

		if( GameApp_ForceCoOp )
			mNextLevelLoadInfo.mGameMode.fAddCoOpFlag( );

		mNextLevelLoadInfo.fFillEmptyWithDefaults( );
		mNextLevelLoadInfo.fLog( );

		mNextLevelQueued = true;

		if( mNextLevelLoadInfo.mGameMode.fIsNet( ) )
			FX::tParticleSystem::fSetEmissionReductionFactor( fReduceParticlesNetworked );
		else
			FX::tParticleSystem::fSetEmissionReductionFactor( fReduceParticlesLocal );

		fFadeThroughBlack( 0.4f, 0.4f, 0.4f );
	}
	void tGameApp::fLoadLevelFromScript( u32 mapFront, u32 levelIndex, Sqrat::Function& levelInfoCallback )
	{
		tLevelLoadInfo levelLoadInfo = fQueryLevelLoadInfoFromTable( ( GameFlags::tMAP_TYPE )mapFront, levelIndex );
		if( !levelInfoCallback.IsNull( ) )
			levelInfoCallback.Execute( &levelLoadInfo );
		fLoadLevel( levelLoadInfo );
	}
	void tGameApp::fLoadFrontEnd( b32 firstBootUp )
	{
		tLevelLoadInfo levelLoadInfo = fQueryLevelLoadInfoFromTable( GameFlags::cMAP_TYPE_FRONTEND, 0 );
		if( firstBootUp )
			levelLoadInfo.mLoadScript = fExtraDemoMode( ) ? cExtraBootUpGameScriptPath : cBootUpGameScriptPath;
		fLoadLevel( levelLoadInfo );
	}
	void tGameApp::fReloadCurrentLevel( )
	{
		mCurrentLevelLoadInfo.mSkipBriefing = true;
		mCurrentLevelLoadInfo.mHighestWaveReached = -1;
		fLoadLevel( mCurrentLevelLoadInfo );
	}
	void tGameApp::fConfigureAppForGameMode( const tLevelLoadInfo& levelLoadInfo )
	{
		const tGameMode& gameMode = levelLoadInfo.mGameMode;
		sigassert( mPlayers.fCount( ) >= 1 && "Sanity: Must have a first player already" );

		if( gameMode.fIsSinglePlayer( ) )
		{
			fSetSimulate( true );

			// Single player == 1 player
			mPlayers.fResize( 1 );

			mPlayers[ 0 ] = mFrontEndPlayer;
			mPlayers[ 0 ]->fSetCountry( (GameFlags::tCOUNTRY)levelLoadInfo.mCountry );
		}
		else
		{
			mPlayers.fResize( 2 );

			// If none of the players are the front end player, then we need to reassign
			u32 remoteIndex;
			if( mPlayers[ 0 ] == mFrontEndPlayer )
			{	
				remoteIndex = 1;
			}
			else if( mPlayers[ 1 ] == mFrontEndPlayer )
			{
				remoteIndex = 0;
			}
			else
			{
				mPlayers[ 0 ] = mFrontEndPlayer;
				remoteIndex = 1;
			}

			if( gameMode.fIsNet( ) )
			{
				tUserArray remoteUsers;
				mGameAppSession.fQueryRemoteUsers( remoteUsers );

				if( remoteUsers.fCount( ) )
				{
					// If the player doesn't exist, the user doesn't exist, or the user isn't the remote user
					if( !mPlayers[ remoteIndex ] || !mPlayers[ remoteIndex ]->fUser( ) || ( mPlayers[ remoteIndex ]->fUser( ) != remoteUsers[ 0 ] ) )
					{
						// Assign the remote user to the player
						mPlayers[ remoteIndex ].fReset( NEW tPlayer( remoteUsers[ 0 ] ) );
					}

					mPlayers[ remoteIndex ]->fUser( )->fDevMenu( ).fInitDevMenu( mDefaultFont );
				}
				else
				{
					log_warning( 0, "No remote user existed for net game - creating false user" );
					mPlayers[ remoteIndex ].fReset( NEW tPlayer( tUserPtr( NEW tUser( tUserInfo( ) ) ) ) );
				}

				const u32 localIndex = remoteIndex == 0 ? 1 : 0;
				
				// If the player doesn't exist, the user doesn't exist, or the user isn't the front end player's user
				if( !mPlayers[ localIndex ] || !mPlayers[ localIndex ]->fUser( ) || ( mPlayers[ localIndex ]->fUser( ) != mFrontEndPlayer->fUser( ) ) )
				{
					mPlayers[ localIndex ].fReset( NEW tPlayer( mFrontEndPlayer->fUser( ) ) );
					mFrontEndPlayer = mPlayers[ localIndex ];
					tXLSP::fInstance( ).fSetFrontEndPlayer( mFrontEndPlayer->fUser( ) );
				}
			}
			else
			{
				fSetSimulate( true );

				tPlayerPtr & ptr = mPlayers[ 1 ];

				// We don't have a valid player/user here so find a user and create a player
				if( ptr.fNull( ) || ptr->fUser( ).fNull( ) || !ptr->fUser( )->fIsLocal( ) || !ptr->fUser( )->fSignedIn( ) )
				{
					u32 indexOfP1User = mPlayers[ 0 ]->fUser( )->fLocalHwIndex( );
					sigassert( indexOfP1User != ~0 && "Sanity: DefaultPlayer associated with invalid user" );
					
					/*sigassert( mSecondLocalUser );
					u32 nextValidUser = mLocalUsers.fIndexOf( mSecondLocalUser );
					sigassert( nextValidUser != ~0 && "Sanity: No valid user for the second player" );

					ptr.fReset( NEW tPlayer( mLocalUsers[ nextValidUser ] ) );
					ptr->fSetPlayerIndex( 1 );*/
				}
			}

			// If the game mode is net we have to ensure a consistent player sorting,
			// namely that mPlayers[ 0 ] is the host player on both boxes
			if( gameMode.fIsNet( ) )
			{
				// Ensure that player 0 is the host
				if( mPlayers[ 0 ]->fUser( )->fIsLocal( ) && !mGameAppSession.fIsHost( ) )
					fSwap( mPlayers[ 0 ], mPlayers[ 1 ] );

				sigassert( !mPlayers[ 0 ]->fUser( )->fIsLocal( ) || mGameAppSession.fIsHost( ) );
			}

			sigassert( mPlayers[ 0 ] );
			sigassert( mPlayers[ 1 ] );
			mPlayers[ 0 ]->fSetCountry( (GameFlags::tCOUNTRY)levelLoadInfo.mCountry );
			mPlayers[ 1 ]->fSetCountry( (GameFlags::tCOUNTRY)(gameMode.fIsVersus( ) ? mPlayers[ 0 ]->fEnemyCountry( ) : mPlayers[ 0 ]->fCountry( )) );
		}

		//mSecondLocalUser.fRelease( ); //this should always be released if we are starting a game, no matter the mode
		fConfigureViewportsForGameMode( levelLoadInfo.mGameMode ); 
	}

	void tGameApp::fConfigureViewportsForGameMode( const tGameMode& gameMode )
	{
		const b32 splitScreen = gameMode.fIsSplitScreen( );
		const u32 numLocalUsers = splitScreen ? 2 : 1;
		const u32 numViewports = gameMode.fIsMultiPlayer( ) && !mSingleScreenCoop ? 2 : 1;

		fScreen( )->fSetLimitShadowMapLayerCount( splitScreen ? Game_Renderer_Shadows_SplitScreen : Game_Renderer_Shadows_OneScreen );
		if( Game_Renderer_GroundCover_ForceForever )
			Gfx::tGroundCoverCloud::fSetForcedVisibility( Gfx::tGroundCoverCloudDef::cVisibilityForever );
		else
			Gfx::tGroundCoverCloud::fSetForcedVisibility( Gfx::tGroundCoverCloudDef::cVisibilityCount );

		tUserArray remoteUsers;
		mGameAppSession.fQueryRemoteUsers( remoteUsers );

		// If we don't have any remote users in a net game then we had to have
		// created one in fConfigureAppForGameMode and so we need to grab that and use it
		if( !remoteUsers.fCount( ) && gameMode.fIsNet( ) )
		{
			const u32 playerCount = mPlayers.fCount( );
			for( u32 p = 0; p < playerCount; ++p )
			{
				if( !mPlayers[ p ]->fUser( )->fIsLocal( ) )
				{
					remoteUsers.fPushBack( mPlayers[ p ]->fUser( ) );
					break;
				}
			}
		}

		if( mSoundSystem )
		{
			b32 frontEnd = gameMode.fIsFrontEnd( );

			u32 mask = 0; 
			u32 masterMask = 0;
			for( u32 i = 0; i < mPlayers.fCount( ); ++i ) 
			{ 
				masterMask = fSetBits( mask, (1<<( mPlayers[ i ]->fListenerIndex( ) )) ); 
				if( mPlayers[ i ]->fUser( )->fIsLocal( ) ) 
					mask = fSetBits( mask, (1<<( mPlayers[ i ]->fListenerIndex( ) )) ); 
			}

			if( masterMask == 0 && frontEnd )
				masterMask = 1;

			mSoundSystem->fSetMasterMask( masterMask );
			mSoundSystem->fSetListenerMask( mask );
		}

		tGameAppBase::fChangeViewportCount( numViewports, numLocalUsers, true, &remoteUsers );
		sigassert( mPlayers.fCount( ) == 1 || ( mPlayers[ 0 ]->fUser( ) != mPlayers[ 1 ]->fUser( ) ) );
		fAssignViewportsToPlayers( gameMode );

		fSetViewportProportions( gameMode );

		for( u32 i = 0; i < fScreen( )->fGetViewportCount( ); ++i )
			fScreen( )->fViewport( i )->fSetPostEffectSequence( tStringPtr( "gameCam" ) );
	}
	void tGameApp::fAssignViewportsToPlayers( const tGameMode& gameMode )
	{
		const b32 splitScreen = gameMode.fIsSplitScreen( ) && !mSingleScreenCoop;
		if( splitScreen )
		{
			fFrontEndPlayer( )->fUser( )->fSetViewport( fScreen( ), fScreen( )->fViewport( 0 ) );
			fSecondaryPlayer( )->fUser( )->fSetViewport( fScreen( ), fScreen( )->fViewport( 1 ) );
		}
	}
	void tGameApp::fSetViewportProportions( const tGameMode& gameMode )
	{
		const b32 splitScreen = gameMode.fIsSplitScreen( ) && !mSingleScreenCoop;
		if( splitScreen )
		{
			const f32 barWidth = 4.0f;
			const f32 screenWidth = 1280;
			const f32 screenHeight = 720;
			Math::tRect barRect;
			if( Game_SplitHorizontal )
			{
				fFrontEndPlayer( )->fUser( )->fViewport( )->fSetClipBox( Math::tRect( 0.0f, 0.0f, 0.5f, 1.0f ) );
				fSecondaryPlayer( )->fUser( )->fViewport( )->fSetClipBox( Math::tRect( 0.5f, 0.0f, 1.0f, 1.0f ) );
				barRect.mL = -screenWidth * 0.5f;
				barRect.mR = screenWidth * 0.5f;
				barRect.mT = -barWidth * 0.5f;
				barRect.mB = barWidth * 0.5f;
			}
			else
			{
				fFrontEndPlayer( )->fUser( )->fViewport( )->fSetClipBox( Math::tRect( 0.0f, 0.0f, 1.0f, 0.5f ) );
				fSecondaryPlayer( )->fUser( )->fViewport( )->fSetClipBox( Math::tRect( 0.0f, 0.5f, 1.0f, 1.0f ) );
				barRect.mT = -screenHeight * 0.5f;
				barRect.mB = screenHeight * 0.5f;
				barRect.mL = -barWidth * 0.5f;
				barRect.mR = barWidth * 0.5f;
			}

			// Create black bar to separate the viewports
			mBlackBar.fCanvas( )->fDynamicCast< Gui::tTexturedQuad >( )->fSetRect( barRect );
			mBlackBar.fCanvas( )->fDynamicCast< Gui::tTexturedQuad >( )->fSetPosition( screenWidth * 0.5f, screenHeight * 0.5f, 0.9f );
			mRootCanvas.fAddChild( mBlackBar );
		}
		else
		{
			fFrontEndPlayer( )->fUser( )->fViewport( )->fSetClipBox( Math::tRect( 0.0f, 0.0f, 1.0f, 1.0f ) );

			if( tPlayer * otherPlayer = fSecondaryPlayer( ) )
			{
				if( otherPlayer->fUser( )->fViewport( ) != fFrontEndPlayer( )->fUser( )->fViewport( ) &&
					!otherPlayer->fUser( )->fViewport( )->fIsVirtual( ) )
					log_warning( 0, "There is a second player with a unique non-virtual viewport in single screen mode" );
			}

			mRootCanvas.fRemoveChild( mBlackBar );
		}
	}
	void tGameApp::fSetCurrentLevel( tLevelLogic* level )
	{
		if( mCurrentLevel )
			log_warning( 0, "Level already exists!  There are probably multiple level scripts." );
		mCurrentLevel = level;
	}
	Sqrat::Object tGameApp::fCurrentLevelForScript( ) 
	{ 
		return mCurrentLevel 
			? mCurrentLevel->fOwnerEntity( )->fScriptLogicObject( ) 
			: Sqrat::Object( ); 
	}
	void tGameApp::fSetCurrentLevelForScript( Sqrat::Object obj ) 
	{ 
		fSetCurrentLevel( tScriptOrCodeObjectPtr<tLevelLogic>( obj ).fCodeObject( ) ); 
	}
	void tGameApp::fOnLevelSpawnBegin( ) 
	{
		mSpawningCurrentLevel = true;
		mGameAppSession.fOnLevelSpawn( );
	}
	void tGameApp::fOnLevelSpawnEnd( )
	{
		mSpawningCurrentLevel = false;

		if( mCurrentSaveGame )
		{
			mCurrentSaveGame->fOnLevelSpawnEnd( );
			mCurrentSaveGame.fRelease( );
		}
	}

	u32 tGameApp::fNumLevelsInTable( u32 mapType ) const
	{
		sigassert( mapType >= GameFlags::cMAP_TYPE_FRONTEND && mapType < GameFlags::cMAP_TYPE_COUNT );
		const tDataTableFile& levelResources = fDataTable( cDataTableLevelResources );
		const tDataTable& table = levelResources.fIndexTable( mapType );
		return table.fRowCount( );
	}
	u32 tGameApp::fNumCampaignLevels( u32 dlc ) const
	{
		sigassert( dlc < GameFlags::cDLC_COUNT );
		const tDataTableFile& levelResources = fDataTable( cDataTableLevelResources );
		const tDataTable& table = levelResources.fIndexTable( GameFlags::cMAP_TYPE_CAMPAIGN );
		
		u32 levelCount = 0;
		for( u32 i = 0; i < table.fRowCount( ); ++i )
		{
			if( table.fIndexByRowCol< u32 >( i, cLevelResourceDlcNumber ) == dlc )
				levelCount++;
		}
		return levelCount;
	}
	u32 tGameApp::fNextCampaignLevel( u32 index, u32 dlc ) const
	{
		sigassert( dlc < GameFlags::cDLC_COUNT );
		const tDataTableFile& levelResources = fDataTable( cDataTableLevelResources );
		const tDataTable& table = levelResources.fIndexTable( GameFlags::cMAP_TYPE_CAMPAIGN );

		u32 nextLevelIndex = index + 1;

		for( u32 nextLevelIndex = index + 1; nextLevelIndex < table.fRowCount( ); ++nextLevelIndex )
		{
			if( table.fIndexByRowCol< u32 >( nextLevelIndex, cLevelResourceDlcNumber ) == dlc )
				return nextLevelIndex;
		}
		return -1;
	}
	b32 tGameApp::fIsCampaignComplete( u32 dlc, tPlayer* player ) const
	{
		sigassert( dlc < GameFlags::cDLC_COUNT );
		sigassert( player );
		const tDataTableFile& levelResources = fDataTable( cDataTableLevelResources );
		const tDataTable& table = levelResources.fIndexTable( GameFlags::cMAP_TYPE_CAMPAIGN );

		const u32 highestLevel = player->fProfile( )->fGetHighestLevelReached( dlc );
		for( u32 i = highestLevel; i < table.fRowCount( ); ++i )
		{
			if( table.fIndexByRowCol< u32 >( i, cLevelResourceDlcNumber ) == dlc )
				return false;
		}
		return true;
	}
	const char* tGameApp::fLevelNameInTable( u32 mapType, u32 ithLevel ) const
	{
		const tDataTableFile& levelResources = fDataTable( cDataTableLevelResources );
		const tDataTable& table = levelResources.fIndexTable( mapType );

		if( ithLevel >= table.fRowCount( ) )
		{
			log_warning( 0, "Level: " << ithLevel << " does not exist in LevelResources:" << table.fName( ) );
			return NULL;
		}

		return table.fRowName( ithLevel ).fCStr( );
	}
	u32 tGameApp::fLevelDLCInTable( u32 mapType, u32 ithLevel ) const
	{
		const tDataTableFile& levelResources = fDataTable( cDataTableLevelResources );
		const tDataTable& table = levelResources.fIndexTable( mapType );

		if( ithLevel >= table.fRowCount( ) )
		{
			log_warning( 0, "Level: " << ithLevel << " does not exist in LevelResources:" << table.fName( ) );
			return NULL;
		}

		return table.fIndexByRowCol<u32>( ithLevel, cLevelResourceDlcNumber );
	}
	u32 tGameApp::fRandomLevelIndex( tRandom& rand, GameFlags::tMAP_TYPE mapType, u32 legalAddOnFlags ) const
	{
		const u32 numLevelsInTable = fNumLevelsInTable( mapType );

		if( numLevelsInTable > 0 )
		{
			tGrowableArray<u32> levelsTriedAlready;
			levelsTriedAlready.fSetCapacity( numLevelsInTable );

			while( true )
			{
				const u32 randomIndex = ( u32 )rand.fIntInRange( 0, numLevelsInTable - 1 );
				if( levelsTriedAlready.fFind( randomIndex ) )
					continue; // already tried this level

				tLevelLoadInfo levelLoadInfo = fQueryLevelLoadInfoFromTable( mapType, randomIndex );
				if( levelLoadInfo.mDlcNumber == 0 )
					return randomIndex; // dlc number 0 means "main game", not an add-on, so it's always legal
				if( fTestBits( legalAddOnFlags, 1 << ( levelLoadInfo.mDlcNumber ) ) )
					return randomIndex; // this map is part of an add-on which is included in the legal add-on flags

				// mark level as "tried"
				levelsTriedAlready.fPushBack( randomIndex );
				if( levelsTriedAlready.fCount( ) == numLevelsInTable )
					break; // tried all the levels in the table, couldn't find a valid one
			}
		}

		return ~0;
	}

	u32 tGameApp::fNumDebugLevels( ) const
	{
		return fNumLevelsInTable( GameFlags::cMAP_TYPE_DEVSINGLEPLAYER );
	}
	const char* tGameApp::fDebugLevelName( u32 ithLevel ) const
	{
		return fLevelNameInTable( GameFlags::cMAP_TYPE_DEVSINGLEPLAYER, ithLevel );
	}

	void tGameApp::fRewind( tUser* user, u32 waveIndex )
	{
		sigassert( user );
		tPlayer* player = fGetPlayerByUser( user );
		sigassert( player );

		if( fGameMode( ).fIsNet( ) )
		{
			fGameAppSession( )->fNetRewindBegin( );

			sigassert( fIsUserHostFromScript( user ) );
			if( !fGameAppSession( )->fIsHost( ) )
			{
				//log_output( 0, "waiting on host to REWIND" << std::endl );
				return; //we will want to wait for a msg from the host letting us know it is time!
			}
		}

		// These ifs are just kind of file system fail safes.
		if( (s32)waveIndex > mCurrentLevelLoadInfo.mHighestWaveReached &&	//current level has written to the rewind.
			player->fProfile( )->fLastPlayedRewindWave( ) != waveIndex )	//profile says we've written to the profile
		{
			log_warning( 0, " Have not reached wave Restarting level as fail safe." << waveIndex );
			fReloadCurrentLevel( );
		}
		else
		{
			log_line( Log::cFlagRewind, " Requested rewind to " << waveIndex );		
			if( waveIndex > 0 || fCurrentLevelDemand( )->fSaveFirstWave( ) )
			{
				if( fGameMode( ).fIsNet( ) )
				{
					sigassert( fCurrentLevel( )->fMapType( ) == GameFlags::cMAP_TYPE_CAMPAIGN && "SANITY CHECK" );
					fGameAppSession( )->fRewindMap( user, waveIndex );
				}
				else
				{
					std::string path =  tSaveGame::fGenerateFileName( waveIndex, fGameMode( ).fIsMultiPlayer( ) );
					log_line( Log::cFlagRewind, " LoadingSaveGame: " << path );
					tSaveGameStorageDesc saveDesc( player->fSaveDeviceId( ), tStringPtr( path.c_str( ) ), tLocalizedString( L"QuickieSave" ) );
					fSetLevelLoadState( NEW tGameLoadAppState( cDefaultLoadScriptPath, saveDesc, true, false ) );
				}
			}
			else
			{
				if( fGameMode( ).fIsNet( ) )
				{
					fGameAppSession( )->fRestartMap( );
				}
				else
				{
					fReloadCurrentLevel( );
				}
			}
		}
	}

	//------------------------------------------------------------------------------
	namespace
	{
		static const tStringPtr cRewindSaveName( "rewind_save" );


		class tRewindGameStorageSaveInstance : public Gui::tSaveUI::tGameStorageSaveInstance
		{	
		public:
			tRewindGameStorageSaveInstance( const tUserPtr& user, const tSaveGameStorageDesc& storageDesc, tGrowableArray<byte>& data )
				: Gui::tSaveUI::tGameStorageSaveInstance( user, storageDesc, data )
			{ }

			virtual void fPostErrors( )
			{
				if( mErrored )
					tGameApp::fInstance( ).fShowErrorDialog( tGameApp::cCouldntSaveFile, mUser.fGetRawPtr( ) );
			}
		};
	}

	void tGameApp::fSaveRewind( u32 waveIndex, b32 firstWave ) 
	{	
		log_line( Log::cFlagRewind, "Save rewind: Wave: " << waveIndex << " highest: " << mCurrentLevelLoadInfo.mHighestWaveReached );

		// This stuff is really complicated and i'm not sure it's all necessary.
		//  But basically if have any level progress (highest wave reached > -1), we dont want to overwrite any existing progress (waveindex < highest reached)
		// It all seems to work but basically just rewind and ensure that no data is lost.
		if( mCurrentLevelLoadInfo.mHighestWaveReached != -1 && (mCurrentLevelLoadInfo.mHighestWaveReached >= (s32)waveIndex || firstWave) )
			return; //dont oversave some stuff

		tPlayer * player = fFrontEndPlayer( );
		sigassert( player );

		if( !fCurrentLevelDemand( )->fVictoryOrDefeat( ) 
			&& !fCurrentLevelDemand( )->fDisableRewind( )
			&& ( fCurrentLevelDemand( )->fMapType( ) == GameFlags::cMAP_TYPE_CAMPAIGN || fCurrentLevelDemand( )->fMapType( ) == GameFlags::cMAP_TYPE_DEVSINGLEPLAYER )
			)
		{
			//otherwise would be just a preview
			b32 realSave = !firstWave || fCurrentLevelDemand( )->fSaveFirstWave( ) || fCurrentLevelDemand( )->fRewindCount( );

			if( realSave )
			{
				tPlayer* host = mPlayers[ 0 ].fGetRawPtr( );
				sigassert( host );

				if( host->fDoesntWantRewind( ) )
				{
					// on first real save warn them and disable rewind
					fCurrentLevelDemand( )->fSetDisableRewind( true );

					if( fGameMode( ).fIsNet( ) )
					{
						//inform both people
						for( u32 i = 0; i < fPlayerCount( ); ++i )
							fShowErrorDialog( tGameApp::cCouldntSaveFile, fPlayers( )[ i ]->fUser( ).fGetRawPtr( ) );
					}
					else // else just one guy, local mp or single player
						fShowErrorDialog( tGameApp::cCouldntSaveFile, host->fUser( ).fGetRawPtr( ) );
				}
			}

			if( realSave )
			{
				mCurrentLevelLoadInfo.mHighestWaveReached = fMax( (s32)waveIndex, 0 );

				if( fGameMode( ).fIsNet( ) && !fGameAppSession( )->fIsHost( ) )
					realSave = false;
			}

			// Get the level save data
			tSaveGameData saveData;
			tSaveGameRewindPreviewPtr savePreview( NEW tSaveGameRewindPreview( ) );

			if( !fBuildSaveData( saveData, *savePreview ) )
			{
				log_warning( 0, "Failed to save rewind" );
				return;
			}

			saveData.mId = waveIndex;

			const b32 multiPlayer = fGameMode( ).fIsMultiPlayer( );

			std::string path = tSaveGame::fGenerateFileName( waveIndex, multiPlayer );
			if( realSave )
			{
				// Serialize the save data
				tGameArchiveSave archive;
				archive.fSaveLoad( saveData );
				//archive.fEncrypt( ); // no encryption until future tu :(

				tLocalizedString displayName;
#ifdef platform_xbox360

				DWORD length = XCONTENT_MAX_DISPLAYNAME_LENGTH;
				wchar_t str[ XCONTENT_MAX_DISPLAYNAME_LENGTH ];
				XResourceGetString( X_STRINGID_TITLE, str, &length, NULL );
				//displayName = tLocalizedString( str ).fJoinWithLocString( fLocString( cRewindSaveName ) ).fJoinWithCString( StringUtil::fToString( waveIndex ).c_str( ) );
				displayName = fLocString( cRewindSaveName );
				displayName.fReplace( "title", tLocalizedString( str ) );
				displayName.fReplaceValue( "waveIndex", (s32)waveIndex );

				if( multiPlayer )
					displayName.fJoinWithCString( " MP" );
#endif
				if( !displayName.length( ) )
					displayName = tLocalizedString( L"QuickieSave" );

				log_line( Log::cFlagRewind, " SavingSaveGame: " << path );
				tSaveGameStorageDesc saveDesc( player->fSaveDeviceId( ), tStringPtr( path.c_str( ) ), displayName );
				fAddSaveInstance( Gui::tSaveUI::tSaveInstancePtr( NEW tRewindGameStorageSaveInstance(
					fFrontEndPlayer( )->fUser( ), 
					saveDesc, 
					archive.fBuffer( ) ) ) );

				if( !multiPlayer )
				{
					// Only single player games are resumable from the front end.
					player->fProfile( )->fSetLastPlayedWave( mCurrentLevelLoadInfo.mLevelIndex, fMax( mCurrentLevelLoadInfo.mHighestWaveReached, 0 ) );
				}
			}
			else
				log_line( Log::cFlagRewind, " JustPreviewingSaveGame: " << path );

			if( mRewindSaves.fCount( ) <= waveIndex )
				mRewindSaves.fSetCount( waveIndex + 1 );

			mRewindSaves[ waveIndex ] = savePreview;
		}
	}

	//------------------------------------------------------------------------------
	b32 tGameApp::fBuildSaveData( tSaveGameData & saveData, tSaveGameRewindPreview & savePreview )
	{
		for( u32 p = 0; p < mPlayers.fCount( ); ++p )
		{
			const tPlayerPtr & player = mPlayers[ p ];

			tPlayerSaveData & psd = saveData.mPlayerData[ p ];

			// Player Data // complimentary to tSaveGame::fOnLevelSpawnEnd
			psd.mMoney = player->fInGameMoney( );
			psd.mTickets = player->fTicketsLeft( );
			psd.mCamera = player->fUser( )->fViewport( )->fLogicCamera( ).fGetTripod( );
			psd.mSkippableSeconds = player->fSkipableSeconds( );
			psd.mWaveKills = player->fWaveKillList( );
			psd.mSpeedKills = player->fSpeedKillList( );
			psd.mBombingRuns = player->fBombingRunList( );
			psd.mStats = player->fStats( ).fStats( );
			psd.mBarrage = player->fBarrageController( )->fCurrentBarrageIndex( );
		}

		savePreview.mMoney = saveData.mPlayerData[ 0 ].mMoney;
		savePreview.mMoneyPlayer2 = mPlayers.fCount( ) > 1 ? saveData.mPlayerData[ 1 ].mMoney : ~0;
		savePreview.mTickets = saveData.mPlayerData[ 0 ].mTickets;

		// Level Data
		saveData.mLevelData.mLoadInfo = mCurrentLevelLoadInfo;

		// Entities
		b32 success = mCurrentLevel->fGatherEntitySaveData( saveData, savePreview );
		return success;
	}

	void tGameApp::fAddSaveInstance( const Gui::tSaveUI::tSaveInstancePtr& save )
	{
		// Create a save UI if one isn't present
		if( !mSaveUI )
		{
			mSaveUI = Gui::tSaveUIPtr(
				NEW Gui::tSaveUI(
				mResourceDepot->fQuery( 
				tResourceId::fMake<tScriptFile>( cDefaultSaveScriptPath ) ) ) );
		}

		mSaveUI->fAddSave( save );
	}

	//------------------------------------------------------------------------------
	tLevelLoadInfo tGameApp::fGetSaveGameLevelLoadInfo( tGameArchive & archive )
	{
		sigassert( archive.fMode() == tGameArchive::cModeLoad );

		// unarchive decrypted data into save game
		mCurrentSaveGame = tRefCounterPtr<tSaveGame>( NEW tSaveGame( ) );
		archive.fSaveLoad( *mCurrentSaveGame );

		log_line( Log::cFlagRewind, " SaveGameLoaded, wave: " << mCurrentSaveGame->mId);

		// Setup level load info
		mNextLevelLoadInfo = mCurrentSaveGame->mLevelData.mLoadInfo;
		mNextLevelLoadInfo.fFillEmptyWithDefaults( );
		mNextLevelLoadInfo.mHighestWaveReached = mCurrentSaveGame->mId;
		mNextLevelLoadInfo.mSkipBriefing = true; // Don't ever reply the briefing on a rewind
		mNextLevelLoadInfo.fLog( );

		mGameAppSession.fOnLoadLevel( true );

		return mNextLevelLoadInfo;
	}

	//------------------------------------------------------------------------------
	void tGameApp::fSetWaveListTable( const tResourcePtr& waveListResource )
	{
		// unload previous if it's different from the current
		if( mDataTables[ cDataTableWaveList ] && mDataTables[ cDataTableWaveList ] != waveListResource )
			mDataTables[ cDataTableWaveList ]->fUnload( this );

		// store current
		mDataTables[ cDataTableWaveList ] = waveListResource;

		// load current
		if( mDataTables[ cDataTableWaveList ] )
			mDataTables[ cDataTableWaveList ]->fLoadDefault( this );
	}
	void tGameApp::fSetupTracers( )
	{
		if( Gameplay_Tracers_ApplyTable )
			log_warning( 0, "Gameplay_Tracers_ApplyTable enabled! (performance warning)" );

		const tDataTable& dt = fTracersTable( ).fTable( );

		for( u32 i = 0; i < dt.fRowCount( ); ++i )
		{
			FX::tTracerTrailDef def;
			def.mTexture = fResourceDepot( )->fQuery( tResourceId::fMake< Gfx::tTextureFile >( tFilePathPtr( dt.fIndexByRowCol<tStringPtr>( i, cTracerDataColumnTexture ) ) ) );
			def.mTint = dt.fIndexByRowCol<Math::tVec4f>( i, cTracerDataColumnTint );
			def.mWidth = dt.fIndexByRowCol<f32>( i, cTracerDataColumnWidth );
			def.mLifeSpan = dt.fIndexByRowCol<f32>( i, cTracerDataColumnLifeSpan );
			def.mSpinRate = dt.fIndexByRowCol<f32>( i, cTracerDataColumnSpinRate );
			def.mLeadDistance = dt.fIndexByRowCol<f32>( i, cTracerDataColumnLeadDistance );
			def.mRealTime = dt.fIndexByRowCol<b32>( i, cTracerDataColumnRealTime );
			def.mBeefyQuads = dt.fIndexByRowCol<b32>( i, cTracerDataColumnBeefy );

			if( dt.fIndexByRowCol<b32>( i, cTracerDataColumnAdditive ) )
				def.mRenderState.fSetDstBlendMode( Gfx::tRenderState::cBlendOne );

			mTracerTrailDefs.fPushBack( def  );
		}
	}
	void tGameApp::fStartupSfx( )
	{
		mSoundSystem.fReset( NEW Audio::tSystem( ) );
		Audio::tSystem::fSetDefaultSystem( mSoundSystem );

		mStartupParams.mAudioOptions.mBaseResourcePath = fResourceDepot( )->fRootPath( );
		mStartupParams.mAudioOptions.mSoundBanks.fPushBack( tFilePathPtr( "game:\\audio\\Init.bnk" ) );
		mStartupParams.mAudioOptions.mSoundBanks.fPushBack( tFilePathPtr( "game:\\audio\\MainBank.bnk" ) );

		// if we have any dlc packages, load the dlc content.
		if( mDLCEnumerator.fCountByType( tDLCPackage::cDLCPackage ) || mDLCEnumerator.fCountByType( tDLCPackage::cDLCCompatibilityPackage ) || Debug_ForceLoadDLC )
		{
#ifdef platform_xbox360

			if( Debug_ForceLoadDLC )
			{
				mStartupParams.mAudioOptions.mFilePackages.fPushBack( tFilePathPtr( "game:\\audio\\DLC01.pck" ) );
				mStartupParams.mAudioOptions.mFilePackages.fPushBack( tFilePathPtr( "game:\\audio\\DLC02.pck" ) );
			}
			else
			{
				// load everything we have and let wwise figure it out
				for( u32 p = 0; p < mDLCEnumerator.fPackageCount( ); ++p )
				{
					tDLCPackage* pck = mDLCEnumerator.fPackage( p ).fGetRawPtr( );
					if( pck->mType == tDLCPackage::cDLCPackage || pck->mType == tDLCPackage::cDLCCompatibilityPackage )
					{
						//// log all dlc files
						//tFilePathPtrList filesAll;
						//FileSystem::fGetFileNamesInFolder( filesAll, pck->fDriveFolder( ), true, true );
						//for( u32 i = 0; i < filesAll.fCount( ); ++i )
						//	log_line( 0, "Found DLC File: " << filesAll[ i ] );

						tFilePathPtrList files;
						FileSystem::fGetFileNamesInFolder( files, pck->fDriveFolder( ), true, true, tFilePathPtr( ".pck" ) );
						mStartupParams.mAudioOptions.mFilePackages.fJoin( files );
						log_line( 0, "Found: " << files.fCount( ) << " audio .pcks" );

						break; //all packages contain the same data
					}
				}
			}

			mStartupParams.mAudioOptions.mSoundBanks.fPushBack( tFilePathPtr( "DLC01.bnk" ) );
			mStartupParams.mAudioOptions.mSoundBanks.fPushBack( tFilePathPtr( "DLC02.bnk" ) );
#endif
		}

		mSoundSystem->fInitialize( mStartupParams.mAudioOptions );

		// Don't run logic on the Audio thread
		mSceneGraph->fDisallowLogicThread( mStartupParams.mAudioOptions.mThreadIndex );
		
		if( !Audio::tSystem::fGlobalDisable( ) && !Debug_SoundSystem_DisableMusic )
			mSoundSystem->fMasterSource( )->fHandleEvent( fExtraDemoMode( ) ? AK::EVENTS::PLAY_EXTRA : AK::EVENTS::PLAY_MUSIC );
	}
	//------------------------------------------------------------------------------
	void tGameApp::fTickSaveUI( float dt ) 
	{
		profile_pix("fTickSaveUI");
		sigassert(mSaveUI);

		if( mSaveUI->fState() == Gui::tSaveUI::cStateLoading )
		{
			if( mSaveUI->fScriptLoadCompleted( ) )
			{
				// Try to start saving; release on failure
				if (!mSaveUI->fBeginSaving( ))
					mSaveUI.fRelease();
			}
		}

		else if( mSaveUI->fIsFinishedSaving( ) )
			mSaveUI.fRelease( );
		else
			mSaveUI->fOnTick( dt );
	}

	//------------------------------------------------------------------------------
	void tGameApp::fForEachPlayerFromScript( Sqrat::Function& func, Sqrat::Object& obj )
	{
		const u32 count = mPlayers.fCount( );
		for( u32 p = 0; p < count; ++p )
			func.Execute( mPlayers[ p ].fGetRawPtr( ), obj );
	}

	//------------------------------------------------------------------------------
	b32 tGameApp::fJoinInviteSession( )
	{
		if( mGameInvite.fNull( ) )
		{
			log_warning( 0, "No invite to join" );
			return false;
		}
		else
		{
			mGameAppSession.fJoinSessionFromInvite( *mGameInvite );
			mGameInvite.fRelease( );
			log_line( 0, "Cleared invite data after starting join process" );
		}

		return true;
	}

	void tGameApp::fClearInvite( ) 
	{
		mGameInvite.fRelease( ); 
	}

	//------------------------------------------------------------------------------
	void tGameApp::fFireInviteEvent( )
	{
		if( !fIsFullVersion( ) )
		{
			mGameAppSession.fOnGameInviteNeedFullVer( );
			return;
		}

		if( mGameInvite.fNull( ) )
		{
			log_warning( 0, "No invite existed to fire event with" );
			return;
		}

		// Cache the index as the event from the game app session may clear the game invite
		u32 localHwIdx = mGameInvite->mLocalHwIndex;

		// Since warning messages generally kill themselves on the INVITE_ACCEPTED message
		// and game levels must use a warning about progress being lost we send this message
		// first to the warning canvas and then to the game app session for sending to the
		// level ui script
		mWarningCanvas.fHandleCanvasEvent( 
			Logic::tEvent( 
				ApplicationEvent::cSessionInviteAccepted, 
				NEW Logic::tIntEventContext( localHwIdx ) ) );

		// Fires the event with event caching during level load
		mGameAppSession.fOnGameInviteAccepted( );	
	}

	//------------------------------------------------------------------------------
	void tGameApp::fAwardAchievementToAllPlayers( u32 achievement )
	{
		const u32 count = mPlayers.fCount( );
		for( u32 p = 0; p < count; ++p )
			if( mPlayers[ p ] && (mPlayers[ p ]->fUser( )->fIsLocal( ) || fGameMode( ).fIsCoOp( )) )
				mPlayers[ p ]->fAwardAchievement( achievement );
	}

	void tGameApp::fAwardDeferredAchievementToAllPlayers( u32 achievement )
	{
		const u32 count = mPlayers.fCount( );
		for( u32 p = 0; p < count; ++p )
			if( mPlayers[ p ] && (mPlayers[ p ]->fUser( )->fIsLocal( ) || fGameMode( ).fIsCoOp( )) )
				mPlayers[ p ]->fAwardAchievementDeferred( achievement );
	}

	void tGameApp::fAwardAvatarAwardToAllPlayers( u32 award )
	{
		const u32 count = mPlayers.fCount( );
		for( u32 p = 0; p < count; ++p )
			if( mPlayers[ p ] && (mPlayers[ p ]->fUser( )->fIsLocal( ) || fGameMode( ).fIsCoOp( )) )
				mPlayers[ p ]->fAwardAvatarAward( award );
	}


	void tGameApp::fSetSingleScreenControlIndex( u32 index )
	{
		mSingleScreenControl = fMin( index, mPlayers.fCount( ) - 1 );
	}

	tPlayer* tGameApp::fSingleScreenControlPlayer( ) const
	{
		if( mSingleScreenControl < mPlayers.fCount( ) )
			return mPlayers[ mSingleScreenControl ].fGetRawPtr( );
		else
			return mPlayers[ 0 ].fGetRawPtr( );
	}

	void tGameApp::fSetSingleScreenCoop( b32 enable )
	{
		if( mSingleScreenCoop == enable )
			return;

		mSingleScreenCoop = enable;
		mSingleScreenControl = 0;
		fResetSingleScreenViewports();
	}

	void tGameApp::fResetSingleScreenViewports( )
	{
		fAssignViewportsToPlayers( fGameMode( ) );
	}

	namespace
	{
		enum tDebrisColumns
		{
			cDebrisTableGravity,
			cDebrisTableUpVelMax,
			cDebrisTableUpVelMin,
			cDebrisTableHorzVelMax,
			cDebrisTableTorqueArm,
			cDebrisTableBounceRestitution,
			cDebrisTableBounceFriction,
			cDebrisTableBounceHeight,
			cDebrisTableLifeMax,
			cDebrisTableInheritVelocityFactor,
			cDebrisTableVelocityMax,
			cDebrisTableCollisionDelay,
			cDebrisTableEffectMod
		};
	}
	void tGameApp::fSetupDebris( )
	{
		if( Gameplay_Debris_ApplyTable )
			log_warning( 0, "Gameplay_Debris_ApplyTable enabled! (performance warning)" );

		for( u32 t = 0; t < GameFlags::cDEBRIS_TYPE_COUNT; ++t )
		{
			const tDataTable& params = tGameApp::fInstance( ).fDataTable( tGameApp::cDataTableBreakables ).fIndexTable( 0 );

			mDebrisLogicDefs[ t ].mGravity =				params.fIndexByRowCol<f32>( t, cDebrisTableGravity );
			mDebrisLogicDefs[ t ].mUpVelMax =				params.fIndexByRowCol<f32>( t, cDebrisTableUpVelMax );
			mDebrisLogicDefs[ t ].mUpVelMin =				params.fIndexByRowCol<f32>( t, cDebrisTableUpVelMin );
			mDebrisLogicDefs[ t ].mHorzVelMax =				params.fIndexByRowCol<f32>( t, cDebrisTableHorzVelMax );
			mDebrisLogicDefs[ t ].mTorqueArm =				params.fIndexByRowCol<f32>( t, cDebrisTableTorqueArm );
			mDebrisLogicDefs[ t ].mBounceRestitution =		params.fIndexByRowCol<f32>( t, cDebrisTableBounceRestitution );
			mDebrisLogicDefs[ t ].mBounceFriction =			params.fIndexByRowCol<f32>( t, cDebrisTableBounceFriction );
			mDebrisLogicDefs[ t ].mBounceHeight =			params.fIndexByRowCol<f32>( t, cDebrisTableBounceHeight );
			mDebrisLogicDefs[ t ].mLifeMax =				params.fIndexByRowCol<f32>( t, cDebrisTableLifeMax );
			mDebrisLogicDefs[ t ].mInheritedVelocityFactor = params.fIndexByRowCol<f32>( t, cDebrisTableInheritVelocityFactor );
			mDebrisLogicDefs[ t ].mVelocityMax =			params.fIndexByRowCol<f32>( t, cDebrisTableVelocityMax );
			mDebrisLogicDefs[ t ].mCollisionDelay =			params.fIndexByRowCol<f32>( t, cDebrisTableCollisionDelay );	
			mDebrisLogicDefs[ t ].mEffectMod =				params.fIndexByRowCol<f32>( t, cDebrisTableEffectMod );			
		}
	}
	tDebrisLogicDef& tGameApp::fDebrisLogicDef( GameFlags::tDEBRIS_TYPE type )
	{ 
		//re apply table only for debugging/tweaking
		if( Gameplay_Debris_ApplyTable )
			fSetupDebris( );

		return mDebrisLogicDefs[ type ]; 
	}
	const FX::tTracerTrailDef& tGameApp::fTracerTrailDef( u32 type )
	{ 
		//re apply table only for debugging/tweaking
		if( Gameplay_Tracers_ApplyTable )
			fSetupTracers( );

		return mTracerTrailDefs[ type ]; 
	}
	b32 tGameApp::fLevelLockedByDefault( u32 mapType, u32 level ) const
	{
		const tDataTable& dt = fDataTable( cDataTableLevelResources ).fIndexTable( mapType );
		return (b32)dt.fIndexByRowCol<f32>( level, cLevelResourceLocked );
	}
	tFilePathPtr tGameApp::fUnitResourcePath( u32 unitID, u32 country ) const
	{
		const tStringPtr& unitStr = GameFlags::fUNIT_IDEnumToValueString( unitID );
		const tStringHashDataTable& dt = fUnitsSharedTable( country );
		u32 index = dt.fRowIndex( unitStr );
		if( index == ~0 ) 
		{
			log_warning( 0, "Could not find row in units shared table: " << unitStr );
			return tFilePathPtr( );
		}
		return dt.fIndexByRowCol<tFilePathPtr>(index, tUnitLogic::cUnitSharedResourcePath );
	}
	u32 tGameApp::fUnitType( u32 unitID, u32 country )
	{
		const tStringPtr& unitStr = GameFlags::fUNIT_IDEnumToValueString( unitID );
		const tStringHashDataTable& dt = fUnitsSharedTable( country );
		u32 index = dt.fRowIndex( unitStr );
		if( index == ~0 ) 
		{
			log_warning( 0, "Could not find row in units shared table: " << unitStr );
			return ~0;
		}
		tStringPtr type = dt.fIndexByRowCol<tStringPtr>(index, tUnitLogic::cUnitSharedUnitType );
		return GameFlags::fUNIT_TYPEValueStringToEnum( type );
	}
	tFilePathPtr tGameApp::fUnitWaveIconPath( u32 unitID, u32 country ) const
	{
		const tStringPtr& unitStr = GameFlags::fUNIT_IDEnumToValueString( unitID );
		const tStringHashDataTable& dt = fUnitsSharedTable( country );
		u32 index = dt.fRowIndex( unitStr );
		if( index == ~0 ) 
		{
			log_warning( 0, "Could not find row in units shared table for wave icon path. UnitString: " << unitStr << " UnitID: " << unitID );
			return tFilePathPtr( );
		}
		return dt.fIndexByRowCol<tFilePathPtr>(index, tUnitLogic::cUnitSharedWaveIconPath );
	}
	u32 tGameApp::fUnitIDAlias( u32 unitID, u32 country ) const
	{
		const tStringPtr& unitStr = GameFlags::fUNIT_IDEnumToValueString( unitID );
		const tStringHashDataTable& dt = fUnitsSharedTable( country );
		u32 index = dt.fRowIndex( unitStr );
		if( index == ~0 ) 
		{
			log_warning( 0, "Could not find row in units shared table: " << unitStr );
			return ~0;
		}
		tStringPtr str = dt.fIndexByRowCol<tStringPtr>(index, tUnitLogic::cUnitSharedUnitIDAlias );
		u32 id = GameFlags::fUNIT_IDValueStringToEnum( str );
		if( id >= GameFlags::cUNIT_ID_COUNT ) id = ~0;
		return id;
	}
	u32	tGameApp::fOverChargeCost( u32 unitID, u32 country ) const
	{
		const tStringPtr& unitStr = GameFlags::fUNIT_IDEnumToValueString( unitID );
		const tStringHashDataTable& dt = fUnitsSharedTable( country );
		u32 index = dt.fRowIndex( unitStr );
		if( index == ~0 ) 
		{
			log_warning( 0, "Could not find row in units shared table: " << unitStr );
			return 0;
		}
		return dt.fIndexByRowCol<u32>(index, tUnitLogic::cUnitSharedOverChargeComboCost );
	}
	tStringPtr tGameApp::fUnitWaveAudioSwitch( u32 unitID, u32 country ) const
	{
		const tStringPtr& unitStr = GameFlags::fUNIT_IDEnumToValueString( unitID );
		const tStringHashDataTable& dt = fUnitsSharedTable( country );
		u32 index = dt.fRowIndex( unitStr );
		if( index == ~0 ) 
		{
			log_warning( 0, "Could not find row in units shared table: " << unitStr );
			return tStringPtr( );
		}
		return dt.fIndexByRowCol<tStringPtr>(index, tUnitLogic::cUnitSharedWaveLaunchAudioSwitch );
	}

	/// Get all players on a certain team
	/// \return An array of players
	void tGameApp::fFindAllTeamPlayers( u32 team, tGrowableArray< tPlayerPtr > & out )
	{
		for( u32 i = 0; i < mPlayers.fCount( ); ++i )
		{
			if( mPlayers[ i ]->fIsActive( ) && mPlayers[ i ]->fTeam( ) == team )
				out.fPushBack( mPlayers[ i ] );
		}
	}

	void tGameApp::fFindAllEnemyPlayers( u32 team, tGrowableArray< tPlayerPtr > & out )
	{
		fFindAllTeamPlayers( tPlayer::fDefaultEnemyTeam( team ), out ); 
	}

	tPlayer* tGameApp::fOtherPlayer( const tPlayer* me )
	{
		if( mPlayers.fCount( ) == 1 )
		{
			sigassert( me == mPlayers[ 0 ].fGetRawPtr( ) );
			return NULL;
		}
		else
		{
			sigassert( mPlayers.fCount( ) == 2 );
			if( mPlayers[ 0 ].fGetRawPtr( ) == me )
				return mPlayers[ 1 ].fGetRawPtr( );
			else
			{
				sigassert( mPlayers[ 1 ].fGetRawPtr( ) == me );
				return mPlayers[ 0 ].fGetRawPtr( );
			}
		}
	}

	tPlayer* tGameApp::fGetPlayerByHwIndex( u32 hwIndex )
	{
		for( u32 i = 0; i < mPlayers.fCount( ); ++i )
		{
			if( mPlayers[ i ]->fUser( )->fLocalHwIndex( ) == hwIndex )
				return mPlayers[ i ].fGetRawPtr( );
		}
		return NULL;
	}

	tPlayer* tGameApp::fGetPlayerByTeam( u32 team )
	{
		for( u32 i = 0; i < mPlayers.fCount( ); ++i )
			if( mPlayers[ i ]->fTeam( ) == team )
				return mPlayers[ i ].fGetRawPtr( );
		return NULL;
	}

	tPlayer* tGameApp::fGetPlayerByCountry( u32 country )
	{
		for( u32 i = 0; i < mPlayers.fCount( ); ++i )
			if( mPlayers[ i ]->fCountry( ) == country )
				return mPlayers[ i ].fGetRawPtr( );
		return NULL;
	}	

	tPlayer * tGameApp::fGetPlayerByUser( tUser* user )
	{
		if( !user )
		{
			tScriptVm::fDumpCallstack( );
			sigassert( user );
		}
		u32 playerIndex = fWhichPlayer( user );
		sigassert( playerIndex != ~0 && playerIndex < mPlayers.fCount( ) && "Invalid player index from fWhichUser" );
		return fGetPlayer( playerIndex );
	}

	tPlayer* tGameApp::fGetEnemyPlayerByCountry( u32 country )
	{
		for( u32 i = 0; i < mPlayers.fCount( ); ++i )
			if( mPlayers[ i ]->fEnemyCountry( ) == country )
				return mPlayers[ i ].fGetRawPtr( );
		return NULL;
	}

	bool tGameApp::fSetPlayerLocalUserFromScript( u32 player, u32 user )
	{
		if( fGameMode( ).fIsNet( ) )
		{
			log_warning( 0, "Attempt to set player local user while in networked game" );
			return false;
		}

		if( player == mPlayers.fCount( ) )
		{
			// If the user is active and there is only one player right now
			if( user < mLocalUsers.fCount( ) && mPlayers.fCount( ) == 1 )
			{
				mPlayers.fPushBack( tPlayerPtr( NEW tPlayer( mLocalUsers[ user ] ) ) );
				return !mPlayers.fBack( )->fUserKicked( );
			}
			return false;
		}
		if( player > mPlayers.fCount( ) || user >= mLocalUsers.fCount( ) )
			return false;

		tUserPtr oldUser = mPlayers[ player ]->fUser( );
		tUserPtr newUser = mLocalUsers[ user ];

		// Swap Viewports
		if( oldUser != newUser )
		{
			Gfx::tViewportPtr tempPort = oldUser->fViewport( );
			Gfx::tScreenPtr tempScrn = oldUser->fScreen( );
			oldUser->fSetViewport( newUser->fScreen( ), newUser->fViewport( ) );
			newUser->fSetViewport( tempScrn, tempPort );
		}

		mPlayers[ player ]->fSetUser( newUser );
		return !mPlayers[ player ]->fUserKicked( );
	}

	Sig::u32 tGameApp::fWhichUser( tUser* user )
	{
		for( u32 i = 0; i < mLocalUsers.fCount( ); ++i )
		{
			if( mLocalUsers[ i ].fGetRawPtr( ) == user )
				return i;
		}

		return ~0;
	}

	Sig::u32 tGameApp::fWhichPlayer( tUser* user )
	{
		for( u32 i = 0; i < mPlayers.fCount( ); ++i )
		{
			if( mPlayers[ i ]->fUser( ) == user )
				return i;
		}

		return ~0;
	}

	Math::tVec3f tGameApp::fDefaultTurretDirection( u32 team ) const
	{
		return mTeamOrientation[ team ].fZAxis( );
	}
	void tGameApp::fSetTeamOrientation( tEntity* orient )
	{
		sigassert( orient );
		u32 team = orient->fQueryEnumValue( GameFlags::cENUM_TEAM, GameFlags::cTEAM_COUNT );
		if( team == GameFlags::cTEAM_COUNT )
		{
			log_warning( 0, "Team orientation entity does not contain a team enum." );
			return;
		}

		mTeamOrientationSet[ team ] = true;
		mTeamOrientation[ team ] = orient->fObjectToWorld( );
	}
	tPlayer* tGameApp::fAnyPlayerWithButtonsDown( u32 buttonMask, u32 notPressedButtons )
	{
		for( u32 i = 0; i < mPlayers.fCount( ); ++i )
		{
			if(  mPlayers[ i ]->fIsActive( ) && 
				 mPlayers[ i ]->fGamepad( ).fButtonDown( buttonMask ) &&
				!mPlayers[ i ]->fGamepad( ).fButtonHeld( notPressedButtons ) )
				return mPlayers[ i ].fGetRawPtr( );
		}
		return 0;
	}
	tPlayer* tGameApp::fAnyLocalPlayerWithButtonsDown( u32 buttonMask, u32 notPressedButtons )
	{
		for( u32 i = 0; i < mPlayers.fCount( ); ++i )
		{
			if(  mPlayers[ i ]->fIsActive( ) && 
				mPlayers[ i ]->fGamepad( ).fButtonDown( buttonMask ) &&
				!mPlayers[ i ]->fGamepad( ).fButtonHeld( notPressedButtons ) &&
				mPlayers[ i ]->fUser( )->fIsLocal( ) )
				return mPlayers[ i ].fGetRawPtr( );
		}
		return 0;
	}

	tPlayer* tGameApp::fAnyPlayerOpeningPauseMenu( )
	{
		return fAnyPlayerWithButtonsDown( Input::tGamepad::cButtonStart, Input::tGamepad::cButtonSelect );
	}

	static const u32 cButtonsNotPressedWhenPressingRewindButton = Input::tGamepad::cButtonA | Input::tGamepad::cButtonB | Input::tGamepad::cButtonX | Input::tGamepad::cButtonY | Input::tGamepad::cButtonStart | Input::tGamepad::cButtonDPadDown;
	tPlayer* tGameApp::fAnyPlayerOpeningRewindMenu( )
	{
		if( !Debug_UI_DisableRewindMenu )
			return fAnyPlayerWithButtonsDown( Input::tGamepad::cButtonSelect, cButtonsNotPressedWhenPressingRewindButton );
		else
			return NULL;
	}

	tPlayer* tGameApp::fHostPlayerOpeningRewindMenu( ) const
	{
		if( !fGameMode( ).fIsNet( ) )
			return NULL;

		for( u32 i = 0; i < mPlayers.fCount( ); ++i )
		{
			if( !fIsUserHostFromScript( mPlayers[ i ]->fUserFromScript( ) ) )
				continue;

			if(  mPlayers[ i ]->fIsActive( ) && 
				 mPlayers[ i ]->fGamepad( ).fButtonDown( Input::tGamepad::cButtonSelect ) &&
				!mPlayers[ i ]->fGamepad( ).fButtonHeld( cButtonsNotPressedWhenPressingRewindButton ) )
				return mPlayers[ i ].fGetRawPtr( );
		}
		
		return NULL;
	}

	namespace
	{
		void fSetCampaignRP( const tUserPtr& user, u32 index, u32 context )
		{
			if( index >= array_length(GameSession::RichPresence::cContextCampaignName2Values) )
			{
				log_warning( 0, "Level index higher than cContextCampaignName2Values size" );
				index = array_length(GameSession::RichPresence::cContextCampaignName2Values) - 1;
			}

			user->fSetContext( GameSession::RichPresence::cContextCampaignName2, GameSession::RichPresence::cContextCampaignName2Values[ index ] );
			user->fSetContext( tUser::cUserContextPresence, context );
		}
		void fSetSurvivalRP( const tUserPtr& user, u32 index, u32 context )
		{
			if( index >= array_length(GameSession::RichPresence::cContextSurvivalNameValues) )
			{
				log_warning( 0, "Level index higher than cContextSurvivalNameValues size" );
				index = array_length(GameSession::RichPresence::cContextSurvivalNameValues) - 1;
			}

			user->fSetContext( GameSession::RichPresence::cContextSurvivalName, GameSession::RichPresence::cContextSurvivalNameValues[ index ] );
			user->fSetContext( tUser::cUserContextPresence, context );
		}
		void fSetVersusRP( const tUserPtr& user, u32 index, u32 context )
		{
			// index 0 is "versus default"  The real strings start at 1
			u32 adjustedIndex = index + 1;

			if( adjustedIndex >= array_length(GameSession::RichPresence::cContextH2HNameValues) )
			{
				log_warning( 0, "Level index higher than cContextH2HNameValues size" );
				adjustedIndex = 0;
			}

			user->fSetContext( GameSession::RichPresence::cContextH2HName, GameSession::RichPresence::cContextH2HNameValues[ adjustedIndex ] );
			user->fSetContext( tUser::cUserContextPresence, context );
		}
		void fSetMinigameRP( const tUserPtr& user, u32 index, u32 context )
		{
			// index 0 is "minigame default"  The real strings start at 1
			u32 adjustedIndex = index + 1;

			if( adjustedIndex >= array_length(GameSession::RichPresence::cContextMinigameNameValues) )
			{
				log_warning( 0, "Level index higher than cContextMinigameNameValues size" );
				adjustedIndex = array_length(GameSession::RichPresence::cContextMinigameNameValues) - 1;
			}

			user->fSetContext( GameSession::RichPresence::cContextMinigameName, GameSession::RichPresence::cContextMinigameNameValues[ adjustedIndex ] );
			user->fSetContext( tUser::cUserContextPresence, context );
		}
	}

	void tGameApp::fApplyRichPresence( )
	{
		const tGameMode& gm = fGameMode( );
		const tLevelLoadInfo& info = fCurrentLevelLoadInfo( );
		b32 isTrial = fCurrentLevel( ) && fCurrentLevel( )->fIsTrial( );

		sigassert( cCAMPAIGN_LEVEL_COUNT == fNumLevelsInTable( GameFlags::cMAP_TYPE_CAMPAIGN ) );
		sigassert( cH2H_LEVEL_COUNT == fNumLevelsInTable( GameFlags::cMAP_TYPE_HEADTOHEAD ) );
		sigassert( cSURVIVAL_LEVEL_COUNT == fNumLevelsInTable( GameFlags::cMAP_TYPE_SURVIVAL ) );
		sigassert( cMINIGAME_LEVEL_COUNT == fNumLevelsInTable( GameFlags::cMAP_TYPE_MINIGAME ) );

		if( gm.fIsFrontEnd( ) )
		{
			for( u32 i = 0; i < mLocalUsers.fCount( ); ++i )
			{
				u32 player = fWhichPlayer( mLocalUsers[ i ].fGetRawPtr( ) );
				if( player != ~0 )
				{
					if( mInMultiplayerLobby )
						mLocalUsers[ i ]->fSetContext( tUser::cUserContextPresence, GameSession::RichPresence::cRichPresenceLobby );
					else
						mLocalUsers[ i ]->fSetContext( tUser::cUserContextPresence, GameSession::RichPresence::cRichPresenceMenu );
				}
				else
					mLocalUsers[ i ]->fSetContext( tUser::cUserContextPresence, GameSession::RichPresence::cRichPresenceMenu );
			}
		}
		else if( info.mMapType == GameFlags::cMAP_TYPE_HEADTOHEAD )
		{
			for( u32 i = 0; i < mLocalUsers.fCount( ); ++i )
			{
				u32 player = fWhichPlayer( mLocalUsers[ i ].fGetRawPtr( ) );
				if( player != ~0 )
				{
					u32 rp = ( mPlayers[ player ]->fTeam( ) == GameFlags::cTEAM_BLUE ) ? GameSession::RichPresence::cRichPresenceHeadToHeadUSA : GameSession::RichPresence::cRichPresenceHeadToHeadUSSR;
					fSetVersusRP( mLocalUsers[ i ], info.mLevelIndex, rp );
				}
				else
					mLocalUsers[ i ]->fSetContext( tUser::cUserContextPresence, GameSession::RichPresence::cRichPresenceNotParticipating );
			}
		}
		else if( info.mMapType == GameFlags::cMAP_TYPE_CAMPAIGN )
		{
			for( u32 i = 0; i < mLocalUsers.fCount( ); ++i )
			{
				u32 player = fWhichPlayer( mLocalUsers[ i ].fGetRawPtr( ) );
				if( player != ~0 )
				{
					if( gm.fIsCoOp( ) )
						fSetCampaignRP( mLocalUsers[ i ], info.mLevelIndex, GameSession::RichPresence::cRichPresenceCampaignMP );
					else
						fSetCampaignRP( mLocalUsers[ i ], info.mLevelIndex, GameSession::RichPresence::cRichPresenceCampaignSP );
				}
				else
					mLocalUsers[ i ]->fSetContext( tUser::cUserContextPresence, GameSession::RichPresence::cRichPresenceNotParticipating );
			}
		}
		else if( info.mMapType == GameFlags::cMAP_TYPE_SURVIVAL )
		{
			for( u32 i = 0; i < mLocalUsers.fCount( ); ++i )
			{
				u32 player = fWhichPlayer( mLocalUsers[ i ].fGetRawPtr( ) );
				if( player != ~0 )
				{
					if( gm.fIsCoOp( ) )
						fSetSurvivalRP( mLocalUsers[ i ], info.mLevelIndex, GameSession::RichPresence::cRichPresenceSurvivalMP );
					else
						fSetSurvivalRP( mLocalUsers[ i ], info.mLevelIndex, GameSession::RichPresence::cRichPresenceSurvivalSP );
				}
				else
					mLocalUsers[ i ]->fSetContext( tUser::cUserContextPresence, GameSession::RichPresence::cRichPresenceNotParticipating );
			}
		}
		else if( info.mMapType == GameFlags::cMAP_TYPE_MINIGAME )
		{
			for( u32 i = 0; i < mLocalUsers.fCount( ); ++i )
			{
				u32 player = fWhichPlayer( mLocalUsers[ i ].fGetRawPtr( ) );
				if( player != ~0 )
				{
					if( gm.fIsCoOp( ) )
						fSetMinigameRP( mLocalUsers[ i ], info.mLevelIndex, GameSession::RichPresence::cRichPresenceMinigameMP );
					else
						fSetMinigameRP( mLocalUsers[ i ], info.mLevelIndex, GameSession::RichPresence::cRichPresenceMinigameSP );
				}
				else
					mLocalUsers[ i ]->fSetContext( tUser::cUserContextPresence, GameSession::RichPresence::cRichPresenceNotParticipating );
			}
		}
		else
		{
			for( u32 i = 0; i < mLocalUsers.fCount( ); ++i )
				mLocalUsers[ i ]->fSetContext( tUser::cUserContextPresence, GameSession::RichPresence::cRichPresenceMenu );
		}
	}

	void tGameApp::fSetInMultiplayerLobby( b32 inLobby ) 
	{ 
		mInMultiplayerLobby = inLobby;
		fApplyRichPresence( );
	}

	void tGameApp::fOnDlcInstalled( )
	{
		// we dont want to promote the game to dlc mode while the app is running.
		//  Prompt player with desctructive action dialog to rebot.

		if( fFrontEndPlayer( ) && fFrontEndPlayer( )->fUser( ) )
			fShowErrorDialog( cDLCInstalled, fFrontEndPlayer( )->fUser( ).fGetRawPtr( ) );
	}

	void tGameApp::fOnFullVersionInstalled( )
	{
		tGameApp::fInstance( ).fFullLicenseAcquired( );

		if( mCurrentLevel )
			mCurrentLevel->fGamePurchased( );
	}

	void tGameApp::fFullLicenseAcquired( )
	{
		// Save players profiles.
		for( u32 i = 0; i < fPlayers( ).fCount( ); ++i )
			if( fPlayers( )[ i ] && fPlayers( )[ i ]->fUser( )->fIsLocal( ) )
			{
				fPlayers( )[ i ]->fAwardSessionAwards( );
				fPlayers( )[ i ]->fSaveProfile( );
			}

		// TODO Flush leaderboards.
	}

	void tGameApp::fSetBrightness( f32 brightness )
	{
		sigassert( fScreen( )->fDevice( ) );
		fScreen( )->fDevice( )->fSetGammaRamp( brightness );
	}

	void tGameApp::fSetMusicVolume( f32 vol )
	{
		sigassert( mSoundSystem );
		mSoundSystem->fSetUserMusicVolume( vol );
	}

	void tGameApp::fSetSfxVolume( f32 vol )
	{
		sigassert( mSoundSystem );
		mSoundSystem->fSetUserSfxVolume( vol );
	}

	void tGameApp::fSetHUDVolume( f32 vol )
	{
		sigassert( mSoundSystem );
		mSoundSystem->fSetUserHUDVolume( vol );
	}

	void tGameApp::fSetListeningMode( u32 mode )
	{
		sigassert( mSoundSystem );
		sigassert( mode < Audio::tSystem::cListeningModeCount );
		mSoundSystem->fSetListenerMode( mode );
	}

	b32 tGameApp::fIsDisplayCase( ) const 
	{ 
		return fCurrentLevel( ) && fCurrentLevel( )->fIsDisplayCase( ); 
	}

	u32 tGameApp::fDefaultTeamFromCountry( u32 country ) const
	{
		switch( country )
		{
		case GameFlags::cCOUNTRY_DEFAULT:
			return GameFlags::cTEAM_NONE;
		case GameFlags::cCOUNTRY_USA:
			return GameFlags::cTEAM_BLUE;
		case GameFlags::cCOUNTRY_USSR:
			return GameFlags::cTEAM_RED;
		}

		return GameFlags::cTEAM_NONE;
	}
	u32 tGameApp::fDefaultCountryFromTeam( u32 team ) const
	{
		switch( team )
		{
		case GameFlags::cTEAM_NONE:
			return GameFlags::cCOUNTRY_DEFAULT;
		case GameFlags::cTEAM_BLUE:
			return GameFlags::cCOUNTRY_USA;
		case GameFlags::cTEAM_RED:
			return GameFlags::cCOUNTRY_USSR;
		}

		return GameFlags::cCOUNTRY_DEFAULT;
	}
	void tGameApp::fSetUnitUpgradeProgressionTable( const tFilePathPtr& path )
	{
		mDataTables[ cDataTableUnitUpgradeProgression ] = mResourceDepot->fQuery( tResourceId::fMake< tDataTableFile >( path ) );
	}
	void tGameApp::fAddTimedCallback( f32 time, const Sqrat::Function& func )
	{
		mCallbacks.fPushBack( tTimedCallback( time, func, false ) );
	}
	void tGameApp::fAddPausableTimedCallback( f32 time, const Sqrat::Function& func )
	{
		mCallbacks.fPushBack( tTimedCallback( time, func, true ) );
	}
	void tGameApp::fScriptAudioEvent( const char *event )
	{
		if( !Perf_Audio_Disable2D )
		{
			sigassert( mSoundSystem );
			sigassert( mSoundSystem->fMasterSource( ) );
			mSoundSystem->fMasterSource( )->fHandleEvent( event );
		}
	}
	void tGameApp::fSetAudioParam( const char *param, f32 val )
	{
		if( !Perf_Audio_Disable2D )
		{
			sigassert( mSoundSystem );
			sigassert( mSoundSystem->fMasterSource( ) );
			mSoundSystem->fMasterSource( )->fSetGameParam( tStringPtr( param ), val );
		}
	}
	b32 tGameApp::fRewindMenuDisabled( ) const
	{
		return Debug_UI_DisableRewindMenu;
	}
	void tGameApp::fAskPlayerToBuyGame( )
	{
		if( !fIsFullVersion( ) )
		{
			mAskPlayerToBuy = true;
		}
	}
	tSaveGameRewindPreview* tGameApp::fRewindPreview( u32 waveIndex )
	{
		if( waveIndex >= mRewindSaves.fCount( ) )
			return NULL;
		else
			return mRewindSaves[ waveIndex ].fGetRawPtr( );
	}

	Sqrat::Object tGameApp::fAC130BarrageData( u32 vpIndex ) const
	{
		tGameApp& game = tGameApp::fInstance( );
		tLevelLogic* level = game.fCurrentLevel( );
		tPlayer* player = 0;
		tDynamicArray< tPlayerPtr >& players = game.fPlayers( );
		for( u32 i = 0; i < game.fPlayerCount( ); ++i )
		{
			if( vpIndex == players[ i ]->fUser( )->fViewportIndex( ) )
			{
				player = players[ i ].fGetRawPtr( );
				break;
			}
		}

		sigassert( player && "No player found with that viewport index" );

		Sqrat::Table results;
		if( level->fMapType( ) != GameFlags::cMAP_TYPE_MINIGAME )
		{
			const tBarragePtr& barrage = player->fCurrentBarrage( );
			if( !barrage.fIsNull( ) )
			{
				// Time left in Barrage
				tBarrage* barrageData = barrage.fCodeObject( );
				Sqrat::Object barrageScript = barrage.fScriptObject( );
				if( barrageData )
				{
					results.SetValue( _SC( "barrageData" ), barrageScript );
					results.SetValue( _SC( "progress" ), barrageData->fProgress( ) );
				}
			}
		}
		else
		{
			results.SetValue( _SC( "timeLeft" ), level->fMiniGameTime( ) );
		}

		// North Angle compared to camera
		const Gfx::tCamera& camera = game.fScreen( )->fViewport( vpIndex )->fRenderCamera( );
		Math::tVec3f dir = camera.fGetTripod( ).mLookAt - camera.fGetTripod( ).mEye;
		dir.fProjectToXZAndNormalize( );
		const f32 angle = Math::fAtan2( dir.x, dir.z );
		results.SetValue( _SC( "northAngle" ), angle );

		// Position of camera
		results.SetValue( _SC( "position" ), camera.fGetTripod( ).mEye );

		return results;
	}

	Math::tRect tGameApp::fComputeScreenSafeRect( ) const
	{
		return mScreen->fComputeSafeRect( );
	}

	void tGameApp::fFadeThroughBlack( f32 inTime, f32 holdTime, f32 outTime )
	{
		Sqrat::Function f( tScriptVm::fInstance( ).fRootTable( ), "FadeThroughBlack" );
		if( !f.IsNull( ) )
			f.Execute( inTime, holdTime, outTime );
	}

	void tGameApp::fShowErrorDialogBox( const char* locId, b32 cancelAble, tUser* user, u32 data, b32 pause, b32 acceptAnyInput )
	{
		if( !user )
			user = fFrontEndPlayer( )->fUser( ).fGetRawPtr( );

		Sqrat::Function f( tScriptVm::fInstance( ).fRootTable( ), "CanvasCreateModalDialogBox" );
		if( !f.IsNull( ) )
			f.Execute( locId, user, Sqrat::Object( Sqrat::Table( ).SetValue( "Value", data ).SetValue( "CanCancel", cancelAble ) ), pause, acceptAnyInput );
	}

	namespace
	{
		static const tStringPtr cCouldntSaveProfileLocString( "error_couldntsaveprofile" );
		static const tStringPtr cCouldntLoadLevelLocString( "error_couldntloadlevel" );
		static const tStringPtr cCouldntLoadMenuLocString( "error_couldntloadmenu" );

		static const tStringPtr cCorruptSaveFileLocString( "error_corruptsavefile" );

		//static const tStringPtr cCorruptSaveFileLocString2( "error_corruptsavefile2" );
		static const tStringPtr cMissingSaveFileLocString( "error_missingsavefile" );
		static const tStringPtr cCouldntSaveFileLocString( "error_couldntsavefile" );
		static const tStringPtr cNoSaveDeviceLocString( "error_couldntsavefile" );

		static const tStringPtr cProfileDeviceRemovedLocString( "error_profiledeviceremoved" );
		static const tStringPtr cRewindListDeviceRemovedLocString( "error_rewindlistdeviceremoved" );

		static const tStringPtr cCorruptDLCLocString( "error_corruptdlc" );
		static const tStringPtr cUnlicensedDLCLocString( "error_unlicenseddlc" );
		static const tStringPtr cInstalledDLCLocString( "error_isntalleddlc" );

		static const tStringPtr cMustHaveTUWarningLocString( "error_must_have_tu" );
	}

	void tGameApp::fShowErrorDialog( tErrorTypes error, tUser* user )
	{
		if( !user || !user->fIsUsedByGame( ) )
		{
			sigassert( !"Error dialogs are only supported for users used by the game" );
			return;
		}

		if( !user->fIsLocal( ) )
			return;

		// Don't double send this error
		if( error == cCouldntSaveProfile && mLoadingFrontEndBecauseProfileCouldntSave )
			return;

#ifdef platform_xbox360
		// Queue the error up - this delayed behavior ensures determinism with errors
		// in networked games
		mGameAppSession.fQueueErrorDialog( error, user->fPlatformId( ) );
#endif
	}

	void tGameApp::fReallyShowErrorDialog( u32 error, tPlatformUserId userId )
	{
		// Convert the user id to a player
		tPlayer * player = NULL;
		for( u32 p = 0; p < mPlayers.fCount( ); ++p )
		{
			if( mPlayers[ p ]->fUser( )->fPlatformId( ) == userId )
			{
				player = mPlayers[ p ].fGetRawPtr( );
				break;
			}
		}

		// No player?!?
		if( !player )
		{
			log_warning( 0, "UserId could not be converted to a player when showing an error dialog" );
			return;
		}

		tStringPtr msg;

		b32 pause = true;
		b32 cancelAble = false;
		b32 acceptAnyInput = false;

		switch( error )
		{
		case cCouldntSaveProfile: 
			msg = cCouldntSaveProfileLocString;  
			cancelAble = true;
			if( mLoadingFrontEndBecauseProfileCouldntSave )
				return;
			break;
		case cCouldntLoadLevel: 
			msg = cCouldntLoadLevelLocString;  
			pause = false;
			fLoadFrontEnd( ); 
			break;
		case cCouldntLoadMenu: 
			msg = cCouldntLoadMenuLocString;  
			pause = false;
			fQuitAsync( );
			break;
		case cCorruptSaveFile: 
			msg = cCorruptSaveFileLocString;
			pause = false;
			fLoadFrontEnd( );
		case cMissingSaveFile: 
			msg = cMissingSaveFileLocString;
			pause = false;
			fLoadFrontEnd( );
			break;
		case cCouldntSaveFile: 
			{
				msg = cCouldntSaveFileLocString;  
				if( tGameApp::fInstance( ).fCurrentLevel( ) )
					tGameApp::fInstance( ).fCurrentLevel( )->fSetDisableRewind( true );

				for( u32 i = 0; i < fPlayerCount( ); ++i )
					fPlayers( )[ i ]->fProfile( )->fInvalidateLastRewind( );
			}
			break;
		case cNoSaveDevice: 
			msg = cNoSaveDeviceLocString;
			break;
		case cProfileDeviceRemoved:
			msg = cProfileDeviceRemovedLocString;
			cancelAble = true;
			break;
		case cRewindListDeviceRemoved:
			msg = cRewindListDeviceRemovedLocString;
			if( tGameApp::fInstance( ).fCurrentLevel( ) )
				tGameApp::fInstance( ).fCurrentLevel( )->fSetDisableRewind( true );
			break;
		case cCorruptDLC:
			msg = cCorruptDLCLocString;
			break;
		case cUnLicensedDLC:
			msg = cUnlicensedDLCLocString;
			cancelAble = true;
			acceptAnyInput = true;
			break;
		case cDLCInstalled:
			msg = cInstalledDLCLocString;
			cancelAble = true;
			acceptAnyInput = true;
			break;
		case cTitleUpdateWarning:
			msg = cMustHaveTUWarningLocString;
			break;
		};

		if( msg.fExists( ) )
		{
			pause = pause && !fGameMode( ).fIsFrontEnd( ) && fGameMode( ).fIsLocal( );

			fShowErrorDialogBox( msg.fCStr( ), cancelAble, player->fUser( ).fGetRawPtr( ), error, pause, acceptAnyInput );
		}
	}

	// returns true if succeed
	b32 tGameApp::fUserProfileConfirm( tUser* user, b32 confirm, b32 saveOnSuccess )
	{
		b32 success = true;

		if( confirm )
		{
			// try to write again
			tPlayer* player = tGameApp::fInstance( ).fGetPlayerByUser( user );
			if( !player->fUser( )->fCanWriteToProfile( ) )
			{
				success = false;
			}
			else if( saveOnSuccess )
				player->fSaveProfile( );
		}
		else
		{
			if( !mCurrentLevelLoadInfo.fIsFrontEnd( ) )
				mLoadingFrontEndBecauseProfileCouldntSave = true;

			// kick them out of the session
			mGameAppSession.fSendSessionEvent( ApplicationEvent::cProflieStorageDeviceRemoved, user->fLocalHwIndex( ) );
		}

		return success;
	}
	
	b32 tGameApp::fErrorDialogConfirm( u32 data, b32 confirmOrCancel, tUser* user )
	{
		b32 allowClose = true;

		if( !user->fIsLocal( ) )
			return allowClose;

		switch( data )
		{
		case cCouldntSaveProfile: 
			allowClose = fUserProfileConfirm( user, confirmOrCancel, true ); 
			break;
		case cCouldntLoadLevel:
			break;
		case cCouldntLoadMenu: 
			break;
		case cCorruptSaveFile: 
			break;
		case cMissingSaveFile: 
			break;
		case cCouldntSaveFile: 
			break;
		case cNoSaveDevice:
			break;
		case cProfileDeviceRemoved:
			allowClose = fUserProfileConfirm( user, confirmOrCancel, false );
			break;
		case cRewindListDeviceRemoved:
			break;
		case cCorruptDLC:
			break;
		case cUnLicensedDLC:
			if( confirmOrCancel )
				tApplication::fInstance( ).fQuitAsync( true ); //reboot
			break;
		case cDLCInstalled:
			if( confirmOrCancel )
				tApplication::fInstance( ).fQuitAsync( true ); //reboot
			break;
		case cTitleUpdateWarning:
			break;
		};

		return allowClose;
	}
	
	void tGameApp::fResourceLoadFailure( tResource* resource )
	{
		// for safetys sake we're not getting fancy.
		//  if it's a legit release build, we just kick them to the dashboard.
		//  if it's an internal build, we assert so that the developer can see the log warnings and fix them.
		#ifdef build_release
			fQuitAsync( );
		#else
			sigassert( !"You need to fix the resource issues shown in the log!" );
		#endif


//		if( !mHasEverReachedFrontEnd )
		//		{
		//#ifdef build_release
		//			fQuitAsync( );
		//#else
		//			sigassert( !"You need to fix the resource issues shown in the log!" );
		//#endif
//		}
//		else
//		{
//			if( mCurrentLevelLoadInfo.fIsFrontEnd( ) )
//			{
//				fShowErrorDialog( cCouldntLoadMenu, NULL );
//			}
//			else
//			{
//				fShowErrorDialog( cCouldntLoadLevel, NULL );
//			}
//		}
	}

	u8 tGameApp::fGetLocalizedThousandsSeparator( ) const
	{
		switch( fLocale( ) )
		{
		case GameFlags::cLOCALE_NO_LOCALE:
		case GameFlags::cLOCALE_AUSTRALIA:
		case GameFlags::cLOCALE_CANADA:
		case GameFlags::cLOCALE_CHINA:
		case GameFlags::cLOCALE_HONG_KONG:
		case GameFlags::cLOCALE_INDIA:
		case GameFlags::cLOCALE_IRELAND:
		case GameFlags::cLOCALE_JAPAN:
		case GameFlags::cLOCALE_KOREA:
		case GameFlags::cLOCALE_MEXICO:
		case GameFlags::cLOCALE_NEW_ZEALAND:
		case GameFlags::cLOCALE_SINGAPORE:
		case GameFlags::cLOCALE_SWITZERLAND:
		case GameFlags::cLOCALE_TAIWAN:
		case GameFlags::cLOCALE_GREAT_BRITAIN:
		case GameFlags::cLOCALE_UNITED_STATES:
		default:
			return ',';

		case GameFlags::cLOCALE_AUSTRIA:
		case GameFlags::cLOCALE_BELGIUM:
		case GameFlags::cLOCALE_BRAZIL:
		case GameFlags::cLOCALE_CHILE:
		case GameFlags::cLOCALE_COLOMBIA:
		case GameFlags::cLOCALE_CZECH_REPUBLIC:
		case GameFlags::cLOCALE_DENMARK:
		case GameFlags::cLOCALE_FINLAND:
		case GameFlags::cLOCALE_GERMANY:
		case GameFlags::cLOCALE_GREECE:
		case GameFlags::cLOCALE_HUNGARY:
		case GameFlags::cLOCALE_ITALY:
		case GameFlags::cLOCALE_NETHERLANDS:
		case GameFlags::cLOCALE_NORWAY:
		case GameFlags::cLOCALE_POLAND:
		case GameFlags::cLOCALE_PORTUGAL:
		case GameFlags::cLOCALE_SLOVAK_REPUBLIC:
		case GameFlags::cLOCALE_SOUTH_AFRICA:
		case GameFlags::cLOCALE_SPAIN:
		case GameFlags::cLOCALE_SWEDEN:
		case GameFlags::cLOCALE_RUSSIAN_FEDERATION:
			return '.';

		case GameFlags::cLOCALE_FRANCE:
			return ' ';
		}
	}

	u8 tGameApp::fGetLocalizedDecimalSeparator( ) const
	{
		switch( fLocale( ) )
		{
		case GameFlags::cLOCALE_NO_LOCALE:
		case GameFlags::cLOCALE_AUSTRALIA:
		case GameFlags::cLOCALE_CANADA:
		case GameFlags::cLOCALE_CHINA:
		case GameFlags::cLOCALE_HONG_KONG:
		case GameFlags::cLOCALE_INDIA:
		case GameFlags::cLOCALE_IRELAND:
		case GameFlags::cLOCALE_JAPAN:
		case GameFlags::cLOCALE_KOREA:
		case GameFlags::cLOCALE_MEXICO:
		case GameFlags::cLOCALE_NEW_ZEALAND:
		case GameFlags::cLOCALE_SINGAPORE:
		case GameFlags::cLOCALE_SWITZERLAND:
		case GameFlags::cLOCALE_TAIWAN:
		case GameFlags::cLOCALE_GREAT_BRITAIN:
		case GameFlags::cLOCALE_UNITED_STATES:
		default:
			return '.';

		case GameFlags::cLOCALE_AUSTRIA:
		case GameFlags::cLOCALE_BELGIUM:
		case GameFlags::cLOCALE_BRAZIL:
		case GameFlags::cLOCALE_CHILE:
		case GameFlags::cLOCALE_COLOMBIA:
		case GameFlags::cLOCALE_CZECH_REPUBLIC:
		case GameFlags::cLOCALE_DENMARK:
		case GameFlags::cLOCALE_FINLAND:
		case GameFlags::cLOCALE_FRANCE:
		case GameFlags::cLOCALE_GERMANY:
		case GameFlags::cLOCALE_GREECE:
		case GameFlags::cLOCALE_HUNGARY:
		case GameFlags::cLOCALE_ITALY:
		case GameFlags::cLOCALE_NETHERLANDS:
		case GameFlags::cLOCALE_NORWAY:
		case GameFlags::cLOCALE_POLAND:
		case GameFlags::cLOCALE_PORTUGAL:
		case GameFlags::cLOCALE_SLOVAK_REPUBLIC:
		case GameFlags::cLOCALE_SOUTH_AFRICA:
		case GameFlags::cLOCALE_SPAIN:
		case GameFlags::cLOCALE_SWEDEN:
		case GameFlags::cLOCALE_RUSSIAN_FEDERATION:
			return ',';
		}
	}

	namespace
	{
		b32 fDebugUIGamercardViewDisabled( tGameApp* g )
		{
			return Debug_UI_DisableGamercardView;
		}

		b32 fDebugMainMenuEnabled( tGameApp* g )
		{
			return Debug_DebugMainMenuEnabled;
		}

		b32 fDebugFastBootupScreens( tGameApp* g )
		{
			return Debug_FastBootupScreens;
		}

		b32 fIsWideLanguage( tGameApp* g )
		{
			switch( g->fLanguage( ) )
			{
			case GameFlags::cLANGUAGE_GERMAN:
			case GameFlags::cLANGUAGE_SPANISH:
			case GameFlags::cLANGUAGE_FRENCH:
			case GameFlags::cLANGUAGE_ITALIAN:
				return true;

			default:
				return false;
			}
		}

		b32 fIsAsianLanguage( tGameApp* g )
		{
			switch( g->fLanguage( ) )
			{
			case GameFlags::cLANGUAGE_JAPANESE:
			case GameFlags::cLANGUAGE_KOREAN:
			case GameFlags::cLANGUAGE_CHINESE:
				return true;

			default:
				return false;
			}
		}
	}

	tLocalizedString tGameApp::fGetLocalLevelName( u32 mapType, u32 index )
	{
		if( mapType < GameFlags::cMAP_TYPE_COUNT && index < fNumLevelsInTable( mapType ) )
		{
			const tLevelLoadInfo& info = fQueryLevelLoadInfoFromTable( mapType, index );
			return info.mMapDisplayName;
		}
		
		return tLocalizedString( );
	}

	bool tGameApp::fIsUserHostFromScript( tUser* user ) const
	{
		sigassert( user );
		const b32 hosting = mGameAppSession.fIsHost( );
		const b32 local = user->fIsLocal( );
		return ( local && hosting ) || ( !local && !hosting );
	}

	bool tGameApp::fIsRewindEnabledFromScript( ) const
	{
		sigassert( mCurrentLevel );

		if( mCurrentLevel->fDisableRewind( ) )
			return false;
		
		if( mCurrentLevel->fMapType( ) != GameFlags::cMAP_TYPE_CAMPAIGN )
			return false;

		return true;
	}

	void tGameApp::fWriteCachedLeaderboards( )
	{
		// Kick off score writer for stats queued while the session was missing
		// If it fails, forget about the stats

		tPlayer* sessionOwner = fFrontEndPlayer( );
		if( !( sessionOwner && fCachedScoreWriter( ).fStartSession( sessionOwner->fUser( ) ) ) )
			fCachedScoreWriter( ).fReset( );
	}

	b32 tGameApp::fIsOneManArmy( ) const
	{
		return ( fCurrentLevelLoadInfo( ).mMapType == GameFlags::cMAP_TYPE_SURVIVAL && fCurrentLevelLoadInfo( ).mChallengeMode == GameFlags::cCHALLENGE_MODE_COMMANDO );
	}

	/// \return true if NOT compatible
	b32 tGameApp::fPlayerIncompatibilityCheck( )
	{
		if( fPlayerCount( ) < 2 )
			return false;
		else if( fGetPlayer( 0 )->fUser( )->fIsLocal( ) && fGetPlayer( 1 )->fUser( )->fIsLocal( ) )
			return false;
		else
		{
			//you either both need something, or both need nothing to play.
			// using XOR (if both people have it installed, this will return false, and if they both dont, it will return false)
			// if( one does and one one doesnt ) returns true.
			return ((fGetPlayer( 0 )->fUser( )->fAddOnsInstalled( ) != 0) ^ (fGetPlayer( 1 )->fUser( )->fAddOnsInstalled( ) != 0));
		}
	}

	void tLevelLoadInfo::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tLevelLoadInfo, Sqrat::DefaultAllocator<tLevelLoadInfo>> classDesc( vm.fSq( ) );
		classDesc
			.Prop( _SC("GameMode"),				&tLevelLoadInfo::fGameModeForScript )
			.Var( _SC("MapType"),				&tLevelLoadInfo::mMapType )
			.Var( _SC("LevelIndex"),			&tLevelLoadInfo::mLevelIndex )
			.Func( _SC("GetMapPath"),			&tLevelLoadInfo::fMapPath )
			.Func( _SC("GetLoadScript"),		&tLevelLoadInfo::fLoadScript )
			.Func( _SC("GetWaveList"),			&tLevelLoadInfo::fWaveList )
			.Prop( _SC("DescriptionLocId"),		&tLevelLoadInfo::fDescriptionLocId )
			.Func( _SC("GetPreviewImagePath"),	&tLevelLoadInfo::fPreviewImage )
			.Var( _SC("MapDisplayName"),		&tLevelLoadInfo::mMapDisplayName )
			.Var( _SC("Country"),				&tLevelLoadInfo::mCountry )
			.Var( _SC("DlcNumber"),				&tLevelLoadInfo::mDlcNumber )
			.Var( _SC("AvailableInTrial"),		&tLevelLoadInfo::mAvailableInTrial )
			.Var( _SC("SkipBriefing"),			&tLevelLoadInfo::mSkipBriefing)
			.Var( _SC("Tickets"),				&tLevelLoadInfo::mTickets )
			.Var( _SC("Money"),					&tLevelLoadInfo::mMoney )
			.Var( _SC("Difficulty"),			&tLevelLoadInfo::mDifficulty )
			.Var( _SC("ChallengeMode"),			&tLevelLoadInfo::mChallengeMode )
			.Prop(_SC("ChallengeVehicles"),		&tLevelLoadInfo::fChallengeVehicles, &tLevelLoadInfo::fSetChallengeVehicles)
			.Prop(_SC("ChallengeBarrages"),		&tLevelLoadInfo::fChallengeBarrages, &tLevelLoadInfo::fSetChallengeBarrages)
			.Prop(_SC("ChallengeTurretAI"),		&tLevelLoadInfo::fChallengeTurretAI, &tLevelLoadInfo::fSetChallengeTurretAI)
			.Prop(_SC("RankDesc"),				&tLevelLoadInfo::fRankDescLocKey)
			.Func(_SC("RankThreshold"),			&tLevelLoadInfo::fRankThreshold)
			;
		vm.fRootTable( ).Bind( _SC("LevelLoadInfo"), classDesc ); 
	}

	void tGameApp::fExportScriptInterface( tScriptVm& vm )
	{
		for( u32 i = 0; i < GameFlags::cEVENT_COUNT; ++ i )
		{
			vm.fConstTable( ).Const( _SC(GameFlags::Detail::cGameEventNames[ i ].fCStr( )), int( i ) );
		}

		Sqrat::DerivedClass<tGameApp, tGameAppBase, Sqrat::NoConstructor> classDesc( vm.fSq( ) );

		// bind "global"ish stuff here, like data tables
		classDesc
			.Prop(_SC("GameMode"),									&tGameApp::fGameMode)
			.Prop(_SC("CurrentLevel"),								&tGameApp::fCurrentLevelForScript, &tGameApp::fSetCurrentLevelForScript)
			.Prop(_SC("SpawningCurrentLevel"),						&tGameApp::fSpawningCurrentLevel)
			.Prop(_SC("FrontEndPlayer"),							&tGameApp::fFrontEndPlayer)
			.Prop(_SC("SecondaryPlayer"),							&tGameApp::fSecondaryPlayer)
			.Prop(_SC("PlayerCount"),								&tGameApp::fPlayerCount)
			.Func(_SC("OtherPlayer"),								&tGameApp::fOtherPlayer)
			.Prop(_SC("HasInvite"),									&tGameApp::fHasInviteFromScript)
			.Func(_SC("GetPlayer"),									&tGameApp::fGetPlayer)
			.Func(_SC("SetPlayerLocalUser"),						&tGameApp::fSetPlayerLocalUserFromScript)
			.Func(_SC("GetLocalUser"),								&tGameApp::fGetLocalUser)
			.Prop(_SC("LocalUserCount"),							&tGameApp::fLocalUserCount)
			.Func(_SC("WhichUser"),									&tGameApp::fWhichUser)
			.Func(_SC("WhichPlayer"),								&tGameApp::fWhichPlayer)
			.Func(_SC("GetPlayerByUser"),							&tGameApp::fGetPlayerByUser)
			.Func(_SC("LoadLevel"),									&tGameApp::fLoadLevelFromScript)
			.Func(_SC("GetDifficulty"),								&tGameApp::fDifficulty)
			.Func(_SC("GetChallengeMode"),							&tGameApp::fChallengeMode)
			.Func(_SC("ReloadCurrentLevel"),						&tGameApp::fReloadCurrentLevel)
			.Func(_SC("AnyPlayerWithButtonsDown"),					&tGameApp::fAnyPlayerWithButtonsDown)
			.Func(_SC("AnyLocalPlayerWithButtonsDown"),				&tGameApp::fAnyLocalPlayerWithButtonsDown)
			.Func(_SC("AnyPlayerOpeningPauseMenu"),					&tGameApp::fAnyPlayerOpeningPauseMenu)
			.Func(_SC("AnyPlayerOpeningRewindMenu"),				&tGameApp::fAnyPlayerOpeningRewindMenu)
			.Func<void (tGameApp::*)(void)>(_SC("LoadFrontEnd"),	&tGameApp::fLoadFrontEnd)
			.Func(_SC("Pause"),										&tGameApp::fPause)
			.Func(_SC("Paused"),									&tGameApp::fPaused)
			.Prop(_SC("CurrentLevelDisplayName"),					&tGameApp::fCurrentLevelDisplayName)
			.Func(_SC("NumLevelsInTable"),							&tGameApp::fNumLevelsInTable)
			.Func(_SC("NumCampaignLevels"),							&tGameApp::fNumCampaignLevels)
			.Func(_SC("NextCampaignLevel"),							&tGameApp::fNextCampaignLevel)
			.Func(_SC("IsCampaignComplete"),						&tGameApp::fIsCampaignComplete)
			.Func(_SC("LevelNameInTable"),							&tGameApp::fLevelNameInTable)
			.Func(_SC("GetLevelLoadInfo"),							&tGameApp::fLevelLoadInfo)
			.Prop(_SC("CurrentLevelLoadInfo"),						&tGameApp::fCurrentLevelLoadInfo)
			.Prop(_SC("NextLevelLoadInfo"),							&tGameApp::fNextLevelLoadInfo)
			.Prop(_SC("DebugLevelCount"),							&tGameApp::fNumDebugLevels)
			.Func(_SC("DebugLevelName"),							&tGameApp::fDebugLevelName)
			.Func(_SC("SetTeamOrientation"),						&tGameApp::fSetTeamOrientation)
			.Func(_SC("UnitWaveIconPath"),							&tGameApp::fUnitWaveIconPath)
			.Func(_SC("SetUnitUpgradeProgressionTable"),			&tGameApp::fSetUnitUpgradeProgressionTable)
			.Prop(_SC("AAACantLose"),								&tGameApp::fAAACantLose)
			.Func(_SC("AddTimedCallback"),							&tGameApp::fAddTimedCallback)
			.Func(_SC("AddPausableTimedCallback"),					&tGameApp::fAddPausableTimedCallback)
			.Func(_SC("DefaultTeamFromCountry"),					&tGameApp::fDefaultTeamFromCountry)
			.Func(_SC("AudioEvent"),								&tGameApp::fScriptAudioEvent)
			.Func(_SC("SetAudioParam"),								&tGameApp::fSetAudioParam)
			.Func(_SC("ForEachPlayer"),								&tGameApp::fForEachPlayerFromScript)
			.Func(_SC("JoinInviteSession"),							&tGameApp::fJoinInviteSession)
			.Func(_SC("FireInviteEvent"),							&tGameApp::fFireInviteEvent)
			.Func(_SC("ClearInvite"),								&tGameApp::fClearInvite)
			.Prop(_SC("GameAppSession"),							&tGameApp::fGameAppSession)
			.Func(_SC("DoAskPlayerToBuyGame"),						&tGameApp::fAskPlayerToBuyGame)
			.Prop(_SC("FromSaveGame"),								&tGameApp::fFromSaveGame)
			.Var(_SC("AskPlayerToBuyGame"),							&tGameApp::mAskPlayerToBuy)
			.Var(_SC("AskPlayerToBuyGameCallback"),					&tGameApp::mAskPlayerToBuyCallback)
			.Func(_SC("RewindPreview"),								&tGameApp::fRewindPreview)
			.Prop(_SC("RewindMenuDisabled"),						&tGameApp::fRewindMenuDisabled)
			.Func(_SC("Rewind"),									&tGameApp::fRewind)
			.Prop(_SC("HighestRewindableWaveIndex"),				&tGameApp::fHighestRewindableWaveIndex)
			.Func(_SC("AC130BarrageData"),							&tGameApp::fAC130BarrageData)
			.Prop(_SC("SingleScreenCoop"),							&tGameApp::fSingleScreenCoopEnabled, &tGameApp::fSetSingleScreenCoop)
			.Prop(_SC("SingleScreenControlIndex"),					&tGameApp::fSingleScreenControlIndex, &tGameApp::fSetSingleScreenControlIndex)
			.Prop(_SC("PAXDemoMode"),								&tGameApp::fPAXDemoModeScript)
			.Prop(_SC("E3Mode"),									&tGameApp::fE3ModeScript)
			.Func(_SC("SetBrightness"),								&tGameApp::fSetBrightness)
			.Func(_SC("SetMusicVolume"),							&tGameApp::fSetMusicVolume)
			.Func(_SC("SetSfxVolume"),								&tGameApp::fSetSfxVolume)
			.Func(_SC("SetHUDVolume"),								&tGameApp::fSetHUDVolume)
			.Func(_SC("SetListeningMode"),							&tGameApp::fSetListeningMode)
			.Func(_SC("ComputeScreenSafeRect"),						&tGameApp::fComputeScreenSafeRect)
			//.Func(_SC("SetSecondLocalUser"),						&tGameApp::fSetSecondLocalUserFromScript)
			.Func(_SC("SetPlayerFromUser"),							&tGameApp::fSetPlayerFromUser)
			.GlobalFunc( _SC("GamercardViewDisabled"),				&fDebugUIGamercardViewDisabled)
			.GlobalFunc( _SC("DebugMainMenuEnabled"),				&fDebugMainMenuEnabled)
			.GlobalFunc( _SC("DebugFastBootupScreens"),				&fDebugFastBootupScreens)
			.Prop(_SC("Language"),									&tGameApp::fLanguage)
			.Prop(_SC("Region"),									&tGameApp::fRegion)
			.Prop(_SC("Locale"),									&tGameApp::fLocale)
			.GlobalFunc( _SC("IsWideLanguage"),						&fIsWideLanguage)
			.GlobalFunc( _SC("IsAsianLanguage"),					&fIsAsianLanguage)
			.Prop(_SC("InMultiplayerLobby"),						&tGameApp::fInMultiplayerLobby, &tGameApp::fSetInMultiplayerLobby)
			.Func(_SC("ErrorDialogConfirm"),						&tGameApp::fErrorDialogConfirm)
			.Func(_SC("GetPlayerByHwIndex"),						&tGameApp::fGetPlayerByHwIndex)
			.Func(_SC("GetLocalLevelName"),							&tGameApp::fGetLocalLevelName)
			.Var(_SC("LevelNameReadyToRead"),						&tGameApp::mLevelNameReadyToRead)	
			.Func(_SC("IsUserHost"),								&tGameApp::fIsUserHostFromScript)
			.Prop(_SC("RewindEnabled"),								&tGameApp::fIsRewindEnabledFromScript)
			.Func(_SC("HostPlayerOpeningRewindMenu"),				&tGameApp::fHostPlayerOpeningRewindMenu)
			.Func(_SC("ApplyRichPresence"),							&tGameApp::fApplyRichPresence)
			.Prop(_SC("OneManArmy"),								&tGameApp::fIsOneManArmy)
			.Func(_SC("PlayerIncompatibilityCheck"),				&tGameApp::fPlayerIncompatibilityCheck)
			.Var(_SC("DifficultyOverride"),						&tGameApp::mDifficultyOverride)	
			;

		vm.fRootTable( ).Bind(_SC("tGameApp"), classDesc );

		vm.fRootTable( ).SetInstance(_SC("GameApp"), &fInstance( ));
		vm.fRootTable( ).SetInstance(_SC("GameAppSession"), &fInstance( ).mGameAppSession);

		vm.fRootTable( ).SetInstance(_SC("COLOR_DIRTY_WHITE"), &cColorDirtyWhite);
		vm.fRootTable( ).SetInstance(_SC("COLOR_CLEAN_WHITE"), &cColorCleanWhite);
		vm.fRootTable( ).SetInstance(_SC("COLOR_LOCKED_GREEN"), &cColorLockedGreen);
		vm.fRootTable( ).SetInstance(_SC("COLOR_SUSPENDED_BLUE"), &cColorSuspendedBlue);

		vm.fConstTable( ).Const( "FONT_FIXED_SMALL", ( int )cFontFixedSmall );
		vm.fConstTable( ).Const( "FONT_SIMPLE_SMALL", ( int )cFontSimpleSmall );
		vm.fConstTable( ).Const( "FONT_SIMPLE_MED", ( int )cFontSimpleMed );
		vm.fConstTable( ).Const( "FONT_FANCY_MED", ( int )cFontFancyMed );
		vm.fConstTable( ).Const( "FONT_FANCY_LARGE", ( int )cFontFancyLarge );
		vm.fConstTable( ).Const( "FONT_COUNT", ( int )cFontCount );
	}

	void tGameApp::fSnapshotMemoryConsumption( )
	{
#ifdef sig_devmenu
		if( Debug_Stats_Snapshot )
		{
			std::stringstream path;
#ifdef platform_xbox360
			path << "GAME:\\";
#else
			path << "C:\\";
#endif//platform_xbox360
			path << StringUtil::fNameFromPath( fCurrentLevelLoadInfo( ).fMapPath( ) ) << "_stats.txt";

			const Sig::tFilePathPtr filePath( path.str( ) );

			std::string data = fMakeDebugString( );
			FileSystem::fWriteBufferToFile( data.c_str( ), data.length( ), filePath );
		}
#endif//sig_devmenu
	}

} //namespace Sig


#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
#include "resource.h"
namespace Sig
{
	void tGameApp::fConfigureAppPlatform( tStartupOptions& opts, tPlatformStartupOptions& platformOptsOut )
	{
		platformOptsOut.mIconId = IDI_APPICON;
		opts.mFixedTimeStep = 1.f / 30.f;
		opts.mUpdateDelay = opts.mFixedTimeStep;
	}
}
#elif defined( platform_xbox360 )
namespace Sig
{
	void tGameApp::fConfigureAppPlatform( tStartupOptions& opts, tPlatformStartupOptions& platformOptsOut )
	{
		opts.mFixedTimeStep = 1.f / 30.f;
	}
}
#else
namespace Sig
{
	void tGameApp::fConfigureAppPlatform( tStartupOptions& opts, tPlatformStartupOptions& platformOptsOut ) { }
}
#endif
