sigimport "Gameplay/mobile/common/WheeledMobileLogic.nut"
sigimport "gameplay/characters/infantry/ussr/infantry_basic_01.sigml"
sigimport "Gameplay/mobile/common/MobileTurretLogic.nut"
sigimport "Anims/Vehicles/Blue/tank_m60patton_maingun/tank_m60patton_maingun.anipk"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_APC_AT_01( )
}


class USSR_APC_AT_01 extends ScriptWheeledVehicleLogic
{
	constructor( )
	{
		ScriptWheeledVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USSR_APC_AT_01"

	function OnSpawn( )
	{
		

//		local gunBank = WeaponStation( 0 ).Bank( 1 )
		local cannonBank = WeaponStation( 0 ).Bank( 0 )
//		gunBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
		cannonBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER

		local cannon = cannonBank.AddWeapon( "USSR_BTR80_CANNON", "cannon" )
		local cannonE = cannon.SetTurretEntityNamed( "cannon" )
		cannonE.Logic.Animatable.MotionMap = USSR_APCAT_01_GunMoMap( )
		cannonE.Logic.Animatable.ExecuteMotionState( "Idle", { Weapon = cannon } )
		
		SetDestroyedEffect( "Medium_Vehicle_Explosion" )

		ScriptWheeledVehicleLogic.OnSpawn( )
		
	}
}

class USSR_APCAT_01_GunMoMap extends MobileTurretMoMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Blue/tank_m60patton_maingun/tank_m60patton_maingun.anipk" )
	}
}