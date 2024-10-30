sigimport "Art/units/vehicles/attachments/ropes_goals.goaml"
sigimport "Anims/vehicles/attachments/rope/rope.anipk"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = ::RopeSlideLogic( )
}

class RopeSlideLogic extends AnimatedBreakableLogic
{		
	constructor( )
	{
		::AnimatedBreakableLogic.constructor( )
	}

	function DebugTypeName( )
		return "KeyLogic"

	function OnSpawn( )
	{				
		SetMotionMap( )
		SetMasterGoal( )
		
		TakesDamage = 0

		::AnimatedBreakableLogic.OnSpawn( )
	}	
	
	function SetMotionMap( )
	{
		Animatable.MotionMap = ::RopeSlideMoMap( )
	}
	function SetMasterGoal( )
	{
		GoalDriven.MasterGoal = ::Rope_Slide_Goals( this, { } )
	}
}


class RopeSlideMoMap extends Anim.MotionMap
{
	animPack = null
	
	constructor( )
	{
		::Anim.MotionMap.constructor( )
		animPack = GetAnimPack( "Anims/vehicles/attachments/rope/rope.anipk" )
	}

	function Idle( params )
	{
		local track = ::Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "retract" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.StartTime = track.Anim.OneShotLength - 0.1
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		track.Push( Stack )
	}

	function Retract( params )
	{
		local track = ::Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "retract" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		track.Push( Stack )
		return track.Anim.OneShotLength
	}
	
	function DeployedIdle( params )
	{
		local track = ::Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "idle" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		
		track.Push( Stack )
	}
	
	function Drop( params )
	{		
		local track = ::Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "drop" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		track.Push( Stack )
		return track.Anim.OneShotLength
	}
	
	function Deploy( params )
	{
		local track = ::Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "deploy" )
		track.BlendIn = 0.2
		track.BlendOut = 0.2
		
		track.Push( Stack )
		return track.Anim.OneShotLength
	}
}