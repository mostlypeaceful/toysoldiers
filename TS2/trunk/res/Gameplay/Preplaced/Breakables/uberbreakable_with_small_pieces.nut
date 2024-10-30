sigimport "Effects/Entities/Misc/breakablepuff_small.sigml"
sigimport "Effects/Entities/Misc/uberbreakable_clump_small.sigml"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = UberBreakableLogic( )
	entity.Logic.OnPieceDestroyEffect = "Ubr_Sml_Pc_01"
	//entity.Logic.OnClumpDestroyEffect = "Ubr_Sml_Clmp_01"
}
