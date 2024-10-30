
// Requires
sigimport "gui/scripts/weapons/turrets/ac130_base_hud.nut"

// Resources
sigimport "Gui/Textures/Weapons/MachineGun/reticle_circular01_g.png"

sigexport function CanvasCreateWeaponUI( weaponUI )
{
	return AC130_MG_HUD( weaponUI )
}

class AC130_MG_HUD extends AC130_Base_HUD
{
	constructor( weaponUI )
	{
		::AC130_Base_HUD.constructor( weaponUI, "mg", ::Math.Vec3.Construct( 0.47, 0.83, 0.26 ) )
		SetSouthpawSetting( SETTINGS_TURRETSSOUTHPAW )

		AddControl( GAMEPAD_BUTTON_RTHUMB_MINMAG, "Aim" )
		AddControl( GAMEPAD_BUTTON_RTRIGGER, "Fire" )
		AddControl( GAMEPAD_BUTTON_DPAD_DOWN, "AC130_AutogunName" )
	}
}
