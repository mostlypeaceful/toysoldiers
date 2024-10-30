sigimport "Gameplay/mobile/common/WheeledMobileLogic.nut"
sigimport "Gameplay/mobile/common/MobileTurretLogic.nut"
sigimport "gui/textures/waveicons/ussr/vehicle_car_g.png"
sigimport "Anims/Vehicles/Blue/tank_m60patton_lilgun/tank_m60patton_lilgun.anipk"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Car_01( )
}


class USSR_Car_01 extends ScriptWheeledVehicleLogic
{
	constructor( )
	{
		ScriptWheeledVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USSR_Car_01"

	function OnSpawn( )
	{

		local gunBank = WeaponStation( 0 ).Bank( 1 )
//		local cannonBank = WeaponStation( 0 ).Bank( 0 )
		gunBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
//		cannonBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
			
		local gun = gunBank.AddWeapon( "USSR_BRDM_MG", "lilgun" )
		local gunE = gun.SetTurretEntityNamed( "lilgun" )
		gunE.Logic.Animatable.MotionMap = USSR_Car_01_GunMoMap( )
		gunE.Logic.Animatable.ExecuteMotionState( "Idle", { Weapon = gun } )

		ScriptWheeledVehicleLogic.OnSpawn( )
		
		SetDestroyedEffect( "Small_Vehicle_Explosion" )
		// The syntax for AddCargo is path, spawnrate, should_stop_while_dropping, remove_cargo_after_dropping
		// Please remove the comment above and make this take a string into the cargo table: AddCargo( "gameplay/characters/infantry/ussr/infantry_basic_01.sigml", 3, 1, 1, 0 )
	}
}


class USSR_Car_01_GunMoMap extends MobileTurretMoMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Blue/tank_m60patton_lilgun/tank_m60patton_lilgun.anipk" )
	}
}