sigimport "Gameplay/Preplaced/Breakables/generic_animated_goal.goaml"
sigimport "Anims/Preplaced/Shared/oilpump_horsehead/oilpump_horsehead.anipk"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = OilpumpLogic( )
}


class OilpumpLogic extends AnimatedBreakableLogic
{		
	constructor( )
	{
		AnimatedBreakableLogic.constructor( )
	}

	function DebugTypeName( )
		return "OilpumpLogicLogic"

	function OnSpawn( )
	{				
		SetMotionMap( )
		SetMasterGoal( )

		AnimatedBreakableLogic.OnSpawn( )
	}	
	
	function SetMotionMap( ) Animatable.MotionMap = OilpumpMoMap( )	
	function SetMasterGoal( ) GoalDriven.MasterGoal = GenericAnimatedBreakableGoal( this, { } )
}


class OilpumpMoMap extends Anim.MotionMap
{
	animPack = null
	
	constructor( )
	{
		Anim.MotionMap.constructor( )
		animPack = GetAnimPack( "Anims/Preplaced/Shared/oilpump_horsehead/oilpump_horsehead.anipk" )
	}

	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "idle" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.StartTime = ObjectiveRand.Float( 0.0, track.Anim.OneShotLength )
		track.TimeScale = ObjectiveRand.Float( 0.80, 1.0 )
		track.Push( Stack )
	}
}