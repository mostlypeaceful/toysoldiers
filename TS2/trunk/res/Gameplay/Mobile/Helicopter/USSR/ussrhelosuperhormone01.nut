sigimport "Gameplay/Mobile/Helicopter/Common/HoverVehicleLogic.nut"
sigimport "Gameplay/Mobile/Helicopter/ussr/ussrheloattack01momap.nut"
sigimport "Gui/Textures/WaveIcons/USSR/vehicle_helihormone_g.png"
sigimport "Anims/Vehicles/Red/helicopter_hormone/hormone.anipk"
sigimport "Gameplay/Turrets/Common/CrewmanMoMap.nut"
sigimport "Anims/Characters/red/artillery/artillery.anipk"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Helo_Super_Hormone( )
}


class USSR_Helo_Super_Hormone extends HoverVehicleLogic
{
	constructor( )
	{
		HoverVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USSR_Helo_Super_Hormone"

	function OnSpawn( )
	{	
		WeaponStation( 0 ).ToggleCameraButton = GAMEPAD_BUTTON_RTHUMB

		local gunBank = WeaponStation( 0 ).Bank( 0 )
		gunBank.AddWeapon( "USSR_SUPERHORMONE_MG", "gatlinggun1,gatlinggun2" )
		gunBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER		
		
		local rocketBank = WeaponStation( 0 ).Bank( 1 )
		rocketBank.AddWeapon( "USSR_SUPERHORMONE_ROCKETS", "rockets" )
		rocketBank.FireMode = FIRE_MODE_ALTERNATE
		rocketBank.TriggerButton = GAMEPAD_BUTTON_LTRIGGER
		
		HoverVehicleLogic.OnSpawn( )
		SetDestroyedEffect( "Helicopter_Explosion" )
	}

	function SetMotionMap( ) 
		Animatable.MotionMap = USSRHeloSuperHormoneMoMap( this )
	
}

class USSRHeloSuperHormoneMoMap extends USSRHeloAttack01MoMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Red/helicopter_hormone/hormone.anipk" )
	}
	
	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "idle" )
		track.BlendIn = 0.0
		track.BlendOut = 0.0
		
		track.Push( Stack )
	}

	function Startup( params )
	{
		Forward( params )
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "takeoff" )
		track.BlendIn = 0.0
		track.BlendOut = 0.1
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		track.Push( Stack )
		
		return track.Anim.OneShotLength - 0.4
	}
	
	function ShutDown( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "exit" )
		track.BlendIn = 0.2
		track.BlendOut = 0.1
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		track.Push( Stack )
		
		return track.Anim.OneShotLength - 0.3
	}	
	
	function Forward( params )
	{	
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "forward" )
		track.BlendIn = 0.0
		track.BlendOut = 0.0
		
		track.Push( Stack )		
	}
}

class USSRHeloSuperHormoneCrewmanAMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/red/artillery/artillery.anipk", "helicopter_hormone", "crewmana", false, false )
		idleBlendIn = 0.0
	}
	
	function Startup( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( animPrefix + "_takeoff_" + animSuffix )
		track.BlendIn = 0.2
		track.BlendOut = 0.1
		track.Flags = ANIM_TRACK_CLAMP_TIME
		track.Push( Stack )
		
		return track.Anim.OneShotLength
		
	}
	
	function Shutdown( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( animPrefix + "_exit_" + animSuffix )
		track.BlendIn = 0.2
		track.BlendOut = 0.1
		track.Flags = ANIM_TRACK_CLAMP_TIME
		track.Push( Stack )
		
		return track.Anim.OneShotLength
		
	}
	
	function Forward( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( animPrefix + "_forward_" + animSuffix )
		track.BlendIn = 0.0
		track.BlendOut = 0.0
		track.Push( Stack )
	}
}

class USSRHeloSuperHormoneCrewmanBMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/red/artillery/artillery.anipk", "helicopter_hormone", "crewmanb", false, false )
		idleBlendIn = 0.0
	}
	
	function Startup( params )
	{

		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( animPrefix + "_takeoff_" + animSuffix )
		track.BlendIn = 0.2
		track.BlendOut = 0.1
		track.Flags = ANIM_TRACK_CLAMP_TIME
		track.Push( Stack )
		
		return track.Anim.OneShotLength
		
	}
	
	function Shutdown( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( animPrefix + "_exit_" + animSuffix )
		track.BlendIn = 0.21
		track.BlendOut = 0.1
		track.Flags = ANIM_TRACK_CLAMP_TIME
		track.Push( Stack )
		
		return track.Anim.OneShotLength
		
	}
	
	function Forward( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( animPrefix + "_forward_" + animSuffix )
		track.BlendIn = 0.0
		track.BlendOut = 0.0
		track.Push( Stack )
	}
}