sigimport "Gameplay/Turrets/Common/CrewmanMoMap.nut"
sigimport "Gameplay/Turrets/Common/TurretMoMap.nut"
sigimport "Anims/Turrets/AntiAir/Blue/aa_chaparral/aa_chaparral.anipk"
sigimport "Anims/Characters/Blue/artillery/artillery.anipk"

class USA_AntiAir_02_MotionMap extends TurretMotionMap
{	
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Turrets/AntiAir/Blue/aa_chaparral/aa_chaparral.anipk" )
		UseMuzzleTrack( 2, 85 )
	}
	
	function FireOneShot( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "fire" )
		track.BlendIn = 0.01
		track.BlendOut = 0.01
		track.Tag = "fireOneShot"
		track.TimeScale = 2.0

		//PushTurretPitchTrack( params, animPack, "aim_down", "aim_up" )
		//PushTurretOrientTrack( )
		
		Stack.RemoveTracksWithTag( track.Tag );
		track.Push( Stack )

		return (track.Anim.OneShotLength / track.TimeScale - 0.1)
	}
}


class USA_AA02CrewmanAMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Blue/artillery/artillery.anipk", "aa_chaparral", "crewmana", false, false )
		UseMuzzleTrack( 2, 85 )
	}
	
	function FireOneShot( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( animPrefix + "_fire_" + animSuffix )
		track.BlendIn = 0.01
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_CLAMP_TIME;
		track.TimeScale = 2.0
		track.Push( Stack )
			
		return track.Anim.OneShotLength / track.TimeScale
		
	}
	function RandomAnim( params )
    {
        local randChance = ObjectiveRand.Int( 0, 100 )
        local track = Anim.KeyFrameTrack( )
        if (randChance < 25 )                       
        {	
			switch( params.Context )                                                                     
			{
				case WEAPON_STATE_IDLE:                                          
				track.Anim = sharedAnimPack.Find( "aa_chaparral_idle_alt_crewmana" )                                       
				break;                                       
				case WEAPON_STATE_FIRING:
				return 0.0                            
				break;
				case WEAPON_STATE_RELOADING:
				return 0.0
				break;
				case WEAPON_STATE_AIMING:
				track.Anim = sharedAnimPack.Find( "aa_chaparral_idle_alt_crewmana" )
				break;
			}
			track.BlendIn = 0.3
			track.BlendOut = 0.3
			track.Push( Stack )
			return track.Anim.OneShotLength
		}
		else 
		return 0.0
	}
}


class USA_AA02CrewmanBMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Blue/artillery/artillery.anipk", "aa_chaparral", "crewmanb", false, false )
		UseMuzzleTrack( 2, 85 )
	}

	function FireOneShot( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( animPrefix + "_fire_" + animSuffix )
		track.BlendIn = 0.01
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_CLAMP_TIME;
		track.TimeScale = 2.0
		track.Push( Stack )
			
		return track.Anim.OneShotLength / track.TimeScale
		
	}
	
	function RandomAnim( params )
    {
        local randChance = ObjectiveRand.Int( 0, 100 )
        local track = Anim.KeyFrameTrack( )
        if (randChance < 25 )                       
        {	
			switch( params.Context )                                                                     
			{
				case WEAPON_STATE_IDLE:                                          
				track.Anim = sharedAnimPack.Find( "aa_chaparral_idle_alt_crewmanb" )                                       
				break;                                       
				case WEAPON_STATE_FIRING:
				return 0.0                            
				break;
				case WEAPON_STATE_RELOADING:
				return 0.0
				break;
				case WEAPON_STATE_AIMING:
				track.Anim = sharedAnimPack.Find( "aa_chaparral_idle_alt_crewmanb" )
				break;
			}
			track.BlendIn = 0.3
			track.BlendOut = 0.3
			track.Push( Stack )
			return track.Anim.OneShotLength
		}
		else 
		return 0.0
	}
}


class USA_AA02CrewmanCMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Blue/artillery/artillery.anipk", "aa_chaparral", "crewmanc", false, false )
		UseMuzzleTrack( 2, 85 )
	}
	
	function FireOneShot( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( animPrefix + "_fire_" + animSuffix )
		track.BlendIn = 0.01
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_CLAMP_TIME;
		track.TimeScale = 2.0
		track.Push( Stack )
			
		return track.Anim.OneShotLength / track.TimeScale
		
	}
	
	function RandomAnim( params )
    {
        local randChance = ObjectiveRand.Int( 0, 100 )
        local track = Anim.KeyFrameTrack( )
        if (randChance < 25 )                       
        {	
			switch( params.Context )                                                                     
			{
				case WEAPON_STATE_IDLE:                                          
				track.Anim = sharedAnimPack.Find( "aa_chaparral_idle_alt_crewmanc" )                                       
				break;                                       
				case WEAPON_STATE_FIRING:
				return 0.0                            
				break;
				case WEAPON_STATE_RELOADING:
				return 0.0
				break;
				case WEAPON_STATE_AIMING:
				track.Anim = sharedAnimPack.Find( "aa_chaparral_idle_alt_crewmanc" )
				break;
			}
			track.BlendIn = 0.3
			track.BlendOut = 0.3
			track.Push( Stack )
			return track.Anim.OneShotLength
		}
		else 
		return 0.0
	}
}

class USA_AA02CrewmanDMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Blue/artillery/artillery.anipk", "aa_chaparral", "crewmand", true, true )
		UseMuzzleTrack( 2, 85 )
	}
	
	function FireOneShot( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( animPrefix + "_fire_" + animSuffix )
		track.BlendIn = 0.01
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_CLAMP_TIME;
		track.TimeScale = 2.0
		track.Push( Stack )
			
		return track.Anim.OneShotLength / track.TimeScale
		
	}
}
