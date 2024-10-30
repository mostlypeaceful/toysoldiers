sigimport "Gameplay/Characters/Common/baseMoMap.nut"
sigimport "Gameplay/characters/common/InfantryAI.goaml"
sigimport "Effects/Entities/Soldiers/dust_impact.sigml"
sigimport "Effects/Entities/Soldiers/fire_trail.sigml"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = SoldierLogic( )
}

// Base soldier logic.  If specific logic is required for basic/elite/etc infantry, then create it and derive from this

class SoldierLogic extends CharacterLogic
{	
	keepOnMap = false
	
	constructor( )
	{
		CharacterLogic.constructor( )
		
		SingleShotWeaponID = "USSR_INFANTRY_RIFLE" 
	}
	
	function DebugTypeName( )
		return "SoldierLogic"
		
	function OnSpawn( )
	{
		SetMotionMap( ) // before on spawn
		
		CharacterLogic.OnSpawn( )
		
		SetMasterGoal( ) // after on spawn

		PostGoalSet( ) // after goals
	}

	// overrideable methods (i.e., these are defined as separate functions so they can be overridden by derived types)

	function SetMotionMap( )
		Animatable.MotionMap = SoldierMotionMap( this )

	function SetMasterGoal( )
	{
		GoalDriven.MasterGoal = InfantryGoal( this, { } )
	}
}


