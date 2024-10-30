
sigexport function EntityOnCreate( entity )
{
	entity.Logic = A10GrumbleProjectileLogic( )
}

class A10GrumbleProjectileLogic extends RocketLogic
{
	constructor( )
	{
		RocketLogic.constructor( )
		NextGuidanceMode = GUIDANCE_MODE_FULL_LOCK
		GuidanceKickinTimer = 0.55
		ThrustDelayTimer = 0.35
		SmoothThrust = true
	}

	function DebugTypeName( )
		return "A10GrumbleProjectileLogic"
}
