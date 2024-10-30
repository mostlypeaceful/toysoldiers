sigimport "Gameplay/Mobile/Helicopter/Common/HoverVehicleLogic.nut"
sigimport "gameplay/characters/infantry/usa/infantry_paratrooper_01.sigml"
sigimport "gui/textures/waveicons/usa/infantry_lvl1_g.png"
sigimport "gui/textures/waveicons/usa/vehicle_helitransport_g.png"
sigimport "gameplay/characters/infantry/usa/infantry_elite_01.sigml"
sigimport "Gameplay/Mobile/Helicopter/USA/usahelotransport01momap.nut"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_Helo_Transport_01( )
}


class USA_Helo_Transport_01 extends HoverVehicleLogic
{
	constructor( )
	{
		HoverVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USA_Helo_Transport_01"

	function OnSpawn( )
	{	

		// The syntax for AddCargo is path, spawnrate, should_stop_while_dropping, remove_cargo_after_dropping
		// Please remove the comment above and make this take a string into the cargo table: AddCargo( "gameplay/characters/infantry/usa/infantry_paratrooper_01.sigml", 7, 0.5, 1, 0 )
		
		AddCargo( "USA_Fast_Ropers" )
		
		HoverVehicleLogic.OnSpawn( )
		SetDestroyedEffect( "Helicopter_Explosion" )
	}
	
		function SetMotionMap( ) 
		Animatable.MotionMap = USAHeloTransport01MoMap( this )
}
