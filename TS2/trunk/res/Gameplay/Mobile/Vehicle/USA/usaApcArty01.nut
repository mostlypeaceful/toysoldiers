sigimport "Gameplay/mobile/common/WheeledMobileLogic.nut"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_APC_Arty_01( )
}


class USA_APC_Arty_01 extends ScriptWheeledVehicleLogic
{
	constructor( )
	{
		ScriptWheeledVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USA_APC_Arty_01"

	function OnSpawn( )
	{
		local gunBank = WeaponStation( 0 ).Bank( 1 )
//		local cannonBank = WeaponStation( 0 ).Bank( 0 )
		gunBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
//		cannonBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
			
		local gun = gunBank.AddWeapon( "USA_M113_MORTAR", "lilgun" )
		local gunE = gun.SetTurretEntityNamed( "lilgun" )
		gunE.Logic.Animatable.MotionMap = USSR_APCMG_01_GunMoMap( )
		gunE.Logic.Animatable.ExecuteMotionState( "Idle", { Weapon = gun } )
		SetDestroyedEffect( "Small_Vehicle_Explosion" )
		

// USA M113 Mortar
		ScriptWheeledVehicleLogic.OnSpawn( )
	}
}
