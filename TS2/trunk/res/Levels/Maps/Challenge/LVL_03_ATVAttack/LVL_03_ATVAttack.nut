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
sigimport "gui/scripts/hud/minigametimerui.nut"

sigexport function EntityOnCreate( entity )
{
	RegisterLevelLogic( entity, LevelLogic_Challenge_lvl03_atvattack( ) )
}

class LevelLogic_Challenge_lvl03_atvattack extends GameStandardLevelLogic
{

	wave_01 = null
	wave_02 = null
	wave_03 = null
	wave_04 = null
	wave_05 = null
	
	timerUI = null
	
	function OnSpawn( )
	{
		GoalDriven.MasterGoal = ShootingGalleryMasterGoal( this, { } )
		ConfigureMiniGame( )

		::GameStandardLevelLogic.OnSpawn( )		
		
		// Initialize wavelist variables
		
		wave_01 = AddWaveList( "WaveList01" )			
		wave_02 = AddWaveList( "WaveList02" )		
		wave_03 = AddWaveList( "WaveList03" )	
		wave_04 = AddWaveList( "WaveList04" )	
		
		wave_01.AssignPickup = PICKUPS_TINY_BATTERY_AMOUNT
		wave_02.AssignPickup = PICKUPS_TINY_BATTERY_AMOUNT
		wave_03.AssignPickup = PICKUPS_TINY_BATTERY_AMOUNT
		wave_04.AssignPickup = PICKUPS_TINY_BATTERY_AMOUNT

		
		LevelIntro2P( "nis_intro_campath", "nis_intro_campath", null )
		onLevelIntroComplete = EndOfCamera
		
		DisableOverkills = 1
		
		ShowEnemiesAliveList( false )
		ShowWaveListUI( false )
		
		::GameApp.ForEachPlayer( function( player, data )
		{
			CommonMiniGamePlayerSetup( player )
		}.bindenv( this ) , null )
		
		timerUI = ::MinigameTimeUI( this, ControllingPlayer( ) )
	}
	
	function OnDelete( )
	{
		if( timerUI )
			timerUI.DeleteSelf( )
		::GameStandardLevelLogic.OnDelete( )
	}

	function SpawnPreIntroWaves( )
	{
	}

	function ConfigureMiniGame( )
	{
		miniGameDuration = 0
		miniGameLeaderBoard = GameAppSession.CurrentMiniGameLeaderBoard( )
		miniGameBeginLocKey = "beginMiniGame3"
		miniGameKillFunction = MiniGameKillFunction
		miniGameStartStopFunction = MiniGameStartStopFunction
		TutDisableVehicleInput = 1
		CommonMiniGameSetup( )
	}
	
	function EndOfCamera( )
	{
		HandleTutorialEvent( TUTORIAL_EVENT_BEGIN )
		SetCurrentLevelProgression( LEVEL_PROGRESSION_INTRO )
		local atv = ControllingPlayer( ).LockInUnit( "playerATV" )
		if( !is_null( atv ) )
			atv.RegisterForLevelEvent( LEVEL_EVENT_UNIT_DESTROYED, VehicleMiniGameDestroyed.bindenv( this ) )
	}

	function MiniGameKillFunction( context )
	{
		return 1
	}
	
	function ResetMap( )
	{
		local waves = [ wave_01, wave_02, wave_03, wave_04 ]
		
		// kill waves
		for( local i = 0; i < waves.len( ); ++i )
		{
			waves[ i ].KillEveryone( )
			waves[ i ].Pause( )
			waves[ i ].Reset( )
		}
	}
	
	function MiniGameStartStopFunction( start, preStart )
	{
		CommonMiniGameStartStop( start, preStart, [ wave_03 ], 0 )
				
		if( start )
		{
			if( preStart )
			{
				local player = ControllingPlayer( )
				player.PopTiltShiftCamera( )
				wave_03.Activate( )
				if( is_null( player.CurrentUnit ) )
					player.LockInUnit( "playerATV" )
			}
			else
				TutDisableVehicleInput = 0
		}
		else
		{
			TutDisableVehicleInput = 1
			ResetMap( )
		}
		
		CommonMiniGameStartStop( start, preStart, [ wave_03 ], 1 )
	}
	
	function VehicleReachedVolume( proximityLogic )
	{
		local player = GameApp.GetPlayer( 0 )
		if( PlayerInProximity( player, proximityLogic ) )
		{
			local name = proximityLogic.OwnerEntity.GetName( )
			print( "reached volume named: " + name )
			
			if( name == "wave_01_volume" )
				{
					wave_01.Activate( )
					wave_02.Pause( )
					wave_02.KillEveryone( )
					wave_02.Reset( )
					wave_03.Pause( )
					wave_03.KillEveryone( )
					wave_03.Reset( )
				}
			else if( name == "wave_02_volume" )
				{
					wave_02.Activate( )
					wave_04.Pause( )
					wave_04.KillEveryone( )
					wave_04.Reset( )
					wave_03.Pause( )
					wave_03.KillEveryone( )
					wave_03.Reset( )
				}
			else if( name == "wave_03_volume" )
				{
					wave_03.Activate( )
					wave_04.Pause( )
					wave_04.KillEveryone( )
					wave_04.Reset( )
					wave_01.Pause( )
					wave_01.KillEveryone( )
					wave_01.Reset( )
				}
			else if( name == "wave_04_volume" )
				{
					wave_04.Activate( )
					wave_01.Pause( )
					wave_01.KillEveryone( )
					wave_01.Reset( )
					wave_02.Pause( )
					wave_02.KillEveryone( )
					wave_02.Reset( )
				}
		}
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
