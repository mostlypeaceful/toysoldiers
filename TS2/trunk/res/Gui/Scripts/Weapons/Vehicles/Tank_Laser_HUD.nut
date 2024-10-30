// Medium rare tank. Mmmm

// Requires
sigimport "Gui/Scripts/weapons/gunweapon.nut"

// Resources
sigimport "Gui/Textures/Weapons/MachineGun/reticle_oval_w_ammo_g.png"

sigexport function CanvasCreateWeaponUI( weaponUI )
{
	return ::LaserTankWeaponScript( weaponUI )
}

class LaserTankWeaponScript extends GunWeaponUI
{	
	constructor( weaponUI )
	{
		::GunWeaponUI.constructor( weaponUI, Reticle( "Gui/Textures/Weapons/MachineGun/reticle_oval_w_ammo_g.png" ) )
		SetSouthpawSetting( SETTINGS_VEHICLESSOUTHPAW )
		
		AddControl( GAMEPAD_BUTTON_LTHUMB_MINMAG, "Move" )
		AddControl( GAMEPAD_BUTTON_RTHUMB_MINMAG, "Aim" )
		AddControl( GAMEPAD_BUTTON_RTRIGGER, "FireLaser" )
	}
}
