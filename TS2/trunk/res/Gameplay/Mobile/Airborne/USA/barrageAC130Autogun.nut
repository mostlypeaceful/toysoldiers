
sigimport "Gameplay/Turrets/Common/TurretLogic.nut"
sigimport "gameplay/mobile/airborne/usa/barrageac130gunmomap.nut"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_AC130_AUTOGUN_01( );
}

sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.CreateDefaultArtillerySoldiers( entity )
}

class USA_AC130_AUTOGUN_01 extends BaseTurretLogic
{
	function DebugTypeName( )
		return "USA_AC130_AUTOGUN_01"

	function OnSpawn( )
	{
		DisableYawConstraintAdjust = 1
		QuickSwitchCamera = 1
		SetDestroyedEffect( "Small_Turret_Explosion" )
		
		WeaponStation( 0 ).Bank( 0 ).AddWeapon( "AC130_AUTOGUN", "" )
		
		IdleTarget = GameApp.CurrentLevel.NamedObject( "ac130Target" )
		
		BaseTurretLogic.OnSpawn( )
	}	
	
	function SetMotionMap( )
	{
		Animatable.MotionMap = USA_AC130_Gun_MotionMap ( this )	
	}
	
}
