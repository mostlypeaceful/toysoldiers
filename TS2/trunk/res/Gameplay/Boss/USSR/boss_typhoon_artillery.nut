sigimport "Gameplay/Turrets/Common/TurretLogic.nut"
sigimport "Anims/Bosses/Red/typhoon_artillery/typhoon_artillery.anipk"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = TyphoonArtillery( );
}

sigexport function EntityOnChildrenCreate( entity )
{
}

class TyphoonArtillery extends BaseTurretLogic
{
	function DebugTypeName( )
		return "TyphoonArtillery"

	function OnSpawn( )
	{
		WeaponStation( 0 ).Bank( 0 ).AddWeapon( "TURRET_M270MLRS_HOWITZER", "" )
		
		BaseTurretLogic.OnSpawn( )
	}	
	
	function SetMotionMap( )
	{
		Animatable.MotionMap = TyphoonArtillery_MotionMap( this )
	}
}

class TyphoonArtillery_MotionMap extends TurretMotionMap
{	
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Bosses/Red/typhoon_artillery/typhoon_artillery.anipk" )
		UseMuzzleTrack( 0, 40 )
	}
}