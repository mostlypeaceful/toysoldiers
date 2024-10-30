sigvars Atmosphere
@[ParticleColor] { "Particle Color", (0.9, 0.85, 0.7, 0.8), [ 0.0:2.0 ], "RGBA Particle Tint" }
@[Fog_MinDist] { "Fog Min Dist", 75.0, [ 0:1000 ], "Fog starts at this distance (from the camera)" }
@[Fog_FadeDist] { "Fog Fade Dist", 160.70, [ 0:1000 ], "Fog starts at this distance (from the camera)" }
@[Fog_Clamp] { "Fog Min Dist", (0.20,0.60), [ 0.0:1.0 ], "Fog clamp (min, max)" }
@[Fog_Color] { "Fog Color", (0.52,0.51,0.44), [ 0.0:1.0 ], "RGB Fog Tint" }
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
sigimport "gameplay/characters/passenger/usa/passenger_elite.sigml"
sigimport "gameplay/characters/passenger/ussr/passenger_elite.sigml"

sigexport function EntityOnCreate( entity )
{
	RegisterLevelLogic( entity, LevelLogic_Campaign_lvl12( ) )
}

class LevelLogic_Campaign_lvl12 extends GameStandardLevelLogic
{
	wave_01 = null
	wave_02 = null

	constructor( )
	{
		GameStandardLevelLogic.constructor( )
		gDefaultBlueTeamPassengerSigml = "gameplay/characters/passenger/usa/passenger_elite.sigml"
		gDefaultRedTeamPassengerSigml = "gameplay/characters/passenger/ussr/passenger_elite.sigml"
	}
	
	function OnSpawn( )
	{
		::GameStandardLevelLogic.OnSpawn( )
		
		wave_01 = AddWaveList( "Wave01" )
		wave_01.SetLooping( false )	
		wave_02 = AddWaveList( "Wave02" )
		wave_02.SetLooping( false )	

		LevelIntro2P( "nis_intro_campath", "nis_intro_campath_02", null, true, false )

		//undertow: destroy 5 aircraft with the anti-tank iii.
		//the sound: repair 5 times.
		//rank: machine gun iii kills
		InitDecoration( 0, 5, function( event ) { return PlayerKilledTheseUnitsWithOneOfTheseUnits( event, [ UNIT_TYPE_AIR ], [ UNIT_ID_TURRET_AT_03 ] ) }.bindenv( this ) )
		InitDecoration( 1, 5, function( event ) { return event.EventID == TUTORIAL_EVENT_TURRET_REPAIR }.bindenv( this ) )
		InitChallenge( function( event ) { return  }.bindenv( this ) )
	}

	function SpawnPreIntroWaves( )
	{
		wave_01.Activate()
		wave_02.Activate()
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
