sigexport function EntityOnCreate( entity )
{
	local exp = ExplosionLogic( )
	entity.Logic = exp
	exp.FullSize = 15.0
	exp.GrowRate = 100.0
	exp.HitPoints = 100.0
}