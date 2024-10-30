sigvars Atmosphere
@[ParticleColor] { "Particle Color", (0.9, 0.85, 0.7, 0.8), [ 0.0:2.0 ], "RGBA Particle Tint" }
@[Fog_MinDist] { "Fog Min Dist", 0, [ 0:1000 ], "Fog starts at this distance (from the camera)" }
@[Fog_FadeDist] { "Fog Fade Dist", 362, [ 0:1000 ], "Fog starts at this distance (from the camera)" }
@[Fog_Clamp] { "Fog Min Dist", (0.20,1.0), [ 0.0:1.0 ], "Fog clamp (min, max)" }
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
sigimport "gui/scripts/frontend/frontend.nut"
sigimport "levels/scripts/common/game_standard.nut"

sigexport function EntityOnCreate( entity )
{
	local levelLogic = FrontEndLogic( )
	entity.Logic = levelLogic
	GameApp.CurrentLevel = levelLogic
}


class FrontEndLogic extends GameStandardLevelLogic
{
	wave01 = null
	UnslowDown = null

	function OnSpawn( )
	{
		::GameStandardLevelLogic.OnSpawn( )
		if( !::GameApp.HasInvite )
			SetRootMenu( FrontEndRootMenu( FrontEndRootMenuStartMode.Normal ) )
		else
			SetRootMenu( FrontEndRootMenu( FrontEndRootMenuStartMode.FromInvite ) )
		InitCamera( )
		InitWaves( )
	}
	function InitCamera( )
	{
		local cameraPointCount = CameraPointCount( )
		if( cameraPointCount > 0 )
		{
			local cameraPoint = GetCameraPoint( SubjectiveRand.Int( 0, cameraPointCount - 1 ) )
			GameApp.FrontEndPlayer.ClearCameraStack( )
			GameApp.FrontEndPlayer.PushFrontEndCamera( cameraPoint )
		}
	}
	
	function InitWaves( )
	{
		wave01 = AddWaveList( "WaveList01" )
		wave01.Activate( )
		wave01.SetLooping( true )
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

