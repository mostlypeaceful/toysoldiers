sigimport "Effects/Entities/Misc/breakablepuff.sigml"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = UberBreakableLogic( )
	entity.Logic.OnPieceDestroyEffect = "Ubr_Sml_Pc_01"
}
