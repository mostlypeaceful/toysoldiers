sigvars Atmosphere
@[ParticleColor] { "Particle Color", (1.0, 1.0, 1.0, 1.0), [ 0.0:2.0 ], "RGBA Particle Tint" }
@[Fog_MinDist] { "Fog Min Dist", 75.0, [ 0:1000 ], "Fog starts at this distance (from the camera)" }
@[Fog_FadeDist] { "Fog Fade Dist", 300.0, [ 0:1000 ], "Fog starts at this distance (from the camera)" }
@[Fog_Clamp] { "Fog Min Dist", (0.0, 0.175), [ 0.0:1.0 ], "Fog clamp (min, max)" }
@[Fog_Color] { "Fog Color", (0.5, 0.6, 0.7), [ 0.0:1.0 ], "RGB Fog Tint" }
@[Shadows_Amount] { "Shadows Amount", 0.6, [ 0:1000 ], "" }
@[Shadows_Dist] { "Shadows Dist", 500.0, [ 0:1000 ], "" }
@[Shadows_Near] { "Shadows Near", 0.0, [ 0:1000 ], "" }
@[Shadows_Far] { "Shadows Far", 1000.0, [ 0:2000 ], "" }
@[Shadows_Width] { "Shadows Width", 600.0, [ 0:1000 ], "" }
@[Shadows_Height] { "Shadows Height", 600.0, [ 0:1000 ], "" }
@[Post_Saturation] { "PostFx Saturation", (1.0, 1.0, 1.0), [ 0.0:4.0 ], "" }
@[Post_Contrast] { "PostFx Contrast", (1.0, 1.0, 1.0), [ 0.0:4.0 ], "" }
@[Post_Exposure] { "PostFx Exposure", (1.0, 1.0, 1.0), [ 0.0:4.0 ], "" }

sigvars Tutorial
@[Dialog_Timeout] { "TimeOut", 3.0, [ 0.0:5.0 ], "TODO COMMENT" } // Amount of time the tutorial messages remain on screen
@[Dialog_Default_Transition] { "Default Transition", 4, [ 0.0:5.0 ], "TODO COMMENT" } 

sigimport "Levels/Maps/Campaign/LVL_00_Trial/TrialGoals.goaml"
sigimport "Levels/Scripts/Common/game_standard.nut"
sigimport "gui/scripts/hud/minigamecomboui.nut"

sigexport function EntityOnCreate( entity )
{
	RegisterLevelLogic( entity, LevelLogic_Campaign_lvlTrial( ) )
}

class LevelLogic_Campaign_lvlTrial extends GameStandardLevelLogic
{
	shelfWave = null
	mainWave = null
	loopingTransportWave = null
	playerHeliWave = null
	pathFootball = null
	pathForked = null
	phase = 0
	
	firstPhaseCameraBox = null
	secondPhaseCameraBox = null
	thirdPhaseCameraBox = null
	buildSiteCameraBox = null
	
	galleryEffect = null
		
	howitzerComplete = false
	secondPhaseEnemiesFinished = false
	secondPhaseUnit = UNIT_ID_TURRET_AT_03
	okToExit = false
	
	helicopterProgress = 0
	
	// for goamls
	InfantryAlarmShown = null
	VehicleAlarmShown = null
	AirAlarmShown = null
	LuckyStarShown = null
	APCMsgShown = null
	AlarmPromptDuration = null
	AlarmPromptShow = null
	
	minigameCombo = null
	
	constructor( )
	{
		::GameStandardLevelLogic.constructor( )
		
		gDefaultBlueTeamPassengerSigml = "gameplay/characters/passenger/usa/passenger_01.sigml"
		gDefaultRedTeamPassengerSigml = "gameplay/characters/passenger/ussr/passenger_01.sigml"		
		
		NeutralAudioTimer = 30.0 // ##.# second neutral timer
		
		//upgrade a turret.
		//pilot the attack helo.
		//rank: anti-tank i kills
		InitDecoration( 0, null, function( event ) { return event.EventID == TUTORIAL_EVENT_TURRET_UPGRADE }.bindenv( this ) )
		InitDecoration( 1, null, function( event ) { return ( event.EventID == TUTORIAL_EVENT_USE_UNIT && event.CurrentUnitID == UNIT_ID_HELO_ATTACK_01 ) }.bindenv( this ) )
		InitChallenge( function( event ) { return PlayerKilledTheseUnitsWithOneOfTheseUnits( event, [ UNIT_TYPE_INFANTRY, UNIT_TYPE_VEHICLE, UNIT_TYPE_AIR ], [ UNIT_ID_TURRET_AT_01 ] ) }.bindenv( this ) )
	}

