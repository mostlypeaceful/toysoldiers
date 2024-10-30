sigexport function EntityOnCreate( entity )
{
	local exp = ExplosionLogic( )
	entity.Logic = exp
	exp.FullSize = 10.0
	exp.GrowRate = 20.0
	exp.HitPoints = 500.0
}