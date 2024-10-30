
sigimport "Gameplay/Turrets/Common/TurretLogic.nut"
sigimport "gameplay/mobile/helicopter/ussr/ussr_homer_aa_motionmap.nut"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Homer_AA_01( );
}
class USSR_Homer_AA_01 extends BaseTurretLogic
{
	function DebugTypeName( )
		return "USSR_Homer_AA_01"

	function OnSpawn( )
	{
		WeaponStation( 0 ).Bank( 0 ).AddWeapon( "BOSS_HOMER_AA", "" )
		
		BaseTurretLogic.OnSpawn( )
	}	
	
	function SetMotionMap( )
	{
		Animatable.MotionMap = ussr_homer_aa_motionmap( this )
	}
}

