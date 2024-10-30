sigimport "Gameplay/Preplaced/Breakables/generic_animated_goal.goaml"
sigimport "Anims/Generic/windup/windup.anipk"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = BatteryRechargeLogic( )
}


class BatteryRechargeLogic extends AnimatedBreakableLogic
{		
	constructor( )
	{
		AnimatedBreakableLogic.constructor( )
	}

	function DebugTypeName( )
		return "BatteryRechargeLogic"

	function OnSpawn( )
	{				
		SetMotionMap( )
		SetMasterGoal( )

		AnimatedBreakableLogic.OnSpawn( )
		
		TakesDamage = 0
		UseDefaultEndTransition = 1
	}	
	
	function SetMotionMap( ) Animatable.MotionMap = BatteryRechargeMoMap( )	
	function SetMasterGoal( ) GoalDriven.MasterGoal = GenericAnimatedBreakableGoal( this, { } )
}


class BatteryRechargeMoMap extends Anim.MotionMap
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
		track.Anim = animPack.Find( "battery_idle" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		
		track.Push( Stack )
	}
}