	function OnSpawn( )
	{				
		::GameStandardLevelLogic.OnSpawn( )
		
		TutSaveFirstWave = 1
		
		// for goamls
		InfantryAlarmShown = 0
		VehicleAlarmShown = 0
		AirAlarmShown = 0
		LuckyStarShown = 0
		APCMsgShown = 0
		AlarmPromptDuration = 1.0
		AlarmPromptShow = 5.0
				
		phase = 0

		// Set Up Named Camera Boxes
		firstPhaseCameraBox = OwnerEntity.FirstChildWithName( "firstCameraBox" )
		secondPhaseCameraBox = OwnerEntity.FirstChildWithName( "secondCameraBox" )
		thirdPhaseCameraBox = OwnerEntity.FirstChildWithName( "thirdCameraBox" )
		buildSiteCameraBox = OwnerEntity.FirstChildWithName( "buildSiteCameraBox" )
		
		galleryEffect = NamedObject( "galleryEffect" )
				
		shelfWave = AddWaveList( "ShelfGame" )
		shelfWave.Saveable = 0
		
		mainWave = AddWaveList( "MainSection" )
		
		loopingTransportWave = AddWaveList( "LoopingTransport" )
		loopingTransportWave.Saveable = 0
		
		playerHeliWave = AddWaveList( "PlayerHeli" )
		playerHeliWave.SetMakeSelectableUnits( 1 )
		playerHeliWave.Saveable = 0
		
		// Set Up Paratrooper Paths	
		pathFootball = NamedPathPoint( "path_infantry_ussr_transport_01" )
		if( !is_null(  pathFootball ) )
			 pathFootball.Logic.Accessible = 1
		pathForked = NamedPathPoint( "path_infantry_ussr_transport_02" )
		if( !is_null( pathForked ) )
			pathForked.Logic.Accessible = 0
		
		// set do_TestBoss to 'true' to only run boss test wave
		local do_TestBoss = false
		local do_TestMainWave = false
		
		if( do_TestBoss )
		{
			RegisterCameraBox( thirdPhaseCameraBox )
			playerHeliWave.Activate( )	
			AddWaveList( "WaveListBossTest" ).Activate( )
			GameApp.CurrentLevel.SetUIWaveList( "WaveListBossTest" )
			return
		}		
		
		local isCoop = ::GameApp.GameMode.IsCoOp
		if( isCoop || ::GameApp.FromSaveGame || do_TestMainWave || ::GameApp.GetDifficulty( ) == DIFFICULTY_GENERAL )
		{
			// Skip Tutorials, straight to final list
			RegisterCameraBox( thirdPhaseCameraBox )
			SwitchToMainWave( )
			playerHeliWave.Activate( )
			GoalDriven.MasterGoal = TrialMasterGoal( this, { } )
			HandleTutorialEvent( TUTORIAL_EVENT_SKIP )
			LevelIntro2P( "coopPath_01", "coopPath_02", null, false, true )
		}
		else
		{
			// Start Tutorials
			GoalDriven.MasterGoal = TrialMasterGoal( this, { } )
			FirstPhaseTutorialMessages( )
			
			//LevelIntro2P( "camPath", "camPath", null, false, true )
			//onLevelIntroComplete = FirstPhaseTutorialMessages
		}
		
		minigameCombo = ::MinigameComboUI( ControllingPlayer( ) )
	}
	
	function InitialPlayerSetup( player, data )
	{
		player.ShowScoreUI( 0 )
		player.ShowTickets( 0 )
		player.ShowMoney( 0 )
		player.DisableTiltShift = 1
	}

	function SpawnPreIntroWaves( )
	{
	}
	
	function OnDelete( )
	{
		if( minigameCombo )
			minigameCombo.DeleteSelf( )
		GameStandardLevelLogic.OnDelete( )
	}
	
	function OnTicketsCountChanged( count, player )
	{
		// Dont lose when count gets to zero
	}
	
	function OnNoWavesOrEnemiesLeft( )
	{
		// dont win when all enemies killed
	}
	
	// Function to control the branching paths of paratroopers
	function TurnOnBranchingPath( )
	{
		//pathFootball.Logic.Accessible = 1
		//pathForked.Logic.Accessible = 1
	}
	
