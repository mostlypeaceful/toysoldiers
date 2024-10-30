
sigexport function EntityOnCreate( entity )
{
	entity.Logic = BreakableLogic( )
	entity.Logic.SetDestroyedEffect( "Arcade_Golden_Breakable_Destroyed" )
}
