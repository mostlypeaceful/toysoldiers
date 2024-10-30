sigexport function EntityOnCreate( entity )
{
	entity.Logic = BreakableLogic( )
	entity.Logic.SetDestroyedEffect( "Target_Destroyed_400" )
}
