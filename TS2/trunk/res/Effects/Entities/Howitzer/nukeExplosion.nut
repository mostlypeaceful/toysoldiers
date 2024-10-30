sigexport function EntityOnCreate( entity )
{
	local exp = ExplosionLogic( )
	entity.Logic = exp
	exp.FullSize = 0.1
	exp.GrowRate = 0.1
	exp.HitPoints = 0.0
	
	local intensity = 2
	exp.LightColor = Math.Vec4.Construct(intensity,intensity,intensity, 1 )
	exp.LightSize = 350.0
	exp.LightExpandTime = 1.0
	exp.LightCollapseTime = 0.35
	exp.LightHeight = 10.0
}