
sigimport "Gameplay/Turrets/Common/TurretLogic.nut"
sigimport "gameplay/boss/usa/usaboss_supertank_logic.nut"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_Turret_Supertank_Phalanx( );
}
sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.CreateDefaultArtillerySoldiers( entity )
}
class USA_Turret_Supertank_Phalanx extends BaseTurretLogic
{
	function DebugTypeName( )
		return "USA_Turret_Supertank_Phalanx"

	function OnSpawn( )
	{
		WeaponStation( 0 ).Bank( 0 ).AddWeapon( "TURRET_SUPERTANK_PHALANX", "" ).SetTurretEntity( OwnerEntity )
		
		BaseTurretLogic.OnSpawn( )
	}	
	
	function SetMotionMap( )
	{
		Animatable.MotionMap = USA_Supertank_Phalanx_GunMoMap( this )
	}
}
