
sigimport "Gameplay/Turrets/Common/TurretMoMap.nut"
sigimport "Anims/Turrets/Artillery/Red/artillery_scud/artillery_scud.anipk"
sigimport "Anims/Characters/Red/artillery/artillery.anipk"

class USSR_Artillery_03_MotionMap extends TurretMotionMap
{	
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Turrets/Artillery/Red/artillery_scud/artillery_scud.anipk" )
		UseMuzzleTrack( 0, 89 )
	}
}

class USSR_Artillery03CrewmanAMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Red/artillery/artillery.anipk", "artillery_scud", "crewmana", false, false )
		UseMuzzleTrack( 0, 89 )
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
				track.Anim = sharedAnimPack.Find( "artillery_scud_idle_alt_crewmana" )                                       
				break;                                       
				case WEAPON_STATE_FIRING:
				return 0.0                            
				break;
				case WEAPON_STATE_RELOADING:
				return 0.0
				break;
				case WEAPON_STATE_AIMING:
				track.Anim = sharedAnimPack.Find( "artillery_scud_idle_alt_crewmana" )
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

class USSR_Artillery03CrewmanBMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Red/artillery/artillery.anipk", "artillery_scud", "crewmanb", false, false )
		UseMuzzleTrack( 0, 0 )
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
				track.Anim = sharedAnimPack.Find( "artillery_scud_idle_alt_crewmanb" )                                       
				break;                                       
				case WEAPON_STATE_FIRING:
				return 0.0                            
				break;
				case WEAPON_STATE_RELOADING:
				return 0.0
				break;
				case WEAPON_STATE_AIMING:
				track.Anim = sharedAnimPack.Find( "artillery_scud_idle_alt_crewmanb" )
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
	
class USSR_Artillery03CrewmanCMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Red/artillery/artillery.anipk", "artillery_scud", "crewmanc", false, false )
		UseMuzzleTrack( 0, 0 )
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
				track.Anim = sharedAnimPack.Find( "artillery_scud_idle_alt_crewmanc" )                                       
				break;                                       
				case WEAPON_STATE_FIRING:
				return 0.0                            
				break;
				case WEAPON_STATE_RELOADING:
				return 0.0
				break;
				case WEAPON_STATE_AIMING:
				track.Anim = sharedAnimPack.Find( "artillery_scud_idle_alt_crewmanc" )
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

class USSR_Artillery03CrewmanDMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Red/artillery/artillery.anipk", "artillery_scud", "crewmand", false, false )
		UseMuzzleTrack( 0, 0 )
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
				track.Anim = sharedAnimPack.Find( "artillery_scud_idle_alt_crewmand" )                                       
				break;                                       
				case WEAPON_STATE_FIRING:
				return 0.0                            
				break;
				case WEAPON_STATE_RELOADING:
				return 0.0
				break;
				case WEAPON_STATE_AIMING:
				track.Anim = sharedAnimPack.Find( "artillery_scud_idle_alt_crewmand" )
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