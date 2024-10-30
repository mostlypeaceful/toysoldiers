sigimport "Gameplay/Characters/Passenger/Common/passengerlogic.nut"
sigimport "Art/Characters/Red/russian_elite_head_1.mshml"
sigimport "Art/Characters/Red/russian_elite_debris.sigml"

sigimport "Art/Characters/Red/russian_gasmask_albino.mshml"
sigimport "Art/Characters/Red/russian_gasmask_black.mshml"
sigimport "Art/Characters/Red/russian_head_3.mshml"
sigimport "Art/Characters/Red/russian_furryhat.mshml"
sigimport "Art/Characters/Red/russian_nvg.mshml"
sigimport "Art/Characters/Red/russian_backpack_1.mshml"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSRArtillerySoldier02Logic( CreateArtillerySoldierMoMap( entity ) )
}
sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.Outfit( entity )
}

class USSRArtillerySoldier02Logic extends ArtillerySoldierLogic
{
	function GetHead( )
	 {
		 local heads = {}
		 heads[0] <- "Art/Characters/Red/russian_elite_head_1.mshml"
		 heads[1] <- "Art/Characters/Red/russian_gasmask_albino.mshml"
		 heads[2] <- "Art/Characters/Red/russian_gasmask_black.mshml"
		 heads[3] <- "Art/Characters/Red/russian_head_3.mshml"

		 local headRand = ObjectiveRand.Int(0,3)
		 return heads[headRand]
	}
	function GetHelmet( )
	 {
		 local helms = {}
		 helms[0] <- "Art/Characters/Red/russian_furryhat.mshml"
		 helms[1] <- "Art/Characters/Red/russian_furryhat.mshml"
		 helms[2] <- "Art/Characters/Red/russian_nvg.mshml"
		 helms[3] <- "Art/Characters/Red/russian_nvg.mshml"

		
		local helmRand = ObjectiveRand.Int(0,3)
		return helms[helmRand]
	 }

}
