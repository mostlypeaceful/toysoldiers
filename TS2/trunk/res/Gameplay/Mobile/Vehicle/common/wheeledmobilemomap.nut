sigimport "Gameplay/mobile/common/mobilemomap.nut"

class VehicleMotionMap extends MobileMotionMap
{
	function TankTreads( params )
	{	
		local track = Anim.TankTreadAnimTrack( )
		track.LeftAnim = animPack.Find( "left_tread" )
		track.RightAnim = animPack.Find( "right_tread" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.Vehicle = Logic
		
		track.Push( Stack )
	}
}


class TankVehicleMotionMap extends VehicleMotionMap
{
	constructor( logic, path )
	{		
		VehicleMotionMap.constructor( logic )
		animPack = GetAnimPack( path )
	}
	
	function SetAnimPack( )
	{
		// INTENITONALLY LEFT EMPTY, INTIALIZED IN CONSTRUCTOR
	}

	function Forward( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "forward" )
		track.BlendIn = 0.0
		track.BlendOut = 0.0
		
		track.Push( Stack )
		
		TankTreads( params )
	}
	
	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "idle" )
		track.BlendIn = 0.0
		track.BlendOut = 0.0
		
		track.Push( Stack )
		
		TankTreads( params )
	}
}