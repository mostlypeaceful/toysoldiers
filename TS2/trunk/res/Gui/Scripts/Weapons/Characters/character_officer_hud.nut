// Officer Rambo reporting for duty!

// Requires
sigimport "gui/scripts/weapons/gunweapon.nut"

sigexport function CanvasCreateWeaponUI( weaponUI )
{
	return Officer_WeaponUI( weaponUI )
}

class Officer_WeaponUI extends GunWeaponUI
{
	constructor( weaponUI )
	{
		::GunWeaponUI.constructor( weaponUI )
		SetSouthpawSetting( SETTINGS_TURRETSSOUTHPAW )

		AddControl( GAMEPAD_BUTTON_LTHUMB_MINMAG, "Move" )
		AddControl( GAMEPAD_BUTTON_LTHUMB, "Sprint" )
		AddControl( GAMEPAD_BUTTON_RTHUMB_MINMAG, "Aim" )
		AddControl( GAMEPAD_BUTTON_RTRIGGER, "FireGun" )
		AddControl( GAMEPAD_BUTTON_LTRIGGER, "FireRockets" )
		AddControl( GAMEPAD_BUTTON_A, "Jump" )
	}
}