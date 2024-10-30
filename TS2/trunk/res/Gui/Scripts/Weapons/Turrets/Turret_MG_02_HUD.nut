// Say hello to my little friend!

// Requires
sigimport "Gui/Scripts/weapons/gunweapon.nut"

// Resources
sigimport "Gui/Textures/Weapons/MachineGun/scope_g.png"

sigexport function CanvasCreateWeaponUI( weaponUI )
{
	return USA_Machine_Gun_02_WeaponScript( weaponUI )
}

class USA_Machine_Gun_02_WeaponScript extends GunWeaponUI
{	
	constructor( weaponUI )
	{
		GunWeaponUI.constructor( weaponUI )
		SetScope( "Gui/Textures/Weapons/MachineGun/scope_g.png" )
		SetSouthpawSetting( SETTINGS_TURRETSSOUTHPAW )
		
		AddControl( GAMEPAD_BUTTON_RTHUMB_MINMAG, "Aim" )
		AddControl( GAMEPAD_BUTTON_RTRIGGER, "Fire" )
		AddControl( GAMEPAD_BUTTON_LTRIGGER, "Scope" )
	}
}
