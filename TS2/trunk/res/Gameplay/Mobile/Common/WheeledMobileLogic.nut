sigimport "Gameplay/mobile/common/MobileGoals.goaml"
sigimport "gameplay/mobile/vehicle/common/wheeledmobilemomap.nut"

class ScriptWheeledVehicleLogic extends WheeledVehicleLogic
{
	constructor( )
	{
		WheeledVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "ScriptWheeledVehicleLogic"

	function OnSpawn( )
	{
		SetMotionMap( )
		
		WheeledVehicleLogic.OnSpawn( )
		
		SetMasterGoal( )

		Animatable.OnSpawn( )
		
		FindHitpointLinkedChildren( )
	}

	// overrideable methods (i.e., these are defined as separate functions so they can be overridden by derived types)

	function SetMotionMap( )
		Animatable.MotionMap = VehicleMotionMap( this )

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
	
	function CargoDropSpawn( table )
	{
	}
}
