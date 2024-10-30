sigimport "Gameplay/Preplaced/Breakables/generic_animated_goal.goaml"
sigimport "Anims/Generic/windup/windup.anipk"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = DiscLogic( )
	GameApp.CurrentLevel.AddTarget( entity )
}


class DiscLogic extends AnimatedBreakableLogic
{		
	constructor( )
	{
		AnimatedBreakableLogic.constructor( )
	}

	function DebugTypeName( )
		return "DiscLogic"

	function OnSpawn( )
	{				
		SetMotionMap( )
		SetMasterGoal( )

		AnimatedBreakableLogic.OnSpawn( )
	}	
	
	function SetMotionMap( ) Animatable.MotionMap = DiscMoMap( )	
	function SetMasterGoal( ) GoalDriven.MasterGoal = GenericAnimatedBreakableGoal( this, { } )
}


class DiscMoMap extends Anim.MotionMap
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
		track.Anim = animPack.Find( "idle_1rps" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_RESUME_TIME
		
		local throttle = Logic.OwnerEntity.GetEnumValue( ENUM_THROTTLE )
		if( throttle == -1 ) 
			throttle = 1.0
		else
			 throttle = throttle.tofloat( ) / THROTTLE_10
		
		if( ObjectiveRand.Bool( ) )
			throttle *= -1.0
			
		track.TimeScale = 0.20 * throttle
		
		track.Push( Stack )
	}
	
	function ReApply( params )
	{
		Idle( params )
	}
}