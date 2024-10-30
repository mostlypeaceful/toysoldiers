sigimport "Gameplay/Turrets/Common/CrewmanMoMap.nut"
sigimport "Gameplay/Turrets/Common/TurretMoMap.nut"
sigimport "Anims/Turrets/AntiAir/Red/aa_zu232/aa_zu232.anipk"
sigimport "Anims/Characters/Red/artillery/artillery.anipk"

class USSR_AntiAir_01_MotionMap extends TurretMotionMap
{	
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Turrets/AntiAir/Red/aa_zu232/aa_zu232.anipk" )
		UseMuzzleTrack( 0, 85 )
	}
	
	function Recoil( params )
	{
		local anim = "fire_left"
		if( params.MuzzleID == 1 )
			anim = "fire_right"
			
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( anim )
		track.BlendIn = 0.01
		track.BlendOut = 0.01
		
		track.Push( Stack )

		return (track.Anim.OneShotLength - 0.25)
	}
	function FireOneShot( params )
	{
		return 1.0
	}

}


class USSR_AA01CrewmanAMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Red/artillery/artillery.anipk", "aa_zu232", "crewmana", true, false )
		UseMuzzleTrack( 0, 85 )
	}
}


class USSR_AA01CrewmanBMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Red/artillery/artillery.anipk", "aa_zu232", "crewmanb", true, false )
		UseMuzzleTrack( 0, 85 )
	}
}


class USSR_AA01CrewmanCMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Red/artillery/artillery.anipk", "aa_zu232", "crewmanc", true, false )
		UseMuzzleTrack( 0, 85 )
	}
}

class USSR_AA01CrewmanDMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Red/artillery/artillery.anipk", "aa_zu232", "crewmand", true, false )
		UseMuzzleTrack( 0, 85 )
	}
}

