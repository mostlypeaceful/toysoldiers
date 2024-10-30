// Kaboom!

// Requires
sigimport "gui/scripts/weapons/gunweapon.nut"

sigexport function CanvasCreateWeaponUI( weaponUI )
{
	return Howitzer01WeaponScript( weaponUI )
}

class Howitzer01WeaponScript extends GunWeaponUI
{
	constructor( weaponUI )
	{
		GunWeaponUI.constructor( weaponUI, Reticle( ) )
		SetSouthpawSetting( SETTINGS_TURRETSSOUTHPAW )
		SetAltSouthpawSetting( SETTINGS_SHELLCAMSOUTHPAW )

		AddControl( GAMEPAD_BUTTON_LTHUMB_MINMAG, "Adjust_View" )
		AddControl( GAMEPAD_BUTTON_RTHUMB_MINMAG, "Aim" )
		AddControl( GAMEPAD_BUTTON_RTRIGGER, "Fire" )
		AddControl( GAMEPAD_BUTTON_RTRIGGER, "Shell_Cam_Use" )
		AddControl( [ GAMEPAD_BUTTON_LSHOULDER, GAMEPAD_BUTTON_RSHOULDER ], "Turn" )
		
		AddAltControl( GAMEPAD_BUTTON_RTHUMB_MINMAG, "Aim" )
		AddAltControl( GAMEPAD_BUTTON_LTHUMB_MINMAG, "Speed" )
		AddAltControl( GAMEPAD_BUTTON_RTRIGGER, "Shell_Cam_Exit" )
	}	
}
