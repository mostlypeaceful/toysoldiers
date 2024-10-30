
sigexport function EntityOnCreate( entity )
{
	entity.Logic = BreakableLogic( )
	entity.Logic.SetDestroyedEffect( "Building_Break_Apart" )
}
