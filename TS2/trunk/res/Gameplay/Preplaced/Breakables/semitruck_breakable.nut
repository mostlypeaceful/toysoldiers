
sigexport function EntityOnCreate( entity )
{
	entity.Logic = BreakableLogic( )
	entity.Logic.SetDestroyedEffect( "Semi_Truck_Explosion" )
}
