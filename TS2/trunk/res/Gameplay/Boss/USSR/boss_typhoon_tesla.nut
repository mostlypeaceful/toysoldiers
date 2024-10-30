sigimport "Gameplay/Turrets/Common/TurretLogic.nut"
sigimport "Anims/Bosses/Red/typhoon_tesla/typhoon_tesla.anipk"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = TyphoonTesla( );
}

sigexport function EntityOnChildrenCreate( entity )
{
}

class TyphoonTesla extends BaseTurretLogic
{
	function DebugTypeName( )
		return "TyphoonTesla"

	function OnSpawn( )
	{
		WeaponStation( 0 ).Bank( 0 ).AddWeapon( "TURRET_M270MLRS_HOWITZER", "" )
		
		BaseTurretLogic.OnSpawn( )
	}	
	
	function SetMotionMap( )
	{
		Animatable.MotionMap = TyphoonTesla_MotionMap( this )
	}
}

class TyphoonTesla_MotionMap extends TurretMotionMap
{	
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Bosses/Red/typhoon_tesla/typhoon_tesla.anipk" )
		UseMuzzleTrack( 0, 50 )
	}
}