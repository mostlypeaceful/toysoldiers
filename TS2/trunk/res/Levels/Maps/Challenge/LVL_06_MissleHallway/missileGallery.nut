sigimport "Gameplay/Preplaced/Breakables/generic_animated_goal.goaml"
sigimport "Anims/preplaced/shared/shootinggallery/shootinggallery.anipk"

sigexport function EntityOnCreate( entity )
{
	entity.Logic =  MissileGalleryLogic( )
}


class  MissileGalleryLogic extends AnimatedBreakableLogic
{		
	constructor( )
	{
		AnimatedBreakableLogic.constructor( )
	}

	function DebugTypeName( )
		return "ShootingGalleryLogic"

	function OnSpawn( )
	{				
		SetMotionMap( )
		SetMasterGoal( )
		TakesDamage = 0

		AnimatedBreakableLogic.OnSpawn( )
		
		GameApp.CurrentLevel.RegisterNamedObject( OwnerEntity )
	}	
	
	function SetMotionMap( ) Animatable.MotionMap = MissileGalleryMoMap( )	
	function SetMasterGoal( ) GoalDriven.MasterGoal = GenericAnimatedBreakableGoal( this, { } )
}


class MissileGalleryMoMap extends Anim.MotionMap
{
	animPack = null
	
	constructor( )
	{
		Anim.MotionMap.constructor( )
		animPack = GetAnimPack( "Anims/preplaced/shared/shootinggallery/shootinggallery.anipk" )
	}

	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "idle" )
		track.BlendIn = 0.0
		track.BlendOut = 0.0
		//track.StartTime = ObjectiveRand.Float( 0.0, track.anim.LoopingLength )
		track.StartTime = ObjectiveRand.Float( 0.0, 5.0 )
		
		track.Push( Stack )
	}
	
	function ReApply( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "run" )
		track.BlendIn = 0.0
		track.BlendOut = 0.0
		//track.Flags = ANIM_TRACK_CLAMP_TIME
		track.StartTime = ObjectiveRand.Float( 5.0, track.anim.OneShotLength )
		
		track.Push( Stack )
	}
}