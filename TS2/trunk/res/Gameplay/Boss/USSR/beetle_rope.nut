sigimport "Gameplay/Preplaced/Breakables/generic_animated_goal.goaml"
sigimport "Anims/Vehicles/Attachments/rope/rope.anipk"


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
	entity.Logic = BeetleRopeLogic( CreateDeliveryBaseMoMap( entity ))
}


class BeetleRopeLogic extends AnimatedBreakableLogic
{		
	hasFlownOff = 0
	moMapToSet = null
	
	constructor( moMap )
	{	
		if( !moMap )
			moMap = BeetleRopeMoMap_A

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


class BeetleRopeMoMap_A extends Anim.MotionMap
{
	animPack = null
	
	constructor( )
	{
		Anim.MotionMap.constructor( )
		animPack = GetAnimPack( "Anims/Vehicles/Attachments/rope/rope.anipk" )
	}

	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "beetle_flyin_ropea" )
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
		track.Anim = animPack.Find( "idle" )
		track.BlendIn = 0.2
		track.BlendOut = 0.2
		track.Push( Stack )
		
		//print( "helo reapply" )
		
		return track.Anim.OneShotLength - track.BlendOut
	}
}

class BeetleRopeMoMap_B extends Anim.MotionMap
{
	animPack = null
	
	constructor( )
	{
		Anim.MotionMap.constructor( )
		animPack = GetAnimPack( "Anims/Vehicles/Attachments/rope/rope.anipk" )
	}

	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "beetle_flyin_ropeb" )
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
		track.Anim = animPack.Find( "idle" )
		track.BlendIn = 0.2
		track.BlendOut = 0.2
		track.Push( Stack )
		
		//print( "helo reapply" )
		
		return track.Anim.OneShotLength - track.BlendOut
	}
}

class BeetleRopeMoMap_C extends Anim.MotionMap
{
	animPack = null
	
	constructor( )
	{
		Anim.MotionMap.constructor( )
		animPack = GetAnimPack( "Anims/Vehicles/Attachments/rope/rope.anipk" )
	}

	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "beetle_flyin_ropec" )
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
		track.Anim = animPack.Find( "idle" )
		track.BlendIn = 0.2
		track.BlendOut = 0.2
		track.Push( Stack )
		
		//print( "helo reapply" )
		
		return track.Anim.OneShotLength - track.BlendOut
	}
}


class BeetleRopeMoMap_D extends Anim.MotionMap
{
	animPack = null
	
	constructor( )
	{
		Anim.MotionMap.constructor( )
		animPack = GetAnimPack( "Anims/Vehicles/Attachments/rope/rope.anipk" )
	}

	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "beetle_flyin_roped" )
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
		track.Anim = animPack.Find( "idle" )
		track.BlendIn = 0.2
		track.BlendOut = 0.2
		track.Push( Stack )
		
		//print( "helo reapply" )
		
		return track.Anim.OneShotLength - track.BlendOut
	}
}