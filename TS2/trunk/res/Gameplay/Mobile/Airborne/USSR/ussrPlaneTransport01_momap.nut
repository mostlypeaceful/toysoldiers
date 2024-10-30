sigimport "Gameplay/Mobile/Airborne/Common/airbornemomap.nut"
sigimport "Anims/Vehicles/Red/transport_an12cub/an12cub.anipk"

class USSRPlaneTransport01MoMap extends AirborneMotionMap
{	
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Red/transport_an12cub/an12cub.anipk" )
	}

	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "idle" )
		track.BlendIn = 4.0
		track.BlendOut = 0.0
		
		track.Push( Stack )
	}

	function Startup( params )
	{
		/*local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "takeoff" )
		track.BlendIn = 0.2
		track.BlendOut = 0.01
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		track.Push( Stack )
		
		return track.Anim.OneShotLength - 0.4*/
	}
	
	function Forward( params )
	{	
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "forward" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		
		track.Push( Stack )	
			
			
		local track2 = Anim.AirborneAnimTrack( )
		track2.FastLeftAnim = animPack.Find( "maxspeed_turn_left" )
		track2.FastRightAnim = animPack.Find( "maxspeed_turn_right" )
		track2.SlowLeftAnim = animPack.Find( "minspeed_turn_left" )
		track2.SlowRightAnim = animPack.Find( "minspeed_turn_right" )
		track2.BlendIn = 0.2
		track2.BlendOut = 0.0
		
		track2.Airborne = Logic
		track2.Push( Stack )
	}
	
	function CargoBegin( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "deploy" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		track.Push( Stack )

		return (track.Anim.OneShotLength - 0.25)
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
		track.BlendOut = 0.2
		
		track.Push( Stack )

		return (track.Anim.OneShotLength - 0.25)
	}
}
