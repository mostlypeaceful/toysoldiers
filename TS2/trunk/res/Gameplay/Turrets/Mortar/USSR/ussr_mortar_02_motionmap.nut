sigimport "Gameplay/Turrets/Common/CrewmanMoMap.nut"
sigimport "Gameplay/Turrets/Common/TurretMoMap.nut"
sigimport "Anims/Turrets/Mortar/Red/mortar_m1938/mortar_m1938.anipk"
sigimport "Anims/Characters/Red/artillery/artillery.anipk"

class USSR_Mortar_02_MotionMap extends TurretMotionMap
{	
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Turrets/Mortar/Red/mortar_m1938/mortar_m1938.anipk" )
		UseMuzzleTrack( 35, 89 )
	}
}

class USSR_Mortar02CrewmanAMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Red/artillery/artillery.anipk", "mortar_m1938", "crewmana", true, false )
		UseMuzzleTrack( 45, 89 )
	}
}


class USSR_Mortar02CrewmanBMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Red/artillery/artillery.anipk", "mortar_m1938", "crewmanb", true, false )
		UseMuzzleTrack( 45, 89 )
	}
}
class USSR_Mortar02CrewmanCMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Red/artillery/artillery.anipk", "mortar_m1938", "crewmanc", true, false )
		UseMuzzleTrack( 45, 89 )
	}
}