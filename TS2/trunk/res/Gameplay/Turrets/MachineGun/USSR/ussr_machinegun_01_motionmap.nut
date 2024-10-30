
sigimport "Gameplay/Turrets/Common/TurretMoMap.nut"
sigimport "Anims/Turrets/MachineGun/Red/mgun_pkp/mgun_pkp.anipk"
sigimport "Anims/Characters/Red/artillery/artillery.anipk"

class USSR_MachineGun_01_MotionMap extends TurretMotionMap
{	
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Turrets/MachineGun/Red/mgun_pkp/mgun_pkp.anipk" )
		UseMuzzleTrack( -15, 10 )
	}
}

class USSRMG01CrewmanAMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Red/artillery/artillery.anipk", "mgun_pkp", "crewmana", true, true )
		UseMuzzleTrack( -15, 10 )
	}
}

class USSRMG01CrewmanBMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Red/artillery/artillery.anipk", "mgun_pkp", "crewmanb", true, false )
		UseMuzzleTrack( -15, 10 )
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
				track.Anim = sharedAnimPack.Find( "mgun_pkp_idle_alt_crewmanb" )                                       
				break;                                       
				case WEAPON_STATE_FIRING:
				return 0.0                            
				break;
				case WEAPON_STATE_RELOADING:
				return 0.0
				break;
				case WEAPON_STATE_AIMING:
				track.Anim = sharedAnimPack.Find( "mgun_pkp_idle_alt_crewmanb" )
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

class USSRMG01CrewmanCMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Red/artillery/artillery.anipk", "mgun_pkp", "crewmanc", true, false )
		UseMuzzleTrack( -15, 10 )
	}
}