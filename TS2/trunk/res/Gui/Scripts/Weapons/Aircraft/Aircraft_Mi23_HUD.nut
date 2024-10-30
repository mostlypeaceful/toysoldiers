// Script for the Mig 23

// Requires
sigimport "gui/scripts/weapons/targetingweapon.nut"
sigimport "gui/scripts/weapons/autoaimreticle.nut"

sigexport function CanvasCreateWeaponUI( weaponUI )
{
	return Mi23WeaponUI( weaponUI )
}

class Mi23WeaponUI extends TargetingWeaponUI
{
	constructor( weaponUI )
	{
		::TargetingWeaponUI.constructor( weaponUI, AutoAimReticle( ) )
		SetSouthpawSetting( SETTINGS_PLANESSOUTHPAW )

		AddControl( GAMEPAD_BUTTON_LTRIGGER, "LockonMissiles" )
		AddControl( GAMEPAD_BUTTON_RTRIGGER, "FireVulcan" )
		AddControl( GAMEPAD_BUTTON_LTHUMB_MINMAG, "Speed" )
		AddControl( [ GAMEPAD_BUTTON_LSHOULDER, GAMEPAD_BUTTON_RSHOULDER ], "Immelmann", true )
		AddControl( [ GAMEPAD_BUTTON_LSHOULDER, GAMEPAD_BUTTON_RSHOULDER ], "Roll" )
		AddControl( GAMEPAD_BUTTON_LTHUMB, "Bombing" )
		AddControl( GAMEPAD_BUTTON_A, "Boost" )
		
		AddAltControl( GAMEPAD_BUTTON_LTRIGGER, "LockonMissiles" )
		AddAltControl( GAMEPAD_BUTTON_RTRIGGER, "DropNapalm" )
		AddAltControl( GAMEPAD_BUTTON_LTHUMB_MINMAG, "Speed" )
		AddAltControl( GAMEPAD_BUTTON_LTHUMB, "Exit_Bombing" )
	}
}
