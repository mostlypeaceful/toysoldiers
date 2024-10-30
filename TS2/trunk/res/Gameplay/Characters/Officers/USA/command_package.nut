sigimport "Gameplay/characters/officers/usa/rambo_package_goals.goaml"
sigimport "Anims/Characters/Blue/Commando/Commando_package.anipk"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = RamboPackageLogic( )
}


class RamboPackageLogic extends AnimatedBreakableLogic
{		
	constructor( )
	{
		AnimatedBreakableLogic.constructor( )
	}

	function DebugTypeName( )
		return "RamboPackageLogic"

	function OnSpawn( )
	{				
		SetMotionMap( )
		SetMasterGoal( )

		TakesDamage = 0

		AnimatedBreakableLogic.OnSpawn( )
	}	
	
	function SetMotionMap( ) Animatable.MotionMap = RamboPackageMoMap( )	
	function SetMasterGoal( ) GoalDriven.MasterGoal = RamboPackageGoals( this, { } )
}


class RamboPackageMoMap extends Anim.MotionMap
{
	animPack = null
	
	constructor( )
	{
		Anim.MotionMap.constructor( )
		animPack = GetAnimPack( "Anims/Characters/Blue/Commando/Commando_package.anipk" )
	}

	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "enter_fall" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		
		track.Push( Stack )
	}

	function ReApply( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "enter_land" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		track.Push( Stack )
		
		return track.ScaledOneShotLength
	}
}