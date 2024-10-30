
sigimport "Gameplay/Turrets/Common/TurretLogic.nut"
sigimport "gameplay/mobile/helicopter/ussr/ussr_homer_aab_motionmap.nut"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Homer_AAB_01( );
}
class USSR_Homer_AAB_01 extends BaseTurretLogic
{
	function DebugTypeName( )
		return "USSR_Homer_AAB_01"

	function OnSpawn( )
	{
		WeaponStation( 0 ).Bank( 0 ).AddWeapon( "BOSS_HOMER_MGB", "" )
		
		BaseTurretLogic.OnSpawn( )
	}	
	
	function SetMotionMap( )
	{
		Animatable.MotionMap = ussr_homer_aab_motionmap( this )
	}
}

