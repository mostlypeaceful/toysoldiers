
sigexport function EntityOnCreate( entity )
{

	local logic = ShellLogic( )
	entity.Logic = logic
	
	logic.MinTimeMultiplier = 0.3
	logic.MaxTimeMultiplier = 1.6
	logic.UserTimeMultiplier = 0.7
	logic.HitEffectOverride = "MortarSmokeExplosion"
//	logic.ShellCamSteerRate = 0.4
}
