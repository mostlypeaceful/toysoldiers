sigimport "Gameplay/Characters/Passenger/Common/passengerlogic.nut"
sigimport "Art/Characters/Red/russian_head_1.mshml"
sigimport "Art/Characters/Red/russian_head_2.mshml"
sigimport "Art/Characters/Red/russian_head_3.mshml"
sigimport "Art/Characters/Red/russian_head_4.mshml"
sigimport "Art/Characters/Red/russian_head_5.mshml"
sigimport "Art/Characters/Red/russian_helm_1.mshml"
sigimport "Art/Characters/Red/russian_furryhat.mshml"
sigimport "Art/Characters/Red/russian_tankhelmet_nongoggled.mshml"
sigimport "Art/Characters/Red/russian_tankhelmet_goggled.mshml"
sigimport "Art/Characters/Red/russian_backpack_1.mshml"
sigimport "Art/Characters/Red/AKBackpack.sigml"


sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSRArtillerySoldierLogic( CreateArtillerySoldierMoMap( entity ) )
}
sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.Outfit( entity )
}

class USSRArtillerySoldierLogic extends ArtillerySoldierLogic
{
	function GetHead( )
	 {
		 local heads = {}
		 heads[0] <- "Art/Characters/Red/russian_head_1.mshml"
		 heads[1] <- "Art/Characters/Red/russian_head_2.mshml"
		 heads[2] <- "Art/Characters/Red/russian_head_3.mshml"
		 heads[3] <- "Art/Characters/Red/russian_head_4.mshml"
		 heads[4] <- "Art/Characters/Red/russian_head_5.mshml"

		 local headRand = ObjectiveRand.Int(0,4)
		 return heads[headRand]
	 }
	function GetHelmet( ) 
	{
		 local helms = {}
		 helms[0] <- "Art/Characters/Red/russian_helm_1.mshml"
		 helms[1] <- "Art/Characters/Red/russian_furryhat.mshml"
		 helms[2] <- "Art/Characters/Red/russian_tankhelmet_nongoggled.mshml"
		 helms[3] <- "Art/Characters/Red/russian_tankhelmet_goggled.mshml"
		 helms[4] <- "Art/Characters/Red/russian_helm_1.mshml"
		 helms[5] <- "Art/Characters/Red/russian_helm_1.mshml"
		 helms[6] <- ""
		
		local helmRand = ObjectiveRand.Int(0,6)
		return helms[helmRand]
	}

	 function GetBackpack( )
	 {
		local backpackRand = ObjectiveRand.Int(0,3)
		local mybackpack = {}
		 mybackpack[0] <- "Art/Characters/Red/russian_backpack_1.mshml"
		 mybackpack[1] <- "Art/Characters/Red/russian_backpack_1.mshml"
		 mybackpack[2] <- "Art/Characters/Red/AKBackpack.sigml"
		 mybackpack[3] <- ""
		return mybackpack[backpackRand]
	 }

}
