sigimport "Gameplay/Characters/Passenger/Common/passengerlogic.nut"
sigimport "Art/Characters/Blue/usa_head_1.mshml"
sigimport "Art/Characters/Blue/usa_head_2.mshml"
sigimport "Art/Characters/Blue/usa_head_3.mshml"
sigimport "Art/Characters/Blue/usa_head_4.mshml"
sigimport "Art/Characters/Blue/usa_head_5.mshml"
sigimport "Art/Characters/Blue/usa_head_6.mshml"
sigimport "Art/Characters/Blue/usa_gasmask_1.mshml"
sigimport "Art/Characters/Blue/usa_helm_1.mshml"
sigimport "Art/Characters/Blue/usa_backpack_2.mshml"
sigimport "Art/Characters/Blue/usa_backpack_1.mshml"
sigimport "Art/Characters/Blue/backpackgun.sigml"
sigimport "Art/Characters/Blue/american_cap.mshml"
//sigimport "Art/Characters/Blue/american_sailorhat.mshml"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USAArtillerySoldierLogic( CreateArtillerySoldierMoMap( entity ) )
}
sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.Outfit( entity )
}

class USAArtillerySoldierLogic extends ArtillerySoldierLogic
{
	function GetHead( )
	 {
		 local heads = {}
		 heads[0] <- "Art/Characters/Blue/usa_head_1.mshml"
		 heads[1] <- "Art/Characters/Blue/usa_head_2.mshml"
		 heads[2] <- "Art/Characters/Blue/usa_head_3.mshml"
		 heads[3] <- "Art/Characters/Blue/usa_head_4.mshml"
		 heads[4] <- "Art/Characters/Blue/usa_head_5.mshml"
		 heads[5] <- "Art/Characters/Blue/usa_head_6.mshml"
		 heads[6] <- "Art/Characters/Blue/usa_head_6.mshml"
		 heads[7] <- "Art/Characters/Blue/usa_gasmask_1.mshml"
		 heads[8] <- "Art/Characters/Blue/usa_head_4.mshml"

		
		 local headRand = ObjectiveRand.Int(0,8)
		  return heads[headRand]
	}

	function GetHelmet( )
	 {
		local helms = {}
		 helms[0] <- "Art/Characters/Blue/usa_helm_1.mshml"
		 helms[1] <- "Art/Characters/Blue/american_cap.mshml"
		 helms[2] <- "Art/Characters/Blue/usa_helm_1.mshml"
		 helms[3] <- "Art/Characters/Blue/usa_helm_1.mshml"
		 helms[4] <- "Art/Characters/Blue/usa_helm_1.mshml"
		 helms[5] <- ""
		
		 local helmRand = ObjectiveRand.Int(0,5)
		 return helms[helmRand]
	 }

	 function GetBackpack( )
	 {
		local backpackRand = ObjectiveRand.Int(0,3)
		local mybackpack = {}
		 mybackpack[0] <- "Art/Characters/Blue/usa_backpack_1.mshml"
		 mybackpack[1] <- "Art/Characters/Blue/usa_backpack_2.mshml"
		 mybackpack[2] <- "Art/Characters/Blue/backpackgun.sigml"
		 mybackpack[3] <- ""
		return mybackpack[backpackRand]
	 }

}
