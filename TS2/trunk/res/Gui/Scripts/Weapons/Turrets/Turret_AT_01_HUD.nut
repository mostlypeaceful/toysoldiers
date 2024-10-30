// Anti-tank turret

// Requires
sigimport "Gui/Scripts/weapons/gunweapon.nut"

// Resources
sigimport "Gui/Textures/Weapons/MachineGun/reticle_circular01_g.png"

sigexport function CanvasCreateWeaponUI( weaponUI )
{
	return AT01WeaponScript( weaponUI )
}

class AT01WeaponScript extends GunWeaponUI
{
	constructor( weaponUI )
	{
		GunWeaponUI.constructor( weaponUI, Reticle( "Gui/Textures/Weapons/MachineGun/reticle_circular01_g.png" ) )
		SetShellCamReticle( Reticle( ) )
		SetSouthpawSetting( SETTINGS_TURRETSSOUTHPAW )
		SetAltSouthpawSetting( SETTINGS_SHELLCAMSOUTHPAW )

		AddControl( GAMEPAD_BUTTON_RTHUMB_MINMAG, "Aim" )
		AddControl( GAMEPAD_BUTTON_RTRIGGER, "Fire" )
		AddControl( GAMEPAD_BUTTON_RTRIGGER, "Shell_Cam_Use" )
		
		AddAltControl( GAMEPAD_BUTTON_RTHUMB_MINMAG, "Aim" )
		AddAltControl( GAMEPAD_BUTTON_LTHUMB_MINMAG, "Speed" )
		if( ::GameApp.CurrentLevel.MapType != MAP_TYPE_MINIGAME )
			AddAltControl( GAMEPAD_BUTTON_RTRIGGER, "Shell_Cam_Exit" )
	}
}
