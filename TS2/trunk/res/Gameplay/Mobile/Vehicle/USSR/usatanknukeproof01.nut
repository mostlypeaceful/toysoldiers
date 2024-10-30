sigimport "Gameplay/mobile/common/WheeledMobileLogic.nut"
sigimport "Gameplay/mobile/common/MobileTurretLogic.nut"
sigimport "Anims/Vehicles/Red/tank_lasertank_maingun/tank_lasertank_maingun.anipk"
sigimport "gui/textures/waveicons/USSR/vehicle_tank01_g.png"
sigimport "Anims/Vehicles/Red/tank_lasertank/tank_lasertank.anipk"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_Tank_Nukeproof_01( )
}

class USA_Tank_Nukeproof_01 extends ScriptWheeledVehicleLogic
{
	constructor( )
	{
		ScriptWheeledVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USA_Tank_Nukeproof_01"

	function OnSpawn( )
	{
		
//		local gunBank = WeaponStation( 0 ).Bank( 1 )
		local cannonBank = WeaponStation( 0 ).Bank( 0 )
//		gunBank.TriggerButton = GAMEPAD_BUTTON_LTRIGGER
		cannonBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
			
//		local gun = gunBank.AddWeapon( "USSR_T55_MG", "lilgun" )
//		local gunE = gun.SetTurretEntityNamed( "lilgun" )
//		gunE.Logic.Animatable.MotionMap = USSR_Tank_Nukeproof_01_SecGunMoMap( )
//		gunE.Logic.Animatable.ExecuteMotionState( "Idle", { Weapon = gun } )

		local cannon = cannonBank.AddWeapon( "USSR_NUKEPROOF_TANK_LASER", "cannon" )
		local cannonE = cannon.SetTurretEntityNamed( "cannon" )
		cannonE.Logic.Animatable.MotionMap = USA_Tank_Nukeproof_01_GunMoMap( )
		cannonE.Logic.Animatable.ExecuteMotionState( "Idle", { Weapon = cannon } )
		
		ScriptWheeledVehicleLogic.OnSpawn( )
		SetDestroyedEffect( "Medium_Vehicle_Explosion" )
	}
	function SetMotionMap( ) 
		Animatable.MotionMap = TankVehicleMotionMap( this, "Anims/Vehicles/Red/tank_lasertank/tank_lasertank.anipk" )
}


class USA_Tank_Nukeproof_01_GunMoMap extends MobileTurretMoMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Red/tank_lasertank_maingun/tank_lasertank_maingun.anipk" )
	}
}

//class USSR_Tank_Nukeproof_01_SecGunMoMap extends MobileTurretMoMap
//{
//	function SetAnimPack( )
//	{
//		animPack = GetAnimPack( "Anims/Vehicles/Blue/tank_m60patton_lilgun/tank_m60patton_lilgun.anipk" )
//	}
//}