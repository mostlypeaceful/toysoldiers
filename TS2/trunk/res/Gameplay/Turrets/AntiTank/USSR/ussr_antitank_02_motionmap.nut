sigimport "Gameplay/Turrets/Common/CrewmanMoMap.nut"
sigimport "Gameplay/Turrets/Common/TurretMoMap.nut"
sigimport "Anims/Turrets/AntiTank/Red/at_b10recoilless/at_b10recoilless.anipk"
sigimport "Anims/Characters/Red/artillery/artillery.anipk"

class USSR_AntiTank_02_MotionMap extends TurretMotionMap
{	
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Turrets/AntiTank/Red/at_b10recoilless/at_b10recoilless.anipk" )
		UseMuzzleTrack( -10, 7 )
	}
}

class USSR_AT02CrewmanAMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Red/artillery/artillery.anipk", "at_b10recoilless", "crewmana", true, false )
		UseMuzzleTrack( -10, 7 )
	}
}


class USSR_AT02CrewmanBMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Red/artillery/artillery.anipk", "at_b10recoilless", "crewmanb", true, false )
		UseMuzzleTrack( -10, 7 )
	}
}


class USSR_AT02CrewmanCMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Red/artillery/artillery.anipk", "at_b10recoilless", "crewmanc", true, true )
		UseMuzzleTrack( -10, 7 )
	}
}
