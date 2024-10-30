sigimport "Gameplay/Mobile/Helicopter/Common/HoverVehicleLogic.nut"
sigimport "Gameplay/Mobile/Helicopter/USSR/ussrhelotransport01_momap.nut"
sigimport "gui/textures/waveicons/ussr/vehicle_helitransport_g.png"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Helo_Transport_02( )
}


class USSR_Helo_Transport_02 extends HoverVehicleLogic
{
	constructor( )
	{
		HoverVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USSR_Helo_Transport_02"

	function OnSpawn( )
	{	
		// The syntax for AddCargo is path, spawnrate, should_stop_while_dropping, remove_cargo_after_dropping
		// Please remove the comment above and make this take a string into the cargo table: AddCargo( "gameplay/characters/infantry/ussr/infantry_paratrooper_01.sigml", 15, 0.5, 1, 0 )

// Needs to change Huey Machine Gun to USSR Hip MG, and Huey Rocket to USSR Hip Rocket
		local gunBank = WeaponStation( 0 ).Bank( 0 )
		gunBank.AddWeapon( "USSR_HIP_MG", "leftgun" )	
		gunBank.AddWeapon( "USSR_HIP_MG", "rightgun" )
		gunBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
			
		local rocketBank = WeaponStation( 0 ).Bank( 1 )
		rocketBank.AddWeapon( "USSR_HIP_ROCKET", "leftrocket" )
		rocketBank.AddWeapon( "USSR_HIP_ROCKET", "rightrocket" )
		rocketBank.TriggerButton = GAMEPAD_BUTTON_LTRIGGER
		
		HoverVehicleLogic.OnSpawn( )
		SetDestroyedEffect( "Helicopter_Explosion" )
	}

	function SetMotionMap( ) 
		Animatable.MotionMap = USSRHeloTransport01MoMap( this )


}
