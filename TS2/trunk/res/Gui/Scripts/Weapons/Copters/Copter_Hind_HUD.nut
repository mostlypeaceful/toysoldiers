// Script for the Hind HUD

// Requires
sigimport "gui/scripts/weapons/targetingweapon.nut"
sigimport "gui/scripts/weapons/autoaimreticle.nut"

sigexport function CanvasCreateWeaponUI( weaponUI )
{
	return HindWeaponUI( weaponUI )
}

class HindWeaponUI extends TargetingWeaponUI 
{
	constructor( weaponUI )
	{
		::TargetingWeaponUI.constructor( weaponUI, AutoAimReticle( ) )
		SetSouthpawSetting( SETTINGS_VEHICLESSOUTHPAW )

		AddControl( GAMEPAD_BUTTON_LTHUMB_MINMAG, "Move" )
		AddControl( GAMEPAD_BUTTON_RTHUMB_MINMAG, "Aim" )
		AddControl( GAMEPAD_BUTTON_LTRIGGER, "Controls_LaunchMissiles" )
		AddControl( GAMEPAD_BUTTON_LTRIGGER, "LockonMissiles" )
		AddControl( GAMEPAD_BUTTON_RTRIGGER, "FireGun" )
		AddControl( [ GAMEPAD_BUTTON_LSHOULDER, GAMEPAD_BUTTON_RSHOULDER ], "ChangeAltitude" )
		AddControl( GAMEPAD_BUTTON_RTHUMB, "NightVision" )
		
		local alternateCamEffect = weaponUI.ScreenEffects( WEAPON_SCREEN_EFFECT_SPECIAL )
		alternateCamEffect.NightVision = 1
		
		local alternateCam = alternateCamEffect.Data
		alternateCam.Dof = Math.Vec4.Construct( 0.942, 0.383, 1.752, 0.723 )
		alternateCam.Saturation = Math.Vec3.Construct( 0.0, 0.0, 0.0 )
		alternateCam.Contrast = Math.Vec3.Construct( 3.0, 3.0, 3.0 )
		alternateCam.Exposure = Math.Vec3.Construct( 1.35, 2.05, 1.11 )
		alternateCam.UnitTint = Math.Vec4.Construct( 10, 1.0, 1.0, 1.0 )
	}
}
