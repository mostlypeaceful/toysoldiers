
sigimport "Gameplay/Turrets/Common/TurretLogic.nut"
sigimport "gameplay/mobile/helicopter/ussr/ussrhomermg.nut"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_FlyingTank_MG( );
}
class USSR_FlyingTank_MG extends BaseTurretLogic
{
	function DebugTypeName( )
		return "USSR_FlyingTank_MG"

	function OnSpawn( )
	{
		WeaponStation( 0 ).Bank( 0 ).AddWeapon( "BOSS_FLYINGTANK_MG", "" )
		SetDamageTintColor( Math.Vec4.Construct( 3,0,0,0 ) )
		
		BaseTurretLogic.OnSpawn( )
	}	
	
	function SetMotionMap( )
	{
		Animatable.MotionMap = USSRBossHomerMGMoMap( this )
	}
}
