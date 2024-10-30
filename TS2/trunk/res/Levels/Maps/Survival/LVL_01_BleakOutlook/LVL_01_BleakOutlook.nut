sigvars Atmosphere
@[ParticleColor] { "Particle Color", (0.9, 0.85, 0.7, 0.8), [ 0.0:2.0 ], "RGBA Particle Tint" }
@[Fog_MinDist] { "Fog Min Dist", 12.00, [ 0:1000 ], "Fog starts at this distance (from the camera)" }
@[Fog_FadeDist] { "Fog Fade Dist", 285.00, [ 0:1000 ], "Fog starts at this distance (from the camera)" }
@[Fog_Clamp] { "Fog Min Dist", (0.20,0.57), [ 0.0:1.0 ], "Fog clamp (min, max)" }
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

sigexport function EntityOnCreate( entity )
{
	RegisterLevelLogic( entity, LevelLogic_Challenge_lvl01( ) )
	
}

class LevelLogic_Challenge_lvl01 extends GameStandardLevelLogic
{
	// Declare wavelist variables
	wave_variable_01 = null
	specialWave = null
	
	constructor( )
	{
		::GameStandardLevelLogic.constructor( )
		
		//GameApp.SetChallengeMode( CHALLENGE_MODE_LOCKDOWN ) //dont do this except for testing
		
		SetupSurvival(  )
	}
		
	function OnSpawn( )
	{
		::GameStandardLevelLogic.OnSpawn( )
		
		SetSurvivalWaves( AddWaveList( "Wave01" ), AddWaveList( "Wave02" ) )
		
		LevelIntro2P( "introCam", "introCam_02", null )
	}

	function SpawnPreIntroWaves( )
	{
		SurvivalStart( )
	}
	
	function OnWaveListFinished( waveList )
	{
		//print( "OnWaveListFinised Reached" )
		//print( waveList.Name( ) )
		//if( waveList.Name( ) == "Wave01" )
		//{
		//	print( "if( waveList.Name == Wave01 ) Reached " )
		//	wave_variable_01.Pause( )
		//	wave_variable_02.Activate( )
		//	wave_variable_02.SetLooping( true )	
		//}
		//if( waveList == wave_variable_01 )
		//{
		//	print( "if( waveList == wave_variable_01 ) Reached " )
		//	wave_variable_01.Pause( )
		//	wave_variable_02.Activate( )
		//	wave_variable_02.SetLooping( true )	
		//}
	}
	
	
	
	// in wavelist script functions
	
	function PlatformPurchased( entity )
    {
        PlatformPrice *= 2.0
    }

	function ProcessTutorialEvent( event )
	{
		//rank: attack helo kills
		if( PlayerKilledTheseUnitsWithOneOfTheseUnits( event, [ UNIT_TYPE_INFANTRY, UNIT_TYPE_VEHICLE, UNIT_TYPE_AIR ], [ UNIT_ID_HELO_ATTACK_01 ] ) )
		{
			IncRankProgress( event.Player, 1 )
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
