
sigimport "Gameplay/Turrets/Common/TurretLogic.nut"
sigimport "gameplay/turrets/artillery/ussr/ussr_artillery_01_motionmap.nut"


sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Turret_Artillery_01( );
}
sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.CreateDefaultArtillerySoldiers( entity )
}

class USSR_Turret_Artillery_01 extends BaseTurretLogic
{
	function DebugTypeName( )
		return "USSR_Turret_Artillery_01"

	function OnSpawn( )
	{
		WeaponStation( 0 ).Bank( 0 ).AddWeapon( "TURRET_2A18_HOWITZER", "" )
		
		BaseTurretLogic.OnSpawn( )
	}	
	
	function SetMotionMap( )
	{
		Animatable.MotionMap = USSR_Artillery_01_MotionMap( this )
	}
}
