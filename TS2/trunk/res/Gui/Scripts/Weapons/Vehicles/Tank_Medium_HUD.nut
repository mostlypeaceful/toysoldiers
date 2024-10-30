// Medium rare tank. Mmmm

// Requires
sigimport "Gui/Scripts/weapons/gunweapon.nut"

// Resources
sigimport "Gui/Textures/Weapons/MachineGun/reticle_oval02_g.png"

sigexport function CanvasCreateWeaponUI( weaponUI )
{
	return ::MedTankWeaponScript( weaponUI )
}

class MedTankWeaponScript extends GunWeaponUI
{	
	constructor( weaponUI )
	{
		::GunWeaponUI.constructor( weaponUI, Reticle( "Gui/Textures/Weapons/MachineGun/reticle_oval02_g.png" ) )
		SetSouthpawSetting( SETTINGS_VEHICLESSOUTHPAW )
		
		AddControl( GAMEPAD_BUTTON_LTHUMB_MINMAG, "Move" )
		AddControl( GAMEPAD_BUTTON_RTHUMB_MINMAG, "Aim" )
		AddControl( GAMEPAD_BUTTON_LTRIGGER, "Flamethrower" )
		AddControl( GAMEPAD_BUTTON_RTRIGGER, "FireCannon" )
	}
}

