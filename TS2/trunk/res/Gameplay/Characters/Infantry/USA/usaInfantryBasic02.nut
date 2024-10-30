sigimport "Gameplay/Characters/Infantry/Common/InfantryLogic.nut"
sigimport "Art/Characters/Blue/usa_infantry_debris.sigml"
sigimport "Art/Characters/Blue/usa_head_1.mshml"
sigimport "Art/Characters/Blue/usa_head_2.mshml"
sigimport "Art/Characters/Blue/usa_head_3.mshml"
sigimport "Art/Characters/Blue/usa_head_4.mshml"
sigimport "Art/Characters/Blue/usa_head_5.mshml"
sigimport "Art/Characters/Blue/usa_head_6.mshml"
sigimport "Art/Characters/Blue/usa_helm_1.mshml"
sigimport "Art/Characters/Blue/usa_1950s_helm.mshml"
sigimport "Art/Characters/Blue/american_cap.mshml"
sigimport "Art/Characters/Blue/american_sailorhat.mshml"
sigimport "Art/Characters/Blue/usa_backpack_1.mshml"
sigimport "Art/Characters/Blue/usa_backpack_2.mshml"
sigimport "Gui/Textures/WaveIcons/USA/infantry_lvl1_g.png"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_Infantry_Basic_02( )
}

sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.Outfit( entity )
}

class USA_Infantry_Basic_02 extends InfantryLogic
{
	constructor( )
	{
		InfantryLogic.constructor( )
		SoldierLogic.SetAlternateDebrisMesh( "Art/Characters/Blue/usa_infantry_debris.sigml" )
	}
	
	function DebugTypeName( )
		return "USA_Infantry_Basic_01"
		
	function OnSpawn( )
	{		
		InfantryLogic.OnSpawn( )
	}
	
	function GetHead( )
	 {
		 local heads = {}
		 heads[0] <- "Art/Characters/Blue/usa_head_1.mshml"
		 heads[1] <- "Art/Characters/Blue/usa_head_2.mshml"
		 heads[2] <- "Art/Characters/Blue/usa_head_3.mshml"
		 heads[3] <- "Art/Characters/Blue/usa_head_4.mshml"
		 heads[4] <- "Art/Characters/Blue/usa_head_5.mshml"
		 heads[5] <- "Art/Characters/Blue/usa_head_6.mshml"
		
		 local headRand = ObjectiveRand.Int(0,5)
		  		return heads[headRand]
	}
	
	function GetHelmet( )
	 {
		local helms = {}
		 helms[0] <- "Art/Characters/Blue/usa_1950s_helm.mshml"
		 helms[1] <- "Art/Characters/Blue/american_cap.mshml"
		 helms[2] <- "Art/Characters/Blue/usa_1950s_helm.mshml"
		
		 local helmRand = ObjectiveRand.Int(0,2)
		 return helms[helmRand]
	 }
	 function GetBackpack( )
	 {
		 local backpackRand = ObjectiveRand.Int(0,1)
		 if( backpackRand)
			return "Art/Characters/Blue/usa_backpack_1.mshml"
		 else
			return "Art/Characters/Blue/usa_backpack_2.mshml"
	 }
}
