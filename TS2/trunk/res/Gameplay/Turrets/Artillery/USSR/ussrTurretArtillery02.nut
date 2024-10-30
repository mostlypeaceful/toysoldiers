
sigimport "Gameplay/Turrets/Common/TurretLogic.nut"
sigimport "gameplay/turrets/artillery/ussr/ussr_artillery_02_motionmap.nut"


sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Turret_Artillery_02( );
}
sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.CreateDefaultArtillerySoldiers( entity )
}

class USSR_Turret_Artillery_02 extends BaseTurretLogic
{
	function DebugTypeName( )
		return "USSR_Turret_Artillery_02"

	function OnSpawn( )
	{
		WeaponStation( 0 ).Bank( 0 ).AddWeapon( "TURRET_2A36GIATSINT_HOWITZER", "" )
		
		BaseTurretLogic.OnSpawn( )
	}	
	
	function SetMotionMap( )
	{
		Animatable.MotionMap = USSR_Artillery_02_MotionMap( this )
	}
}
