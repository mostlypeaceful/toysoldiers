sigimport "Gameplay/Mobile/Helicopter/Common/HoverMoMap.nut"
sigimport "Anims/Bosses/Red/mi12_homer/mi12_homer.anipk"

class USSRBossHomerMoMap extends HoverMotionMap
{	
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Bosses/Red/mi12_homer/mi12_homer.anipk" )
	}

	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "idle" )
		track.BlendIn = 4.0
		track.BlendOut = 0.0
		
		track.Push( Stack )
	}
/*
	function Startup( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "takeoff" )
		track.BlendIn = 0.2
		track.BlendOut = 0.01
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		track.Push( Stack )
		
		return track.Anim.OneShotLength - 0.4
	}
*/	
	function Forward( params )
	{	
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "forward" )
		track.BlendIn = 1.0
		track.BlendOut = 0.0
		
		track.Push( Stack )		
	}
	
	function Critical( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "critical" )
		track.BlendIn = 0.0
		track.BlendOut = 0.3
		
		track.Push( Stack )
		return track.Anim.OneShotLength - track.BlendOut	
	}
	
	function Death( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "death" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		track.Push( Stack )	
		return track.Anim.OneShotLength - track.BlendOut
	}
	
	function CargoBegin( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "open_hatch" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		track.Push( Stack )

		return (track.Anim.OneShotLength - 0.25)
	}
		
	function CargoEnd( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "close_hatch" )
		track.BlendIn = 0.2
		track.BlendOut = 0.2
		
		track.Push( Stack )

		return (track.Anim.OneShotLength - 0.25)
	}
}
