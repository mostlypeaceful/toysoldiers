sigimport "Gameplay/Mobile/Airborne/Common/Airbornelogic.nut"
sigimport "gui/textures/waveicons/usa/vehicle_ac130_g.png"
sigimport "Gameplay/Mobile/Airborne/usa/usaplanetransport01_momap.nut"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_Plane_Transport_02( )
}

class USA_Plane_Transport_02 extends AirborneVehicleLogic
{
	constructor( )
	{
		AirborneVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USA_Plane_Transport_02"

	function OnSpawn( )
	{
		DamageModifier = 0
		// Weapons: None
		AirborneVehicleLogic.OnSpawn( )
		SlaveLinkTurrentChildren = 1
		SetDestroyedEffect( "Bomber_Explosion" )
	}	

	function SetMotionMap( )
		Animatable.MotionMap = USAPlaneTransport01MoMap( this )		
}
