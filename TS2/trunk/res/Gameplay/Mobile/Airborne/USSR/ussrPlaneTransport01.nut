sigimport "Gameplay/Mobile/Airborne/Common/Airbornelogic.nut"
sigimport "gameplay/characters/infantry/ussr/infantry_paratrooper_01.sigml"
sigimport "gameplay/mobile/vehicle/ussr/apc_mg_01_on_palette.sigml"
sigimport "gameplay/mobile/vehicle/ussr/tank_medium_01_on_palette.sigml"
sigimport "gameplay/mobile/vehicle/ussr/tank_heavy_01_on_palette.sigml"
sigimport "gameplay/mobile/vehicle/ussr/ifv_01_on_palette.sigml"
sigimport "gui/textures/waveicons/USSR/infantry_lvl1_g.png"
sigimport "gui/textures/waveicons/ussr/vehicle_transport_g.png"
sigimport "Gameplay/Mobile/Airborne/ussr/ussrplanetransport01_momap.nut"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Plane_Transport_01( )
}

class USSR_Plane_Transport_01 extends AirborneVehicleLogic
{
	constructor( )
	{
		AirborneVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USSR_Plane_Transport_01"

	function OnSpawn( )
	{
		AirborneVehicleLogic.OnSpawn( )
		SetDestroyedEffect( "Bomber_Explosion" )
		SetRandomFlyTimeRemaining( 0 )
		AddCargo( "USSR_Transport" )
		AddCargo( "USSR_Tank_Paradrop" )
		AddCargo( "USSR_MTank_Paradrop" )
		AddCargo( "USSR_HTank_Paradrop" )
		AddCargo( "USSR_IFV_Paradrop" )
		DisableEvasion = 1


	}
	
	function SetMotionMap( )
		Animatable.MotionMap = USSRPlaneTransport01MoMap( this )

}
