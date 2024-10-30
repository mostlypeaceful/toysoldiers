sigimport "Gameplay/Turrets/Common/CrewmanMoMap.nut"
sigimport "Gameplay/Turrets/Common/TurretMoMap.nut"
sigimport "Anims/Turrets/Artillery/Blue/artillery_m198/artillery_m198.anipk"

class USA_Artillery_02_MotionMap extends TurretMotionMap
{	
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Turrets/Artillery/Blue/artillery_m198/artillery_m198.anipk" )
		UseMuzzleTrack( 0, 40 )
	}
}

class USA_Artillery02CrewmanAMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Blue/artillery/artillery.anipk", "artillery_m198", "crewmana", true, false )
		UseMuzzleTrack( 0, 40 )
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


class USA_Artillery02CrewmanBMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Blue/artillery/artillery.anipk", "artillery_m198", "crewmanb", true, false )
		UseMuzzleTrack( 0, 40 )
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


class USA_Artillery02CrewmanCMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Blue/artillery/artillery.anipk", "artillery_m198", "crewmanc", true, false ) 
		UseMuzzleTrack( 0, 40 )
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

class USA_Artillery02CrewmanDMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Blue/artillery/artillery.anipk", "artillery_m198", "crewmand", true, false ) 
		UseMuzzleTrack( 0, 40 )
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