
sigexport function EntityOnCreate( entity )
{
	entity.Logic = RocketFullLockerProjectileLogic( )
}

class RocketFullLockerProjectileLogic extends RocketLogic
{
	constructor( )
	{
		RocketLogic.constructor( )
		GuidanceMode = GUIDANCE_MODE_FULL_LOCK
	}

	function DebugTypeName( )
		return "RocketLockerProjectileLogic"
}
