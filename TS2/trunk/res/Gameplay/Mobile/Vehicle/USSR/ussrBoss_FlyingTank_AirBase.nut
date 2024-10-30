sigimport "Gameplay/Mobile/Airborne/Common/Airbornelogic.nut"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = AntonovFlyingBase( )
}

class AntonovFlyingBase extends AirborneVehicleLogic
{
	constructor( )
	{
		AirborneVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "AntonovFlyingBase"

	function OnSpawn( )
	{
		SetRandomFlyTimeRemaining( 10 )
		AirborneVehicleLogic.OnSpawn( )
	}
}
