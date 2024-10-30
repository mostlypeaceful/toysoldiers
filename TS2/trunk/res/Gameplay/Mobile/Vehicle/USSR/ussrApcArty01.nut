sigimport "Gameplay/mobile/common/WheeledMobileLogic.nut"
sigimport "Gameplay/mobile/common/MobileTurretLogic.nut"
sigimport "Anims/Vehicles/Blue/tank_m60patton_lilgun/tank_m60patton_lilgun.anipk"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_APC_Arty_01( )
}


class USSR_APC_Arty_01 extends ScriptWheeledVehicleLogic
{
	constructor( )
	{
		ScriptWheeledVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USSR_APC_Arty_01"

	function OnSpawn( )
	{

		local gunBank = WeaponStation( 0 ).Bank( 1 )
//		local cannonBank = WeaponStation( 0 ).Bank( 0 )
		gunBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
//		cannonBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
			
		local gun = gunBank.AddWeapon( "USSR_BTR60_MORTAR", "lilgun" )
		local gunE = gun.SetTurretEntityNamed( "lilgun" )
		gunE.Logic.Animatable.MotionMap = USSR_APCARTY_01_GunMoMap( )
		gunE.Logic.Animatable.ExecuteMotionState( "Idle", { Weapon = gun } )

		SetDestroyedEffect( "Small_Vehicle_Explosion" )
		ScriptWheeledVehicleLogic.OnSpawn( )
	}
}


class USSR_APCARTY_01_GunMoMap extends MobileTurretMoMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Blue/tank_m60patton_lilgun/tank_m60patton_lilgun.anipk" )
	}
}