
sigexport function EntityOnCreate( entity )
{
	entity.Logic = RocketLockerProjectileLogic( )
}

class RocketLockerProjectileLogic extends RocketLogic
{
	constructor( )
	{
		RocketLogic.constructor( )
		GuidanceMode = GUIDANCE_MODE_LAZY_LOCK
	}

	function DebugTypeName( )
		return "RocketLockerProjectileLogic"
}
