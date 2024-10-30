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
	RegisterLevelLogic( entity, LevelLogic_Campaign_lvl13( ) )
}

class LevelLogic_Campaign_lvl13 extends GameStandardLevelLogic
{
	wave01 = null
	wave02 = null
	
	function OnSpawn( )
	{
		// Introduce the idea of rotating artillery, so in case people missed it, it's introduced here.
		::GameStandardLevelLogic.OnSpawn( )

		wave01 = AddWaveList( "WaveList01" )
		wave01.SetLooping( false )
		wave02 = AddWaveList( "WaveList01a" )
		wave02.SetLooping( false )

		LevelIntro2P( "nis_intro_campath", "nis_intro_campath_02", null, true, false )
		
		//oomph!: earn a speed bonus against 15 infantry.
		//helpful: earn 50 assists
		//rank: attack helicopter kills
		InitDecoration( 0, 15, function( event ) { return PlayerSpeedBonusWithOneOfTheseWeapons( event, UNIT_TYPE_INFANTRY, -1 ) }.bindenv( this ) )
		InitDecoration( 1, 50, function( event ) { return event.EventID == TUTORIAL_EVENT_UNIT_DESTROYED && event.CheckFlag( KILL_FLAG_ASSIST ) }.bindenv( this ) )
		InitChallenge( function( event ) { return PlayerKilledTheseUnitsWithOneOfTheseUnits( event, [ UNIT_TYPE_INFANTRY, UNIT_TYPE_VEHICLE, UNIT_TYPE_AIR ], [ UNIT_ID_HELO_ATTACK_01 ] ) }.bindenv( this ) )
	}
	
	function SpawnPreIntroWaves( )
	{
		wave01.Activate()
		wave02.Activate()
	}

	function ArtilleryTurningTutorial( )
	{
		PopupMessage( "ArtilleryTurningTutorial", ::Math.Vec4.Construct( 1.0, 1.0, 1.0, 1 ), FONT_FANCY_MED, null, ::Math.Vec3.Construct(0,-50,0) )		
	}

	function ArtilleryCameraTutorial( )
	{
		PopupMessage( "ArtilleryCameraTutorial", ::Math.Vec4.Construct( 1.0, 1.0, 1.0, 1 ), FONT_FANCY_MED, null, ::Math.Vec3.Construct(0,-50,0) )		
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
