sigimport "Gameplay/mobile/common/WheeledMobileLogic.nut"
sigimport "gameplay/characters/infantry/ussr/infantry_basic_01.sigml"
sigimport "Gameplay/mobile/common/MobileTurretLogic.nut"
sigimport "Gameplay/mobile/vehicle/ussr/ussrapcmg01_momap.nut"
sigimport "Anims/Vehicles/Blue/tank_m60patton_lilgun/tank_m60patton_lilgun.anipk"
sigimport "gui/textures/waveicons/USSR/infantry_lvl1_g.png"
sigimport "gui/textures/waveicons/USSR/vehicle_apc04_g.png"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_APC_MG_01( )
}


class USSR_APC_MG_01 extends ScriptWheeledVehicleLogic
{
	constructor( )
	{
		ScriptWheeledVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USSR_APC_MG_01"

	function OnSpawn( )
	{
		
		
		local gunBank = WeaponStation( 0 ).Bank( 1 )
//		local cannonBank = WeaponStation( 0 ).Bank( 0 )
		gunBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
//		cannonBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
			
		local gun = gunBank.AddWeapon( "USSR_BTR60_MG", "lilgun" )
		local gunE = gun.SetTurretEntityNamed( "lilgun" )
		gunE.Logic.Animatable.MotionMap = USSR_APCMG_01_GunMoMap( )
		gunE.Logic.Animatable.ExecuteMotionState( "Idle", { Weapon = gun } )

		ScriptWheeledVehicleLogic.OnSpawn( )
		
		AddCargo( "USSR_APC" )
		SetDestroyedEffect( "Medium_Vehicle_Explosion" )

	}
	
	function SetMotionMap( ) 
		Animatable.MotionMap = USSR_APCMG01MoMap( this )
}



class USSR_APCMG_01_GunMoMap extends MobileTurretMoMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Blue/tank_m60patton_lilgun/tank_m60patton_lilgun.anipk" )
	}
}