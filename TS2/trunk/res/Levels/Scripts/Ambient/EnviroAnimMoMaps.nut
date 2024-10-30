


class TestEnviroAnimMoMap extends Anim.MotionMap
{
	animPack = null
	constructor( )
	{
		Anim.MotionMap.constructor( )
		animPack = GetAnimPack( "Gameplay/Anims/Shared/basic.anipk" )
	}
	function Spawn( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "amb_vomit" )
		track.BlendOut = 0.0
		track.Push( Stack )
	}
}

