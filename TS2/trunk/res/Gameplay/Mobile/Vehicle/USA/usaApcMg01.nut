sigimport "Gameplay/mobile/common/WheeledMobileLogic.nut"
sigimport "gameplay/characters/infantry/usa/infantry_basic_01.sigml"
sigimport "Gameplay/mobile/common/MobileTurretLogic.nut"
sigimport "Gameplay/mobile/vehicle/usa/usaapcmg01_momap.nut"
sigimport "Anims/Vehicles/Blue/tank_m60patton_lilgun/tank_m60patton_lilgun.anipk"
sigimport "gui/textures/waveicons/USA/infantry_lvl1_g.png"
sigimport "gui/textures/waveicons/USA/vehicle_apc01_g.png"



sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_APC_MG_01( )
}


class USA_APC_MG_01 extends ScriptWheeledVehicleLogic
{
	constructor( )
	{
		ScriptWheeledVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USA_APC_MG_01"

	function OnSpawn( )
	{
// USA M113 MG

		local gunBank = WeaponStation( 0 ).Bank( 1 )
//		local cannonBank = WeaponStation( 0 ).Bank( 0 )
		gunBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
//		cannonBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
			
		local gun = gunBank.AddWeapon( "USA_M113_MG", "lilgun" )
		local gunE = gun.SetTurretEntityNamed( "lilgun" )
		gunE.Logic.Animatable.MotionMap = USA_APCMG_01_GunMoMap( )
		gunE.Logic.Animatable.ExecuteMotionState( "Idle", { Weapon = gun } )

		
		ScriptWheeledVehicleLogic.OnSpawn( )
		
		AddCargo( "USA_APC" )
		SetDestroyedEffect( "Small_Vehicle_Explosion" )

	}
	
	function SetMotionMap( ) 
		Animatable.MotionMap = USA_APCMG01MoMap( this )
}

class USA_APCMG_01_GunMoMap extends MobileTurretMoMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Blue/tank_m60patton_lilgun/tank_m60patton_lilgun.anipk" )
	}
}