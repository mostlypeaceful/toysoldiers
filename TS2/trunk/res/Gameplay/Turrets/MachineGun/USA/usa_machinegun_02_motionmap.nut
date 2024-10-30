sigimport "Gameplay/Turrets/Common/TurretMoMap.nut"
sigimport "Gameplay/Turrets/Common/CrewmanMoMap.nut"
sigimport "Anims/Turrets/MachineGun/Blue/mgun_m60/mgun_m60.anipk"
sigimport "Anims/Characters/Blue/artillery/artillery.anipk"

class USA_MachineGun_02_MotionMap extends TurretMotionMap
{	
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Turrets/MachineGun/Blue/mgun_m60/mgun_m60.anipk" )
		UseMuzzleTrack( -15, 10 )
	}
}

class USA_MG02CrewmanAMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Blue/artillery/artillery.anipk", "mgun_m60", "crewmana", true, true )
		UseMuzzleTrack( -15, 10 )
	}
	
}


class USA_MG02CrewmanBMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Blue/artillery/artillery.anipk", "mgun_m60", "crewmanb", true, false )
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
				track.Anim = sharedAnimPack.Find( "mgun_m60_idle_alt_crewmanb" )                                       
				break;                                       
				case WEAPON_STATE_FIRING:
				return 0.0                            
				break;
				case WEAPON_STATE_RELOADING:
				return 0.0
				break;
				case WEAPON_STATE_AIMING:
				track.Anim = sharedAnimPack.Find( "mgun_m60_idle_alt_crewmanb" )
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


class USA_MG02CrewmanCMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Blue/artillery/artillery.anipk", "mgun_m60", "crewmanc", true, false )
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
				track.Anim = sharedAnimPack.Find( "mgun_m60_idle_alt_crewmanc" )                                       
				break;                                       
				case WEAPON_STATE_FIRING:
				return 0.0                            
				break;
				case WEAPON_STATE_RELOADING:
				return 0.0
				break;
				case WEAPON_STATE_AIMING:
				track.Anim = sharedAnimPack.Find( "mgun_m60_idle_alt_crewmanc" )
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
