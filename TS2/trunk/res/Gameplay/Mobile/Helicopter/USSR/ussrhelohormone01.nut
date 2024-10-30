sigimport "Gameplay/Mobile/Helicopter/Common/HoverVehicleLogic.nut"
sigimport "Gameplay/Mobile/Helicopter/USSR/ussrhelotransport01_momap.nut"
sigimport "Gui/Textures/WaveIcons/USSR/vehicle_helihormone_g.png"
sigimport "Anims/Vehicles/Red/helicopter_hormone/hormone.anipk"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Helo_Hormone( )
}


class USSR_Helo_Hormone extends HoverVehicleLogic
{
	constructor( )
	{
		HoverVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USSR_Helo_Hormone"

	function OnSpawn( )
	{	
		
		HoverVehicleLogic.OnSpawn( )
		SetDestroyedEffect( "Helicopter_Explosion" )
	}
	
	function SetMotionMap( ) 
		Animatable.MotionMap = USSRHeloHormoneMoMap( this )
	
}

class USSRHeloHormoneMoMap extends USSRHeloTransport01MoMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Red/helicopter_hormone/hormone.anipk" )
	}
}