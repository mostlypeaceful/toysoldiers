// Flaming turret

// Requires
sigimport "gui/scripts/weapons/gunweapon.nut"

// Resources
sigimport "Gui/Textures/Weapons/MachineGun/reticle_oval02_g.png"

sigexport function CanvasCreateWeaponUI( weaponUI )
{
	return ::Flame01WeaponScript( weaponUI )
}

class Flame01WeaponScript extends GunWeaponUI
{
	constructor( weaponUI )
	{
		::GunWeaponUI.constructor( weaponUI, Reticle( "Gui/Textures/Weapons/MachineGun/reticle_oval02_g.png" ) )
		SetSouthpawSetting( SETTINGS_TURRETSSOUTHPAW )
		
		AddControl( GAMEPAD_BUTTON_RTHUMB_MINMAG, "Aim" )
		AddControl( GAMEPAD_BUTTON_RTRIGGER, "Fire" )
	}	
}