	//Launch player helicopter
	function PlayerHelicopter( )
	{		
		print( "Player heli spawned" )
		playerHeliWave.Activate( )	
	
		//doing focal prompt now instead of fly through
		GameApp.AddTimedCallback( 2, HelicopterFlyThrough.bindenv( this ) )
		GameApp.AddTimedCallback( 3, ContinueTutorial.bindenv( this ) )
	}
	
	// Continue tutorial after some play time
	function ContinueTutorial( )
	{
		print( "tutorial continued" )
		HandleTutorialEvent( TUTORIAL_EVENT_CONTINUE )
	}
	
	function PlacedFirstTurret( )
	{
		// this may get called mutliple times
		//TutOnlyPlaceThisUnit = -1	
		
		//this didnt seem to work due to external refs
		//CleanupMinis( )
	}

	function LockInMG( )
	{
		print( "locking in current unit" )
		GameApp.GetPlayer( 0 ).LockInCurrentUnit( )
	}
	
	function UnlockUpgrading( )
	{
		// todo
		TutDisableSell = 1
		TutDisableUse = 1
		TutDisableRepair = 1
		TutDisableUpgrade = 0
	}
	
	function UpgradingComplete( )
	{
		TutDisableSell = 0
		TutDisableUse = 0
		TutDisableRepair = 0
		TutDisableUpgrade = 0
	}
	
	function GiveBarrage( )
	{		
		TutDisableBarrage = 0
		TutDisableSell = 0
		GameApp.GetPlayer( 0 ).RestrictBarrage( "BARRAGE_RAMBO" )
		GameApp.GetPlayer( 0 ).GiveBarrage( 1 )		
		GameApp.GetPlayer( 0 ).SetBarrageSkip( NamedObject( "ramboDropTut" ) )
		
		//GameApp.GetPlayer( 0 ).GiveInstantBarrage( "BARRAGE_RAMBO", NamedObject( "ramboDropTut" ) )
		
		//GameApp.GetPlayer( 0 ).CurrentUnit.SellingDisabled = 0
	}
	
	function UnlockCameraBox( )
	{
		TutLightUpOnlyTutorialPlatforms = 0
		TutOnlyPlaceThisUnit = -1	
		
		RegisterCameraBox( thirdPhaseCameraBox )
		
		GameApp.GetPlayer( 0 ).UnlockFromUnit( 0 )
	}
	
	function SwitchToMainWave( )
	{
		loopingTransportWave.Pause( )
		loopingTransportWave.Reset( )
		
		TutDisableRewind = 0
		TutDisableRandomPickups = 0
		
		// must set the ui before calling activate so that it rewinds properly
		GameApp.CurrentLevel.SetUIWaveList( "MainSection" )
		mainWave.SetLooping( false )
		mainWave.Activate( )
	}
	
	function PresentWaveList( )
	{		
		SwitchToMainWave( )
		::TutorialPresenter.Present( "wavelist", ::GameApp.CurrentLevel.ControllingPlayer( ).User, null, true )
	}
	
	//function BuildTurretForUser( )
	//{
	//	local buildSite = BuildSiteNamed( "back_left_buildsite" )
	//	if( is_null( buildSite ) )
	//		print( "Could not find back left build site!" )
	//	else
	//	{
	//		if( !buildSite.Occupied || buildSite.Unit.Logic.UnitID != UNIT_ID_TURRET_MG_01 )
	//			GameApp.GetPlayer( 0 ).BuildTurret( buildSite, UNIT_ID_TURRET_MG_01 )
	//		else
	//		{
	//			local unit = buildSite.Unit;
	//			unit.SetName( "turret_hijack" )
	//			GameApp.GetPlayer( 0 ).LockInUnit( "turret_hijack" )
	//		}
	//	}
	//}
	
	function OnWaveListFinished( waveList )
	{
		
	}
	
