sigimport "Gameplay/Preplaced/Breakables/generic_animated_goal.goaml"
sigimport "Anims/Generic/windup/windup.anipk"
sigimport "Effects/Entities/Misc/floating_star_pop.sigml"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = PlaneTargetLogic( )
	entity.Logic.Pickup = PICKUPS_PLANETARGETS
}


class PlaneTargetLogic extends AnimatedBreakableLogic
{		
	constructor( )
	{
		AnimatedBreakableLogic.constructor( )
	}

	function DebugTypeName( )
		return "PlaneTargetLogic"

	function OnSpawn( )
	{				
		SetMotionMap( )
		SetMasterGoal( )

		AnimatedBreakableLogic.OnSpawn( )
		
		TakesDamage = 0
		UseDefaultEndTransition = 1
	}
	
	function OnPickedUp( )
	{
		local pop = RootEntity.SpawnChild( "Effects/Entities/Misc/floating_star_pop.sigml" )
		pop.SetPosition( OwnerEntity.GetPosition( ) )
	}
	
	function SetMotionMap( ) Animatable.MotionMap = PlaneTargetMoMap( )	
	function SetMasterGoal( ) GoalDriven.MasterGoal = GenericAnimatedBreakableGoal( this, { } )
}


class PlaneTargetMoMap extends Anim.MotionMap
{
	animPack = null
	
	constructor( )
	{
		Anim.MotionMap.constructor( )
		animPack = GetAnimPack( "Anims/Generic/windup/windup.anipk" )
	}

	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "battery_idle" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		
		track.Push( Stack )
	}
}
