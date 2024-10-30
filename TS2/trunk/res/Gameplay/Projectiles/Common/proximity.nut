sigimport "Effects/Entities/Howitzer/explosion_01.sigml"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = ProximityMineLogic( )
}

class ProximityMineLogic extends LandMineLogic
{
	constructor( )
	{
		LandMineLogic.constructor( )
	}

	function DebugTypeName( )
		return "ProximityMineLogic"
	
	function OnSpawn( )
	{
		SetExplosionPath( "Effects/Entities/Howitzer/explosion_01.sigml" )
		SetMineType( 0, 5 )
		FullSize = 10.0
		GrowRate = 20.0
		HitPoints = 200.0
		LandMineLogic.OnSpawn( )
	}
}
