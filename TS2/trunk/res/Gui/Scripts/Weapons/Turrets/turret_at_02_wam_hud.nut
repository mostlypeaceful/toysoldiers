// Anti Tank 2

// Requires
sigimport "Gui/Scripts/weapons/gunweapon.nut"

// Resources
sigimport "Gui/Textures/Weapons/MachineGun/reticle_circular01_g.png"

sigexport function CanvasCreateWeaponUI( weaponUI )
{
	return AT02WeaponScript_wam( weaponUI )
}

class AT02WeaponScript_wam extends GunWeaponUI
{
	constructor( weaponUI )
	{
		GunWeaponUI.constructor( weaponUI, Reticle( "Gui/Textures/Weapons/MachineGun/reticle_circular01_g.png" ) )	
		//SetScope( "Gui/Textures/Weapons/MachineGun/scope_g.png" )
		SetSouthpawSetting( SETTINGS_TURRETSSOUTHPAW )
		
		AddControl( GAMEPAD_BUTTON_RTHUMB_MINMAG, "Aim" )
		AddControl( GAMEPAD_BUTTON_RTRIGGER, "Fire" )
		//AddControl( GAMEPAD_BUTTON_LTRIGGER, "Scope" )
	}
}
