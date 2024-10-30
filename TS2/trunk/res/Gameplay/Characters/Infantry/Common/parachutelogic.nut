sigimport "Gameplay/Characters/Infantry/Common/parachute.goaml"
sigimport "Anims/Characters/shared/parachute/parachute.anipk"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = ParachuteLogic( )
}

class ParachuteLogic extends VehiclePassengerLogic
{
	ownerCharacter = null
	hasLanded = 0
	
	constructor( )
	{
		VehiclePassengerLogic.constructor( )
		ownerCharacter = null
		hasLanded = 0
	}
	
	function DebugTypeName( )
		return "ParachuteLogic"
		
	function OnSpawn( )
	{
		VehiclePassengerLogic.OnSpawn( )
		ownerCharacter = OwnerEntity.Parent.Logic		
		SetMotionMap( )
		GoalDriven.MasterGoal = ParachuteGoal( this, { } )
		GoalDriven.OnSpawn( this ) //this is needed to activate the goal
		Animatable.OnSpawn( ) //and eliminate the anim pop
	}
	
	function SetMotionMap( )
	{
		Animatable.MotionMap = ParachuteMomap( )
	}
}

class ParachuteMomap extends Anim.MotionMap
{
	animPack = null
	
	constructor()
	{
		Anim.MotionMap.constructor()
		animPack = GetAnimPack("Anims/Characters/shared/parachute/parachute.anipk")		
	}
	
	function Idle( params )
	{
		if( Logic.hasLanded )
		{
			Logic.OwnerEntity.Delete( )
		}
		
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "idle" )
		track.BlendIn = 0.0
		track.BlendOut = 0.0
		track.Tag = "chuteIdle"
	
		track.Push( Stack )
	}
	
	function ChuteIdle( params )
	{
		if( Logic.hasLanded )
		{
			Logic.OwnerEntity.Delete( )
		}
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "chute_idle" )
		track.BlendIn = 0.1
		track.BlendOut = 0.0
		
		track.Push( Stack )
	}
	
	function Deploy( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "deploy" )
		track.BlendIn = 0.1
		track.BlendOut = 0.01
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		track.Push( Stack )
		
		return track.Anim.OneShotLength - 0.2
	}
	
	function Land( params )
	{
		Stack.RemoveTracksWithTag( "chuteIdle" );
		Logic.hasLanded = 1
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "land" )
		track.BlendIn = 0.1
		track.BlendOut = 0.01
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		track.Push( Stack )
		
		return track.Anim.OneShotLength - 0.3
	}
}