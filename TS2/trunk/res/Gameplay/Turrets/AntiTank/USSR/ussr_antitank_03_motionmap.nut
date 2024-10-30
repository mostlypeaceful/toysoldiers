sigimport "Gameplay/Turrets/Common/CrewmanMoMap.nut"
sigimport "Gameplay/Turrets/Common/TurretMoMap.nut"
sigimport "Anims/Turrets/AntiTank/Red/at_at4spigot/at_at4spigot.anipk"
sigimport "Anims/Characters/Red/artillery/artillery.anipk"

class USSR_AntiTank_03_MotionMap extends TurretMotionMap
{	
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Turrets/AntiTank/Red/at_at4spigot/at_at4spigot.anipk" )
		UseMuzzleTrack( -10, 7 )
	}
}

class USSR_AT03CrewmanAMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Red/artillery/artillery.anipk", "at_at4spigot", "crewmana", true, false )
		UseMuzzleTrack( -10, 7 )
	}
}


class USSR_AT03CrewmanBMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Red/artillery/artillery.anipk", "at_at4spigot", "crewmanb", true, false )
		UseMuzzleTrack( -10, 7 )
	}
}


class USSR_AT03CrewmanCMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Red/artillery/artillery.anipk", "at_at4spigot", "crewmanc", true, false )
		UseMuzzleTrack( -10, 7 )
	}
}

