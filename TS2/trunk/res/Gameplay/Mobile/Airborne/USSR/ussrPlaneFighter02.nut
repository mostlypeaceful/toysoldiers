sigimport "Gameplay/Mobile/Airborne/Common/Airbornelogic.nut"
sigimport "gui/textures/waveicons/ussr/vehicle_fighter_g.png"
sigimport "effects/fx/units/vehicles/jet_afterburner_01.fxml"
sigimport "effects/fx/units/vehicles/jet_boost_01.fxml"


sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Plane_Fighter_02( )
}

class USSR_Plane_Fighter_02 extends AirborneVehicleLogic
{
	constructor( )
	{
		AirborneVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USSR_Plane_Fighter_02"

	function OnSpawn( )
	{
// USSR MIG23 Cannon, USSR MIG23 Bomb (LT)
		AirborneVehicleLogic.OnSpawn( )
		InitializeJetEngineFx( "effects/fx/units/vehicles/jet_afterburner_01.fxml", "effects/fx/units/vehicles/jet_boost_01.fxml" )
	}
}
