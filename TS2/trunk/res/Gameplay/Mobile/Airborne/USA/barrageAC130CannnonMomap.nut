sigimport "Gameplay/Turrets/Common/TurretMoMap.nut"
sigimport "Gameplay/Turrets/Common/CrewmanMoMap.nut"
sigimport "Anims/Vehicles/Blue/ac130_turret/ac130_turret.anipk"

class USA_AC130_Cannon_MotionMap extends TurretMotionMap
{	
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Blue/ac130_turret/ac130_turret.anipk" )
	}
	
	function Reload( params ) 
	{
		TurretMotionMap.Aim( params )
		return 10.0
	}

	
}