	//*********** Begin Phase 1 ********
	function FirstPhaseTutorialMessages( )
	{
		print( "first setup" )
							
		SetUIWaveList( "" )
		
		::GameApp.ForEachPlayer( InitialPlayerSetup.bindenv( this ), null )
		
		phase = 1		
		//TutHideKillHoverText = 1
		TutDisableOvercharge = 1
		TutForceWeaponOvercharge = 1
		TutDisableBarrage = 1
		TutDisableSell = 1
		TutDisableRepair = 1
		TutDisableUpgrade = 1
		TutDisableRewind = 1
		TutDisablePlaceMenu = 1
		TutDisableRandomPickups = 1
		TutContinuousCombo = 1
		DisableOverkills = 1
		UseMinigameScoring = 1
		::GameApp.DifficultyOverride = DIFFICULTY_CASUAL
		TutAlwaysShowUnitControls = 1
		
		// no fly in so manualy set to intro, do not copy this around willy nilly
		SetCurrentLevelProgression( LEVEL_PROGRESSION_INTRO )
		
		HandleTutorialEvent( TUTORIAL_EVENT_BEGIN )
		::GameApp.ForEachPlayer( LockInFirstTurret.bindenv( this ), null )
		RegisterCameraBox( firstPhaseCameraBox )
		
		ControllingPlayer( ).ShowMiniMap( false )
		ControllingPlayer( ).ShowOverchargeMeter( false )
	}
	
	function LockInFirstTurret( player, data )
	{
		player.LockInUnit( "startTurret" )
	}
	
	function LockInSecondTurret( )
	{
		GameApp.ForEachPlayer( LockInSecondTurretImp.bindenv( this ), null )
	}
	
	function LockInSecondTurretImp( player, data )
	{
		player.LockInUnit( "middleTurret" )
	}
	
	function UnlockPlayers( force )
	{
		GameApp.ForEachPlayer( UnlockPlayersImp.bindenv( this ), force )
	}
	
	function UnlockPlayersImp( player, data )
	{
		player.UnlockFromUnit( data )
	}
	
	function TutorialSlowDown( slowDown )
	{
		UnslowDown = 0
		if( slowDown )
		{
			if( TutUnitTimeMultiplier > 0.5 )
			{
				PlaySound( "Play_HUD_SlowMotion" )
				TutUnitTimeMultiplier = 0.1
			}
		}
		else
		{
			if( TutUnitTimeMultiplier < 0.5 )
			{
				PlaySound( "Stop_HUD_SlowMotion" )
				TutUnitTimeMultiplier = 1.0
			}
		}
	}
		
	// ************ Begin Phase 2 ****************
	// PHASE 2 CAM
	function NewCameraFlyThrough( callback ) 
	{ 
		UnlockPlayers( 1 )
		
		shelfWave.KillEveryone( )
		shelfWave.Reset( )
		
		RegisterCameraBox( secondPhaseCameraBox )	
		
		LevelIntro2P( "phase2Cam", "phase2Cam", null, false, false  )
		onLevelIntroComplete = callback 		
		
		TutHideKillHoverText = 0
		
		minigameCombo.Hide( )

		phase = 2
	}
	
	//************** Begin Phase 3 ***************************
	// Phase 3 CAM
	function NewCameraFlyThroughB( ) 
	{ 
		UnlockPlayers( 1 )
		
		LevelIntro2P( "phase3Cam", "phase3Cam", null, false, false  ) 
		onLevelIntroComplete = null
		
		RegisterCameraBox( buildSiteCameraBox )
		loopingTransportWave.Activate( )
						
		phase = 3
		TutHideKillHoverText = 0
		TutFreeUpgrades = 1
		TutDisableOvercharge = 0
		TutForceWeaponOvercharge = 0
		UseMinigameScoring = 0
		::GameApp.DifficultyOverride = -1
		TutContinuousCombo = 0
		TutAllowSpeedBonus = 0
		TutDisableLaunchArrows = 0
		TutDisableRTSRings = 0
		TutLightUpOnlyTutorialPlatforms = 1
		
		::GameApp.ForEachPlayer( function( player, data )
		{
			player.ShowMiniMap( 1 )
			player.ShowOverchargeMeter( 1 )
			player.DisableTiltShift = 0
			player.SetUniqueBarrages( 1 )
		}.bindenv( this ) , null )
		
		::GameAppSession.FlushStats( )
	}
	
	function HelicopterFlyThrough( ) 
	{ 
		UnlockPlayers( 1 )
		LevelIntro2P( "heliCam", "heliCam", null, false, false  ) 
		onLevelIntroComplete = LockInHelicopter
	}
	
	function LockInHelicopter( )
	{
		GameApp.GetPlayer( 0 ).LockInUnit( "PlayerHeli" )
		GameApp.GetPlayer( 0 ).UnlockFromUnit( 0 )		
	}
	
	function TutorialFinished( )
	{
		//reset to normal playing mode
		TutFreeUpgrades = 0
		TutAllowCoopTurrets = 0
		TutDisablePlaceMenu = 0
	}	
	
