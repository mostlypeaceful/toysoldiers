sigimport "Gameplay/mobile/common/WheeledMobileLogic.nut"
sigimport "Gameplay/mobile/common/MobileTurretLogic.nut"
sigimport "Anims/Vehicles/Blue/tank_m1a1abrams_maingun/tank_m1a1abrams_maingun.anipk"
sigimport "Anims/Vehicles/Blue/tank_m1a1abrams_largemgun/tank_m1a1abrams_largemgun.anipk"
sigimport "Anims/Vehicles/Blue/tank_m1a1abrams_smallmgun/tank_m1a1abrams_smallmgun.anipk"
sigimport "gameplay/mobile/vehicle/usa/usatankheavy01_motionmap.nut"
sigimport "gui/textures/waveicons/usa/vehicle_tank02_g.png"
sigimport "Anims/Vehicles/Blue/tank_m1a1abrams/tank_m1a1abrams.anipk"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_Tank_Heavy_01( )
}


class USA_Tank_Heavy_01 extends ScriptWheeledVehicleLogic
{
	constructor( )
	{
		ScriptWheeledVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USA_Tank_Heavy_01"

	function OnSpawn( )
	{
		local cannonBank = WeaponStation( 0 ).Bank( 0 )
		cannonBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
		
		local gunBank = WeaponStation( 0 ).Bank( 1 )
		gunBank.TriggerButton = GAMEPAD_BUTTON_LTRIGGER
		
		local gun = gunBank.AddWeapon( "USA_ABRAMS_MG", "lilgun" )
		local gunE = gun.SetTurretEntityNamed( "lilgun" )
		gunE.Logic.Animatable.MotionMap = USA_Tank_Heavy_01_SecGunMoMap( )
		gunE.Logic.Animatable.ExecuteMotionState( "Idle", { Weapon = gun } )
		
		local gun2 = gunBank.AddWeapon( "USA_ABRAMS_MG", "minigun" )
		local gunE2 = gun2.SetTurretEntityNamed( "minigun" )
		gunE2.Logic.Animatable.MotionMap = USA_Tank_Heavy_01_MiniGunMoMap( )
		gunE2.Logic.Animatable.ExecuteMotionState( "Idle", { Weapon = gun2 } )
		
		
		local cannon = cannonBank.AddWeapon( "USA_ABRAMS_CANNON", "cannon" )
		local cannonE = cannon.SetTurretEntityNamed( "cannon" )
		cannonE.Logic.Animatable.MotionMap = USA_Tank_Heavy_01_MainGunMoMap( )
		cannonE.Logic.Animatable.ExecuteMotionState( "Idle", { Weapon = cannon } )
		
		ScriptWheeledVehicleLogic.OnSpawn( )
		SetDestroyedEffect( "Medium_Vehicle_Explosion" )
	}
	
	function SetMotionMap( ) 
		Animatable.MotionMap = TankVehicleMotionMap( this, "Anims/Vehicles/Blue/tank_m1a1abrams/tank_m1a1abrams.anipk" )
}


class USA_Tank_Heavy_01_MainGunMoMap extends MobileTurretMoMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Blue/tank_m1a1abrams_maingun/tank_m1a1abrams_maingun.anipk" )
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

class USA_Tank_Heavy_01_SecGunMoMap extends MobileTurretMoMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Blue/tank_m1a1abrams_largemgun/tank_m1a1abrams_largemgun.anipk" )
	}
}

class USA_Tank_Heavy_01_MiniGunMoMap extends MobileTurretMoMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Blue/tank_m1a1abrams_smallmgun/tank_m1a1abrams_smallmgun.anipk" )
	}
}
