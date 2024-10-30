sigimport "Gameplay/mobile/common/WheeledMobileLogic.nut"
sigimport "Gameplay/mobile/common/MobileTurretLogic.nut"
sigimport "Anims/Vehicles/Blue/tank_m1a1abrams_maingun/tank_m1a1abrams_maingun.anipk"
sigimport "Anims/Vehicles/Blue/tank_m1a1abrams_largemgun/tank_m1a1abrams_largemgun.anipk"
sigimport "Anims/Vehicles/Blue/tank_m1a1abrams_smallmgun/tank_m1a1abrams_smallmgun.anipk"
sigimport "gui/textures/waveicons/USSR/infantry_lvl1_g.png"
sigimport "gameplay/characters/infantry/ussr/infantry_basic_01.sigml"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Miniboss_Tank_02( )
}


class USSR_Miniboss_Tank_02 extends ScriptWheeledVehicleLogic
{
	constructor( )
	{
		ScriptWheeledVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USSR_Miniboss_Tank_02"

	function OnSpawn( )
	{
		local cannonBank = WeaponStation( 0 ).Bank( 0 )
		cannonBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
		
		local gunBank = WeaponStation( 0 ).Bank( 1 )
		gunBank.TriggerButton = GAMEPAD_BUTTON_LTRIGGER
		
		local gun = gunBank.AddWeapon( "USSR_T55_CANNON", "topgun" )
		local gunE = gun.SetTurretEntityNamed( "topgun" )
		gunE.Logic.Animatable.MotionMap = USSR_Tank_Heavy_02_SecGunMoMap( )
		gunE.Logic.Animatable.ExecuteMotionState( "Idle", { Weapon = gun } )
		
		local gun2 = gunBank.AddWeapon( "USSR_T55_MG", "mgs" )
		local gunE2 = gun2.SetTurretEntityNamed( "mgs" )
		gunE2.Logic.Animatable.MotionMap = USSR_Tank_Heavy_02_MiniGunMoMap( )
		gunE2.Logic.Animatable.ExecuteMotionState( "Idle", { Weapon = gun2 } )
		
		local gun3 = gunBank.AddWeapon( "TURRET_M120_MORTAR", "" )
		local gunE3 = gun3.SetTurretEntityNamed( "" )
//		gunE3.Logic.Animatable.MotionMap = USSR_Tank_Heavy_02_MiniGunMoMap( )
//		gunE3.Logic.Animatable.ExecuteMotionState( "Idle", { Weapon = gun3 } )
		
		
		ScriptWheeledVehicleLogic.OnSpawn( )
		SetDestroyedEffect( "Medium_Vehicle_Explosion" )

		// The syntax for AddCargo is path, spawnrate, should_stop_while_dropping, remove_cargo_after_dropping
		// Please remove the comment above and make this take a string into the cargo table: AddCargo( "gameplay/characters/infantry/ussr/infantry_basic_01.sigml", 20, 1, 1, 0 )
	}
}

class USSR_Tank_Heavy_02_MainGunMoMap extends MobileTurretMoMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Blue/tank_m1a1abrams_maingun/tank_m1a1abrams_maingun.anipk" )
	}
}

class USSR_Tank_Heavy_02_SecGunMoMap extends MobileTurretMoMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Blue/tank_m1a1abrams_largemgun/tank_m1a1abrams_largemgun.anipk" )
	}
}


class USSR_Tank_Heavy_02_MiniGunMoMap extends MobileTurretMoMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Blue/tank_m1a1abrams_smallmgun/tank_m1a1abrams_smallmgun.anipk" )
	}
}