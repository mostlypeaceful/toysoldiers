sigimport "Gameplay/Mobile/Helicopter/Common/HoverVehicleLogic.nut"
sigimport "Gameplay/Mobile/Common/MobileGoals.goaml"
sigimport "Gameplay/mobile/helicopter/ussr/fly_goals.goaml"
sigimport "Gameplay/Mobile/Helicopter/USSR/ussr_fly_momap.nut"
sigimport "gui/textures/waveicons/usa/vehicle_helitransport_g.png"


sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Fly_Gold_01( )
}


class USSR_Fly_Gold_01 extends HoverVehicleLogic
{
	constructor( )
	{
		HoverVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USSR_Fly_Gold_01"
		
	function OnSpawn( )
	{	
		HoverVehicleLogic.OnSpawn( )
		SetDestroyedEffect( "GoldenFlyExplode" )
	}
	
	function SetMotionMap( ) 
		Animatable.MotionMap = USSRFly01MoMap( this )
	
	function SetMasterGoal( )
		GoalDriven.MasterGoal = Fly_MainGoal( this, { } )			
}
