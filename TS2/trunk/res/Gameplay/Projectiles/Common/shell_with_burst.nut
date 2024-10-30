
sigimport "Gameplay/Projectiles/USA/Howitzer2Shell_mini.sigml"

sigexport function EntityOnCreate( entity )
{

	local logic = ShellLogic( )
	entity.Logic = logic
	
	logic.MinTimeMultiplier = 0.3
	logic.MaxTimeMultiplier = 1.2
	logic.UserTimeMultiplier = 0.8
	
	logic.BurstCount = 5
	logic.BurstDamageMod = 0.15
	logic.BurstPath = "Gameplay/Projectiles/USA/Howitzer2Shell_mini.sigml"
}