	function CleanupMinis( )
	{
		local a = OwnerEntity.FirstChildWithName( "table_section" )
		if( !is_null( a ) )
			a.Delete( )
			
		local b = OwnerEntity.FirstChildWithName( "shelf_section" )
		if( !is_null( b ) )
			b.Delete( )
	}

	///////////////////////////////////////////////////////////////
	function ConfigureInfantryMiniGame( )
	{
		miniGameDuration = 30
		miniGameLeaderBoard = LEADERBOARD_TRIAL_MINIGAME_1
		miniGameBeginLocKey = "beginKillMiniGame"
		miniGameKillFunction = InfantryMiniGameKillFunction
		miniGameStartStopFunction = InfantryMiniStartStopFunction
		miniGameScoreIndex = 0
		CommonMiniGameSetup( )
	}
	
	function InfantryMiniGameKillFunction( context )
	{
		if( context.EventUnitType == UNIT_TYPE_INFANTRY )
			return ControllingPlayer( ).LastKillValue
		else 
			return 0
	}
	
	function InfantryMiniStartStopFunction( start, preStart )
	{
		CommonMiniGameStartStop( start, preStart, [ shelfWave ], 0 )
		
		if( start )
		{
			if( preStart )
			{
				shelfWave.Activate( )
			}
		}
		
		CommonMiniGameStartStop( start, preStart, [shelfWave ], 1 )
	}
	
	///////////////////////////////////////////////////////////////
	function ConfigureTargetMiniGame( )
	{		
		miniGameDuration = 60
		miniGameLeaderBoard = LEADERBOARD_TRIAL_MINIGAME_2
		miniGameBeginLocKey = "beginTargetMiniGame"
		miniGameKillFunction = TargetMiniGameKillFunction
		miniGameStartStopFunction = TargetMiniStartStopFunction
		miniGameScoreIndex = 1
		TutAllowSpeedBonus = 1
		CommonMiniGameSetup( )
	}
	
	function TargetMiniGameKillFunction( context )
	{
		if( context.EventUnitType == UNIT_TYPE_TARGET )
		{
			if( !is_null( galleryEffect ) )
			{
				galleryEffect.Logic.Pause( 0 )
				galleryEffect.Logic.Reset( )
			}
			return ControllingPlayer( ).LastKillValue
		}
		else 
			return 0
	}
	
	function TargetMiniStartStopFunction( start, preStart )
	{
		CommonMiniGameStartStop( start, preStart, [ ], 0 )
		
		if( start )
		{			
			TutDisableVehicleInput = 1 //still allow them to shoot before this one.

			local gallery = NamedObject( "shootingGallery" )
			if( !is_null( gallery ) )
			{
				if( preStart )
					gallery = LevelLogic.RespawnEntity( gallery )
				else
				{
					local event = LogicEvent.Construct( GAME_EVENT_REAPPLY_MOTION_STATE )
					gallery.Logic.HandleLogicEvent( event )
				}
			}
			else
				print( "could not find shootingGallery!" );
		}
		
		CommonMiniGameStartStop( start, preStart, [ ], 1 )
	}
	
	function MinigamePrompt( )
	{
		//this gets called when the minigame ready thing shows up
		GameStandardLevelLogic.MinigamePrompt( )
		
		if( miniGameLeaderBoard == LEADERBOARD_TRIAL_MINIGAME_2 )		
			TutDisableVehicleInput = 0 //allow the player to keep shooting in this minigame
	}
	
	function ProcessTutorialEvent( event )
	{
		::GameStandardLevelLogic.ProcessTutorialEvent( event )
		
		if( minigameCombo && TutContinuousCombo )
			minigameCombo.ProcessTutorialEvent( event )
	}
	
