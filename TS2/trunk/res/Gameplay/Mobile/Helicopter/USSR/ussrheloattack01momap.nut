sigimport "Gameplay/Mobile/Helicopter/Common/HoverMoMap.nut"
sigimport "Gameplay/Turrets/Common/CrewmanMoMap.nut"
sigimport "Anims/Vehicles/Red/helicopter_mi24hind/helicopter_mi24hind.anipk"
sigimport "Anims/Characters/red/artillery/artillery.anipk"

class USSRHeloAttack01MoMap extends HoverMotionMap
{	
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Red/helicopter_mi24hind/helicopter_mi24hind.anipk" )
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

class USSRHeloAttack01CrewmanAMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/red/artillery/artillery.anipk", "heli_mi24hind", "crewmana", false, false )
		idleBlendIn = 0.0
	}
	
	function Startup( params )
	{
		Forward( params )
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( animPrefix + "_takeoff_" + animSuffix )
		track.BlendIn = 0.2
		track.BlendOut = 0.1
		track.Flags = ANIM_TRACK_CLAMP_TIME
		track.Push( Stack )
		
	}
	
	function Shutdown( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( animPrefix + "_exit_" + animSuffix )
		track.BlendIn = 0.2
		track.BlendOut = 0.1
		track.Flags = ANIM_TRACK_CLAMP_TIME
		track.Push( Stack )
		
	}
	
	function Forward( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( animPrefix + "_fly_idle_" + animSuffix )
		track.BlendIn = 0.0
		track.BlendOut = 0.0
		track.Push( Stack )
	}
}

class USSRHeloAttack01CrewmanBMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/red/artillery/artillery.anipk", "heli_mi24hind", "crewmanb", false, false )
		idleBlendIn = 0.0
	}
	
	function Startup( params )
	{
		Forward( params )
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( animPrefix + "_idle_" + animSuffix )
		track.BlendIn = 0.2
		track.BlendOut = 0.1
		track.Flags = ANIM_TRACK_CLAMP_TIME
		track.Push( Stack )
		
	}
	
	function Shutdown( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( animPrefix + "_idle_" + animSuffix )
		track.BlendIn = 0.2
		track.BlendOut = 0.1
		track.Flags = ANIM_TRACK_CLAMP_TIME
		track.Push( Stack )
		
	}
	
	function Forward( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( animPrefix + "_idle_" + animSuffix )
		track.BlendIn = 0.0
		track.BlendOut = 0.0
		track.Push( Stack )
	}
}

class USSRHeloAttack01CrewmanCMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/red/artillery/artillery.anipk", "heli_mi24hind", "crewmanc", false, false )
		idleBlendIn = 0.0
	}
	
	function Startup( params )
	{
		Forward( params )
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( animPrefix + "_takeoff_" + animSuffix )
		track.BlendIn = 0.2
		track.BlendOut = 0.1
		track.Flags = ANIM_TRACK_CLAMP_TIME
		track.Push( Stack )
		
	}
	
	function Shutdown( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( animPrefix + "_exit_" + animSuffix )
		track.BlendIn = 0.2
		track.BlendOut = 0.1
		track.Flags = ANIM_TRACK_CLAMP_TIME
		track.Push( Stack )
		
	}
	
	function Forward( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( animPrefix + "_fly_idle_" + animSuffix )
		track.BlendIn = 0.0
		track.BlendOut = 0.0
		track.Push( Stack )
	}
}

