
sigimport "Gameplay/Turrets/Common/TurretLogic.nut"
sigimport "gameplay/boss/usa/usaboss_supertank_logic.nut"


sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_Supertank_Rockets( );
}

sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.CreateDefaultArtillerySoldiers( entity )
}

class USA_Supertank_Rockets extends BaseTurretLogic
{
	function DebugTypeName( )
		return "USA_Supertank_Rockets"

	function OnSpawn( )
	{
		WeaponStation( 0 ).Bank( 0 ).AddWeapon( "TURRET_SUPERTANK_ROCKETS", "" )
		
		BaseTurretLogic.OnSpawn( )
	}	
	
	function SetMotionMap( )
	{
		Animatable.MotionMap = USA_Supertank_AA_GunMoMap( this )
	}
}
