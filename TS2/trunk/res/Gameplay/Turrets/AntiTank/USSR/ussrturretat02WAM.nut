
sigimport "Gameplay/Turrets/Common/TurretLogic.nut"
sigimport "gameplay/turrets/antitank/ussr/ussr_antitank_02_motionmap.nut"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Turret_AT_02_wam( );
}
sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.CreateDefaultArtillerySoldiers( entity )
}
class USSR_Turret_AT_02_wam extends BaseTurretLogic
{
	function DebugTypeName( )
		return "USSR_Turret_AT_02"

	function OnSpawn( )
	{
		WeaponStation( 0 ).Bank( 0 ).AddWeapon( "TURRET_B10RECOLLESS_AT_WAM", "" )
		
		BaseTurretLogic.OnSpawn( )
	}	
	
	function SetMotionMap( )
	{
		Animatable.MotionMap = USSR_AntiTank_02_MotionMap( this )
	}
}
