sigvars Atmosphere
@[ParticleColor] { "Particle Color", (0.9, 0.85, 0.7, 0.8), [ 0.0:2.0 ], "RGBA Particle Tint" }
@[Fog_MinDist] { "Fog Min Dist", 50.00, [ 0:1000 ], "Fog starts at this distance (from the camera)" }
@[Fog_FadeDist] { "Fog Fade Dist", 500.00, [ 0:1000 ], "Fog starts at this distance (from the camera)" }
@[Fog_Clamp] { "Fog Min Dist", (0.00,0.40), [ 0.0:1.0 ], "Fog clamp (min, max)" }
@[Fog_Color] { "Fog Color", (0.48,0.57,0.77), [ 0.0:1.0 ], "RGB Fog Tint" }
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
sigimport "gameplay/characters/passenger/usa/passenger_elite.sigml"
sigimport "gameplay/characters/passenger/ussr/passenger_elite.sigml"


sigexport function EntityOnCreate( entity )
{
	RegisterLevelLogic( entity, LevelLogic_Campaign_lvl14( ) )
}

class LevelLogic_Campaign_lvl14 extends GameStandardLevelLogic
{
	wave_01 = null
	wave_02 = null
	playerHeliWave = null
	baseCleared = null

	constructor( )
	{
		::GameStandardLevelLogic.constructor( )
		gDefaultBlueTeamPassengerSigml = "gameplay/characters/passenger/usa/passenger_elite.sigml"
		gDefaultRedTeamPassengerSigml = "gameplay/characters/passenger/ussr/passenger_elite.sigml"
		
		baseCleared = 0
	}

	function OnSpawn( )
	{
		::GameStandardLevelLogic.OnSpawn( )
		
		wave_01 = AddWaveList( "WaveList01" )
		wave_01.SetLooping( false )	
		wave_02 = AddWaveList( "WaveList02" )
		wave_02.SetLooping( true )	
		
		playerHeliWave = AddWaveList( "HeloWave" )
		playerHeliWave.SetMakeSelectableUnits( 1 )
		playerHeliWave.Saveable = 0
		playerHeliWave.DisableAIFire = 1
		
		LevelIntro2P( "nis_intro_campath", "nis_intro_campath_02", null, true, false )
		
		//danger close: call in a nuke.
		//hipster: destroy 4 helo gunships with an attack helo.
		//rank: artillery iii kills
		InitDecoration( 0, null, function( event ) { return event.EventID == TUTORIAL_EVENT_BARRAGE_ACTIVATED && event.WeaponID == "BARRAGE_NUKE" }.bindenv( this ) )
		InitDecoration( 1, 4, function( event ) { return PlayerKilledParticularUnitWithOneOfTheseUnits( event, UNIT_ID_HELO_TRANSPORT_02, [ UNIT_ID_HELO_ATTACK_01 ] ) }.bindenv( this ) )
		InitChallenge( function( event ) { return PlayerKilledTheseUnitsWithOneOfTheseUnits( event, [ UNIT_TYPE_INFANTRY, UNIT_TYPE_VEHICLE, UNIT_TYPE_AIR ], [ UNIT_ID_TURRET_ARTY_03 ] ) }.bindenv( this ) )
	}

	function SpawnPreIntroWaves( )
	{
		wave_01.Activate()
	}

	function OnRegisterSpecialLevelObject( specialLevelObject )
	{
		specialLevelObject.Logic.RegisterForLevelEvent( LEVEL_EVENT_UNIT_DESTROYED, OnBaseCleared.bindenv( this ) )
		
		// be sure to always call the base
		::GameStandardLevelLogic.OnRegisterSpecialLevelObject( specialLevelObject )
	}	

	function OnBaseCleared( entity )
	{
		baseCleared++
		
		if( baseCleared >= 3 )
		{		
			PopupMessage( "Rushmore_Reinforce", ::Math.Vec4.Construct( 1.0, 1.0, 1.0, 1.0 ), FONT_FANCY_MED )
			playerHeliWave.Activate( )	
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
