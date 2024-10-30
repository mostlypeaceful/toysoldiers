// Mortar 1

// Requires
sigimport "gui/scripts/weapons/gunweapon.nut"

sigexport function CanvasCreateWeaponUI( weaponUI )
{
	return ::Mortar01WeaponScript( weaponUI )
}

class Mortar01WeaponScript extends GunWeaponUI
{
	constructor( weaponUI )
	{
		::GunWeaponUI.constructor( weaponUI, Reticle( ) )
		SetSouthpawSetting( SETTINGS_TURRETSSOUTHPAW )
		SetAltSouthpawSetting( SETTINGS_SHELLCAMSOUTHPAW )

		AddControl( GAMEPAD_BUTTON_LTHUMB_MINMAG, "Adjust_View" )
		AddControl( GAMEPAD_BUTTON_RTHUMB_MINMAG, "Aim" )
		AddControl( GAMEPAD_BUTTON_RTRIGGER, "Fire" )
		AddControl( GAMEPAD_BUTTON_RTRIGGER, "Shell_Cam_Use" )
		
		AddAltControl( GAMEPAD_BUTTON_RTHUMB_MINMAG, "Aim" )
		AddAltControl( GAMEPAD_BUTTON_LTHUMB_MINMAG, "Speed" )
		AddAltControl( GAMEPAD_BUTTON_RTRIGGER, "Shell_Cam_Exit" )
	}	
}
