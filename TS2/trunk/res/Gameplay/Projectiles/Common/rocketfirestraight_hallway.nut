
sigexport function EntityOnCreate( entity )
{
	local logic = HallwayRocket( )
	entity.Logic = logic
	
	logic.GuidanceMode = GUIDANCE_MODE_STRAIGHT_FIRE
	logic.MinTimeMultiplier = 0.3
	logic.MaxTimeMultiplier = 1.2
	logic.UserTimeMultiplier = 0.6
	logic.ShellCamSteerRate = 0.2
}

