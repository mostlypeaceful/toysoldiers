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
sigimport "Gui/Textures/WaveIcons/USSR/vehicle_helitransport_g.png"

sigimport "gameplay/mobile/helicopter/ussr/fly_medium_01.sigml"
sigimport "gameplay/mobile/helicopter/ussr/fly_small_01.sigml"
sigimport "gameplay/mobile/helicopter/ussr/fly_large_01.sigml"
sigimport "gameplay/mobile/helicopter/ussr/fly_gold_01.sigml"

sigexport function EntityOnCreate( entity )
{
	RegisterLevelLogic( entity, LevelLogic_Challenge_lvl08_f14attack( ) )
}

class LevelLogic_Challenge_lvl08_f14attack extends GameStandardLevelLogic
{

	wave_01 = null

	
	function OnSpawn( )
	{		
		GoalDriven.MasterGoal = ShootingGalleryMasterGoal( this, { } )
		ConfigureMiniGame( )
		
		::GameStandardLevelLogic.OnSpawn( )
		
		// Initialize wavelist variables
		
		wave_01 = AddWaveList( "WaveList01" )		
	
		wave_01.AssignPickup = PICKUPS_TINY_BATTERY_AMOUNT

		
		LevelIntro2P( "nis_intro_campath", "nis_intro_campath", null, null )
		onLevelIntroComplete = EndOfCamera
		
		timerUI = ::MinigameTimeUI( this, ControllingPlayer( ) )
		
		//::GameApp.ForEachPlayer( function( player, data )
		//{
		//	CommonMiniGamePlayerSetup( player )
		//}.bindenv( this ) , null )
	}

	function SpawnPreIntroWaves( )
	{
		wave_01.Activate( )
	}
	
	function EndOfCamera( )
	{
		HandleTutorialEvent( TUTORIAL_EVENT_BEGIN )
		SetCurrentLevelProgression( LEVEL_PROGRESSION_INTRO )
		local atv = ControllingPlayer( ).LockInUnit( "playerF14" )
		if( !is_null( atv ) )
			atv.RegisterForLevelEvent( LEVEL_EVENT_UNIT_DESTROYED, VehicleMiniGameDestroyed.bindenv( this ) )
	}
	
	function ConfigureMiniGame( )
	{
		miniGameDuration = 0
		miniGameLeaderBoard = GameAppSession.CurrentMiniGameLeaderBoard( )
		miniGameBeginLocKey = "beginMiniGame5"
		miniGameKillFunction = MiniGameKillFunction
		miniGameStartStopFunction = MiniGameStartStopFunction
		TutDisableVehicleInput = 1
		CommonMiniGameSetup( )
	}
	
	function MiniGameKillFunction( context )
	{
		return 1
	}
	

	
	function MiniGameStartStopFunction( start, preStart )
	{
		CommonMiniGameStartStop( start, preStart, [ wave_01 ], 0 )
				
		if( start )
		{
			if( preStart )
			{
				local player = ControllingPlayer( )
				player.PopTiltShiftCamera( )
				wave_01.Activate( )
				if( is_null( player.CurrentUnit ) )
					player.LockInUnit( "playerF14" )
			}
			else
				TutDisableVehicleInput = 0
		}
		else
		{
			TutDisableVehicleInput = 1
		}
		
		CommonMiniGameStartStop( start, preStart, [ wave_01 ], 1 )
		ResetMap( )
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







	



	
	


