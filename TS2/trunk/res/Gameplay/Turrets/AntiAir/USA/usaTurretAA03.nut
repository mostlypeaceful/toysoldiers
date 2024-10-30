
sigimport "Gameplay/Turrets/Common/TurretLogic.nut"
sigimport "gameplay/turrets/antiair/usa/usa_antiair_03_motionmap.nut"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_Turret_AA_03( );
}
sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.CreateDefaultArtillerySoldiers( entity )
}
class USA_Turret_AA_03 extends BaseTurretLogic
{
	function DebugTypeName( )
		return "USA_Turret_AA_03"

	function OnSpawn( )
	{
		local weap = WeaponStation( 0 ).Bank( 0 ).AddWeapon( "TURRET_MIM104PATRIOT_AA", "" )
		weap.SetTurretEntity( OwnerEntity )
		
		BaseTurretLogic.OnSpawn( )
	}	
	
	function SetMotionMap( )
	{
		Animatable.MotionMap = USA_AntiAir_03_MotionMap( this )
	}
}
