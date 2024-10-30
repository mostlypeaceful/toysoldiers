sigimport "Gameplay/Turrets/Common/TurretMoMap.nut"
sigimport "Gameplay/Turrets/Common/CrewmanMoMap.nut"
sigimport "Anims/Turrets/Special/Blue/flamelube/flamelube.anipk"
sigimport "Anims/Characters/Blue/artillery/artillery.anipk"

class USA_Flame_02_MotionMap extends TurretMotionMap
{	
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Turrets/Special/Blue/flamelube/flamelube.anipk" )
		UseMuzzleTrack( -10, 5 )
	}
}

class Special02CrewmanAMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Blue/artillery/artillery.anipk", "flamelube", "crewmana", false, false )
	}
}


class Special02CrewmanBMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Blue/artillery/artillery.anipk", "flamelube", "crewmanb", false, false )
	}
}


class Special02CrewmanCMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Blue/artillery/artillery.anipk", "flamelube", "crewmanc", false, false )
	}
}

class Special02CrewmanDMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Blue/artillery/artillery.anipk", "flamelube", "crewmand", false, false )
	}
}