
sigimport "Gameplay/Turrets/Common/TurretLogic.nut"
sigimport "gameplay/turrets/artillery/usa/usa_artillery_03_motionmap.nut"


sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_Turret_Artillery_03( );
}

sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.CreateDefaultArtillerySoldiers( entity )
}

class USA_Turret_Artillery_03 extends BaseTurretLogic
{
	function DebugTypeName( )
		return "USA_Turret_Artillery_03"

	function OnSpawn( )
	{
		WeaponStation( 0 ).Bank( 0 ).AddWeapon( "TURRET_M270MLRS_HOWITZER", "" )
		
		BaseTurretLogic.OnSpawn( )
	}	
	
	function SetMotionMap( )
	{
		Animatable.MotionMap = USA_Artillery_03_MotionMap( this )
	}
}
