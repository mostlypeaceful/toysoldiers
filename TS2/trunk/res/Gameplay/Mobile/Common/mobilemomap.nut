
//sigimport "Anims/Vehicles/Blue/helicopter_apache/apache.anipk"

class MobileMotionMap extends Anim.MotionMap
{
	animPack = null
	
	constructor( logic )
	{
		Anim.MotionMap.constructor( )
		SetAnimPack( )
	}
	
	function SetAnimPack( )
	{
		//override this
	}

	function Startup( params )
	{
		return 0.0
	}

	function ShutDown( params )
	{
		return 0.0
	}
	
	function Recoil( params )
	{
		return 0.0
	}

	function Idle( params )
	{
		/*local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "idle" )
		track.BlendIn = 4.0
		track.BlendOut = 0.0
		
		track.Push( Stack )*/
	}
	
	function Forward( params )
	{	
		/*local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "forward" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		
		track.Push( Stack )*/	
	}
	
	function SpecialMove( params )
	{	
		/*local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "forward" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		
		track.Push( Stack )*/	
	}
	
	function CargoBegin( params )
	{
		/*local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( anim )
		track.BlendIn = 0.2
		track.BlendOut = 0.2
		
		track.Push( Stack )

		return (track.Anim.OneShotLength - 0.25)*/
		return 1.0
	}
	
	function CargoIdle( params )
	{
		/*local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( anim )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		
		track.Push( Stack )*/
	}
	
	function CargoEnd( params )
	{
		/*local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( anim )
		track.BlendIn = 0.2
		track.BlendOut = 0.2
		
		track.Push( Stack )

		return (track.Anim.OneShotLength - 0.25)*/
		return 1.0
	}

}

