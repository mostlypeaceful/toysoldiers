
sigimport "Gameplay/Turrets/Common/TurretLogic.nut"
sigimport "gameplay/turrets/flame/usa/usa_flame_01_motionmap.nut"
sigimport "Art/Characters/Blue/usa_gasmask_1.mshml"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_Turret_Flame_01( );
}
sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.CreateDefaultArtillerySoldiers( entity )
}
class USA_Turret_Flame_01 extends BaseTurretLogic
{
	function DebugTypeName( )
		return "USA_Turret_Flame_01"

	function OnSpawn( )
	{
		WeaponStation( 0 ).Bank( 0 ).AddWeapon( "TURRET_L1_FLAME", "" )
		
		BaseTurretLogic.OnSpawn( )
	}	
	
	function SetMotionMap( )
	{
		Animatable.MotionMap = USA_Flame_01_MotionMap( this )
	}
	
	function GetHead( ) return "Art/Characters/Blue/usa_gasmask_1.mshml"
}
