
sigimport "Gameplay/Turrets/Common/TurretLogic.nut"
sigimport "gameplay/turrets/antiair/usa/usa_antiair_02_motionmap.nut"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_Turret_AA_02( );
}
sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.CreateDefaultArtillerySoldiers( entity )
}
class USA_Turret_AA_02 extends BaseTurretLogic
{
	function DebugTypeName( )
		return "USA_Turret_AA_02"

	function OnSpawn( )
	{
		WeaponStation( 0 ).Bank( 0 ).AddWeapon( "TURRET_M48CHAPARRAL_AA", "" )
		
		BaseTurretLogic.OnSpawn( )
	}	
	
	function SetMotionMap( )
	{
		Animatable.MotionMap = USA_AntiAir_02_MotionMap( this )
	}
}
