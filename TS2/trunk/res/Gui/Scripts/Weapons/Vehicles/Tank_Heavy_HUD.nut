// Heavy tank

// Requires
sigimport "gui/scripts/weapons/gunweapon.nut"

sigexport function CanvasCreateWeaponUI( weaponUI )
{
	return ::HeavyTankWeaponUI( weaponUI )
}

class HeavyTankWeaponUI extends GunWeaponUI
{
	constructor( weaponUI )
	{
		::GunWeaponUI.constructor( weaponUI )
		SetSouthpawSetting( SETTINGS_VEHICLESSOUTHPAW )

		AddControl( GAMEPAD_BUTTON_LTHUMB_MINMAG, "Move" )
		AddControl( GAMEPAD_BUTTON_RTHUMB_MINMAG, "Aim" )
		AddControl( GAMEPAD_BUTTON_LTRIGGER, "FireMachineGuns" )
		AddControl( GAMEPAD_BUTTON_RTRIGGER, "FireCannon" )
	}
}