
sigimport "Gameplay/Turrets/Common/TurretLogic.nut"
sigimport "gameplay/turrets/artillery/usa/usa_artillery_02_motionmap.nut"


sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_Turret_Artillery_02( );
}

sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.CreateDefaultArtillerySoldiers( entity )
}

class USA_Turret_Artillery_02 extends BaseTurretLogic
{
	function DebugTypeName( )
		return "USA_Turret_Artillery_02"

	function OnSpawn( )
	{
		WeaponStation( 0 ).Bank( 0 ).AddWeapon( "TURRET_M198_HOWITZER", "" )
		
		BaseTurretLogic.OnSpawn( )
	}	
	
	function SetMotionMap( )
	{
		Animatable.MotionMap = USA_Artillery_02_MotionMap( this )
	}
}
