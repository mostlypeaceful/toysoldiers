
sigexport function EntityOnCreate( entity )
{
	local logic = ShellLogic( )
	entity.Logic = logic
	
	logic.MinTimeMultiplier = 0.3
	logic.MaxTimeMultiplier = 1.6
	logic.UserTimeMultiplier = 0.7
}
