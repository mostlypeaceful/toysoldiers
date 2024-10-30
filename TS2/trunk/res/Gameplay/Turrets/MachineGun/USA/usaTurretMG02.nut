
sigimport "Gameplay/Turrets/Common/TurretLogic.nut"
sigimport "gameplay/turrets/machinegun/usa/usa_machinegun_02_motionmap.nut"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_Turret_MG_02( );
}
sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.CreateDefaultArtillerySoldiers( entity )
}
class USA_Turret_MG_02 extends BaseTurretLogic
{
	function DebugTypeName( )
		return "USA_Turret_MG_02"

	function OnSpawn( )
	{
		WeaponStation( 0 ).Bank( 0 ).AddWeapon( "TURRET_M60_MG", "" )

//		local rocketBank = WeaponStation( 0 ).Bank( 1 )
//		rocketBank.AddWeapon( "TURRET_M60_ALT", "" ).SetTurretEntity( OwnerEntity )
//		rocketBank.TriggerButton = GAMEPAD_BUTTON_LTRIGGER
		
		BaseTurretLogic.OnSpawn( )
	}	
	
	function SetMotionMap( )
	{
		Animatable.MotionMap = USA_MachineGun_02_MotionMap( this )
	}
}
