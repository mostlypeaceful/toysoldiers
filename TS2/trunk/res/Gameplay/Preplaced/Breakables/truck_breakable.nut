
sigexport function EntityOnCreate( entity )
{
	entity.Logic = BreakableLogic( )
	
	switch( ObjectiveRand.Int( 1, 2 ) )
	{
	case 1: entity.Logic.SetDestroyedEffect( "Civilian_Truck_Explosion_01" )
		break;
	case 2: entity.Logic.SetDestroyedEffect( "Civilian_Truck_Explosion_02" )
		break;
	}
}
