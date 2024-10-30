sigvars Atmosphere
@[ParticleColor] { "Particle Color", (0.9, 0.85, 0.7, 0.8), [ 0.0:2.0 ], "RGBA Particle Tint" }
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
	RegisterLevelLogic( entity, LevelLogic_Challenge_lvl04_shootinggallerydelux( ) )
}

class LevelLogic_Challenge_lvl04_shootinggallerydelux extends GameStandardLevelLogic
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
		
		ShowEnemiesAliveList( false )
		ShowWaveListUI( false )
		ControllingPlayer( ).ShowScoreUI( false )
		TutContinuousCombo = 1
		DisableOverkills = 1
		UseMinigameScoring = 1
		TutForceWeaponOvercharge = 1
		
		ControllingPlayer( ).ShowMiniMap( false )
		ControllingPlayer( ).ShowOverchargeMeter( false )
		
		wave_01 = AddWaveList( "WaveList01" )
		wave_01.DisableAIFire = 1
		wave_02 = AddWaveList( "WaveList02" )
		wave_02.DisableAIFire = 1
		wave_03 = AddWaveList( "WaveList03" )
		wave_03.DisableAIFire = 1
		wave_04 = AddWaveList( "WaveList04" )
		wave_04.DisableAIFire = 1
		
		//LevelIntro2P( "introCam", "introCam", null )
		//onLevelIntroComplete = LevelIntroComplete
		
		::GameApp.ForEachPlayer( function( player, data )
		{
			player.LockInUnit( "turret" )
			CommonMiniGamePlayerSetup( player )
		}.bindenv( this ) , null )
		
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
		HandleTutorialEvent( TUTORIAL_EVENT_BEGIN )
		SetCurrentLevelProgression( LEVEL_PROGRESSION_INTRO )
	}
	
	function ConfigureMiniGame( )
	{
		miniGameDuration = 90
		miniGameLeaderBoard = GameAppSession.CurrentMiniGameLeaderBoard( )
		miniGameBeginLocKey = "beginMiniGame4"
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
		CommonMiniGameStartStop( start, preStart, [ wave_01, wave_02, wave_03, wave_04 ], 0 )
		
		if( start )
		{
			if( preStart )
			{				
				wave_01.Activate( )
				wave_02.DisableAIFire = 1
				GameApp.CurrentLevel.SetUIWaveList( "WaveList01" )
				
				local player = ControllingPlayer( )
				local unit = player.CurrentUnit
				if( unit.UnitID != UNIT_ID_TURRET_MG_03 )
					player.CurrentUnit.QuickSwitchTo( UNIT_ID_TURRET_MG_03 )
			}
		}	
		
		CommonMiniGameStartStop( start, preStart, [ wave_01, wave_02, wave_03, wave_04 ], 1 )	
	}
	
	
	//functions for turret progression
	function SwitchTurret( newUnit )
	{
		local unit = ControllingPlayer( ).CurrentUnit
		if( unit.UnitID != newUnit )
		{
			ControllingPlayer( ).CurrentUnit.QuickSwitchTo( newUnit )
			PlaySound( "Play_HUD_Turret_TimeUp" )
		}
	}
	function preAT2( )
	{	
		wave_02.Activate( )
		wave_02.SetHealthModifier( 0.5 )
		GameApp.CurrentLevel.SetUIWaveList( "WaveList02" )
	}
	
	function AT2( )
	{
		wave_01.KillEveryone( )
		wave_01.Pause( )
		SwitchTurret( UNIT_ID_TURRET_AT_02 )
	}
	
	function Mo2( )
	{
		wave_02.Pause( )
		wave_03.Activate( )
		GameApp.CurrentLevel.SetUIWaveList( "WaveList03" )
		SwitchTurret( UNIT_ID_TURRET_MORTAR_02 )
	}

	function preAA1( )
	{
		wave_04.Activate( )
		wave_04.SetHealthModifier( 0.05 )
		GameApp.CurrentLevel.SetUIWaveList( "WaveList04" )
	}
	function AA1( )
	{
		wave_02.KillEveryone( )
		wave_02.Pause( )
		SwitchTurret( UNIT_ID_TURRET_AA_01 )
	}

	function ProcessTutorialEvent( event )
	{
		if( PlayerKilledTheseUnitsWithOneOfTheseUnits( event, [ UNIT_TYPE_INFANTRY ], [ UNIT_ID_TURRET_MG_03 ] ) )
		{
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
		PostEffects.SetDepthOfField( Math.Vec4.Construct( 0.994, 0.465, 3.9, 0.5 ) ) // x, y, z, w
	}
}
