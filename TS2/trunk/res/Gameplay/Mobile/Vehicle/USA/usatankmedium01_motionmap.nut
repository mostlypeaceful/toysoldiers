sigimport "Gameplay/Turrets/Common/CrewmanMoMap.nut"


class USATankMedium01_CrewmanAMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Blue/artillery/artillery.anipk", "tank_m60patton", "crewmana", false, false )
	}
	
	function RandomAnim( params )
	{
		return PushRandomIdle( params )
	}
	
	
	function Forward( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( animPrefix + "_idle_" + animSuffix )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.Push( Stack )
	}
	
	function Recoil( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( animPrefix + "_fire_" + animSuffix )
		track.BlendIn = 0.0
		track.BlendOut = 0.1
		track.Push( Stack )
		track.Flags = ANIM_TRACK_CLAMP_TIME;
				
		return track.Anim.OneShotLength
	}
}

