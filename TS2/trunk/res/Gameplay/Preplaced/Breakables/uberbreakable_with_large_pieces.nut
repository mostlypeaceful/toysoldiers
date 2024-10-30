sigimport "Effects/Entities/Misc/breakablepuff_large.sigml"
sigimport "Effects/Entities/Misc/uberbreakable_clump_large.sigml"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = UberBreakableLogic( )
	entity.Logic.OnPieceDestroyEffect = "Ubr_Lrg_Pc_01"
	//entity.Logic.OnClumpDestroyEffect = "Ubr_Lrg_Clmp_01"
}
