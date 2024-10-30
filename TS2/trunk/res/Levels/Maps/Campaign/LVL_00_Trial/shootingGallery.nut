sigimport "Gameplay/Preplaced/Breakables/generic_animated_goal.goaml"
sigimport "Anims/preplaced/shared/shootinggallery/shootinggallery.anipk"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = ShootingGalleryLogic( )
}


class ShootingGalleryLogic extends AnimatedBreakableLogic
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
	
	function SetMotionMap( ) Animatable.MotionMap = ShootingGalleryMoMap( )	
	function SetMasterGoal( ) GoalDriven.MasterGoal = GenericAnimatedBreakableGoal( this, { } )
}


class ShootingGalleryMoMap extends Anim.MotionMap
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
		
		track.Push( Stack )
	}
	
	function ReApply( params )
	{
		print( "gallery apply" )
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "run" )
		track.BlendIn = 1.0
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		track.Push( Stack )
	}
}