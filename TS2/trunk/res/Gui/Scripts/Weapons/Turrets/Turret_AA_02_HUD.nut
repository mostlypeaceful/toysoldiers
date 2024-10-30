// Missiles!

// Required
sigimport "gui/scripts/weapons/targetingweapon.nut"

// Resources
sigimport "gui/textures/weapons/machinegun/reticle_large_square_g.png"

sigexport function CanvasCreateWeaponUI( weaponUI )
{
	return AA02WeaponUI( weaponUI )
}

class AA02WeaponUI extends TargetingWeaponUI
{
	constructor( weaponUI )
	{
		::TargetingWeaponUI.constructor( weaponUI, Reticle( "gui/textures/weapons/machinegun/reticle_large_square_g.png" ) )
		SetSouthpawSetting( SETTINGS_TURRETSSOUTHPAW )

		AddControl( GAMEPAD_BUTTON_RTHUMB_MINMAG, "Aim" )
		AddControl( GAMEPAD_BUTTON_RTRIGGER, "Controls_LaunchMissiles" )
		AddControl( GAMEPAD_BUTTON_RTRIGGER, "LockonMissiles" )
	}
}
