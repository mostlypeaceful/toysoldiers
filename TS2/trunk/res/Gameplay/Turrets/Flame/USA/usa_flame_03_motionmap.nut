sigimport "Gameplay/Turrets/Common/TurretMoMap.nut"
sigimport "Gameplay/Turrets/Common/CrewmanMoMap.nut"
sigimport "Anims/Turrets/Special/Blue/firework_cannon/firework_cannon.anipk"
sigimport "Anims/Characters/Blue/artillery/artillery.anipk"

class USA_Flame_03_MotionMap extends TurretMotionMap
{	
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Turrets/Special/Blue/firework_cannon/firework_cannon.anipk" )
		UseMuzzleTrack( 35, 89 )
	}
	
	function Reload( params )
	{
		local prevTimeOverride = timeOverride
		timeOverride = reloadTimeOverride
		
		local length = PushTurretPitchTrack( animPack, "reload_low", "reload_high", true, 1 )
		PushTurretOrientTrack( )
		
		timeOverride = prevTimeOverride

		return length
	}
	
	function SpinUp( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "startup" )
		track.BlendIn = 0.1
		track.BlendOut = 0.01
		track.TimeScale = 1.0
	//	track.StartTime = Anim.KeyFrameTrack.CurrentTimeOfTrack( track.Anim, Stack )
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		local trackLen = track.Anim.OneShotLength		
		track.Push( Stack )
		
		//return (track.Anim.OneShotLength / track.TimeScale - 0.1 - track.StartTime)
		return (trackLen - 0.1)
	}
	
	function SpinDown( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "winddown" )
		track.BlendIn = 0.1
		track.BlendOut = 0.01
		track.TimeScale = 1.0
		//track.StartTime = Anim.KeyFrameTrack.CurrentTimeOfTrack( track.Anim, Stack )
		track.Flags = ANIM_TRACK_CLAMP_TIME
		Stack.RemoveTracksWithTag( "fireLooping" );
		
		local trackLen = track.Anim.OneShotLength
		//Aim( params )
		track.Push( Stack )		
		//return track.StartTime
		return trackLen
	}
	
	function FireLooping( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "fire" )
		track.BlendIn = 0.1
		track.BlendOut = 0.0
		track.Tag = "fireLooping"

		track.Push( Stack )
		return track.Anim.OneShotLength
	}
}

class Special03CrewmanAMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Blue/artillery/artillery.anipk", "firework_cannon", "crewmana", true, true )
		bankIndex = 1
		UseMuzzleTrack( 35, 89 )
	}
	
	function SpinUp( params )
	{
	}
	
	function SpinDown( params )
	{
	}
}


class Special03CrewmanBMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Blue/artillery/artillery.anipk", "firework_cannon", "crewmanb", false, false )
	}
	
	function SpinDown( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( animPrefix + "_winddown_" + animSuffix )
		track.BlendIn = 0.1
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_CLAMP_TIME
		track.Push( Stack )
	}
}


class Special03CrewmanCMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Blue/artillery/artillery.anipk", "firework_cannon", "crewmanc", false, false )
	}
}
