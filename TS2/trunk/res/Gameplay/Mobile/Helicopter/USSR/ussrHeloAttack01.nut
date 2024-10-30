sigimport "Gameplay/Mobile/Helicopter/Common/HoverVehicleLogic.nut"
sigimport "Gameplay/Mobile/Helicopter/USSR/ussrheloattack01momap.nut"
sigimport "gui/textures/waveicons/ussr/vehicle_heligunner_g.png"


sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Helo_Attack_01( )
}


class USSR_Helo_Attack_01 extends HoverVehicleLogic
{
	constructor( )
	{
		HoverVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USSR_Helo_Attack_01"

	function OnSpawn( )
	{		
		// The syntax for AddCargo is path, spawnrate, should_stop_while_dropping, remove_cargo_after_dropping
		// Please remove the comment above and make this take a string into the cargo table: AddCargo( "gameplay/characters/infantry/ussr/infantry_paratrooper_01.sigml", 8, 0.5, 1, 0 )
		// Please remove the comment above and make this take a string into the cargo table: AddCargo( "gameplay/mobile/vehicle/ussr/apc_aa_01.sigml", 3, 1.5, 1, 0 )
	
		WeaponStation( 0 ).ToggleCameraButton = GAMEPAD_BUTTON_RTHUMB
	
// Needs to change Hell_Fire to USSR HIND Missile, Apache_machine_gun to USSR HIND MG, and Apache_Stinger to USSR Hind Stinger
		local gunBank = WeaponStation( 0 ).Bank( 0 )
		gunBank.AddWeapon( "USSR_HIND_MG", "centergun" )
		gunBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER		
		
		local rocketBank = WeaponStation( 0 ).Bank( 1 )
		rocketBank.AddWeapon( "USSR_HIND_MISSILE", "rockets" )
		rocketBank.FireMode = FIRE_MODE_ALTERNATE
		rocketBank.TriggerButton = GAMEPAD_BUTTON_LTRIGGER
		
		local sideWindBank = WeaponStation( 0 ).Bank( 2 )
		sideWindBank.AddWeapon( "USSR_HIND_STINGER", "wing" )
		sideWindBank.FireMode = FIRE_MODE_ALTERNATE
		sideWindBank.TriggerButton = GAMEPAD_BUTTON_LTRIGGER
		
		HoverVehicleLogic.OnSpawn( )
		SetDestroyedEffect( "Helicopter_Explosion" )
	}
	
	function SetMotionMap( ) 
		Animatable.MotionMap = USSRHeloAttack01MoMap( this )
}
