sigvars Atmosphere
@[ParticleColor] { "Particle Color", (0.90,0.85,0.70,0.80), [ 0.0:2.0 ], "RGBA Particle Tint" }
@[Fog_MinDist] { "Fog Min Dist", 20.00, [ 0:1000 ], "Fog starts at this distance (from the camera)" }
@[Fog_FadeDist] { "Fog Fade Dist", 285.00, [ 0:1000 ], "Fog starts at this distance (from the camera)" }
@[Fog_Clamp] { "Fog Min Dist", (0.20,0.41), [ 0.0:1.0 ], "Fog clamp (min, max)" }
@[Fog_Color] { "Fog Color", (0.57,0.69,0.75), [ 0.0:1.0 ], "RGB Fog Tint" }
@[Shadows_Amount] { "Shadows Amount", 0.60, [ 0:1000 ] }
@[Shadows_Dist] { "Shadows Dist", 500.0, [ 0:1000 ], "" }
@[Shadows_Near] { "Shadows Near", 0.0, [ 0:1000 ], "" }
@[Shadows_Far] { "Shadows Far", 1000.0, [ 0:2000 ], "" }
@[Shadows_Width] { "Shadows Width", 600.0, [ 0:1000 ], "" }
@[Shadows_Height] { "Shadows Height", 600.0, [ 0:1000 ], "" }
@[Post_Saturation] { "PostFx Saturation", (1.0, 1.0, 1.0), [ 0.0:4.0 ], "" }
@[Post_Contrast] { "PostFx Contrast", (1.0, 1.0, 1.0), [ 0.0:4.0 ], "" }
@[Post_Exposure] { "PostFx Exposure", (1.0, 1.0, 1.0), [ 0.0:4.0 ], "" }

sigimport "Levels/Scripts/Common/game_standard.nut"
sigimport "gameplay\characters\passenger\usa\passenger_03.sigml"
sigimport "gui\textures\waveicons\usa\vehicle_atv_g.png"


sigexport function EntityOnCreate( entity )
{
	RegisterLevelLogic( entity, LevelLogic_Campaign_lvl11_dc( ) )
}

class LevelLogic_Campaign_lvl11_dc extends GameStandardLevelLogic
{
	wave_01 = null

	constructor( )
	{
		::GameStandardLevelLogic.constructor( )
		gDefaultBlueTeamPassengerSigml = "gameplay/characters/passenger/usa/passenger_03.sigml"
		gDefaultRedTeamPassengerSigml = "gameplay/characters/passenger/ussr/passenger_01.sigml"
	}

	function OnSpawn( )
	{
		::GameStandardLevelLogic.OnSpawn( )
		
		wave_01 = AddWaveList( "WaveList01" )
		
		//miserable pile of soviets: earn a 100x combo.
		//spared no expense: perform 10 turret upgrades.
		//rank: vehicle kills
		InitDecoration( 0, null, function( event ) { return event.Combo == 100 }.bindenv( this ) )
		InitDecoration( 1, 10, function( event ) { return event.EventID == TUTORIAL_EVENT_TURRET_UPGRADE }.bindenv( this ) )
		InitChallenge( function( event ) { return PlayerKilledTheseUnitsWithOneOfTheseUnits( event, [ UNIT_TYPE_INFANTRY, UNIT_TYPE_VEHICLE, UNIT_TYPE_AIR ], [ UNIT_ID_TANK_MEDIUM_01, UNIT_ID_TANK_HEAVY_01, UNIT_ID_HELO_ATTACK_01, UNIT_ID_HELO_TRANSPORT_01, UNIT_ID_HELO_TRANSPORT_02, UNIT_ID_APC_IFV_01 ] ) }.bindenv( this ) )

		// set do_TestBoss to 'true' to only run boss test wave
		local do_TestBoss = false
		
		if( do_TestBoss )
			AddWaveList( "BossWave" ).Activate( )
		else
			LevelIntro2P( "introCam", "introCam_02", null, true, false )
	}

	function SpawnPreIntroWaves( )
	{
		wave_01.Activate( )
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
		PostEffects.SetDepthOfField( Math.Vec4.Construct( 0.994, 0.465, 2.1, 0.5 ) ) // x, y, z, w
	}
}