sigimport "Gameplay/Mobile/Helicopter/Common/HoverVehicleLogic.nut"
sigimport "Gameplay/Mobile/Helicopter/USSR/ussrhelotransport01_momap.nut"
sigimport "gameplay/characters/infantry/ussr/infantry_elite_01.sigml"
sigimport "gui/textures/waveicons/ussr/infantry_lvl2_g.png"
sigimport "gui/textures/waveicons/ussr/vehicle_helitransport_02_g.png"


sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Helo_Transport_01( )
}


class USSR_Helo_Transport_01 extends HoverVehicleLogic
{
	constructor( )
	{
		HoverVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USSR_Helo_Transport_01"

	function OnSpawn( )
	{	
		
		AddCargo( "USSR_Fast_Ropers" )
		HoverVehicleLogic.OnSpawn( )
		SetDestroyedEffect( "Helicopter_Explosion" )
	}
	
	function SetMotionMap( ) 
		Animatable.MotionMap = USSRHeloTransport01MoMap( this )
	
}
