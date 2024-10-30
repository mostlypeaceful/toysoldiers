sigimport "Gameplay/Mobile/Airborne/Common/Airbornelogic.nut"
sigimport "gameplay/characters/infantry/usa/infantry_paratrooper_01.sigml"
sigimport "gui/textures/waveicons/USA/infantry_lvl1_g.png"
sigimport "gui/textures/waveicons/usa/vehicle_transport_g.png"
sigimport "Gameplay/Mobile/Airborne/usa/usaplanetransport01_momap.nut"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_Plane_Transport_01( )
}

class USA_Plane_Transport_01 extends AirborneVehicleLogic
{
	constructor( )
	{
		AirborneVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USA_Plane_Transport_01"

	function OnSpawn( )
	{
		AirborneVehicleLogic.OnSpawn( )
		SetRandomFlyTimeRemaining( 0 )
		AddCargo( "USA_Transport" )
		DisableEvasion = 1
		SetDestroyedEffect( "Bomber_Explosion" )
	}
	
	function SetMotionMap( )
		Animatable.MotionMap = USAPlaneTransport01MoMap( this )

}
