sigimport "Anims/Bosses/Red/mi12_homer_engine_l/mi12_homer_engine_l.anipk"
sigimport "Anims/Bosses/Red/mi12_homer_engine_r/mi12_homer_engine_r.anipk"


class USSRBossHomerEngineMoMap extends Anim.MotionMap
{
	animPack = null
	
	constructor( )
	{
		Anim.MotionMap.constructor( )
	}

	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "idle" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		
		track.Push( Stack )
	}

	function Damage( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "damaged" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		
		track.Push( Stack )
	}

	function Critical( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "critical" )
		track.BlendIn = 0.2
		track.BlendOut = 0.2
		
		track.Push( Stack )
		return track.Anim.OneShotLength - track.BlendOut
	}

	function Death( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "death" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		track.Push( Stack )
	}
}


class USSRBossHomerEngineLMoMap extends USSRBossHomerEngineMoMap
{	
	constructor()
	{
		USSRBossHomerEngineMoMap.constructor( )
		animPack = GetAnimPack( "Anims/Bosses/Red/mi12_homer_engine_l/mi12_homer_engine_l.anipk" )
	}
}
class USSRBossHomerEngineRMoMap extends USSRBossHomerEngineMoMap
{	
	constructor()
	{
		USSRBossHomerEngineMoMap.constructor( )
		animPack = GetAnimPack( "Anims/Bosses/Red/mi12_homer_engine_r/mi12_homer_engine_r.anipk" )
	}
}