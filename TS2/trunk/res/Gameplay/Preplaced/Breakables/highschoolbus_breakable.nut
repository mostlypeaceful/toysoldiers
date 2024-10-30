
sigexport function EntityOnCreate( entity )
{
	entity.Logic = BreakableLogic( )
	entity.Logic.SetDestroyedEffect( "Highschool_Bus_Explosion" )
}
