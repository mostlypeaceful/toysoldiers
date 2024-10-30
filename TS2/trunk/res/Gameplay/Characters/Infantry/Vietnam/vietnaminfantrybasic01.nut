sigimport "Gameplay/Characters/Infantry/Common/InfantryLogic.nut"
sigimport "Art/Characters/Red/vietcong_elite_debris.sigml"
sigimport "Art/Characters/Red/russian_head_1.mshml"
sigimport "Art/Characters/Red/russian_head_2.mshml"
sigimport "Art/Characters/Red/russian_head_3.mshml"
sigimport "Art/Characters/Red/russian_head_4.mshml"
sigimport "Art/Characters/Red/russian_head_5.mshml"
sigimport "Art/Characters/Red/russian_helm_1.mshml"
sigimport "Art/Characters/Red/russian_backpack_1.mshml"
sigimport "Gui/Textures/WaveIcons/USSR/infantry_lvl1b_g.png"


sigexport function EntityOnCreate( entity )
{
	entity.Logic = Vietnam_Infantry_Basic_01( )
}

sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.Outfit( entity )
}

class Vietnam_Infantry_Basic_01 extends InfantryLogic
{
	constructor( )
	{
		InfantryLogic.constructor( )
		SoldierLogic.SetAlternateDebrisMesh( "Art/Characters/Red/vietcong_elite_debris.sigml" )
	}
	
	function DebugTypeName( )
		return "Vietnam_Infantry_Basic_01"
		
	function OnSpawn( )
	{		
		InfantryLogic.OnSpawn( )
	}
	
	function GetHead( )
	 {
		 local heads = {}
		 heads[0] <- ""

		 local headRand = ObjectiveRand.Int(0,4)
		 return ""
	 }
	function GetHelmet( ) 
	{
		
		local helmRand = ObjectiveRand.Int(0,100)
		if (helmRand < 85 )
			return ""
		else 
			return ""
	}
	
	function GetBackpack( ) 
	{
		local backpackRand = ObjectiveRand.Int(0,1)
		if (backpackRand)
			return ""
		else
			return ""
	}
}
