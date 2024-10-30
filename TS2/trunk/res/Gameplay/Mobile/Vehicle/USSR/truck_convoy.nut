sigimport "Gameplay/mobile/common/WheeledMobileLogic.nut"
sigimport "Gameplay/mobile/common/MobileTurretLogic.nut"
sigimport "gui/textures/waveicons/ussr/vehicle_car_g.png"
sigimport "Anims/Vehicles/Blue/tank_m60patton_lilgun/tank_m60patton_lilgun.anipk"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Convoy_01( )
}


class USSR_Convoy_01 extends ScriptWheeledVehicleLogic
{
	constructor( )
	{
		ScriptWheeledVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USSR_Convoy_01"

	function OnSpawn( )
	{
			
		ScriptWheeledVehicleLogic.OnSpawn( )
		
		SetDestroyedEffect( "Small_Vehicle_Explosion" )
		// The syntax for AddCargo is path, spawnrate, should_stop_while_dropping, remove_cargo_after_dropping
		// Please remove the comment above and make this take a string into the cargo table: AddCargo( "gameplay/characters/infantry/ussr/infantry_basic_01.sigml", 3, 1, 1, 0 )
	}
}


