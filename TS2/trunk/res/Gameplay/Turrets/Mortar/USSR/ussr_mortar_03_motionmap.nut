sigimport "Gameplay/Turrets/Common/CrewmanMoMap.nut"
sigimport "Gameplay/Turrets/Common/TurretMoMap.nut"
sigimport "Anims/Turrets/Mortar/Red/mortar_2s12sani/mortar_2s12sani.anipk"
sigimport "Anims/Characters/Red/artillery/artillery.anipk"

class USSR_Mortar_03_MotionMap extends TurretMotionMap
{	
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Turrets/Mortar/Red/mortar_2s12sani/mortar_2s12sani.anipk" )
		UseMuzzleTrack( 35, 89 )
	}
}

class USSR_Mortar03CrewmanAMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Red/artillery/artillery.anipk", "mortar_2s12sani", "crewmana", true, false )
		UseMuzzleTrack( 45, 89 )
	}
}


class USSR_Mortar03CrewmanBMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Red/artillery/artillery.anipk", "mortar_2s12sani", "crewmanb", true, false )
		UseMuzzleTrack( 45, 89 )
	}
}


class USSR_Mortar03CrewmanCMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Red/artillery/artillery.anipk", "mortar_2s12sani", "crewmanc", true, false )
		UseMuzzleTrack( 45, 89 )
	}
}