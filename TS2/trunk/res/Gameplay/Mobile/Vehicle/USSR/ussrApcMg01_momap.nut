sigimport "Gameplay/Mobile/Common/mobilemomap.nut"
sigimport "Anims/Vehicles/Red/apc_btr60/apc_btr60.anipk"

class USSR_APCMG01MoMap extends MobileMotionMap
{	
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Red/apc_btr60/apc_btr60.anipk" )
	}
	
	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "idle" )
		track.BlendIn = 4.0
		track.BlendOut = 0.0
		
		track.Push( Stack )
	}
	
	function Forward( params )
	{	
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "forward" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		
		track.Push( Stack )		
	}
	
	function CargoBegin( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "deploy" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		track.Push( Stack )

		return (track.Anim.OneShotLength)
	}
	
	function CargoIdle( params )
	{
		/*local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "hover" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		
		track.Push( Stack )*/
	}
	
	function CargoEnd( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "retract" )
		track.BlendIn = 0.2
		track.BlendOut = 0.01

		track.Flags = ANIM_TRACK_CLAMP_TIME		
		track.Push( Stack )

		return (track.Anim.OneShotLength)
	}
}
