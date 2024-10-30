sigimport "Gameplay/Characters/Infantry/Common/InfantryLogic.nut"
sigimport "Art/Characters/Red/russian_infantry_debris.sigml"
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
sigimport "Gui/Textures/WaveIcons/USSR/infantry_lvl1_g.png"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Infantry_Basic_01( )
}

sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.Outfit( entity )
}

class USSR_Infantry_Basic_01 extends InfantryLogic
{
	constructor( )
	{
		InfantryLogic.constructor( )
		SoldierLogic.SetAlternateDebrisMesh( "Art/Characters/Red/russian_infantry_debris.sigml" )
	}
	
	function DebugTypeName( )
		return "USSR_Infantry_Basic_01"
		
	function OnSpawn( )
	{		
		InfantryLogic.OnSpawn( )
	}
	
	function GetHead( )
	 {
		 local heads = {}
		 heads[0] <- "Art/Characters/Red/russian_head_1.mshml"
		 heads[1] <- "Art/Characters/Red/russian_head_4.mshml"
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
		 helms[1] <- "Art/Characters/Red/russian_helm_1.mshml"
		 helms[2] <- "Art/Characters/Red/russian_tankhelmet_nongoggled.mshml"
		 helms[3] <- "Art/Characters/Red/russian_tankhelmet_goggled.mshml"
		 helms[4] <- "Art/Characters/Red/russian_helm_1.mshml"
		 helms[5] <- "Art/Characters/Red/russian_helm_1.mshml"

		local helmRand = ObjectiveRand.Int(0,5)
		return helms[helmRand]
	}
	
	function GetBackpack( ) 
	{
		local backpackRand = ObjectiveRand.Int(0,1)
		if (backpackRand)
			return "Art/Characters/Red/russian_backpack_1.mshml"
		else
			return ""
	}
}
