
sigexport function EntityOnCreate( entity )
{

	local logic = ShellLogic( )
	entity.Logic = logic
	
	logic.MinTimeMultiplier = 0.3
	logic.MaxTimeMultiplier = 1.6
	logic.UserTimeMultiplier = 1.0
	logic.HitEffectOverride = "ShellExplosion1"
//	logic.ShellCamSteerRate = 0.4
}
