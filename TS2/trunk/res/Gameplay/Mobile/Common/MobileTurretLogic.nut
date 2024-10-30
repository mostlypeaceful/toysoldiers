
sigexport function EntityOnCreate( entity )
{
	entity.Logic = MobileTurretLogic( )
}


class MobileTurretMoMap extends Anim.MotionMap
{
	animPack = null
	
	constructor( )
	{
		Anim.MotionMap.constructor( )
		SetAnimPack( )
	}
	
	function SetAnimPack( )
	{
		BreakPoint( "must set anim pack in derived class" )
	}
	
	function PushMuzzleTrack( params, low, high, lowAngle, highAngle )
	{
		local track = Anim.PitchBlendMuzzleTrack( )
		track.LowAnim = animPack.Find( low )
		track.HighAnim = animPack.Find( high )
		track.LowAngle = lowAngle
		track.HighAngle = highAngle
		track.BlendIn = 0.0
		track.BlendOut = 0.0
		track.Weapon = params.Weapon
		track.Flags = ANIM_TRACK_PARTIAL
		track.Push( Stack )
	}
	
	function Idle( params )
	{
		//local track = Anim.KeyFrameTrack( )
		//track.Anim = animPack.Find( "idle" )
		//track.BlendIn = 0.2
		//track.BlendOut = 0.0
		
		//track.Push( Stack )
		
		PushMuzzleTrack( params, "aim_down", "aim_up", -85, 85 )
	}
	
	function Forward( params )
	{		
	}
	
	function Recoil( params )
	{		
		return 0.0
	}
	
	function Critical( params )
	{
		return 0.0
	}

}