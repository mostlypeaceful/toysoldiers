
// Requires
sigimport "gui/scripts/weapons/turrets/ac130_base_hud.nut"

// Resources
sigimport "Gui/Textures/Weapons/MachineGun/reticle_circular01_g.png"

sigexport function CanvasCreateWeaponUI( weaponUI )
{
	return AC130_AUTOGUN_HUD( weaponUI )
}

class AC130_AUTOGUN_HUD extends AC130_Base_HUD
{
	// Display
	overlay = null
	
	constructor( weaponUI )
	{
		::AC130_Base_HUD.constructor( weaponUI, "autogun", ::Math.Vec3.Construct( 0.57, 0.93, 0.36 ) )
		SetSouthpawSetting( SETTINGS_TURRETSSOUTHPAW )

		AddControl( GAMEPAD_BUTTON_RTHUMB_MINMAG, "Aim" )
		AddControl( GAMEPAD_BUTTON_RTRIGGER, "Fire" )
		AddControl( GAMEPAD_BUTTON_DPAD_DOWN, "AC130_CannonName" )
		AddControl( GAMEPAD_BUTTON_DPAD_UP, "AC130_MgName" )
	}
}
