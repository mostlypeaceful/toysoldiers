sigimport "Gameplay/Mobile/Vehicle/Common/wheeledmobilemomap.nut"
sigimport "Anims/Vehicles/Blue/apc_m113/apc_m113.anipk"
sigimport "Gameplay/Turrets/Common/CrewmanMoMap.nut"

class USA_APCMG01MoMap extends VehicleMotionMap
{	
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Blue/apc_m113/apc_m113.anipk" )
	}
	
	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "idle" )
		track.BlendIn = 4.0
		track.BlendOut = 0.0
		
		track.Push( Stack )
		TankTreads( params )
	}
	
	function Forward( params )
	{	
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "forward" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		
		track.Push( Stack )		
		TankTreads( params )
	}
	
	function CargoBegin( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "deploy" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		track.Push( Stack )

		return (track.Anim.OneShotLength - 0.2)
	}
	
	function CargoIdle( params )
	{
		/*local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "hover" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		
		track.Push( Stack )*/
	}
	
	function CargoEnd( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "retract" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_CLAMP_TIME		
		track.Push( Stack )

		return (track.Anim.OneShotLength)
	}
}


class USAAPCMG01_CrewmanMoMap extends CrewmanMoMap
{
	constructor()
	{
		CrewmanMoMap.constructor( "Anims/Characters/Blue/artillery/artillery.anipk", "apc_m113", "gunner", false, false )
	}
	
	function Forward( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( animPrefix + "_idle_" + animSuffix )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.Push( Stack )
	}
	
	function RandomAnim( params )
	{
		return PushRandomIdle( params )
	}

}
