sigimport "Gameplay/mobile/common/WheeledMobileLogic.nut"
sigimport "gameplay/characters/infantry/usa/infantry_elite_01.sigml"
sigimport "gui/textures/waveicons/USA/infantry_lvl1_g.png"
sigimport "gui/textures/waveicons/USA/vehicle_apc05_g.png"
sigimport "Gameplay/mobile/common/MobileTurretLogic.nut"
sigimport "Anims/Vehicles/Blue/tank_m60patton_maingun/tank_m60patton_maingun.anipk"
sigimport "Anims/Vehicles/Blue/tank_m60patton_lilgun/tank_m60patton_lilgun.anipk"
sigimport "Gameplay/mobile/vehicle/usa/usaapcmg01_momap.nut"
sigimport "Anims/Vehicles/Blue/apc_m2bradley/apc_m2bradley.anipk"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_IFV_01( )
}


class USA_IFV_01 extends ScriptWheeledVehicleLogic
{
	constructor( )
	{
		ScriptWheeledVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USA_IFV_01"

	function OnSpawn( )
	{
// USA Bradley Chaingun, USA Bradley TOW

		local gunBank = WeaponStation( 0 ).Bank( 1 )
		local missileBank = WeaponStation( 0 ).Bank( 0 )
		gunBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
		missileBank.TriggerButton = GAMEPAD_BUTTON_LTRIGGER
			
		local gun = gunBank.AddWeapon( "USA_BRADLEY_CHAINGUN", "cannon")
		local gunE = gun.SetTurretEntityNamed( "cannon" )
		gunE.Logic.Animatable.MotionMap = USA_APCIFV_01_GunMoMap( )
		gunE.Logic.Animatable.ExecuteMotionState( "Idle", { Weapon = gun } )

		local missile = missileBank.AddWeapon( "USA_BRADLEY_TOW", "missile")
		local missileE = missile.SetTurretEntityNamed( "missile" )
		missileE.Logic.Animatable.MotionMap = USA_APCIFV_01_LilGunMoMap( )
		missileE.Logic.Animatable.ExecuteMotionState( "Idle", { Weapon = missile } )

		ScriptWheeledVehicleLogic.OnSpawn( )

		AddCargo( "USA_IFV" )
		SetDestroyedEffect( "Medium_Vehicle_Explosion" )
	}
	
	function SetMotionMap( ) 
		Animatable.MotionMap = USA_APCIFV01MoMap( this )
}

class USA_APCIFV01MoMap extends USA_APCMG01MoMap 
{	
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Blue/apc_m2bradley/apc_m2bradley.anipk" )
	}
}
class USA_APCIFV_01_GunMoMap extends MobileTurretMoMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Blue/tank_m60patton_maingun/tank_m60patton_maingun.anipk" )
	}
}

class USA_APCIFV_01_LilGunMoMap extends MobileTurretMoMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Blue/tank_m60patton_lilgun/tank_m60patton_lilgun.anipk" )
	}
}
