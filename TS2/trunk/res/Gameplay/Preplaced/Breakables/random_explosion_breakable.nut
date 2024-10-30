
sigexport function EntityOnCreate( entity )
{
	entity.Logic = BreakableLogic( )
	
	switch( ObjectiveRand.Int( 1, 5 ) )
	{
	case 1: entity.Logic.SetDestroyedEffect( "RocketExplosion01" )
		break;
	case 2: entity.Logic.SetDestroyedEffect( "RocketExplosion04" )
		break;
	case 3: entity.Logic.SetDestroyedEffect( "ShellExplosion1" )
		break;
	case 4: entity.Logic.SetDestroyedEffect( "ShellExplosion2" )
		break;
	case 5: entity.Logic.SetDestroyedEffect( "ShellExplosion3" )
		break;
	}
}
