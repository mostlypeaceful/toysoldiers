sigimport "Gameplay/Mobile/Helicopter/Common/HoverVehicleLogic.nut"
sigimport "Gameplay/Turrets/Common/CrewmanMoMap.nut"
sigimport "gui/textures/waveicons/usa/vehicle_heligunner_g.png"
sigimport "Anims/Vehicles/Blue/jetpack/jetpack.anipk"


sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_JetPack( )
}


class USA_JetPack extends HoverVehicleLogic
{
	constructor( )
	{
		HoverVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USA_JetPack"

	function OnSpawn( )
	{		
		HoverVehicleLogic.OnSpawn( )
		SetDestroyedEffect( "Helicopter_Explosion" )
		TakesDamage = 0
	}
	
	function SetMotionMap( ) 
		Animatable.MotionMap = USA_JetPackMoMap( this )
}

class USA_JetPackMoMap extends HoverMotionMap
{	
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Blue/jetpack/jetpack.anipk" )
	}

	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "idle" )
		track.BlendIn = 0.0
		track.BlendOut = 0.0
		
		track.Push( Stack )
	}

	function Startup( params )
	{
		Forward( params )
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "takeoff" )
		track.BlendIn = 0.0
		track.BlendOut = 0.1
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		track.Push( Stack )
		
		return track.Anim.OneShotLength - 0.4
	}
	
	function ShutDown( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "exit" )
		track.BlendIn = 0.2
		track.BlendOut = 0.1
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		track.Push( Stack )
		
		return track.Anim.OneShotLength - 0.3
	}
	
	function Forward( params )
	{	
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "forward" )
		track.BlendIn = 0.0
		track.BlendOut = 0.0
		
		track.Push( Stack )		
	}
}
