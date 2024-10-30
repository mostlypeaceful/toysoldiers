sigvars Atmosphere
@[ParticleColor] { "Particle Color", (1.0, 1.0, 1.0, 1.0), [ 0.0:2.0 ], "RGBA Particle Tint" }
@[Fog_MinDist] { "Fog Min Dist", 120.0, [ 0:1000 ], "Fog starts at this distance (from the camera)" }
@[Fog_FadeDist] { "Fog Fade Dist", 450.0, [ 0:1000 ], "Fog starts at this distance (from the camera)" }
@[Fog_Clamp] { "Fog Min Dist", (0.070, 0.150), [ 0.0:1.0 ], "Fog clamp (min, max)" }
@[Fog_Color] { "Fog Color", (0.930, 0.940, 0.975), [ 0.0:1.0 ], "RGB Fog Tint" }
@[Shadows_Amount] { "Shadows Amount", 0.75, [ 0:1000 ], "" }
@[Shadows_Dist] { "Shadows Dist", 500.0, [ 0:1000 ], "" }
@[Shadows_Near] { "Shadows Near", 0.0, [ 0:1000 ], "" }
@[Shadows_Far] { "Shadows Far", 1000.0, [ 0:2000 ], "" }
@[Shadows_Width] { "Shadows Width", 850.0, [ 0:1000 ], "" }
@[Shadows_Height] { "Shadows Height", 850.0, [ 0:1000 ], "" }
@[Post_Saturation] { "PostFx Saturation", (1.0, 1.0, 1.0), [ 0.0:4.0 ], "" }
@[Post_Contrast] { "PostFx Contrast", (1.0, 1.0, 1.0), [ 0.0:4.0 ], "" }
@[Post_Exposure] { "PostFx Exposure", (1.0, 1.0, 1.0), [ 0.0:4.0 ], "" }


sigimport "Levels/Scripts/Common/game_standard.nut"

sigexport function EntityOnCreate( entity )
{
	RegisterLevelLogic( entity, LevelLogic_Campaign_lvl08( ) )
}

class LevelLogic_Campaign_lvl08 extends GameStandardLevelLogic
{
	wave_01 = null
	wave_02 = null
	wave_03 = null
	wave_04 = null
	
	function OnSpawn( )
	{
		::GameStandardLevelLogic.OnSpawn( )
		
		wave_01 = AddWaveList( "WaveList01" )
		wave_02 = AddWaveList( "BomberWaves" )
		wave_03 = AddWaveList( "AllyWaveList01" )
		wave_04 = AddWaveList( "CinemaWaveList01" )

		LevelIntro2P( "nis_intro_campath", "nis_intro_campath_02", [ wave_02 ], true, false )
		
		//intimidation: overkill 25 infantry with an ifv.
		//necessary: overkill 10 atvs with an artillery turret
		//rank: makeshift i kills
		InitDecoration( 0, 25, function( event ) { return event.CheckFlag( KILL_FLAG_OVERKILL ) && PlayerKilledTheseUnitsWithOneOfTheseUnits( event, [ UNIT_TYPE_INFANTRY ], [ UNIT_ID_APC_IFV_01 ] ) }.bindenv( this ) )
		InitDecoration( 1, 10, function( event ) { return event.CheckFlag( KILL_FLAG_OVERKILL ) && PlayerKilledParticularUnitWithOneOfTheseUnits( event, UNIT_ID_INFANTRY_ATV, [ UNIT_ID_TURRET_ARTY_01, UNIT_ID_TURRET_ARTY_02, UNIT_ID_TURRET_ARTY_03 ] ) }.bindenv( this ) )
		InitChallenge( function( event ) { return PlayerKilledTheseUnitsWithOneOfTheseUnits( event, [ UNIT_TYPE_INFANTRY, UNIT_TYPE_VEHICLE, UNIT_TYPE_AIR ], [ UNIT_ID_TURRET_FLAME_01 ] ) }.bindenv( this ) )
	}

	function SpawnPreIntroWaves( )
	{
		wave_01.Activate( )
		wave_03.Activate( )
		wave_04.Activate( )
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
		PostEffects.SetDepthOfField( Math.Vec4.Construct( 0.994, 0.465, 3.9, 0.555 ) ) // x, y, z, w
	}	
	
	
}
