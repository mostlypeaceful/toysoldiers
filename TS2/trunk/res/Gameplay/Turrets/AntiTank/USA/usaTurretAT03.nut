
sigimport "Gameplay/Turrets/Common/TurretLogic.nut"
sigimport "gameplay/turrets/antitank/usa/usa_antitank_03_motionmap.nut"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_Turret_AT_03( );
}
sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.CreateDefaultArtillerySoldiers( entity )
}
class USA_Turret_AT_03 extends BaseTurretLogic
{
	function DebugTypeName( )
		return "USA_Turret_AT_03"

	function OnSpawn( )
	{
		WeaponStation( 0 ).Bank( 0 ).AddWeapon( "TURRET_BGM71TOW_AT", "" )
		
		BaseTurretLogic.OnSpawn( )
	}	
	
	function SetMotionMap( )
	{
		Animatable.MotionMap = USA_AntiTank_03_MotionMap( this )
	}
}
