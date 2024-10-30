sigimport "Gameplay/mobile/common/WheeledMobileLogic.nut"
sigimport "gameplay/characters/infantry/ussr/infantry_basic_01.sigml"
sigimport "Gameplay/mobile/common/MobileTurretLogic.nut"
sigimport "anims/vehicles/blue/tank_m1a1abrams_smallmgun/tank_m1a1abrams_smallmgun.anipk"
sigimport "gui/textures/waveicons/ussr/vehicle_tank03_g.png"


sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_ScrewTank_01( )
}


class USSR_ScrewTank_01 extends ScriptWheeledVehicleLogic
{
	constructor( )
	{
		ScriptWheeledVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USSR_ScrewTank_01"

	function OnSpawn( )
	{

		local gunBank = WeaponStation( 0 ).Bank( 1 )
//		local cannonBank = WeaponStation( 0 ).Bank( 0 )
		gunBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
//		cannonBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
			
		local gun = gunBank.AddWeapon( "USSR_SCREW_TANK_MG", "lilgun" )
		local gunE = gun.SetTurretEntityNamed( "lilgun" )
		gunE.Logic.Animatable.MotionMap = USSR_ScrewTank_01_GunMoMap( )
		gunE.Logic.Animatable.ExecuteMotionState( "Idle", { Weapon = gun } )
		
		// set treads
		SetWheelDustSigmlPath( "" )

		ScriptWheeledVehicleLogic.OnSpawn( )
		
		// The syntax for AddCargo is path, spawnrate, should_stop_while_dropping, remove_cargo_after_dropping
		// Please remove the comment above and make this take a string into the cargo table: AddCargo( "gameplay/characters/infantry/ussr/infantry_basic_01.sigml", 3, 1, 1, 0 )
	}
}


class USSR_ScrewTank_01_GunMoMap extends MobileTurretMoMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "anims/vehicles/blue/tank_m1a1abrams_smallmgun/tank_m1a1abrams_smallmgun.anipk" )
	}
}