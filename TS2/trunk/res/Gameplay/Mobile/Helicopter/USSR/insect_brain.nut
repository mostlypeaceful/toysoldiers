sigimport "Gameplay/Mobile/Helicopter/Common/HoverVehicleLogic.nut"
sigimport "Gameplay/Mobile/Common/MobileGoals.goaml"
sigimport "Gameplay/mobile/helicopter/ussr/fly_goals.goaml"
sigimport "Gameplay/Mobile/Helicopter/USSR/ussr_fly_momap.nut"
sigimport "gui/textures/waveicons/usa/vehicle_helitransport_g.png"


sigexport function EntityOnCreate( entity )
{
	entity.Logic = Insect_Brain_01( )
}

class Insect_Brain_01 extends HoverVehicleLogic
{
	constructor( )
	{
		HoverVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "Insect_Brain"

	function OnSpawn( )
	{	
		HoverVehicleLogic.OnSpawn( )
		
		UseDefaultEndTransition = 0		
		SetDestroyedEffect( "InsectSplat" )
	}
	
	function SetMotionMap( ) 
		Animatable.MotionMap = USSRFly01MoMap( this )
	
	function SetMasterGoal( )
		GoalDriven.MasterGoal = Fly_MainGoal( this, { } )	
	
}
