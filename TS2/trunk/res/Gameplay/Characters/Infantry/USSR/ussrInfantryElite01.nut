sigimport "Gameplay/Characters/Infantry/Common/InfantryLogic.nut"
sigimport "Art/Characters/Red/russian_elite_head_1.mshml"
sigimport "Art/Characters/Red/russian_elite_debris.sigml"

sigimport "Art/Characters/Red/russian_gasmask_albino.mshml"
sigimport "Art/Characters/Red/russian_gasmask_black.mshml"
sigimport "Art/Characters/Red/russian_head_3.mshml"
sigimport "Art/Characters/Red/russian_furryhat.mshml"
sigimport "Art/Characters/Red/russian_nvg.mshml"
sigimport "Art/Characters/Red/russian_backpack_1.mshml"

sigimport "Gui/Textures/WaveIcons/USSR/infantry_lvl2_g.png"


sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Infantry_Elite_01( )
}

sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.Outfit( entity )
}

class USSR_Infantry_Elite_01 extends InfantryLogic
{
	constructor( )
	{
		InfantryLogic.constructor( )
		SoldierLogic.SetAlternateDebrisMesh( "Art/Characters/Red/russian_elite_debris.sigml" )
		SingleShotWeaponID = "USSR_INFANTRY_ROCKET"
	}	
	function SetMotionMap( )
		Animatable.MotionMap = EliteMotionMap( this )
		
	function DebugTypeName( )
		return "USSR_Infantry_Elite_01"
		
	function OnSpawn( )
	{		
		SetPropEnum( )
		InfantryLogic.OnSpawn( )
	}
	
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

	function GetBackpack( ) 
	{
		local backpackRand = ObjectiveRand.Int(0,1)
		if (backpackRand == 1)
			return "Art/Characters/Red/russian_backpack_1.mshml"
		else
			return ""
	}

	function SetPropEnum( )
	{
		//local prop = ObjectiveRand.Int(0,4)
		//local propID = -1
		//switch( prop )
		//{
		//	case 0: 
		//		propID = CHARACTER_PROPS_ROCKET_LAUNCHER
		//	break;
		//	case 1: 
		//		propID = CHARACTER_PROPS_PISTOL
		//	break;
		//}
		//
		//if( propID != -1 )
			OwnerEntity.SetEnumValue( ENUM_CHARACTER_PROPS, CHARACTER_PROPS_ROCKET_LAUNCHER )
	}
}
