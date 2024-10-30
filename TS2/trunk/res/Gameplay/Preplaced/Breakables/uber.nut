sigimport "Effects/Entities/Misc/firework_mini_burst.sigml"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = UberBreakableLogic( )
	//entity.Logic.OnDestroyEffect = ""
	//entity.Logic.OnClumpDestroyEffect = "Effects/Entities/Misc/firework_mini_burst.sigml"
	entity.Logic.OnPieceDestroyEffect = "Effects/Entities/Misc/firework_mini_burst.sigml"
}
