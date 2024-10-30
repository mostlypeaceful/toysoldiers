sigimport "Gameplay/Turrets/Common/CrewmanMoMap.nut"
sigimport "Gameplay/Turrets/Common/TurretMoMap.nut"
sigimport "Anims/Turrets/Mortar/Blue/mortar_m252/mortar_m252.anipk"
sigimport "Anims/Characters/Blue/artillery/artillery.anipk"

class USA_Mortar_02_MotionMap extends TurretMotionMap
{	
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Turrets/Mortar/Blue/mortar_m252/mortar_m252.anipk" )
		UseMuzzleTrack( 49, 89 )
	}

}

class USA_Mortar02CrewmanAMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Blue/artillery/artillery.anipk", "mortar_m252", "crewmana", true, true )
		UseMuzzleTrack( 45, 89 )
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


class USA_Mortar02CrewmanBMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Blue/artillery/artillery.anipk", "mortar_m252", "crewmanb", true, false )
		UseMuzzleTrack( 45, 89 )
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


class USA_Mortar02CrewmanCMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Blue/artillery/artillery.anipk", "mortar_m252", "crewmanc", true, false )
		UseMuzzleTrack( 45, 89 )
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