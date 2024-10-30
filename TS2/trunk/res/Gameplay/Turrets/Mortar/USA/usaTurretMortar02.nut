
sigimport "Gameplay/Turrets/Common/TurretLogic.nut"
sigimport "gameplay/turrets/mortar/usa/usa_mortar_02_motionmap.nut"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_Turret_Mortar_02( );
}
sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.CreateDefaultArtillerySoldiers( entity )
}
class USA_Turret_Mortar_02 extends BaseTurretLogic
{
	function DebugTypeName( )
		return "USA_Turret_Mortar_02"

	function OnSpawn( )
	{
		WeaponStation( 0 ).Bank( 0 ).AddWeapon( "TURRET_M252_MORTAR", "" )
		
		BaseTurretLogic.OnSpawn( )
	}	
	
	function SetMotionMap( )
	{
		Animatable.MotionMap = USA_Mortar_02_MotionMap( this )
	}
}
