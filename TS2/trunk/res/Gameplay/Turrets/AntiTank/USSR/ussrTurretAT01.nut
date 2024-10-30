
sigimport "Gameplay/Turrets/Common/TurretLogic.nut"
sigimport "gameplay/turrets/antitank/ussr/ussr_antitank_01_motionmap.nut"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Turret_AT_01( );
}
sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.CreateDefaultArtillerySoldiers( entity )
}
class USSR_Turret_AT_01 extends BaseTurretLogic
{
	function DebugTypeName( )
		return "USSR_Turret_AT_01"

	function OnSpawn( )
	{
		WeaponStation( 0 ).Bank( 0 ).AddWeapon( "TURRET_RPG7_AT", "" )
		
		BaseTurretLogic.OnSpawn( )
	}	
	
	function SetMotionMap( )
	{
		Animatable.MotionMap = USSR_AntiTank_01_MotionMap( this )
	}
}
