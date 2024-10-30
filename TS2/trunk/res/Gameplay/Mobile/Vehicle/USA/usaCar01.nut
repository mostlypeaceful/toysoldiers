sigimport "Gameplay/mobile/common/WheeledMobileLogic.nut"
sigimport "gameplay/characters/infantry/ussr/infantry_basic_01.sigml"
sigimport "Gameplay/mobile/common/MobileTurretLogic.nut"
sigimport "Anims/Vehicles/Blue/tank_m60patton_lilgun/tank_m60patton_lilgun.anipk"
sigimport "Gui/Textures/WaveIcons/USA/vehicle_car_g.png"
sigimport "gameplay/characters/passenger/common/passengermomap.nut"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_Car_01( )
}


class USA_Car_01 extends ScriptWheeledVehicleLogic
{
	constructor( )
	{
		ScriptWheeledVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USA_Car_01"

	function OnSpawn( )
	{
// USA HMWV MG

		local gunBank = WeaponStation( 0 ).Bank( 1 )
//		local cannonBank = WeaponStation( 0 ).Bank( 0 )
		gunBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
//		cannonBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
			
		local gun = gunBank.AddWeapon( "USA_HMWV_MG", "lilgun" )
		local gunE = gun.SetTurretEntityNamed( "lilgun" )
		gunE.Logic.Animatable.MotionMap = USA_Car_01_GunMoMap( )
		gunE.Logic.Animatable.ExecuteMotionState( "Idle", { Weapon = gun } )

		ScriptWheeledVehicleLogic.OnSpawn( )

		// The syntax for AddCargo is path, spawnrate, should_stop_while_dropping, remove_cargo_after_dropping
		// Please remove the comment above and make this take a string into the cargo table: AddCargo( "gameplay/characters/infantry/ussr/infantry_basic_01.sigml", 3, 1, 1, 0 )
	}
	
}

class USA_Car_01_GunMoMap extends MobileTurretMoMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Blue/tank_m60patton_lilgun/tank_m60patton_lilgun.anipk" )
	}
}

class USAJeepCrewmanAMoMap extends ArtillerySoldierDefaultMoMap
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
		track.Anim = animPack.Find( "car_jeep_idle_crewmana" )
		track.BlendIn = 0.0
		track.BlendOut = 0.0
		
		track.Push( Stack )
	}
	
}
class USAJeepCrewmanBMoMap extends ArtillerySoldierDefaultMoMap
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
		track.Anim = animPack.Find( "car_jeep_idle_crewmanb" )
		track.BlendIn = 0.0
		track.BlendOut = 0.0
		
		track.Push( Stack )
	}
}
class USAJeepCrewmanCMoMap extends ArtillerySoldierDefaultMoMap
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
		track.Anim = animPack.Find( "car_jeep_idle_crewmanc" )
		track.BlendIn = 0.0
		track.BlendOut = 0.0
		
		track.Push( Stack )
	}
}