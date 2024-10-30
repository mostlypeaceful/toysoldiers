
sigimport "Gameplay/Turrets/Common/TurretLogic.nut"
sigimport "gameplay/turrets/machinegun/ussr/ussr_machinegun_01_motionmap.nut"
sigimport "Art/Characters/Blue/usa_gasmask_1.mshml"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Turret_Flame_01( );
}
sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.CreateDefaultArtillerySoldiers( entity )
}
class USSR_Turret_Flame_01 extends BaseTurretLogic
{
	function DebugTypeName( )
		return "USSR_Turret_Flame_01"

	function OnSpawn( )
	{
		WeaponStation( 0 ).Bank( 0 ).AddWeapon( "TURRET_L1_FLAME", "" )
		
		BaseTurretLogic.OnSpawn( )
	}	
	
	function SetMotionMap( )
	{
		Animatable.MotionMap = USSR_MachineGun_01_MotionMap( this )
	}
	
	function GetHead( ) return "Art/Characters/Blue/usa_gasmask_1.mshml"
}
