sigimport "Gameplay/Characters/Infantry/Common/InfantryLogic.nut"
sigimport "Gui/Textures/WaveIcons/USA/infantry_lvl1_g.png"


sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Infantry_Mobile_01( )
}

sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.Outfit( entity )
}

class USSR_Infantry_Mobile_01 extends InfantryLogic
{
	constructor( )
	{
		InfantryLogic.constructor( )
	}
	
	function DebugTypeName( )
		return "USSR_Infantry_Mobile_01"
		
	function OnSpawn( )
	{		
		InfantryLogic.OnSpawn( )
	}
}
