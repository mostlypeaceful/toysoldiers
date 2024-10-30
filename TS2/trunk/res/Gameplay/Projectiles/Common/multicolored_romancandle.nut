sigimport "Effects\\Entities\\Projectiles\\romancandles\\romancandle_blue.sigml"
sigimport "Effects\\Entities\\Projectiles\\romancandles\\romancandle_blueer.sigml"
sigimport "Effects\\Entities\\Projectiles\\romancandles\\romancandle_purple.sigml"
sigimport "Effects\\Entities\\Projectiles\\romancandles\\romancandle_red.sigml"
sigimport "Effects\\Entities\\Projectiles\\romancandles\\romancandle_white.sigml"
sigimport "Effects\\Entities\\Projectiles\\romancandles\\romancandle_yellow.sigml"

sigexport function EntityOnCreate( entity )
{
	local logic = ShellLogic( )
	entity.Logic = logic
	
	logic.MinTimeMultiplier = 0.3
	logic.MaxTimeMultiplier = 1.6
	logic.UserTimeMultiplier = 0.7
	
	switch( ObjectiveRand.Int( 1, 6 ) )
	{
	case 1: entity.SpawnChildFromProxy( "Effects\\Entities\\Projectiles\\romancandles\\romancandle_blue.sigml")	
	 break;
	case 2: entity.SpawnChildFromProxy( "Effects\\Entities\\Projectiles\\romancandles\\romancandle_blueer.sigml")	
	 break;
	case 3: entity.SpawnChildFromProxy( "Effects\\Entities\\Projectiles\\romancandles\\romancandle_purple.sigml")	
	 break;
	case 4: entity.SpawnChildFromProxy( "Effects\\Entities\\Projectiles\\romancandles\\romancandle_red.sigml")	
	 break;
	case 5: entity.SpawnChildFromProxy( "Effects\\Entities\\Projectiles\\romancandles\\romancandle_white.sigml")	
	 break;
	case 6: entity.SpawnChildFromProxy( "Effects\\Entities\\Projectiles\\romancandles\\romancandle_yellow.sigml")	
	 break;
	}
}
