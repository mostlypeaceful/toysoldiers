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
sigimport "levels/maps/headtohead/offensivewaves.nut"
// these are not imported by default and cause the game to crash if missing.
sigimport "gameplay/characters/infantry/ussr/infantry_basic_01.sigml"
sigimport "gameplay/characters/infantry/usa/infantry_basic_01.sigml"
// should not be here - level will not show waves without these loaded.

sigexport function EntityOnCreate( entity )
{
	RegisterLevelLogic( entity, LevelLogic_HeadToHead_lvl05( ) )
}

class LevelLogic_HeadToHead_lvl05 extends GameStandardLevelLogic
{
	// Declare wavelist variables
	// wave_variable_01 = null
	// wave_variable_02 = null
	// ...
	
	wave_01 = null
	wave_02 = null
	
	
	function OnSpawn( )
	{
		::GameStandardLevelLogic.OnSpawn( )
		CommonHTHSetup( )
		
		wave_01 = AddWaveList( "Wave01" )
		wave_02 = AddWaveList( "Wave02" )

		LevelIntro2P( "path_intro_cam_01", "path_intro_cam_02", null, true, false )
		
		AddOffensiveWave( offensiveWave_AtvUsa )		
		AddOffensiveWave( offensiveWave_HeavyTankUsa )
		AddOffensiveWave( offensiveWave_HeloUsa )
		AddOffensiveWaveUSABarrage( )
		AddOffensiveWave( offensiveWave_HeloTransportUsa )
		AddOffensiveWave( offensiveWave_IfvUsa ) 

		
		AddOffensiveWave( offensiveWave_AtvUssr )		
		AddOffensiveWave( offensiveWave_HeavyTankUssr )
		AddOffensiveWave( offensiveWave_HeloUssr )
		AddOffensiveWaveUSSRBarrage( )
		AddOffensiveWave( offensiveWave_HeloTransportUssr )
		AddOffensiveWave( offensiveWave_IfvUssr ) 

		
		
	}

	function SpawnPreIntroWaves( )
	{
		wave_01.Activate( )
		wave_02.Activate( )
		wave_01.FinishReadying( )
		wave_02.FinishReadying( )

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
