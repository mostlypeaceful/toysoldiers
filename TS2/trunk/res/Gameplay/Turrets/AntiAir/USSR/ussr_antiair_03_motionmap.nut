sigimport "Gameplay/Turrets/Common/CrewmanMoMap.nut"
sigimport "Gameplay/Turrets/Common/TurretMoMap.nut"
sigimport "Anims/Turrets/AntiAir/Red/aa_gladiator/aa_gladiator.anipk"
sigimport "Anims/Characters/Red/artillery/artillery.anipk"

class USSR_AntiAir_03_MotionMap extends TurretMotionMap
{	
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Turrets/AntiAir/Red/aa_gladiator/aa_gladiator.anipk" )
		UseMuzzleTrack( 2, 85 )
	}
	
	function Recoil( params )
	{
		local anim = "fire"
		switch( params.MuzzleID )
			{
			case 0: anim = "fire_1"; break;
			case 1: anim = "fire_2"; break;
			case 2: anim = "fire_3"; break;
			case 3: anim = "fire_4"; break;
		}
			
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( anim )
		track.BlendIn = 0.01
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_CLAMP_TIME
		track.Push( Stack )

		return (track.Anim.OneShotLength - 0.25)
	}	
	
	function Reload( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "reload" )
		track.BlendIn = 0.01
		track.BlendOut = 0.0
		
		track.Push( Stack )
		PushTurretOrientTrack( )
		
		return track.Anim.OneShotLength - 0.1
	}
}


class USSR_AA03CrewmanAMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Red/artillery/artillery.anipk", "aa_gladiator", "crewmana", true, false )
		UseMuzzleTrack( 2, 85 )
	}
}


class USSR_AA03CrewmanBMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Red/artillery/artillery.anipk", "aa_gladiator", "crewmanb", true, false )
		UseMuzzleTrack( 2, 85 )
	}
}


class USSR_AA03CrewmanCMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Red/artillery/artillery.anipk", "aa_gladiator", "crewmanc", true, false )
		UseMuzzleTrack( 2, 85 )
	}
}

class USSR_AA03CrewmanDMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Red/artillery/artillery.anipk", "aa_gladiator", "crewmand", true, false )
		UseMuzzleTrack( 2, 85 )
	}
}
