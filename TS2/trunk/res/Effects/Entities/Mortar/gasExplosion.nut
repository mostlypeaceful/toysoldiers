sigexport function EntityOnCreate( entity )
{
	local exp = ExplosionLogic( )
	entity.Logic = exp
	exp.FullSize = 5
	exp.GrowRate = 1
	exp.HitPoints = 0

	/*
	exp.LightColor = Math.Vec4.Construct( 0.5, 1.3, 0.1, 1.0 )
	exp.LightSize = 17.0
	exp.LightExpandTime = 0.15
	exp.LightCollapseTime = 0.4
	exp.LightHeight = 1.5
	*/
}