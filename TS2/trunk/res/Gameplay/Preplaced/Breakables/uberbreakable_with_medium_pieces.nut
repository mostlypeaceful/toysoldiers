sigimport "Effects/Entities/Misc/breakablepuff_medium.sigml"
sigimport "Effects/Entities/Misc/uberbreakable_clump_medium.sigml"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = UberBreakableLogic( )
	entity.Logic.OnPieceDestroyEffect = "Ubr_Med_Pc_01"
	//entity.Logic.OnClumpDestroyEffect = "Ubr_Med_Clmp_01"
}
