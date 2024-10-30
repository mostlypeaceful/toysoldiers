sigimport "Gameplay/mobile/common/WheeledMobileLogic.nut"
sigimport "Gameplay/mobile/common/MobileTurretLogic.nut"
sigimport "Anims/Vehicles/Red/tank_t80_maingun/tank_t80_maingun.anipk"
sigimport "Anims/Vehicles/Blue/tank_m1a1abrams_largemgun/tank_m1a1abrams_largemgun.anipk"
sigimport "Anims/Vehicles/Blue/tank_m1a1abrams_smallmgun/tank_m1a1abrams_smallmgun.anipk"
sigimport "gui/textures/waveicons/USSR/vehicle_tank02_g.png"
sigimport "gameplay/mobile/vehicle/ussr/ussrtankheavy01_motionmap.nut"
sigimport "Anims/Vehicles/Red/tank_t80/tank_t80.anipk"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Tank_Heavy_01( )
}


class USSR_Tank_Heavy_01 extends ScriptWheeledVehicleLogic
{
	constructor( )
	{
		ScriptWheeledVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USSR_Tank_Heavy_01"

	function OnSpawn( )
	{
		local cannonBank = WeaponStation( 0 ).Bank( 0 )
		cannonBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
		
		local gunBank = WeaponStation( 0 ).Bank( 1 )
		gunBank.TriggerButton = GAMEPAD_BUTTON_LTRIGGER
		
		local gun = gunBank.AddWeapon( "USSR_T80_MG", "lilgun" )
		local gunE = gun.SetTurretEntityNamed( "lilgun" )
		gunE.Logic.Animatable.MotionMap = USSR_Tank_Heavy_01_SecGunMoMap( )
		gunE.Logic.Animatable.ExecuteMotionState( "Idle", { Weapon = gun } )
		
		local gun2 = gunBank.AddWeapon( "USSR_T80_MG", "minigun" )
		local gunE2 = gun2.SetTurretEntityNamed( "minigun" )
		gunE2.Logic.Animatable.MotionMap = USSR_Tank_Heavy_01_MiniGunMoMap( )
		gunE2.Logic.Animatable.ExecuteMotionState( "Idle", { Weapon = gun2 } )
		
		
		local cannon = cannonBank.AddWeapon( "USSR_T80_CANNON", "cannon" )
		local cannonE = cannon.SetTurretEntityNamed( "cannon" )
		cannonE.Logic.Animatable.MotionMap = USSR_Tank_Heavy_01_MainGunMoMap( )
		cannonE.Logic.Animatable.ExecuteMotionState( "Idle", { Weapon = cannon } )
		
// USSR T55 Cannon, USSR T55 MG
		ScriptWheeledVehicleLogic.OnSpawn( )
		SetDestroyedEffect( "Large_Vehicle_Explosion" )
	}
	
	function SetMotionMap( ) 
		Animatable.MotionMap = TankVehicleMotionMap( this, "Anims/Vehicles/Red/tank_t80/tank_t80.anipk" )
}

class USSR_Tank_Heavy_01_MainGunMoMap extends MobileTurretMoMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Red/tank_t80_maingun/tank_t80_maingun.anipk" )
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

class USSR_Tank_Heavy_01_SecGunMoMap extends MobileTurretMoMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Blue/tank_m1a1abrams_largemgun/tank_m1a1abrams_largemgun.anipk" )
	}
}

class USSR_Tank_Heavy_01_MiniGunMoMap extends MobileTurretMoMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Blue/tank_m1a1abrams_smallmgun/tank_m1a1abrams_smallmgun.anipk" )
	}
}