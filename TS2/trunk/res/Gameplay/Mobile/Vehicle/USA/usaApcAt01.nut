sigimport "Gameplay/mobile/common/WheeledMobileLogic.nut"
sigimport "gameplay/characters/infantry/ussr/infantry_basic_01.sigml"
sigimport "Gameplay/mobile/common/MobileTurretLogic.nut"
sigimport "Anims/Vehicles/Blue/tank_m60patton_lilgun/tank_m60patton_lilgun.anipk"
sigimport "gui/textures/waveicons/USA/infantry_lvl1_g.png"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_APC_AT_01( )
}


class USA_APC_AT_01 extends ScriptWheeledVehicleLogic
{
	constructor( )
	{
		ScriptWheeledVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USA_APC_AT_01"

	function OnSpawn( )
	{
		local gunBank = WeaponStation( 0 ).Bank( 1 )
//		local cannonBank = WeaponStation( 0 ).Bank( 0 )
		gunBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
//		cannonBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
			
		local gun = gunBank.AddWeapon( "USA_M113_TOW", "lilgun" )
		local gunE = gun.SetTurretEntityNamed( "lilgun" )
		gunE.Logic.Animatable.MotionMap = USSR_APCMG_01_GunMoMap( )
		gunE.Logic.Animatable.ExecuteMotionState( "Idle", { Weapon = gun } )
		SetDestroyedEffect( "Small_Vehicle_Explosion" )
		
// USA M113 TOW
		ScriptWheeledVehicleLogic.OnSpawn( )		
		// The syntax for AddCargo is path, spawnrate, should_stop_while_dropping, remove_cargo_after_dropping
		// Please remove the comment above and make this take a string into the cargo table: AddCargo( "gameplay/characters/infantry/usa/infantry_basic_01.sigml", 5, 1, true, 0 )
	}
}
