// IFV Weapon HUD

// Requires
sigimport "gui/scripts/weapons/gunweapon.nut"
sigimport "gui/scripts/weapons/turrets/turret_at_03_hud.nut"


sigexport function CanvasCreateWeaponUI( weaponUI )
{
	return ::IFVWeaponUI( weaponUI )
}

class IFVWeaponUI extends GunWeaponUI
{
	constructor( weaponUI )
	{
		::GunWeaponUI.constructor( weaponUI )
		SetSouthpawSetting( SETTINGS_VEHICLESSOUTHPAW )
		SetAltSouthpawSetting( SETTINGS_SHELLCAMSOUTHPAW )

		AddControl( GAMEPAD_BUTTON_LTHUMB_MINMAG, "Move" )
		AddControl( GAMEPAD_BUTTON_RTHUMB_MINMAG, "Aim" )
		AddControl( GAMEPAD_BUTTON_RTRIGGER, "Fire" )
		AddControl( GAMEPAD_BUTTON_LTRIGGER, "Controls_LaunchMissiles" )
		AddControl( GAMEPAD_BUTTON_LTRIGGER, "Fly_By_Wire" )
		
		AddAltControl( GAMEPAD_BUTTON_RTHUMB_MINMAG, "Aim" )
		AddAltControl( GAMEPAD_BUTTON_LTHUMB_MINMAG, "Speed" )
		AddAltControl( GAMEPAD_BUTTON_LTRIGGER, "Shell_Cam_Exit" )
		
		local alternateCamEffect = weaponUI.ScreenEffects( WEAPON_SCREEN_EFFECT_SHELLCAM )
		local alternateCam = alternateCamEffect.Data
		
		alternateCam.UnitTint = ::Math.Vec4.Construct( 10, 1.0, 1.0, 1.0 )
		
		local filmGrain = alternateCam.FilmGrainOverride
		filmGrain.TextureKey = "video_noise"
		filmGrain.Exposure = ::Math.Vec3.Construct( 1.2, 1.2, 1.2 )
		filmGrain.Saturation = ::Math.Vec3.Construct( 0.0, 0.0, 0.0 )
		filmGrain.GrainFreq = 0
		filmGrain.GrainScale = 0.65
		filmGrain.GrainSpeed = ::Math.Vec2.Construct( 3.5, 0.6 )
		filmGrain.HairsFreq = 0
		filmGrain.HairsScale = -0.85
		filmGrain.HairsSpeed = ::Math.Vec2.Construct( -4.0, 12.5 )
		filmGrain.LinesFreq = 0.0
		filmGrain.LinesScale = 0.4
		filmGrain.LinesSpeed = ::Math.Vec2.Construct( -1.1, -6.7 )
		filmGrain.SmudgeFreq = 1.0
		filmGrain.SmudgeScale = 0.55
		filmGrain.SmudgeSpeed = ::Math.Vec2.Construct( 20.0, 3.0 )

		AddShellCamCanvas( ::FlyByWireOverlay( ) )
		
		// Holy hacks, Batman!
		local ret = AutoAimReticle( )
		ret.size = ::Math.Vec2.Construct( 514, 376 )
		ret.autoAimBox.SetAlpha( 0 )
		ret.RemoveChild( ret.image )
		ret.image = ::AT03_JitteryReticle( )
		ret.image.CenterPivot( )
		ret.image.SetPosition( ::Math.Vec3.Construct( 0, 0, 0 ) )
		ret.AddChild( ret.image )
		SetShellCamReticle( ret )
	}
	
	function TargetingBoxSize( )
	{
		if( shellCamActive )
			return ::Math.Vec2.Construct( 1280, 720 )
		else
			return ::GunWeaponUI.TargetingBoxSize( )
	}
	
	function TargetLockSize( )
	{
		if( shellCamActive )
			return ::Math.Vec2.Construct( 514, 376 )
		else
			return ::GunWeaponUI.TargetingBoxSize( )
	}
	
	function AutoAimBoxSize( ) 
	{ 
		if( shellCamActive )
			return ::Math.Vec2.Construct( 514, 376 )
		else
			return ::GunWeaponUI.TargetingBoxSize( )
	}
	
	function ReticleSize( )
	{
		if( shellCamActive )
			return ::Math.Vec2.Construct( 256, 256 )
		else
			return ::GunWeaponUI.TargetingBoxSize( )
	}
}