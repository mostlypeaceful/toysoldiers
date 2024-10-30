
sigimport "gameplay/preplaced/goals/common/base.nut"
sigimport "gameplay/preplaced/goals/common/goalboxgoals.goaml"
sigimport "Anims/preplaced/shared/goalbox/goalbox.anipk"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = BreakableGoalBoxLogic( )
	GameApp.CurrentLevel.RegisterGoalBox( entity )
}

class BreakableGoalBoxLogic extends GoalBoxBaseLogic
{
	constructor( )
	{
		GoalBoxBaseLogic.constructor( )
	}

	function OnSpawn( )
	{
		SetMotionMap( )
		SetMasterGoal( )
		GoalBoxBaseLogic.OnSpawn( )
	}	
	
	function SetMotionMap( ) 	Animatable.MotionMap = GoalBoxMoMap( )	
	function SetMasterGoal( ) 	GoalDriven.MasterGoal = GoalBoxGoals( this, { } )

	function DebugTypeName( )
		return "BreakableGoalBoxLogic"
}


class GoalBoxMoMap extends Anim.MotionMap
{
	animPack = null
	
	constructor( )
	{
		Anim.MotionMap.constructor( )
		animPack = GetAnimPack( "Anims/preplaced/shared/goalbox/goalbox.anipk" )
	}

	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "idle" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		track.Push( Stack )
	}
	
	function Destroy( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "antonov_death" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		track.Push( Stack )
	}
}
