sigimport "Gameplay/mobile/common/WheeledMobileLogic.nut"
sigimport "Gameplay/Mobile/Vehicle/Common/wheeledmobilemomap.nut"
sigimport "gameplay/characters/infantry/ussr/infantry_elite_01.sigml"
sigimport "gui/textures/waveicons/ussr/infantry_lvl2_g.png"
sigimport "gui/textures/waveicons/ussr/vehicle_apc05_g.png"
sigimport "Gameplay/mobile/common/MobileTurretLogic.nut"
sigimport "Anims/Vehicles/Blue/tank_m60patton_maingun/tank_m60patton_maingun.anipk"
sigimport "Anims/Vehicles/Blue/tank_m60patton_lilgun/tank_m60patton_lilgun.anipk"
sigimport "Anims/Vehicles/Red/apc_bmp/apc_bmp.anipk"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_IFV_01( )
}


class USSR_IFV_01 extends ScriptWheeledVehicleLogic
{
	constructor( )
	{
		ScriptWheeledVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USSR_IFV_01"

	function OnSpawn( )
	{
		

		local gunBank = WeaponStation( 0 ).Bank( 1 )
		local missileBank = WeaponStation( 0 ).Bank( 0 )
		gunBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
		missileBank.TriggerButton = GAMEPAD_BUTTON_LTRIGGER

		local gun = gunBank.AddWeapon( "USSR_BMP_CANNON", "cannon")
		local gunE = gun.SetTurretEntityNamed( "cannon" )
		gunE.Logic.Animatable.MotionMap = USSR_APCIFV_01_GunMoMap( )
		gunE.Logic.Animatable.ExecuteMotionState( "Idle", { Weapon = gun } )

		local missile = missileBank.AddWeapon( "USSR_BMP_MISSILE", "missile")
		local missileE = missile.SetTurretEntityNamed( "missile" )
		missileE.Logic.Animatable.MotionMap = USSR_APCIFV_01_LilGunMoMap( )
		missileE.Logic.Animatable.ExecuteMotionState( "Idle", { Weapon = missile } )

		
		ScriptWheeledVehicleLogic.OnSpawn( )
		
		AddCargo( "USSR_IFV" )
		SetDestroyedEffect( "Medium_Vehicle_Explosion" )

	}
	function SetMotionMap( ) 
		Animatable.MotionMap = USSR_APCIFV01MoMap( this )
}

class USSR_APCIFV01MoMap extends VehicleMotionMap 
{		
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Red/apc_bmp/apc_bmp.anipk" )
	}
	
	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "idle" )
		track.BlendIn = 4.0
		track.BlendOut = 0.0
		
		track.Push( Stack )
		TankTreads( params )
	}
	
	function Forward( params )
	{	
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "forward" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		
		track.Push( Stack )		
		TankTreads( params )
	}
	
	function CargoBegin( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "deploy" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		track.Push( Stack )

		return (track.Anim.OneShotLength - 0.2)
	}
	
	function CargoIdle( params )
	{
		/*local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "hover" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		
		track.Push( Stack )*/
	}
	
	function CargoEnd( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "retract" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_CLAMP_TIME		
		track.Push( Stack )

		return (track.Anim.OneShotLength)
	}
}
class USSR_APCIFV_01_GunMoMap extends MobileTurretMoMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Blue/tank_m60patton_maingun/tank_m60patton_maingun.anipk" )
	}
}

class USSR_APCIFV_01_LilGunMoMap extends MobileTurretMoMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Blue/tank_m60patton_lilgun/tank_m60patton_lilgun.anipk" )
	}
}
