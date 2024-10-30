sigimport "Gameplay/Preplaced/Breakables/generic_animated_goal.goaml"
sigimport "Anims/Vehicles/Red/helicopter_mi8hip/mi8hip.anipk"


function CreateDeliveryBaseMoMap( entity )
{
	local moMapName = entity.GetName( )
	try
	{
		local moMapClass = rawget( moMapName )
		local moMapInstance = moMapClass.instance( )
		moMapInstance.constructor( )
		return moMapInstance
	}
	catch( e ) { }

	print( "Couldn't determine what sort of motion map to make for ArtillerySoldier: " + entity )
	return null
}

sigexport function EntityOnCreate( entity )
{
	entity.Logic = BeetleHeloTransportLogic( CreateDeliveryBaseMoMap( entity ))
}


class BeetleHeloTransportLogic extends AnimatedBreakableLogic
{		
	hasFlownOff = 0
	moMapToSet = null
	
	constructor( moMap )
	{	
		if( !moMap )
			moMap = BeetleHeloTransportMoMap_A

		moMapToSet = moMap
		
		hasFlownOff = 0
		AnimatedBreakableLogic.constructor( )
	}

	function DebugTypeName( )
		return "BeetleHeloTransportLogic"

	function OnSpawn( )
	{				
		SetMasterGoal( )
		
		ApplyRefFrame = 1
		Animatable.MotionMap = moMapToSet
		AnimatedBreakableLogic.OnSpawn( )
	}	
	
	function SetMasterGoal( ) GoalDriven.MasterGoal = GenericAnimatedBreakableGoal( this, { } )
}


class BeetleHeloTransportMoMap_A extends Anim.MotionMap
{
	animPack = null
	
	constructor( )
	{
		Anim.MotionMap.constructor( )
		animPack = GetAnimPack( "Anims/Vehicles/Red/helicopter_mi8hip/mi8hip.anipk" )
	}

	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "beetle_flyin_heloa" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		//track.StartTime = ObjectiveRand.Float( 0.0, track.Anim.OneShotLength )
		track.Push( Stack )
		
		if( Logic.hasFlownOff )
		{
			//print( "helo delete" )
			Logic.OwnerEntity.Delete( )
		}
	}

	function ReApplyOneShot( params )
	{
		Logic.hasFlownOff = 1
		Logic.ApplyRefFrame = 1
		if( !is_null( ::GameApp.CurrentLevel ) )
			Logic.OwnerEntity.Reparent( ::GameApp.CurrentLevel.OwnerEntity )
		
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "fly_off" )
		track.BlendIn = 0.2
		track.BlendOut = 0.2
		track.Push( Stack )
		
		//print( "helo reapply" )
		
		return track.Anim.OneShotLength - track.BlendOut
	}
}

class BeetleHeloTransportMoMap_B extends Anim.MotionMap
{
	animPack = null
	
	constructor( )
	{
		Anim.MotionMap.constructor( )
		animPack = GetAnimPack( "Anims/Vehicles/Red/helicopter_mi8hip/mi8hip.anipk" )
	}

	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "beetle_flyin_helob" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.Push( Stack )
		
		if( Logic.hasFlownOff )
		{
			//print( "helo delete" )
			Logic.OwnerEntity.Delete( )
		}
	}

	function ReApplyOneShot( params )
	{
		Logic.hasFlownOff = 1
		
		if( !is_null( ::GameApp.CurrentLevel ) )
			Logic.OwnerEntity.Reparent( ::GameApp.CurrentLevel.OwnerEntity )
		
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "fly_off" )
		track.BlendIn = 0.2
		track.BlendOut = 0.2
		track.Push( Stack )
		
		//print( "helo reapply" )
		
		return track.Anim.OneShotLength - track.BlendOut
	}
}
class BeetleHeloTransportMoMap_C extends Anim.MotionMap
{
	animPack = null
	
	constructor( )
	{
		Anim.MotionMap.constructor( )
		animPack = GetAnimPack( "Anims/Vehicles/Red/helicopter_mi8hip/mi8hip.anipk" )
	}

	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "beetle_flyin_heloc" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.Push( Stack )
		
		if( Logic.hasFlownOff )
		{
			//print( "helo delete" )
			Logic.OwnerEntity.Delete( )
		}
	}

	function ReApplyOneShot( params )
	{
		Logic.hasFlownOff = 1
		
		if( !is_null( ::GameApp.CurrentLevel ) )
			Logic.OwnerEntity.Reparent( ::GameApp.CurrentLevel.OwnerEntity )
		
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "fly_off" )
		track.BlendIn = 0.2
		track.BlendOut = 0.2
		track.Push( Stack )
		
		//print( "helo reapply" )
		
		return track.Anim.OneShotLength - track.BlendOut
	}
}
class BeetleHeloTransportMoMap_D extends Anim.MotionMap
{
	animPack = null
	
	constructor( )
	{
		Anim.MotionMap.constructor( )
		animPack = GetAnimPack( "Anims/Vehicles/Red/helicopter_mi8hip/mi8hip.anipk" )
	}

	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "beetle_flyin_helod" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.Push( Stack )
		
		if( Logic.hasFlownOff )
		{
			//print( "helo delete" )
			Logic.OwnerEntity.Delete( )
		}
	}

	function ReApplyOneShot( params )
	{
		Logic.hasFlownOff = 1
		
		if( !is_null( ::GameApp.CurrentLevel ) )
			Logic.OwnerEntity.Reparent( ::GameApp.CurrentLevel.OwnerEntity )
		
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "fly_off" )
		track.BlendIn = 0.2
		track.BlendOut = 0.2
		track.Push( Stack )
		
		//print( "helo reapply" )
		
		return track.Anim.OneShotLength - track.BlendOut
	}
}