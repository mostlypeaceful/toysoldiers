sigexport function EntityOnCreate( entity )
{
	local exp = ExplosionLogic( )
	entity.Logic = exp
	exp.FullSize = 5
	exp.GrowRate = 1
	exp.HitPoints = 0.0
	
	/*
	exp.LightColor = Math.Vec4.Construct( 2.0, 2.0, 2.0, 1.0 )
	exp.LightSize = 20.0
	exp.LightExpandTime = 0.15
	exp.LightCollapseTime = 0.4
	exp.LightHeight = 1.5
	*/
}