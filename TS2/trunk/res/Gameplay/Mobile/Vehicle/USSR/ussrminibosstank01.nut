sigimport "Gameplay/mobile/common/WheeledMobileLogic.nut"
sigimport "Gameplay/mobile/common/MobileTurretLogic.nut"
sigimport "Anims/Vehicles/Blue/tank_m1a1abrams_maingun/tank_m1a1abrams_maingun.anipk"
sigimport "Anims/Vehicles/Blue/tank_m1a1abrams_largemgun/tank_m1a1abrams_largemgun.anipk"
sigimport "Anims/Vehicles/Blue/tank_m1a1abrams_smallmgun/tank_m1a1abrams_smallmgun.anipk"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Miniboss_Tank_01( )
}


class USSR_Miniboss_Tank_01 extends ScriptWheeledVehicleLogic
{
	constructor( )
	{
		ScriptWheeledVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USSR_Miniboss_Tank_01"

	function OnSpawn( )
	{
		local cannonBank = WeaponStation( 0 ).Bank( 0 )
		cannonBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
		
		local gunBank = WeaponStation( 0 ).Bank( 1 )
		gunBank.TriggerButton = GAMEPAD_BUTTON_LTRIGGER
		
		local gun = gunBank.AddWeapon( "USSR_T55_MG", "topgun" )
		local gunE = gun.SetTurretEntityNamed( "topgun" )
		gunE.Logic.Animatable.MotionMap = USSR_Tank_Miniboss_01_SecGunMoMap( )
		gunE.Logic.Animatable.ExecuteMotionState( "Idle", { Weapon = gun } )
		
		local gun2 = gunBank.AddWeapon( "USSR_T55_MG", "mgs" )
		local gunE2 = gun2.SetTurretEntityNamed( "mgs" )
		gunE2.Logic.Animatable.MotionMap = USSR_Tank_Miniboss_01_MiniGunMoMap( )
		gunE2.Logic.Animatable.ExecuteMotionState( "Idle", { Weapon = gun2 } )
		
		local gun3 = gunBank.AddWeapon( "TURRET_RPG7_AT", "launchers" )
		local gunE3 = gun3.SetTurretEntityNamed( "launchers" )
//		gunE3.Logic.Animatable.MotionMap = USSR_Tank_Miniboss_01_MiniGunMoMap( )
//		gunE3.Logic.Animatable.ExecuteMotionState( "Idle", { Weapon = gun3 } )
		
		local cannon = cannonBank.AddWeapon( "USSR_T55_CANNON", "cannon" )
		local cannonE = cannon.SetTurretEntityNamed( "cannon" )
		cannonE.Logic.Animatable.MotionMap = USSR_Tank_Miniboss_01_MainGunMoMap( )
		cannonE.Logic.Animatable.ExecuteMotionState( "Idle", { Weapon = cannon } )
		
		SetDestroyedEffect( "Medium_Vehicle_Explosion" )
		
		ScriptWheeledVehicleLogic.OnSpawn( )
	}
}

class USSR_Tank_Miniboss_01_MainGunMoMap extends MobileTurretMoMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Blue/tank_m1a1abrams_maingun/tank_m1a1abrams_maingun.anipk" )
	}
}

class USSR_Tank_Miniboss_01_SecGunMoMap extends MobileTurretMoMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Blue/tank_m1a1abrams_largemgun/tank_m1a1abrams_largemgun.anipk" )
	}
}


class USSR_Tank_Miniboss_01_MiniGunMoMap extends MobileTurretMoMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Blue/tank_m1a1abrams_smallmgun/tank_m1a1abrams_smallmgun.anipk" )
	}
}