
sigimport "Gameplay/Turrets/Common/TurretLogic.nut"
sigimport "gameplay/turrets/antiair/ussr/ussr_antiair_03_motionmap.nut"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Turret_AA_03( );
}
sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.CreateDefaultArtillerySoldiers( entity )
}
class USSR_Turret_AA_03 extends BaseTurretLogic
{
	function DebugTypeName( )
		return "USSR_Turret_AA_03"

	function OnSpawn( )
	{
		local weap = WeaponStation( 0 ).Bank( 0 ).AddWeapon( "TURRET_SA10GRUMBLE_AA", "" )
		weap.SetTurretEntity( OwnerEntity ) //set turret entity for Recoil mostate to get called
		
		BaseTurretLogic.OnSpawn( )
	}	
	
	function SetMotionMap( )
	{
		Animatable.MotionMap = USSR_AntiAir_03_MotionMap( this )
	}
}
