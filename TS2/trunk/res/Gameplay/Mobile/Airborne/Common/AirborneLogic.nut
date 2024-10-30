sigimport "Gameplay/Mobile/Airborne/Common/AirborneMoMap.nut"
sigimport "Gameplay/mobile/common/MobileGoals.goaml"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = AirborneVehicleLogic( )
}

class AirborneVehicleLogic extends AirborneLogic
{
	constructor( )
	{
		AirborneLogic.constructor( )
	}

	function DebugTypeName( )
		return "AirborneVehicleLogic"

	function OnSpawn( )
	{
		ConTrailTracerName = "CON_TRAIL_1"
		SetMotionMap( )
		
		AirborneLogic.OnSpawn( )
		
		SetMasterGoal( )

		Animatable.OnSpawn( )
		
		FindHitpointLinkedChildren( )
	}

	// overrideable methods (i.e., these are defined as separate functions so they can be overridden by derived types)

	function SetMotionMap( )
		Animatable.MotionMap = AirborneMotionMap( this )

	function SetMasterGoal( )
		GoalDriven.MasterGoal = MobileMasterGoal( this, { } )
		
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
	
	function CargoDropSpawn( index )
	{
	}
}
