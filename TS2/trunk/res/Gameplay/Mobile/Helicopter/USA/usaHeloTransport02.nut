sigimport "Gameplay/Mobile/Helicopter/Common/HoverVehicleLogic.nut"
sigimport "Gameplay/Mobile/Helicopter/USA/usahelotransport01momap.nut"
sigimport "gui/textures/waveicons/usa/vehicle_gunship_g.png"


sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_Helo_Transport_02( )
}


class USA_Helo_Transport_02 extends HoverVehicleLogic
{
	constructor( )
	{
		HoverVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USA_Helo_Transport_02"

	function OnSpawn( )
	{	

		// The syntax for AddCargo is path, spawnrate, should_stop_while_dropping, remove_cargo_after_dropping
		// Please remove the comment above and make this take a string into the cargo table: AddCargo( "gameplay/characters/infantry/usa/infantry_paratrooper_01.sigml", 10, 0.5, 1, 0 )

		local gunBank = WeaponStation( 0 ).Bank( 0 )
		gunBank.AddWeapon( "HUEY_MACHINE_GUN", "leftgun" )	
		gunBank.AddWeapon( "HUEY_MACHINE_GUN", "rightgun" )
		gunBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
			
		local rocketBank = WeaponStation( 0 ).Bank( 1 )
		rocketBank.AddWeapon( "HUEY_ROCKET", "leftrocket" )
		rocketBank.AddWeapon( "HUEY_ROCKET", "rightrocket" )
		rocketBank.TriggerButton = GAMEPAD_BUTTON_LTRIGGER
		//rocketBank.FireMode = FIRE_MODE_ALTERNATE
		
		HoverVehicleLogic.OnSpawn( )
		SetDestroyedEffect( "Helicopter_Explosion" )
	}
	
	function SetMotionMap( ) 
		Animatable.MotionMap = USAHeloTransport01MoMap( this )
}
