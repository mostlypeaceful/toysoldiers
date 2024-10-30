sigimport "Gameplay/Characters/Infantry/ussr/ussrinfantrybasic01.nut"
sigimport "Gameplay/Projectiles/USA/proximity.sigml"
sigimport "Art/Characters/Red/russian_infantry_debris.sigml"
sigimport "Art/Characters/Red/russian_cowboyhat.mshml"
sigimport "Gui/Textures/WaveIcons/USSR/infantry_lvl1_g.png"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Infantry_Cowboy_01( )
}

sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.Outfit( entity )
}

class USSR_Infantry_Cowboy_01 extends USSR_Infantry_Basic_01
{
	constructor( )
	{
		USSR_Infantry_Basic_01.constructor( )
	}
	
	function DebugTypeName( )
		return "USSR_Infantry_Cowboy_01"
		
	function OnSpawn( )
	{		
		USSR_Infantry_Basic_01.OnSpawn( )
	}
	function GetHelmet( ) 
	{
		return "Art/Characters/Red/russian_cowboyhat.mshml"
	}
	
}
