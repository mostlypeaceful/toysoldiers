sigimport "Gameplay/Mobile/Helicopter/Common/HoverVehicleLogic.nut"
sigimport "Gameplay/Mobile/Helicopter/USA/usahelotransport01momap.nut"
sigimport "gui/textures/waveicons/usa/vehicle_gunship_g.png"


sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_Helo_Transport_Choplifter_01( )
}


class USA_Helo_Transport_Choplifter_01 extends HoverVehicleLogic
{
	constructor( )
	{
		HoverVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USA_Helo_Transport_Choplifter_01"

	function OnSpawn( )
	{	

		// The syntax for AddCargo is path, spawnrate, should_stop_while_dropping, remove_cargo_after_dropping
		// Please remove the comment above and make this take a string into the cargo table: AddCargo( "gameplay/characters/infantry/usa/infantry_paratrooper_01.sigml", 10, 0.5, 1, 0 )

		local gunBank = WeaponStation( 0 ).Bank( 0 )
		gunBank.AddWeapon( "HUEY_MACHINE_GUN", "leftgun" )	
		gunBank.AddWeapon( "HUEY_MACHINE_GUN", "rightgun" )
		gunBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
			
		local rocketBank = WeaponStation( 0 ).Bank( 1 )
		rocketBank.AddWeapon( "HUEY_ROCKET", "leftrocket" )
		rocketBank.AddWeapon( "HUEY_ROCKET", "rightrocket" )
		rocketBank.TriggerButton = GAMEPAD_BUTTON_LTRIGGER
		//rocketBank.FireMode = FIRE_MODE_ALTERNATE
		
		HoverVehicleLogic.OnSpawn( )
		SetDestroyedEffect( "Helicopter_Explosion" )
	}
	
	function SetMotionMap( ) 
		Animatable.MotionMap = USAHeloChoplifterMoMap( this )
}

class USAHeloChoplifterMoMap extends USAHeloTransport01MoMap
{
	
	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "hangtime_idle" )
		track.BlendIn = 0.1
		track.BlendOut = 0.0
		
		track.Push( Stack )
	}
	
	function Startup( params )
	{	
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "hangtime_takeoff" )
		track.BlendIn = 0.2
		track.BlendOut = 0.1
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		track.Push( Stack )
		
		return track.Anim.OneShotLength - 0.4
	}
	
	function ShutDown( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "hangtime_exit" )
		track.BlendIn = 0.2
		track.BlendOut = 0.1
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		track.Push( Stack )
		
		return track.Anim.OneShotLength - 0.2
	}
}

class USA_HeloChoplifter02CrewmanAMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/blue/artillery/artillery.anipk", "helicopter_uh1iroqouis", "crewmana", false, false )
		idleBlendIn = 0.0
	}
	
	function Idle ( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( animPrefix + "_fly_idle_" + animSuffix )
		track.BlendIn = 0.0
		track.BlendOut = 0.0
		track.Push( Stack )
	}
	function Startup( params )
	{
		return 0.0
		
	}
	
	function Shutdown( params )
	{
		return 0.0
		
	}
	
	function Forward( params )
	{
		return 0.0

	}
}

class USA_HeloChoplifter02CrewmanBMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/blue/artillery/artillery.anipk", "helicopter_uh1iroqouis", "crewmanb", false, false )
		idleBlendIn = 0.0
	}
	
	function Idle ( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( animPrefix + "_fly_idle_" + animSuffix )
		track.BlendIn = 0.0
		track.BlendOut = 0.0
		track.Push( Stack )
	}
	function Startup( params )
	{
		return 0.0
		
	}
	
	function Shutdown( params )
	{
		return 0.0
		
	}
	
	function Forward( params )
	{
		return 0.0

	}
}

