sigexport function EntityOnCreate( entity )
{
	local exp = ExplosionLogic( )
	entity.Logic = exp
	exp.FullSize = 20.0
	exp.GrowRate = 100.0
	exp.HitPoints = 300.0
}