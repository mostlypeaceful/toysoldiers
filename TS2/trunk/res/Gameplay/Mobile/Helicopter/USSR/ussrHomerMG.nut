
sigimport "Gameplay/Turrets/Common/TurretLogic.nut"
sigimport "Gameplay/Turrets/Common/TurretMoMap.nut"
sigimport "Anims/Bosses/Red/mi12_homer_mg/mi12_homer_mg.anipk" 

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Homer_MG_01( );
}
class USSR_Homer_MG_01 extends BaseTurretLogic
{
	function DebugTypeName( )
		return "USSR_Homer_MG_01"

	function OnSpawn( )
	{
		WeaponStation( 0 ).Bank( 0 ).AddWeapon( "BOSS_HOMER_MG", "" )
		
		BaseTurretLogic.OnSpawn( )
	}	
	
	function SetMotionMap( )
	{
		Animatable.MotionMap = USSRBossHomerMGMoMap( this )
	}
}

class USSRBossHomerMGMoMap extends TurretMotionMap
{	
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Bosses/Red/mi12_homer_mg/mi12_homer_mg.anipk"  )
		UseMuzzleTrack( 0, 85 )
	}
	
	function FireOneShot( params )
	{
		return 1.0
	}

}