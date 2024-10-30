sigimport "Gameplay/mobile/airborne/common/airbornemomap.nut"
sigimport "Anims/Vehicles/Red/fighter_mig23flogger/fighter_mig23flogger.anipk"

class USSRPlaneFighter01MoMap extends AirborneMotionMap
{	
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Red/fighter_mig23flogger/fighter_mig23flogger.anipk" )
	}

	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "idle" )
		track.BlendIn = 4.0
		track.BlendOut = 0.0
		
		track.Push( Stack )
	}

	function Startup( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "takeoff" )
		track.BlendIn = 0.2
		track.BlendOut = 0.01
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		track.Push( Stack )
		
		return track.Anim.OneShotLength - 0.4
	}
	
	function Forward( params )
	{	
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "forward" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		
		track.Push( Stack )	
			
			
		local track2 = Anim.AirborneAnimTrack( )
		track2.FastLeftAnim = animPack.Find( "maxspeed_turn_left" )
		track2.FastRightAnim = animPack.Find( "maxspeed_turn_right" )
		track2.SlowLeftAnim = animPack.Find( "minspeed_turn_left" )
		track2.SlowRightAnim = animPack.Find( "minspeed_turn_right" )
		track2.BlendIn = 0.2
		track2.BlendOut = 0.0
		
		track2.Airborne = Logic
		track2.Push( Stack )
	}
	
	function SpecialMove( params )
	{	
		local anim = ""
		local blend = 0.2
		switch( params.Id )
		{
			case SPECIAL_ANIM_ROLL_LEFT: anim = "roll_left"; break;
			case SPECIAL_ANIM_ROLL_RIGHT: anim = "roll_right"; break;
			case SPECIAL_ANIM_180: anim = "180"; blend = 0.1; break;
			default: return 0.0;
		}
		
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( anim )
		track.BlendIn = blend
		track.BlendOut = blend	
		track.Flags = ANIM_TRACK_CLAMP_TIME
			
		track.Push( Stack )	
		
		return track.Anim.OneShotLength - 0.2		
	}
}

//class USSRPlaneFighter01CrewmanAMoMap extends CrewmanMoMap
//{
//	constructor()
//	{
//		CrewmanMoMap.constructor( "Anims/Characters/blue/artillery/artillery.anipk", "apache", "crewmana", false, false )
//		idleBlendIn = 4.0
//	}
//}
//
//class USSRPlaneFighter01CrewmanBMoMap extends CrewmanMoMap
//{
//	constructor()
//	{
//		CrewmanMoMap.constructor( "Anims/Characters/blue/artillery/artillery.anipk", "apache", "crewmanb", false, false )
//		idleBlendIn = 4.0
//	}
//}

