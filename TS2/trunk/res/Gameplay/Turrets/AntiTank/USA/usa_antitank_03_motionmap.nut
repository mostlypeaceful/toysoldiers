sigimport "Gameplay/Turrets/Common/TurretMoMap.nut"
sigimport "Gameplay/Turrets/Common/CrewmanMoMap.nut"
sigimport "Anims/Turrets/AntiTank/Blue/at_bgm71tow/at_bgm71tow.anipk"
sigimport "Anims/Characters/Blue/artillery/artillery.anipk"

class USA_AntiTank_03_MotionMap extends TurretMotionMap
{	
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Turrets/AntiTank/Blue/at_bgm71tow/at_bgm71tow.anipk" )
		UseMuzzleTrack( -10, 7 )
	}

}


class AT03CrewmanAMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Blue/artillery/artillery.anipk", "antitank_bgm71tow", "crewmana", true, true )
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
}


class AT03CrewmanBMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Blue/artillery/artillery.anipk", "antitank_bgm71tow", "crewmanb", true, true )
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
}

class AT03CrewmanCMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Blue/artillery/artillery.anipk", "antitank_bgm71tow", "crewmanc", true, true )
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
}

