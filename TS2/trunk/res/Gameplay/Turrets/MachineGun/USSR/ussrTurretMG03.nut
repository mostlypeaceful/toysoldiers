
sigimport "Gameplay/Turrets/Common/TurretLogic.nut"
sigimport "gameplay/turrets/machinegun/ussr/ussr_machinegun_03_motionmap.nut"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Turret_MG_03( );
}
sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.CreateDefaultArtillerySoldiers( entity )
}

class USSR_Turret_MG_03 extends BaseTurretLogic
{
	function DebugTypeName( )
		return "USSR_Turret_MG_03"

	function OnSpawn( )
	{
		WeaponStation( 0 ).Bank( 0 ).AddWeapon( "TURRET_BALKAN1GRENADE_MG", "" )

		local rocketBank = WeaponStation( 0 ).Bank( 1 )
		rocketBank.AddWeapon( "TURRET_BALKAN1GRENADE_ALT", "" ).SetTurretEntity( OwnerEntity )
		rocketBank.TriggerButton = GAMEPAD_BUTTON_LTRIGGER
		
		BaseTurretLogic.OnSpawn( )
	}	
	
	function SetMotionMap( )
	{
		Animatable.MotionMap = USSR_MachineGun_03_MotionMap( this )
	}
}
