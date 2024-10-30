sigimport "Gameplay/Mobile/Helicopter/Common/HoverVehicleLogic.nut"
sigimport "Gameplay/Mobile/Helicopter/USA/usaheloattack01momap.nut"
sigimport "Gameplay/Turrets/Common/CrewmanMoMap.nut"
sigimport "gui/textures/waveicons/usa/vehicle_heligunner_g.png"



sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_Helo_Attack_01( )
}


class USA_Helo_Attack_01 extends HoverVehicleLogic
{
	constructor( )
	{
		HoverVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USA_Helo_Attack_01"

	function OnSpawn( )
	{
		WeaponStation( 0 ).ToggleCameraButton = GAMEPAD_BUTTON_RTHUMB
		
		local gunBank = WeaponStation( 0 ).Bank( 0 )
		gunBank.AddWeapon( "APACHE_MACHINE_GUN", "centergun" )
		gunBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
		
		local rocketBank = WeaponStation( 0 ).Bank( 1 )
		rocketBank.AddWeapon( "APACHE_HELL_FIRE", "rockets" )
		rocketBank.FireMode = FIRE_MODE_ALTERNATE
		rocketBank.TriggerButton = GAMEPAD_BUTTON_LTRIGGER
		
		local sideWindBank = WeaponStation( 0 ).Bank( 2 )
		sideWindBank.AddWeapon( "APACHE_STINGER", "wing1" )
		sideWindBank.AddWeapon( "APACHE_STINGER", "wing2" )
		sideWindBank.FireMode = FIRE_MODE_ALTERNATE
		sideWindBank.TriggerButton = GAMEPAD_BUTTON_LTRIGGER
		
		HoverVehicleLogic.OnSpawn( )
		SetDestroyedEffect( "Helicopter_Explosion" )
	}
	
	function SetMotionMap( ) 
		Animatable.MotionMap = USAHeloAttack01MoMap( this )
}

