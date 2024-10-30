
sigimport "gameplay/preplaced/breakables/flag.nut"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = WeaponsCaseFlagLogic( )
	::GameApp.CurrentLevel.RegisterNamedObject( entity )
}


class WeaponsCaseFlagLogic extends FlagLogic
{		
	constructor( )
	{
		FlagLogic.constructor( )
	}

	function DebugTypeName( )
		return "WeaponsCaseFlagLogic"

	function OnSpawn( )
	{				
		SetMotionMap( )
		SetMasterGoal( )

		AnimatedBreakableLogic.OnSpawn( )
	}	
	
	function SetMotionMap( ) Animatable.MotionMap = WeaponsCaseFlagMoMap( )	
	function SetMasterGoal( ) GoalDriven.MasterGoal = GenericAnimatedBreakableGoal( this, { } )
}


class WeaponsCaseFlagMoMap extends FlagMoMap
{
	animPack = null
	
	constructor( )
	{
		FlagMoMap.constructor( )
	}

	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "weaponscase_wave" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.Push( Stack )
	}
}