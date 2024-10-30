// Huey (Lewis and the News)

// Requires
sigimport "Gui/Scripts/weapons/gunweapon.nut"

sigexport function CanvasCreateWeaponUI( weaponUI )
{
	return HueyWeaponHUD( weaponUI )
}

class HueyWeaponHUD extends GunWeaponUI
{
	constructor( weaponUI )
	{
		::GunWeaponUI.constructor( weaponUI )
		SetSouthpawSetting( SETTINGS_VEHICLESSOUTHPAW )

		AddControl( GAMEPAD_BUTTON_LTHUMB_MINMAG, "Move" )
		AddControl( GAMEPAD_BUTTON_RTHUMB_MINMAG, "Aim" )
		AddControl( GAMEPAD_BUTTON_LTRIGGER, "FireRockets" )
		AddControl( GAMEPAD_BUTTON_RTRIGGER, "FireGuns" )
		AddControl( [ GAMEPAD_BUTTON_LSHOULDER, GAMEPAD_BUTTON_RSHOULDER ], "ChangeAltitude" )
	}
}
