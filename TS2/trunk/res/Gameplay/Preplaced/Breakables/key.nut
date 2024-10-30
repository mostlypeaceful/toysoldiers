sigimport "Gameplay/Preplaced/Breakables/generic_animated_goal.goaml"
sigimport "Anims/Generic/windup/windup.anipk"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = KeyLogic( )
}


class KeyLogic extends AnimatedBreakableLogic
{		
	constructor( )
	{
		AnimatedBreakableLogic.constructor( )
	}

	function DebugTypeName( )
		return "KeyLogic"

	function OnSpawn( )
	{				
		SetMotionMap( )
		SetMasterGoal( )

		AnimatedBreakableLogic.OnSpawn( )
	}	
	
	function SetMotionMap( ) Animatable.MotionMap = KeyMoMap( )	
	function SetMasterGoal( ) GoalDriven.MasterGoal = GenericAnimatedBreakableGoal( this, { } )
}


class KeyMoMap extends Anim.MotionMap
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
		track.Anim = animPack.Find( "idle" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		
		track.Push( Stack )
	}
}