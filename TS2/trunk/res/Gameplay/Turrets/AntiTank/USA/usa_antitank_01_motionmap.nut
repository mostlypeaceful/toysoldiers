sigimport "Gameplay/Turrets/Common/TurretMoMap.nut"
sigimport "Gameplay/Turrets/Common/CrewmanMoMap.nut"
sigimport "Anims/Turrets/AntiTank/Blue/at_m72law/at_m72law.anipk"
sigimport "Anims/Characters/Blue/artillery/artillery.anipk"

class USA_AntiTank_01_MotionMap extends TurretMotionMap
{	
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Turrets/AntiTank/Blue/at_m72law/at_m72law.anipk" )
		UseMuzzleTrack( -10, 7 )
	}
	
	function GhostIdle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "aim_down" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0

		track.Push( Stack )
	}
	function Idle( params )
	{
		Aim( params )
	}
}

class AT01CrewmanAMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Blue/artillery/artillery.anipk", "antitank_m72law", "crewmana", true, true )
		UseMuzzleTrack( -10, 7 )
	}
	function RandomAnim( params )
	{
		local track = Anim.KeyFrameTrack( )
		local randChance = ObjectiveRand.Int( 0, 100 )
		
		if( randChance < 25 )
		{
			switch( params.Context )
			{
				case WEAPON_STATE_IDLE:
				return 0.0
				case WEAPON_STATE_FIRING:
				//print( "random anim while firing" )
				return 0.0
				case WEAPON_STATE_RELOADING:
				//print( "random anim while relaoding" )
				return 0.0
				case WEAPON_STATE_AIMING:
					track.Anim = sharedAnimPack.Find( animPrefix + "_aim_random_" + animSuffix )
				break;
			}

			track.BlendIn = 0.1
			track.BlendOut = 0.1
			track.Push( Stack )
			track.Flags = ANIM_TRACK_CLAMP_TIME;
				
			return track.Anim.OneShotLength
		}
		else
			return 0.0
	}
	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( animPrefix + "_aim_low_" + animSuffix )
		track.BlendIn = idleBlendIn
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_RESUME_TIME
		track.Push( Stack )
	}
	
	function GhostIdle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( animPrefix + "_aim_low_" + animSuffix )
		track.BlendIn = 0.0
		track.BlendOut = 0.0
		
		track.Push( Stack )
	}
}


class AT01CrewmanBMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Blue/artillery/artillery.anipk", "antitank_m72law", "crewmanb", true, true )
		UseMuzzleTrack( -10, 7 )
	}
	function RandomAnim( params )
	{
		local track = Anim.KeyFrameTrack( )
		local randChance = ObjectiveRand.Int( 0, 100 )
		
		if( randChance < 25 )
		{
			switch( params.Context )
			{
				case WEAPON_STATE_IDLE:
				return 0.0
				case WEAPON_STATE_FIRING:
				//print( "random anim while firing" )
				return 0.0
				case WEAPON_STATE_RELOADING:
				//print( "random anim while relaoding" )
				return 0.0
				case WEAPON_STATE_AIMING:
					track.Anim = sharedAnimPack.Find( animPrefix + "_aim_random_" + animSuffix )
				break;
			}

			track.BlendIn = 0.1
			track.BlendOut = 0.1
			track.Push( Stack )
			track.Flags = ANIM_TRACK_CLAMP_TIME;
				
			return track.Anim.OneShotLength
		}
		else
			return 0.0
	}
	
	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( animPrefix + "_aim_low_" + animSuffix )
		track.BlendIn = idleBlendIn
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_RESUME_TIME
		track.Push( Stack )
	}
	
	function GhostIdle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( animPrefix + "_aim_low_" + animSuffix )
		track.BlendIn = 0.0
		track.BlendOut = 0.0
		
		track.Push( Stack )
	}
}


class AT01CrewmanCMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Blue/artillery/artillery.anipk", "antitank_m72law", "crewmanc", true, false )
		UseMuzzleTrack( -10, 7 )
	}
	function RandomAnim( params )
	{
		local track = Anim.KeyFrameTrack( )
		local randChance = ObjectiveRand.Int( 0, 100 )
		
		if( randChance < 25 )
		{
			switch( params.Context )
			{
				case WEAPON_STATE_IDLE:
				return 0.0
				case WEAPON_STATE_FIRING:
				//print( "random anim while firing" )
				return 0.0
				case WEAPON_STATE_RELOADING:
				//print( "random anim while relaoding" )
				return 0.0
				case WEAPON_STATE_AIMING:
					track.Anim = sharedAnimPack.Find( animPrefix + "_aim_random_" + animSuffix )
				break;
			}

			track.BlendIn = 0.1
			track.BlendOut = 0.1
			track.Push( Stack )
			track.Flags = ANIM_TRACK_CLAMP_TIME;
				
			return track.Anim.OneShotLength
		}
		else
			return 0.0
	}
	
	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( animPrefix + "_aim_low_" + animSuffix )
		track.BlendIn = idleBlendIn
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_RESUME_TIME
		track.Push( Stack )
	}
	
	function GhostIdle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( animPrefix + "_aim_low_" + animSuffix )
		track.BlendIn = 0.0
		track.BlendOut = 0.0
		
		track.Push( Stack )
	}
}
