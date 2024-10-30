sigimport "Gameplay/Preplaced/Breakables/generic_animated_goal.goaml"
sigimport "Anims/Preplaced/Shared/lavalamp/lavalamp.anipk"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = LavaLampLogic( )
}


class LavaLampLogic extends AnimatedBreakableLogic
{		
	constructor( )
	{
		AnimatedBreakableLogic.constructor( )
	}

	function DebugTypeName( )
		return "LavaLampLogic"

	function OnSpawn( )
	{				
		SetMotionMap( )
		SetMasterGoal( )

		AnimatedBreakableLogic.OnSpawn( )
	}	
	
	function SetMotionMap( ) Animatable.MotionMap = LavaLampMoMap( )	
	function SetMasterGoal( ) GoalDriven.MasterGoal = GenericAnimatedBreakableGoal( this, { } )
}


class LavaLampMoMap extends Anim.MotionMap
{
	animPack = null
	
	constructor( )
	{
		Anim.MotionMap.constructor( )
		animPack = GetAnimPack( "Anims/Preplaced/Shared/lavalamp/lavalamp.anipk" )
	}

	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "idle" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.StartTime = ObjectiveRand.Float( 0.0, track.Anim.OneShotLength )
		track.Push( Stack )
	}
}