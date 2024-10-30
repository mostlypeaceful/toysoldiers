sigimport "Gameplay/mobile/common/WheeledMobileLogic.nut"
sigimport "Gameplay/mobile/common/MobileTurretLogic.nut"
sigimport "Anims/Vehicles/Blue/tank_m60patton_maingun/tank_m60patton_maingun.anipk"
sigimport "Anims/Vehicles/Blue/tank_m60patton_lilgun/tank_m60patton_lilgun.anipk"
sigimport "gameplay/mobile/vehicle/usa/usatankmedium01_motionmap.nut"
sigimport "gui/textures/waveicons/usa/vehicle_tank01_g.png"
sigimport "Anims/Vehicles/Blue/tank_m60patton/tank_m60patton.anipk"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_Tank_Medium_01( )
}

class USA_Tank_Medium_01 extends ScriptWheeledVehicleLogic
{
	constructor( )
	{
		ScriptWheeledVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USA_Tank_Medium_01"

	function OnSpawn( )
	{		
		local cannonBank = WeaponStation( 0 ).Bank( 0 )
		cannonBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
		
		local gunBank = WeaponStation( 0 ).Bank( 1 )
		gunBank.TriggerButton = GAMEPAD_BUTTON_LTRIGGER
		
		local gun = gunBank.AddWeapon( "USA_M60_MG", "lilgun" )
		local gunE = gun.SetTurretEntityNamed( "lilgun" )
		gunE.Logic.Animatable.MotionMap = USA_Tank_Medium_01_SecGunMoMap( )
		gunE.Logic.Animatable.ExecuteMotionState( "Idle", { Weapon = gun } )
		
		
		local cannon = cannonBank.AddWeapon( "USA_M60_CANNON", "cannon" )
		local cannonE = cannon.SetTurretEntityNamed( "cannon" )
		cannonE.Logic.Animatable.MotionMap = USA_Tank_Medium_01_MainGunMoMap( )
		cannonE.Logic.Animatable.ExecuteMotionState( "Idle", { Weapon = cannon } )
		
		ScriptWheeledVehicleLogic.OnSpawn( )
		SetDestroyedEffect( "Medium_Vehicle_Explosion" )
	}
	
	function SetMotionMap( ) 
		Animatable.MotionMap = TankVehicleMotionMap( this, "Anims/Vehicles/Blue/tank_m60patton/tank_m60patton.anipk" )
}

class USA_Tank_Medium_01_MainGunMoMap extends MobileTurretMoMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Blue/tank_m60patton_maingun/tank_m60patton_maingun.anipk" )
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

class USA_Tank_Medium_01_SecGunMoMap extends MobileTurretMoMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Blue/tank_m60patton_lilgun/tank_m60patton_lilgun.anipk" )
	}
}


