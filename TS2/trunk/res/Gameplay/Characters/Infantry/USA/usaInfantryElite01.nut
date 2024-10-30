sigimport "Gameplay/Characters/Infantry/Common/InfantryLogic.nut"
sigimport "Art/Characters/Blue/american_elite_debris.sigml"
sigimport "Art/Characters/Blue/usa_head_1.mshml"
sigimport "Art/Characters/Blue/usa_head_2.mshml"
sigimport "Art/Characters/Blue/usa_head_3.mshml"
sigimport "Art/Characters/Blue/usa_head_4.mshml"
sigimport "Art/Characters/Blue/usa_head_5.mshml"
sigimport "Art/Characters/Blue/usa_head_6.mshml"
sigimport "Art/Characters/Blue/usa_gasmask_1.mshml"
sigimport "Art/Characters/Blue/usa_elite_helm_1.mshml"
sigimport "Art/Characters/Blue/usa_helm_2.mshml"
sigimport "Art/Characters/Blue/usa_helm_nvg.mshml"
sigimport "Art/Characters/Blue/usa_backpack_1.mshml"
sigimport "Art/Characters/Blue/usa_backpack_2.mshml"
sigimport "Art/Characters/Blue/backpackgun.sigml"
sigimport "Gui/Textures/WaveIcons/USA/infantry_lvl2_g.png"


sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_Infantry_Elite_01( )
}

sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.Outfit( entity )
}

class USA_Infantry_Elite_01 extends InfantryLogic
{
	constructor( )
	{
		InfantryLogic.constructor( )
		SoldierLogic.SetAlternateDebrisMesh( "Art/Characters/Blue/american_elite_debris.sigml" )
		SingleShotWeaponID = "USA_INFANTRY_ROCKET"
	}
	function SetMotionMap( )
		Animatable.MotionMap = EliteMotionMap( this )

	function DebugTypeName( )
		return "USA_Infantry_Elite_01"
		
	function OnSpawn( )
	{		
		SetPropEnum( )
		InfantryLogic.OnSpawn( )
	}
	
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
		
		 local headRand = ObjectiveRand.Int(0,6)
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

	function GetBackpack( ) 
	{
		local backpackRand = ObjectiveRand.Int(0,2)
		local mybackpack = {}
		mybackpack[0] <- "Art/Characters/Blue/usa_backpack_1.mshml"
		mybackpack[1] <- "Art/Characters/Blue/usa_backpack_2.mshml"
		mybackpack[2] <- "Art/Characters/Blue/backpackgun.sigml"

		return mybackpack[backpackRand]
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






