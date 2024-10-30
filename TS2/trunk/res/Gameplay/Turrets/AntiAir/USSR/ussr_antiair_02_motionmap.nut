sigimport "Gameplay/Turrets/Common/CrewmanMoMap.nut"
sigimport "Gameplay/Turrets/Common/TurretMoMap.nut"
sigimport "Anims/Turrets/AntiAir/Red/aa_s125_launcher/aa_s125_launcher.anipk"
sigimport "Anims/Characters/Red/artillery/artillery.anipk"

class USSR_AntiAir_02_MotionMap extends TurretMotionMap
{	
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Turrets/AntiAir/Red/aa_s125_launcher/aa_s125_launcher.anipk" )
		UseMuzzleTrack( 2, 85 )
	}

}


class USSR_AA02CrewmanAMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Red/artillery/artillery.anipk", "aa_s125_launcher", "crewmana", true, false )
		UseMuzzleTrack( 2, 85 )
	}
}


class USSR_AA02CrewmanBMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Red/artillery/artillery.anipk", "aa_s125_launcher", "crewmanb", true, false )
		UseMuzzleTrack( 2, 85 )
	}
}


class USSR_AA02CrewmanCMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Red/artillery/artillery.anipk", "aa_s125_launcher", "crewmanc", true, false )
		UseMuzzleTrack( 2, 85 )
	}
}
