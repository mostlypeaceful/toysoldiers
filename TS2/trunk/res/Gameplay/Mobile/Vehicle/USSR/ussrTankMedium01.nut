sigimport "Gameplay/mobile/common/WheeledMobileLogic.nut"
sigimport "Gameplay/mobile/common/MobileTurretLogic.nut"
sigimport "Anims/Vehicles/Red/tank_t55/tank_t55.anipk"
sigimport "Anims/Vehicles/Red/tank_t55_maingun/tank_t55_maingun.anipk"
sigimport "Anims/Vehicles/Blue/tank_m60patton_lilgun/tank_m60patton_lilgun.anipk"
sigimport "gui/textures/waveicons/USSR/vehicle_tank01_g.png"
sigimport "gameplay/mobile/vehicle/ussr/ussrtankmedium01_motionmap.nut"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Tank_Medium_01( )
}

class USSR_Tank_Medium_01 extends ScriptWheeledVehicleLogic
{
	constructor( )
	{
		ScriptWheeledVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USSR_Tank_Medium_01"

	function OnSpawn( )
	{
		
		local gunBank = WeaponStation( 0 ).Bank( 1 )
		local cannonBank = WeaponStation( 0 ).Bank( 0 )
		gunBank.TriggerButton = GAMEPAD_BUTTON_LTRIGGER
		cannonBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
			
		local gun = gunBank.AddWeapon( "USSR_T55_MG", "lilgun" )
		local gunE = gun.SetTurretEntityNamed( "lilgun" )
		gunE.Logic.Animatable.MotionMap = USSR_Tank_Medium_01_SecGunMoMap( )
		gunE.Logic.Animatable.ExecuteMotionState( "Idle", { Weapon = gun } )
	

		local cannon = cannonBank.AddWeapon( "USSR_T55_CANNON", "cannon" )
		local cannonE = cannon.SetTurretEntityNamed( "cannon" )
		cannonE.Logic.Animatable.MotionMap = USSR_Tank_Medium_01_GunMoMap( )
		cannonE.Logic.Animatable.ExecuteMotionState( "Idle", { Weapon = cannon } )
		
		// USSR T55 Cannon, USSR T55 MG
		ScriptWheeledVehicleLogic.OnSpawn( )
		SetDestroyedEffect( "Medium_Vehicle_Explosion" )
	}
	
	function SetMotionMap( ) 
		Animatable.MotionMap = TankVehicleMotionMap( this, "Anims/Vehicles/Red/tank_t55/tank_t55.anipk" )
}


class USSR_Tank_Medium_01_GunMoMap extends MobileTurretMoMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Red/tank_t55_maingun/tank_t55_maingun.anipk" )
	}
	
	function Recoil( params )
	{		
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "fire" )
		track.BlendIn = 0.0
		track.BlendOut = 0.1
		track.Push( Stack )
		track.Flags = ANIM_TRACK_CLAMP_TIME;
			
		Logic.OwnerVehicle.ApplyMotionStateToSoldiers( "Recoil" )
		
		return track.Anim.OneShotLength
	}
}

class USSR_Tank_Medium_01_SecGunMoMap extends MobileTurretMoMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Blue/tank_m60patton_lilgun/tank_m60patton_lilgun.anipk" )
	}
}