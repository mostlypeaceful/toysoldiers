sigexport function EntityOnCreate( entity )
{
	local exp = ExplosionLogic( )
	entity.Logic = exp
	exp.FullSize = 5
	exp.GrowRate = 1
	exp.HitPoints = 0.0
	
	/*
	exp.LightColor = Math.Vec4.Construct( 1.4, 1.0, 1.0, 1.0 )
	exp.LightSize = 30.0
	exp.LightExpandTime = 0.35
	exp.LightCollapseTime = 0.6
	exp.LightHeight = 1.5
	*/
}