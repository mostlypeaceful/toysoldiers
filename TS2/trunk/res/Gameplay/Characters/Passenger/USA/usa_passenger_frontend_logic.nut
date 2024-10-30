sigimport "Gameplay/Characters/Passenger/Common/passengerlogic.nut"
sigimport "Art/Characters/Blue/usa_head_6.mshml"
sigimport "Art/Characters/Blue/usa_1950s_helm.mshml"
sigimport "Art/Characters/Blue/american_cap.mshml"
sigimport "Art/Characters/Blue/american_sailorhat.mshml"
sigimport "Art/Characters/Blue/usa_backpack_1.mshml"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USAArtillerySoldierFrontendLogic( CreateArtillerySoldierMoMap( entity ) )
}
sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.Outfit( entity )
}
	 
class USAArtillerySoldierFrontendLogic extends ArtillerySoldierLogic
{

	function GetHead( )
	 {
		 return "Art/Characters/Blue/usa_head_6.mshml"
	}
	
	function GetHelmet( )
	{
		local helms = {}
		 helms[0] <- "Art/Characters/Blue/usa_1950s_helm.mshml"
		 helms[1] <- "Art/Characters/Blue/american_cap.mshml"
		 helms[2] <- "Art/Characters/Blue/american_sailorhat.mshml"
		
		 local helmRand = ObjectiveRand.Int(0,2)
		 return helms[helmRand]
	}
	
	function GetBackpack( )
	{
		local backpackRand = ObjectiveRand.Int(0,1)
		if( backpackRand)
			return "Art/Characters/Blue/usa_backpack_1.mshml"
		else
			return ""
	}
	
}
