sigimport "Gameplay/mobile/common/WheeledMobileLogic.nut"
sigimport "gameplay/characters/infantry/ussr/infantry_basic_01.sigml"
sigimport "Gameplay/mobile/common/MobileTurretLogic.nut"
sigimport "gui/textures/waveicons/USA/vehicle_atv_g.png"
sigimport "Anims/Vehicles/Blue/tank_m60patton/tank_m60patton.anipk"
sigimport "Anims/Vehicles/Blue/tank_m60patton_lilgun/tank_m60patton_lilgun.anipk"
sigimport "Anims/Vehicles/Blue/tank_m60patton_maingun/tank_m60patton_maingun.anipk"
sigimport "gameplay/mobile/vehicle/usa/usatankmedium01_motionmap.nut"
sigimport "gameplay/characters/passenger/common/passengermomap.nut"


sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_CYCLE_01( )
}


class USA_CYCLE_01 extends ScriptWheeledVehicleLogic
{
	constructor( )
	{
		ScriptWheeledVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USA_CYCLE_01"

	function OnSpawn( )
	{
		SetDestroyedEffect( "ATV_Explode" )

		local gunBank = WeaponStation( 0 ).Bank( 1 )
		gunBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER


		local gun = gunBank.AddWeapon( "CYCLE_M40RECOILLESS_AT", "lilgun" )
		local gunE = gun.SetTurretEntityNamed( "lilgun" )
		gunE.Logic.Animatable.MotionMap = USA_Cycle_01_MiniGunMoMap( )
		gunE.Logic.Animatable.ExecuteMotionState( "Idle", { Weapon = gun } )

		ScriptWheeledVehicleLogic.OnSpawn( )
	
	}
}

class USA_Cycle_01_GunMoMap extends MobileTurretMoMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Blue/tank_m60patton_maingun/tank_m60patton_maingun.anipk" )
	}

}

class USA_Cycle_01_MiniGunMoMap extends MobileTurretMoMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Blue/tank_m60patton_lilgun/tank_m60patton_lilgun.anipk" )
	}
}

class USACycleRiderMoMap extends ArtillerySoldierDefaultMoMap
{
	animPack = null
	constructor()
	{
		Anim.MotionMap.constructor()
		sharedAnimPack = GetAnimPack("Anims/Characters/Shared/Base_Soldier/artillery/artillery.anipk")
		animPack = GetAnimPack("Anims/Characters/Blue/artillery/artillery.anipk")
	}
	
	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "supervespa_cycle_idle_crewmana" )
		track.BlendIn = 0.1
		track.BlendOut = 0.0
		
		track.Push( Stack )
	}
	
}