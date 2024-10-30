sigimport "Gameplay/Characters/Passenger/Common/passengerlogic.nut"
sigimport "Art/Characters/Blue/usa_head_1.mshml"
sigimport "Art/Characters/Blue/usa_head_2.mshml"
sigimport "Art/Characters/Blue/usa_head_3.mshml"
sigimport "Art/Characters/Blue/usa_head_4.mshml"
sigimport "Art/Characters/Blue/usa_head_5.mshml"
sigimport "Art/Characters/Blue/usa_head_6.mshml"

sigimport "Art/Characters/Red/korean_head_1.mshml"
sigimport "Art/Characters/Blue/usa_helm_nvg.mshml"
sigimport "Art/Characters/Blue/usa_gasmask_1.mshml"
sigimport "Art/Characters/Blue/usa_elite_helm_1.mshml"
sigimport "Art/Characters/Blue/usa_helm_2.mshml"
sigimport "Art/Characters/Blue/usa_helm_nvg.mshml"
sigimport "Art/Characters/Blue/usa_backpack_1.mshml"
sigimport "Art/Characters/Blue/american_sailorhat.mshml"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USAArtillerySoldierEliteLogic( CreateArtillerySoldierMoMap( entity ) )
}
sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.Outfit( entity )
}

class USAArtillerySoldierEliteLogic extends ArtillerySoldierLogic
{
	function GetHead( )
	 {
		 local heads = {}
		 heads[0] <- "Art/Characters/Blue/usa_head_1.mshml"
		 heads[1] <- "Art/Characters/Blue/usa_head_2.mshml"
		 heads[2] <- "Art/Characters/Blue/usa_gasmask_1.mshml"
		 heads[3] <- "Art/Characters/Blue/usa_head_3.mshml"
		 heads[4] <- "Art/Characters/Blue/usa_head_4.mshml"
		 heads[5] <- "Art/Characters/Blue/usa_head_5.mshml"
		 heads[6] <- "Art/Characters/Blue/usa_head_6.mshml"
		 heads[7] <- "Art/Characters/Red/korean_head_1.mshml"
		
		 local headRand = ObjectiveRand.Int(0,7)
		  		return heads[headRand]
	}
	function GetHelmet( )
	 {
		 local helms = {}
		 helms[0] <- "Art/Characters/Blue/usa_helm_nvg.mshml"
		 helms[1] <- "Art/Characters/Blue/usa_helm_2.mshml"
		 helms[2] <- "Art/Characters/Blue/usa_elite_helm_1.mshml"
		 helms[3] <- "Art/Characters/Blue/usa_helm_nvg.mshml"
		 helms[4] <- "Art/Characters/Blue/usa_helm_nvg.mshml"
		
		local helmRand = ObjectiveRand.Int(0,4)
		return helms[helmRand]
	 }

}
