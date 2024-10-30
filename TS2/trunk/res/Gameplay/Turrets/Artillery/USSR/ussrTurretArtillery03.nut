
sigimport "Gameplay/Turrets/Common/TurretLogic.nut"
sigimport "gameplay/turrets/artillery/ussr/ussr_artillery_03_motionmap.nut"


sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Turret_Artillery_03( );
}
sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.CreateDefaultArtillerySoldiers( entity )
}

class USSR_Turret_Artillery_03 extends BaseTurretLogic
{
	function DebugTypeName( )
		return "USSR_Turret_Artillery_03"

	function OnSpawn( )
	{
		WeaponStation( 0 ).Bank( 0 ).AddWeapon( "TURRET_SCUD_HOWITZER", "" )
		
		BaseTurretLogic.OnSpawn( )
	}	
	
	function SetMotionMap( )
	{
		Animatable.MotionMap = USSR_Artillery_03_MotionMap( this )
	}
}
