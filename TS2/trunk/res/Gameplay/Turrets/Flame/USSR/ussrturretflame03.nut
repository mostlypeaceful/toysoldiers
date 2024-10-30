
sigimport "Gameplay/Turrets/Common/TurretLogic.nut"
sigimport "gameplay/turrets/machinegun/ussr/ussr_machinegun_01_motionmap.nut"


sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Turret_Flame_03( );
}
sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.CreateDefaultArtillerySoldiers( entity )
}
class USSR_Turret_Flame_03 extends BaseTurretLogic
{
	function DebugTypeName( )
		return "USSR_Turret_Flame_03"

	function OnSpawn( )
	{
		WeaponStation( 0 ).Bank( 0 ).AddWeapon( "TURRET_L3_FLAME", "" )
		
		BaseTurretLogic.OnSpawn( )
	}	
	
	function SetMotionMap( )
	{
		Animatable.MotionMap = USSR_MachineGun_01_MotionMap( this )
	}
}
