
sigimport "Gameplay/Turrets/Common/TurretLogic.nut"
sigimport "gameplay/turrets/machinegun/usa/usa_machinegun_01_motionmap.nut"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_Turret_MG_01( );
}

sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.CreateDefaultArtillerySoldiers( entity )
}

class USA_Turret_MG_01 extends BaseTurretLogic
{
	function DebugTypeName( )
		return "USA_Turret_MG_01"

	function OnSpawn( )
	{
		WeaponStation( 0 ).Bank( 0 ).AddWeapon( "TURRET_M2BROWNING_MG", "" )
		
		BaseTurretLogic.OnSpawn( )
		SetDestroyedEffect( "Small_Turret_Explosion" )

	}	
	
	function SetMotionMap( )
	{
		Animatable.MotionMap = USA_MachineGun_01_MotionMap( this )
	}
	
}
