sigvars Atmosphere
@[ParticleColor] { "Particle Color", (1.0, 1.0, 1.0, 0.9), [ 0.0:2.0 ], "RGBA Particle Tint" }
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

sigimport "Levels/Scripts/Common/game_standard.nut"

sigexport function EntityOnCreate( entity )
{
	RegisterLevelLogic( entity, LevelLogic_Challenge_lvl07_flyswatter( ) )
}

class LevelLogic_Challenge_lvl07_flyswatter extends GameStandardLevelLogic
{
	wave_01 = null
	wave_02 = null
	wave_03 = null
	wave_04 = null
	minigameCombo = null
	
	function OnSpawn( )
	{		
		GoalDriven.MasterGoal = ShootingGalleryMasterGoal( this, { } )
		ConfigureMiniGame( )
		
		::GameStandardLevelLogic.OnSpawn( )
		
		//turn off un-needed HUD elements
		ShowEnemiesAliveList( false )
		ShowWaveListUI( false )
		ControllingPlayer( ).ShowScoreUI( false )
		TutContinuousCombo = 1
		UseMinigameScoring = 1
		TutForceWeaponOvercharge = 1
		
		wave_01 = AddWaveList( "WaveList01" )
		
		//LevelIntro2P( "introCam", "introCam", null )
		//onLevelIntroComplete = LevelIntroComplete
		LevelIntroComplete( )
		
		minigameCombo = ::MinigameComboUI( ControllingPlayer( ) )
	}
	
	function OnDelete( )
	{
		if( minigameCombo )
			minigameCombo.DeleteSelf( )
		::GameStandardLevelLogic.OnDelete( )
	}

	function SpawnPreIntroWaves( )
	{
		// waves activated in minigame start stop function
		//wave_01.Activate( )
	}
	
	function LevelIntroComplete( )
	{
		::GameApp.ForEachPlayer( function( player, data )
		{
			player.LockInUnit( "turret" )
			CommonMiniGamePlayerSetup( player )
		}.bindenv( this ) , null )
		
		HandleTutorialEvent( TUTORIAL_EVENT_BEGIN )
		SetCurrentLevelProgression( LEVEL_PROGRESSION_INTRO )
	}
	
	function ConfigureMiniGame( )
	{
		miniGameDuration = 60
		miniGameLeaderBoard = GameAppSession.CurrentMiniGameLeaderBoard( )
		miniGameBeginLocKey = "beginMiniGame11"
		miniGameKillFunction = MiniGameKillFunction
		miniGameStartStopFunction = MiniGameStartStopFunction
		CommonMiniGameSetup( )
	}
	
	function MiniGameKillFunction( context )
	{
		return ControllingPlayer( ).LastKillValue
	}
	
	function MiniGameStartStopFunction( start, preStart )
	{
		CommonMiniGameStartStop( start, preStart, [ wave_01 ], 0 )
		
		if( start && preStart )
		{
			wave_01.Activate( )
			wave_01.SetHealthModifier( 0.1 )
			GameApp.CurrentLevel.SetUIWaveList( "WaveList01" )
			ShowWaveListUI( false )
		}
		
		CommonMiniGameStartStop( start, preStart, [ wave_01 ], 1 )
	}
	
	function ProcessTutorialEvent( event )
	{
		if( PlayerKilledParticularUnitWithOneOfTheseUnits( event, UNIT_ID_FLY_GOLD_01, [ UNIT_ID_TURRET_AA_01 ] ) )
		{
			local player = event.Player
			local entity = event.Entity
			player.SpawnLocText( ::GameApp.LocString( "Golden_Fly_Kill" ), entity.GetPosition( ), ::Math.Vec4.Construct( 1.0, 0.8, 0.0, 1.0 ), 1.0 )
			player.Stats.IncStat( SESSION_STATS_GOLDEN_FLIES_EXPLODED, 1.0 )
			player.Stats.IncStat( SESSION_STATS_GOLDEN_FLIES_BONUS, 1 )
			IncRankProgress( event.Player, 1 )
		}
		
		if( minigameCombo )
			minigameCombo.ProcessTutorialEvent( event )
	}
	
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
		PostEffects.SetDepthOfField( Math.Vec4.Construct( 0.998, 0.465, 3.9, 0.5 ) ) // x, y, z, w
	}
}
