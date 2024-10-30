sigimport "Gameplay/Turrets/Common/TurretLogic.nut"
sigimport "Anims/Bosses/Red/typhoon_aa/typhoon_aa.anipk"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = SubBossTyphoon_AA( );
}

sigexport function EntityOnChildrenCreate( entity )
{
}

class SubBossTyphoon_AA extends BaseTurretLogic
{
	function DebugTypeName( )
		return "SubBossTyphoon_AA"

	function OnSpawn( )
	{
		WeaponStation( 0 ).Bank( 0 ).AddWeapon( "TURRET_S75_DVINA_AA", "" )
		
		BaseTurretLogic.OnSpawn( )
	}	
	
	function SetMotionMap( )
	{
		Animatable.MotionMap = SubBossTyphoon_AA_MotionMap( this )
	}
}

class SubBossTyphoon_AA_MotionMap extends TurretMotionMap
{	
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Bosses/Red/typhoon_aa/typhoon_aa.anipk" )
		UseMuzzleTrack( 2, 85 )
	}

}