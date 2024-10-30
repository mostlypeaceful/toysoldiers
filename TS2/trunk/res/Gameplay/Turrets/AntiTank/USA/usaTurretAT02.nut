
sigimport "Gameplay/Turrets/Common/TurretLogic.nut"
sigimport "gameplay/turrets/antitank/usa/usa_antitank_02_motionmap.nut"


sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_Turret_AT_02( );
}
sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.CreateDefaultArtillerySoldiers( entity )
}
class USA_Turret_AT_02 extends BaseTurretLogic
{
	function DebugTypeName( )
		return "USA_Turret_AT_02"

	function OnSpawn( )
	{
		WeaponStation( 0 ).Bank( 0 ).AddWeapon( "TURRET_M40RECOILLESS_AT", "" )
		
		BaseTurretLogic.OnSpawn( )
	}	
	
	function SetMotionMap( )
	{
		Animatable.MotionMap = USA_AntiTank_02_MotionMap( this )
	}
}
