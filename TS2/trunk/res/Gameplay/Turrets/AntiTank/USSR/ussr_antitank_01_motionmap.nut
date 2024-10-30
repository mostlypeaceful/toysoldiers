sigimport "Gameplay/Turrets/Common/CrewmanMoMap.nut"
sigimport "Gameplay/Turrets/Common/TurretMoMap.nut"
sigimport "Anims/Turrets/AntiTank/Red/at_rpg7/at_rpg7.anipk"
sigimport "Anims/Characters/Red/artillery/artillery.anipk"

class USSR_AntiTank_01_MotionMap extends TurretMotionMap
{	
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Turrets/AntiTank/Red/at_rpg7/at_rpg7.anipk" )
		UseMuzzleTrack( -10, 7 )
	}
}
class USSR_AT01CrewmanAMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Red/artillery/artillery.anipk", "at_rpg7", "crewmana", true, true )
		UseMuzzleTrack( -10, 7 )
	}
}


class USSR_AT01CrewmanBMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Red/artillery/artillery.anipk", "at_rpg7", "crewmanb", true, true )
		UseMuzzleTrack( -10, 7 )
	}
}


class USSR_AT01CrewmanCMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Red/artillery/artillery.anipk", "at_rpg7", "crewmanc", true, true )
		UseMuzzleTrack( -10, 7 )
	}
}
