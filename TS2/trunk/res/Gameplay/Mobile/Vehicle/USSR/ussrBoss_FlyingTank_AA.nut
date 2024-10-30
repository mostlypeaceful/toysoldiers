
sigimport "Gameplay/Turrets/Common/TurretLogic.nut"
sigimport "Gameplay/Turrets/Common/TurretMoMap.nut"
sigimport "Anims/Bosses/Red/mi12_homer_turret_top/mi12_homer_turret_top.anipk"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_FlyingTank_AA( );
}
class USSR_FlyingTank_AA extends BaseTurretLogic
{
	function DebugTypeName( )
		return "USSR_FlyingTank_AA"

	function OnSpawn( )
	{
		WeaponStation( 0 ).Bank( 0 ).AddWeapon( "BOSS_FLYINGTANK_AA", "" )
		SetDamageTintColor( Math.Vec4.Construct( 3,0,0,0 ) )
		
		BaseTurretLogic.OnSpawn( )
	}	
	
	function SetMotionMap( )
	{
		Animatable.MotionMap = USSR_Boss_FlyingTank_AAMoMap( this )
	}
}

class USSR_Boss_FlyingTank_AAMoMap extends TurretMotionMap
{	
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Bosses/Red/mi12_homer_turret_top/mi12_homer_turret_top.anipk" )
		UseMuzzleTrack( 0, 85 )
	}

	function FireOneShot( params )
	{
		return 1.0
	}
}
