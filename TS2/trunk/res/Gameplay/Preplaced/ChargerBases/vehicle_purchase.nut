
sigimport "Gameplay/Preplaced/Breakables/generic_animated_goal.goaml"
sigimport "anims/bases/blue/mp_generator/mp_generator.anipk"


sigexport function EntityOnCreate( entity )
{
	entity.Logic = BatteryChargerLogic( )
	entity.Logic.Animatable.MotionMap = VehiclePurchaseMoMap( )	
	entity.Logic.GoalDriven.MasterGoal = GenericAnimatedBreakableGoal( this, { } )
}


class VehiclePurchaseMoMap extends Anim.MotionMap
{
	animPack = null
	
	constructor( )
	{
		Anim.MotionMap.constructor( )
		animPack = GetAnimPack( "anims/bases/blue/mp_generator/mp_generator.anipk" )
	}

	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "idle" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		
		track.Push( Stack )
	}

	function ReApplyOneShot( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "deploy" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		track.Push( Stack )
		
		return track.Anim.OneShotLength - 0.2
	}
}