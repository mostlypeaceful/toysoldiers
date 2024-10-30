// Browning seriously kicked some ass.

// Requires
sigimport "Gui/Scripts/weapons/gunweapon.nut"

sigexport function CanvasCreateWeaponUI( weaponUI )
{
	return ::USA_Machine_Gun_01_WeaponScript( weaponUI )
}

class USA_Machine_Gun_01_WeaponScript extends GunWeaponUI
{	
	constructor( weaponUI )
	{
		::GunWeaponUI.constructor( weaponUI )
		SetSouthpawSetting( SETTINGS_TURRETSSOUTHPAW )
		
		AddControl( GAMEPAD_BUTTON_RTHUMB_MINMAG, "Aim" )
		AddControl( GAMEPAD_BUTTON_RTRIGGER, "Fire" )
		AddControl( GAMEPAD_BUTTON_LTRIGGER, "Zoom" )
	}
}
