
sigimport "Gameplay/Turrets/Common/TurretLogic.nut"
sigimport "gameplay/turrets/antiair/ussr/ussr_antiair_01_motionmap.nut"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Turret_AA_01( );
}
sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.CreateDefaultArtillerySoldiers( entity )
}
class USSR_Turret_AA_01 extends BaseTurretLogic
{
	function DebugTypeName( )
		return "USSR_Turret_AA_01"

	function OnSpawn( )
	{
		WeaponStation( 0 ).Bank( 0 ).AddWeapon( "TURRET_ZU232_AA", "" ).SetTurretEntity( OwnerEntity )
		
		BaseTurretLogic.OnSpawn( )
	}	
	
	function SetMotionMap( )
	{
		Animatable.MotionMap = USSR_AntiAir_01_MotionMap( this )
	}
}
