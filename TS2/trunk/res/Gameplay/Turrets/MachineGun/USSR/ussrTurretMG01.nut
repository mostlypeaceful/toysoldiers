
sigimport "Gameplay/Turrets/Common/TurretLogic.nut"
sigimport "gameplay/turrets/machinegun/ussr/ussr_machinegun_01_motionmap.nut"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Turret_MG_01( );
}

sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.CreateDefaultArtillerySoldiers( entity )
}

class USSR_Turret_MG_01 extends BaseTurretLogic
{
	function DebugTypeName( )
		return "USSR_Turret_MG_01"

	function OnSpawn( )
	{
		WeaponStation( 0 ).Bank( 0 ).AddWeapon( "TURRET_PKGP_MG", "" )
		
		BaseTurretLogic.OnSpawn( )
		
		SetDestroyedEffect( "Small_Turret_Explosion" )

	}	
	
	function SetMotionMap( )
	{
		Animatable.MotionMap = USSR_MachineGun_01_MotionMap( this )
	}
}
