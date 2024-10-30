sigimport "Gameplay/Characters/Infantry/Common/InfantryLogic.nut"


class BaseOfficerLogic extends UserControllableCharacterLogic
{
	keepOnMap = false
	didSpecialEntrance = false
	
	constructor( )
	{
		UserControllableCharacterLogic.constructor( )
		didSpecialEntrance = false
		keepOnMap = false
	}
	
	function DebugTypeName( )
		return "BaseOfficerLogic"
		
	function OnSpawn( )
	{
		keepOnMap = true
		SetPersonalityType( PERSONALITY_TYPE_COMMANDO )
		
		HasPackage = 1
		
		IKLegs.RayLength = 1.0
		IKLegs.ExtraRay = 0.075
		IKLegs.FootHeight = 0.175
		
		SetMotionMap( ) // before on spawn
		UserControllableCharacterLogic.OnSpawn( )
		SetMasterGoal( ) // after on spawn
		PostGoalSet( ) // after goals
		
		SetStateOverride( 0, 0 )
	}
	
	function Outfit( entity )
	{
		//already have head and helmet
	}
	
	function RealDeath( )
	{
		ExplodeIntoParts( )
		OwnerEntity.Delete( )	
	}
	
	function SetMotionMap( )
		Animatable.MotionMap = SoldierMotionMap( this )

	function SetMasterGoal( )
	{
		GoalDriven.MasterGoal = InfantryGoal( this, { } )
	}
}
