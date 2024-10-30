
sigimport "Gameplay/Turrets/Common/TurretLogic.nut"
sigimport "gameplay/turrets/flame/usa/usa_flame_03_motionmap.nut"


sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_Turret_Flame_03( );
}
sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.CreateDefaultArtillerySoldiers( entity )
}
class USA_Turret_Flame_03 extends BaseTurretLogic
{
	function DebugTypeName( )
		return "USA_Turret_Flame_03"

	function OnSpawn( )
	{
		WeaponStation( 0 ).Bank( 0 ).AddWeapon( "TURRET_L3_FLAME_CANDLESPIN", "" )
		
		local rocketBank = WeaponStation( 0 ).Bank( 1 )
		rocketBank.AddWeapon( "TURRET_L3_FLAME_MORTAR", "romancandleweapon" ).SetTurretEntity( OwnerEntity )
		rocketBank.TriggerButton = GAMEPAD_BUTTON_LTRIGGER		
		
		BaseTurretLogic.OnSpawn( )
	}	
	
	function SetMotionMap( )
	{
		Animatable.MotionMap = USA_Flame_03_MotionMap( this )
	}
}
