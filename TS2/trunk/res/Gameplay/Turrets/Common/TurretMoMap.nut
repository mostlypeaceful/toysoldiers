sigimport "Anims/Turrets/Shared/turrets_shared.anipk"

class TurretMotionMap extends Anim.MotionMap
{
	animPack = null
	sharedAnimPack = null
	muzzleAngleLow = -85
	muzzleAngleHigh = 85
	reloadTimeOverride = null
	timeOverride = null
	
	constructor( logic )
	{
		Anim.MotionMap.constructor( )
		sharedAnimPack = GetAnimPack( "Anims/Turrets/Shared/turrets_shared.anipk" )
		SetAnimPack( )
	}
	
	function SetAnimPack( )
	{
		BreakPoint( "You must set an anim pack in a derived momap!" );
	}
	
	function UseMuzzleTrack( low, high )
	{
		muzzleAngleLow = low
		muzzleAngleHigh = high
	}

	function PushTurretOrientTrack( )
	{
		local track = Anim.TurretOrientTrack( )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		
		track.Turret = Logic
		track.Push( Stack )
	}
	function PushTurretPitchTrack( animPack, down, up, clampTime = false, bankIndex = 0 )
	{
		local track = Anim.PitchBlendMuzzleTrack( )
		track.LowAnim = animPack.Find( down )
		track.HighAnim = animPack.Find( up )
		track.LowAngle = muzzleAngleLow
		track.HighAngle = muzzleAngleHigh
		track.BlendIn = 0.0
		track.BlendOut = 0.0
		
		if( clampTime )
			track.Flags = ANIM_TRACK_CLAMP_TIME
		
		local trackLen = track.LowAnim.OneShotLength
		if( timeOverride )
		{
			track.TimeScale = track.LowAnim.OneShotLength / timeOverride
			trackLen = timeOverride
		}
		
		track.Weapon = Logic.Weapon( 0, bankIndex, 0 )
		track.Push( Stack )
		
		return trackLen
	}

	function GhostIdle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "idle_ai" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0

		track.Push( Stack )
	}
	function Idle( params )
	{
		local idletrack = Anim.KeyFrameTrack( )
		idletrack.Anim = animPack.Find( "idle_ai" )
		idletrack.BlendIn = 0.1
		idletrack.BlendOut = 0.0
		idletrack.Flags = ANIM_TRACK_RESUME_TIME
		
		idletrack.Push( Stack )
		
		PushTurretOrientTrack( )
	}
	function SpinUp( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "startup" )
		track.BlendIn = 0.01
		track.BlendOut = 0.0
		track.TimeScale = 1.0
		track.StartTime = Anim.KeyFrameTrack.CurrentTimeOfTrack( track.Anim, Stack )
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		track.Push( Stack )
		
		return (track.Anim.OneShotLength / track.TimeScale - 0.1 - track.StartTime)
	}
	function SpinDown( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "startup" )
		track.BlendIn = 0.01
		track.BlendOut = 0.0
		track.TimeScale = -1.0
		track.StartTime = Anim.KeyFrameTrack.CurrentTimeOfTrack( track.Anim, Stack )
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		Aim( params )
		track.Push( Stack )		
		return track.StartTime
	}
	function Aim( params )
	{
		//local idletrack = Anim.KeyFrameTrack( )
		//idletrack.Anim = animPack.Find( "idle" )
		//idletrack.BlendIn = 0.1
		//idletrack.BlendOut = 0.0
		//
		//idletrack.Push( Stack )
		
		local bank = 0
		if( Logic.UnitID == UNIT_ID_TURRET_FLAME_03 )
		{
			bank = 1
		}
			
		PushTurretPitchTrack( animPack, "aim_down", "aim_up", false, bank )
		PushTurretOrientTrack( )
	}
	function Reload( params )
	{
		local prevTimeOverride = timeOverride
		timeOverride = reloadTimeOverride
		
		local length = PushTurretPitchTrack( animPack, "reload_low", "reload_high", true )
		PushTurretOrientTrack( )
		
		timeOverride = prevTimeOverride

		return length
	}
	function FireOneShot( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "fire" )
		track.BlendIn = 0.01
		track.BlendOut = 0.01
		track.Tag = "fireOneShot"

		//PushTurretPitchTrack( params, animPack, "aim_down", "aim_up" )
		//PushTurretOrientTrack( )
		
		Stack.RemoveTracksWithTag( track.Tag );
		track.Push( Stack )

		return (track.Anim.OneShotLength  - 0.1)
	}
	function Recoil( params )
	{
		return 0
	}
	function FireLooping( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "fire" )
		track.BlendIn = 0.01
		track.BlendOut = 0.0
		
		track.Push( Stack )
		
		// just incase its called from a oneshot goal
		return (track.Anim.OneShotLength - 0.2)
	}
	function Repair( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "idle_ai" )
		track.BlendIn = 0.1
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_RESUME_TIME
		
		track.Push( Stack )
		
		local idletrack = Anim.KeyFrameTrack( )
		idletrack.Anim = sharedAnimPack.Find( "repair" )
		idletrack.BlendIn = 0.1
		idletrack.BlendOut = 0.0
		
		idletrack.Push( Stack )
		
		return (idletrack.Anim.OneShotLength  - 0.1)
	}
	function Upgrade( params )
	{
		local idletrack = Anim.KeyFrameTrack( )
		idletrack.Anim = animPack.Find( "idle_ai" )
		idletrack.BlendIn = 0.1
		idletrack.BlendOut = 0.0
		idletrack.Flags = ANIM_TRACK_RESUME_TIME
		
		idletrack.Push( Stack )
		
		return 9.0
	}
}

