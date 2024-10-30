sigimport "Gameplay/Mobile/Helicopter/Common/HoverMoMap.nut"
sigimport "Gameplay/mobile/common/MobileGoals.goaml"

class HoverVehicleLogic extends HoverLogic
{
	constructor( )
	{
		HoverLogic.constructor( )
	}

	function DebugTypeName( )
		return "HoverVehicleLogic"

	function OnSpawn( )
	{
		SetMotionMap( )
		
		HoverLogic.OnSpawn( )
		
		SetMasterGoal( )

		Animatable.OnSpawn( )
		
		FindHitpointLinkedChildren( )
	}

	// overrideable methods (i.e., these are defined as separate functions so they can be overridden by derived types)
	function SetMotionMap( ) Animatable.MotionMap = HoverMotionMap( this )
	function SetMasterGoal( ) GoalDriven.MasterGoal = MobileMasterGoal( this, { } )
		
	function OnReachedEndOfPath( ) // called from Reached End of Path event in goaml
	{
		if( GoalBoxCheck( ) )
			ReachedEnemyGoal( )
			
		ShutDown( 1 )
		ShutDown( 0 )
		OwnerEntity.Delete( )
	}
	
	function SpecialEntranceStartStop( startStop )
	{
		
	}
	
	function CargoDropSpawn( table )
	{
		if( ::GameApp.CurrentLevelLoadInfo.MapType == MAP_TYPE_MINIGAME )
		{
			::GameApp.CurrentLevel.MiniGameCargoDropSpawn( table.Unit, table.Index )
		}
	}
}
