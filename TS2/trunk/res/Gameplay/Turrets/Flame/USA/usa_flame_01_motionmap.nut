sigimport "Gameplay/Turrets/Common/TurretMoMap.nut"
sigimport "Gameplay/Turrets/Common/CrewmanMoMap.nut"
sigimport "Anims/Turrets/Special/Blue/bugspray/bugspray.anipk"
sigimport "Anims/Characters/Blue/artillery/artillery.anipk"

class USA_Flame_01_MotionMap extends TurretMotionMap
{	
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Turrets/Special/Blue/bugspray/bugspray.anipk" )
	}
}

class Special01CrewmanAMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Blue/artillery/artillery.anipk", "bugspray", "crewmana", false, false)
	}
}


class Special01CrewmanBMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Blue/artillery/artillery.anipk", "bugspray", "crewmanb", false, false )
	}
}


class Special01CrewmanCMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Blue/artillery/artillery.anipk", "bugspray", "crewmanc", false, false )
	}
}
