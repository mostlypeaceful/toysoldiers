sigimport "Gameplay/Characters/Passenger/Common/passengerlogic.nut"

//sigimport "Art/Characters/Blue/usa_helm_2.mshml"
//sigimport "Art/Characters/Blue/usa_helm_nvg.mshml"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USAArtillerySoldier03Logic( CreateArtillerySoldierMoMap( entity ) )
}
sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.Outfit( entity )
}

class USAArtillerySoldier03Logic extends ArtillerySoldierLogic
{
	function GetHead( )
	 {
		 return ""//Art/Characters/Blue/usa_head_6.mshml"
	}
	function GetHelmet( )
	 {
		local helms = {}
		 //helms[0] <- "Art/Characters/Blue/usa_1950s_helm.mshml"
//		 helms[1] <- "Art/Characters/Blue/usa_helm_2.mshml"
//		 helms[1] <- "Art/Characters/Blue/usa_helm_nvg.mshml"
		
		 //local helmRand = 0 //ObjectiveRand.Int(0,1)
		 //return helms[helmRand]
		 return ""
	 }
}