	// Atmosphere
	function SetAtmosphere( )
	{
		Gfx.SetFlatParticleColor( Math.Vec4.Construct( @[ParticleColor].x, @[ParticleColor].y, @[ParticleColor].z, @[ParticleColor].w ) )
		Gfx.SetFog(
			Math.Vec4.Construct( @[Fog_MinDist], @[Fog_FadeDist], @[Fog_Clamp].x, @[Fog_Clamp].y ),
			Math.Vec3.Construct( @[Fog_Color].x, @[Fog_Color].y, @[Fog_Color].z ) )
		Gfx.SetShadows( 
			@[Shadows_Amount],
			@[Shadows_Dist],
			@[Shadows_Near],
			@[Shadows_Far],
			@[Shadows_Width],
			@[Shadows_Height] )
		PostEffects.SetSaturation( Math.Vec3.Construct( @[Post_Saturation].x, @[Post_Saturation].y, @[Post_Saturation].z ) ) // r, g, b
		PostEffects.SetContrast( Math.Vec3.Construct( @[Post_Contrast].x, @[Post_Contrast].y, @[Post_Contrast].z ) ) // r, g, b
		PostEffects.SetExposure( Math.Vec3.Construct( @[Post_Exposure].x, @[Post_Exposure].y, @[Post_Exposure].z ) ) // r, g, b
		PostEffects.SetDepthOfField( Math.Vec4.Construct( 0.994, 0.465, 3.9, 0.5 ) ) // x, y, z, w
	}
	
	
	// *********** BEGIN TUTORIAL SECTION! *******************
		
	
	// list of gamepad symbols:
	/*
	GAMEPAD_BUTTON_START
	GAMEPAD_BUTTON_SELECT
	GAMEPAD_BUTTON_A
	GAMEPAD_BUTTON_B
	GAMEPAD_BUTTON_X
	GAMEPAD_BUTTON_Y"
	GAMEPAD_BUTTON_DPAD_RIGHT
	GAMEPAD_BUTTON_DPAD_UP
	GAMEPAD_BUTTON_DPAD_LEFT
	GAMEPAD_BUTTON_DPAD_DOWN
	GAMEPAD_BUTTON_LSHOULDER
	GAMEPAD_BUTTON_LTHUMB
	GAMEPAD_BUTTON_LTRIGGER
	GAMEPAD_BUTTON_LTHUMB_MAXMAG
	GAMEPAD_BUTTON_LTHUMB_MINMAG
	GAMEPAD_BUTTON_RSHOULDER
	GAMEPAD_BUTTON_RTHUMB
	GAMEPAD_BUTTON_RTRIGGER
	GAMEPAD_BUTTON_RTHUMB_MAXMAG
	GAMEPAD_BUTTON_RTHUMB_MINMAG
	
	Event members
	
	.EventID
	.EventValue - 0-1 for begin/end use, unit id for built or destroyed, button id for input
	.EventUnitType ?
	.CurrentUnitID - current unit id, for input events and use_unit events
	.ShellCaming - 1 if in shell came mode, only set for input events
	.Entity ?
	.Player
	.PlatformName
	.WeaponID
	.Combo

	
	// Tutorial Event Ids
	// event value's of ~0 will match any eventid
	TUTORIAL_EVENT_INPUT, event value is a gamepad button (above )
	TUTORIAL_EVENT_UNIT_BUILT, event value is the unit id for the unit built
	TUTORIAL_EVENT_UNIT_DESTROYED, event value is the unit id for the unit destroyed
	TUTORIAL_EVENT_USE_UNIT, event value is 1 for start use, 0 for end use	
	TUTORIAL_EVENT_TURRET_UPGRADE, eventvalue is the new unit id
	TUTORIAL_EVENT_TURRET_REPAIR,  eventvalue is the unit id
	TUTORIAL_EVENT_TURRET_SELL,    eventvalue is the unit id
	TUTORIAL_EVENT_BEGIN
	TUTORIAL_EVENT_CONTINUE
	TUTORIAL_EVENT_USE_MENU
	TUTORIAL_EVENT_OVERCHARGE
	TUTORIAL_EVENT_BARRAGE_RECEIVED
	TUTORIAL_EVENT_BARRAGE_ACTIVATED
	TUTORIAL_EVENT_MINIGAME_BEGIN
	TUTORIAL_EVENT_MINIGAME_END
	TUTORIAL_EVENT_MINIGAME_RESTART
	TUTORIAL_EVENT_MINIGAME_CONTINUE
	TUTORIAL_EVENT_MINIGAME_DEFEAT
	TUTORIAL_EVENT_PLACE_MENU
	TUTORIAL_EVENT_BARRAGE_ENDED
	TUTORIAL_EVENT_BARRAGE_USED
	TUTORIAL_EVENT_COUNT
	
	
	To check if someone pressed the l stick in shell cam for the howitzer use this event
	TutorialEvent.Construct( TUTORIAL_EVENT_INPUT, GAMEPAD_BUTTON_LTHUMB_MINMAG, UNIT_ID_TURRET_ARTY_01, 1 )
	*/
}

	
	
	
	


