sigimport "Gameplay/Turrets/Common/TurretMoMap.nut"
sigimport "Gameplay/Turrets/Common/CrewmanMoMap.nut"
sigimport "Anims/Turrets/MachineGun/Blue/mgun_gatling/mgun_gatling.anipk"
sigimport "Anims/Characters/Blue/artillery/artillery.anipk"

class USA_MachineGun_03_MotionMap extends TurretMotionMap
{	
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Turrets/MachineGun/Blue/mgun_gatling/mgun_gatling.anipk" )
		UseMuzzleTrack( -15, 10 )
	}
	
	function Recoil( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "fire_secondary" )
		track.BlendIn = 0.01
		track.BlendOut = 0.01
		track.Flags = ANIM_TRACK_CLAMP_TIME
		track.Push( Stack )
		
		Logic.ApplyMotionStateToSoldiers( "Recoil" )

		return (track.Anim.OneShotLength)
	}

}

class USA_MG03CrewmanAMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Blue/artillery/artillery.anipk", "mgun_gatling", "crewmana", true, true )
		UseMuzzleTrack( -15, 10 )
	}

	function Recoil( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( "mgun_gatling_fire_secondary_crewmana" )
		track.BlendIn = 0.05
		track.BlendOut = 0.15
		track.Flags = ANIM_TRACK_CLAMP_TIME
		track.Push( Stack )

		
		return (track.Anim.OneShotLength)
	}

}


class USA_MG03CrewmanBMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Blue/artillery/artillery.anipk", "mgun_gatling", "crewmanb", false, false )
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
				track.Anim = sharedAnimPack.Find( "mgun_gatling_idle_alt_crewmanb" )                                       
				break;                                       
				case WEAPON_STATE_FIRING:
				return 0.0                            
				break;
				case WEAPON_STATE_RELOADING:
				return 0.0
				break;
				case WEAPON_STATE_AIMING:
				track.Anim = sharedAnimPack.Find( "mgun_gatling_idle_alt_crewmanb" )
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


class USA_MG03CrewmanCMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Blue/artillery/artillery.anipk", "mgun_gatling", "crewmanc", false, false )
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
				track.Anim = sharedAnimPack.Find( "mgun_gatling_idle_alt_crewmanc" )                                       
				break;                                       
				case WEAPON_STATE_FIRING:
				return 0.0                            
				break;
				case WEAPON_STATE_RELOADING:
				return 0.0
				break;
				case WEAPON_STATE_AIMING:
				track.Anim = sharedAnimPack.Find( "mgun_gatling_idle_alt_crewmanc" )
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
