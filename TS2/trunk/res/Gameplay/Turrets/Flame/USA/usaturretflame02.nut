
sigimport "Gameplay/Turrets/Common/TurretLogic.nut"
sigimport "gameplay/turrets/flame/usa/usa_flame_02_motionmap.nut"


sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_Turret_Flame_02( );
}
sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.CreateDefaultArtillerySoldiers( entity )
}
class USA_Turret_Flame_02 extends BaseTurretLogic
{
	function DebugTypeName( )
		return "USA_Turret_Flame_02"

	function OnSpawn( )
	{
		WeaponStation( 0 ).Bank( 0 ).AddWeapon( "TURRET_L2_FLAME", "" )
		
		BaseTurretLogic.OnSpawn( )
	}	
	
	function SetMotionMap( )
	{
		Animatable.MotionMap = USA_Flame_02_MotionMap( this )
	}
}
