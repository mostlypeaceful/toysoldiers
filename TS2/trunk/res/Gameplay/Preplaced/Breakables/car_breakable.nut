
sigexport function EntityOnCreate( entity )
{
	entity.Logic = BreakableLogic( )
	
	switch( ObjectiveRand.Int( 1, 3 ) )
	{
	case 1: entity.Logic.SetDestroyedEffect( "Civilian_Car_Explosion_01" )
		break;
	case 2: entity.Logic.SetDestroyedEffect( "Civilian_Car_Explosion_02" )
		break;
	case 3: entity.Logic.SetDestroyedEffect( "Civilian_Car_Explosion_03" )
		break;
	}
}
