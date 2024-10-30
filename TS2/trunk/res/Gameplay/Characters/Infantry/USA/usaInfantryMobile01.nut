sigimport "Gameplay/Characters/Infantry/Common/InfantryLogic.nut"
sigimport "Gui/Textures/WaveIcons/USSR/infantry_lvl2_g.png"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_Infantry_Mobile_01( )
}

sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.Outfit( entity )
}

class USA_Infantry_Mobile_01 extends InfantryLogic
{
	constructor( )
	{
		InfantryLogic.constructor( )
	}
	
	function DebugTypeName( )
		return "USA_Infantry_Mobile_01"
		
	function OnSpawn( )
	{		
		InfantryLogic.OnSpawn( )
	}
}
