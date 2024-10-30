sigimport "Gameplay/Preplaced/Breakables/generic_animated_goal.goaml"
sigimport "Anims/Generic/windup/windup.anipk"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = WhackLogic( )
}


class WhackLogic extends AnimatedBreakableLogic
{		
	constructor( )
	{
		AnimatedBreakableLogic.constructor( )
	}

	function DebugTypeName( )
		return "WhackLogic"

	function OnSpawn( )
	{				
		SetMotionMap( )
		SetMasterGoal( )

		AnimatedBreakableLogic.OnSpawn( )
		
		local pickUpLoc = OwnerEntity.GetEnumValue( ENUM_PICKUPS )
		if( pickUpLoc != -1 )
			Pickup = pickUpLoc
		
		HandleLogicEvent( LogicEvent.Construct( GAME_EVENT_REAPPLY_ONESHOT_MOTION_STATE ) )
	}	
	
	function SetMotionMap( ) Animatable.MotionMap = WhackMoMap( )	
	function SetMasterGoal( ) GoalDriven.MasterGoal = GenericAnimatedBreakableGoal( this, { } )
}


class WhackMoMap extends Anim.MotionMap
{
	animPack = null
	shown = 0
	
	constructor( )
	{
		shown = 0
	
		Anim.MotionMap.constructor( )
		animPack = GetAnimPack( "Anims/Generic/windup/windup.anipk" )
	}

	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "whac_idle" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.TimeScale = 0.0
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		track.Push( Stack )
		
		if( shown == 1 )
			Logic.OwnerEntity.Delete( )
	}

	function ReApplyOneShot( params )
	{
		shown = 1
		
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "whac_idle" )
		track.BlendIn = 0.2
		track.BlendOut = 0.2
		track.Flags = ANIM_TRACK_CLAMP_TIME
			
		track.Push( Stack )
		
		return track.Anim.OneShotLength - track.BlendOut
	}
}