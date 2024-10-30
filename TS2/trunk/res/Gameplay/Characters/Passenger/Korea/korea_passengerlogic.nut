sigimport "Gameplay/Characters/Infantry/Common/InfantryLogic.nut"
sigimport "Art/Characters/Red/korean_infantry_debris.sigml"
sigimport "Art/Characters/Red/korean_head_1.mshml"
sigimport "Art/Characters/Red/korean_helm_1.mshml"
sigimport "Art/Characters/Red/korean_cap.mshml"
sigimport "Art/Characters/Red/korean_softhelmet_nongoggled.mshml"
sigimport "Art/Characters/Red/korean_softhelmet_goggled.mshml"
sigimport "Art/Characters/Red/chinese_helmet.mshml"
sigimport "Gui/Textures/WaveIcons/USSR/infantry_lvl1c_g.png"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = KoreaArtillerySoldierLogic( CreateArtillerySoldierMoMap( entity ) )
}
sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.Outfit( entity )
}

class KoreaArtillerySoldierLogic extends ArtillerySoldierLogic
{
	function GetHead( )
	 {
		 local heads = {}
		 heads[0] <- "Art/Characters/Red/korean_head_1.mshml"

		 //local headRand = ObjectiveRand.Int(0,4)
		 return heads[0]
	 }
	function GetHelmet( ) 
	{
		 local helms = {}
		 helms[0] <- "Art/Characters/Red/korean_helm_1.mshml"
		 helms[1] <- "Art/Characters/Red/korean_helm_1.mshml"
		 helms[2] <- "Art/Characters/Red/korean_cap.mshml"
		 helms[3] <- "Art/Characters/Red/korean_cap.mshml"
		 helms[4] <- "Art/Characters/Red/korean_softhelmet_nongoggled.mshml"
		 helms[5] <- "Art/Characters/Red/korean_softhelmet_goggled.mshml"
		 helms[6] <- "Art/Characters/Red/chinese_helmet.mshml"
		 helms[7] <- "Art/Characters/Red/chinese_helmet.mshml"
		 helms[8] <- ""
		 
		 local helmRand = ObjectiveRand.Int(0,8)
		 return helms[helmRand]
	}
	
		function GetBackpack( ) 
	{
		//local backpackRand = ObjectiveRand.Int(0,1)
		//if (backpackRand)
		//	return "Art/Characters/Red/russian_backpack_1.mshml"
		//else
			return ""
	}
}
