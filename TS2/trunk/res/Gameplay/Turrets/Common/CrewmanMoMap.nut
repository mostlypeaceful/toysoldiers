sigimport "Anims/Characters/Shared/Base_Soldier/infantry/infantry.anipk"

class CrewmanMoMap extends Anim.MotionMap
{
	oldAnimPack = null
	sharedAnimPack = null
	animPrefix = ""
	animSuffix = ""
	usePitchAimTracks = false
	usePitchReloadTracks = false
	useMuzzleTrack = false
	idleBlendIn = 0.2
	muzzleTrackLow = 0
	muzzleTrackHigh = 0
	bankIndex = 0
	reloadTimeOverride = null
	timeOverride = null
	
	constructor( animPackName, animPrefix_, animSuffix_, usePitchAimTracks_, usePitchReloadTracks_ )
	{
		Anim.MotionMap.constructor()
		sharedAnimPack = GetAnimPack( animPackName )
		if( !sharedAnimPack )
			print( "Could not find animpack: " + animPackName )
			
		oldAnimPack = GetAnimPack( "Anims/Characters/Shared/Base_Soldier/infantry/infantry.anipk" )
		animPrefix = animPrefix_
		animSuffix = animSuffix_	
		usePitchAimTracks = usePitchAimTracks_
		usePitchReloadTracks = usePitchReloadTracks_
		idleBlendIn = 0.2
		useMuzzleTrack = false
		muzzleTrackLow = 0
		muzzleTrackHigh = 0
		bankIndex = 0
	}
	
	function UseMuzzleTrack( lowAngle, highAngle )
	{
		useMuzzleTrack = true
		muzzleTrackLow = lowAngle
		muzzleTrackHigh = highAngle
	}
	
	function PushPitchTrack( params, down, up )
	{
		local track = null
		 
		if( useMuzzleTrack )
		{
			track = Anim.PitchBlendMuzzleTrack( )
			track.LowAnim = sharedAnimPack.Find( down )
			track.HighAnim = sharedAnimPack.Find( up )
			track.LowAngle = muzzleTrackLow
			track.HighAngle = muzzleTrackHigh
			track.BlendIn = 0.1
			track.BlendOut = 0.0
			track.Weapon = params.Turret.Weapon( 0, bankIndex, 0 )
		}
		else
		{
			track = Anim.PitchBlendTrack( )
			track.LowAnim = sharedAnimPack.Find( down )
			track.HighAnim = sharedAnimPack.Find( up )
			track.BlendIn = 0.1
			track.BlendOut = 0.0
			track.Turret = params.Turret
		}
		
		local trackLen = track.LowAnim.OneShotLength
		if( timeOverride )
		{
			track.TimeScale = track.LowAnim.OneShotLength / timeOverride
			trackLen = timeOverride
		}
		
		track.Push( Stack )
		
		return trackLen
	}
	
	function GhostIdle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( animPrefix + "_idle_" + animSuffix )
		track.BlendIn = 0.0
		track.BlendOut = 0.0
		
		track.Push( Stack )
	}
	
	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( animPrefix + "_idle_" + animSuffix )
		track.BlendIn = idleBlendIn
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_RESUME_TIME
		track.Push( Stack )
	}
	
	function Aim( params )
	{
		if( usePitchAimTracks && ("Turret" in params) )
			PushPitchTrack( params, animPrefix + "_aim_low_" + animSuffix, animPrefix + "_aim_high_" + animSuffix )
		else
		{
			local track = Anim.KeyFrameTrack( )
			track.Anim = sharedAnimPack.Find( animPrefix + "_idle_" + animSuffix )
			track.BlendIn = 0.15
			track.BlendOut = 0.0
			track.Push( Stack )
		}
	}
	
	function SpinUp( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( animPrefix + "_startup_" + animSuffix )
		track.BlendIn = 0.15
		track.BlendOut = 0.1
		track.Push( Stack )
	}
	
	function SpinDown( params )
	{
		Aim( params )
	}
	
	function FireOneShot( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( animPrefix + "_fire_" + animSuffix )
		track.BlendIn = 0.1
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_CLAMP_TIME;
		
		track.Push( Stack )
			
		return track.Anim.OneShotLength
	}
	
	function Recoil ( params )
	{
		return 0 
	}
	
	function FireLooping( params )
	{
		local track = Anim.KeyFrameTrack( )
		
		//if( Logic.OwnerEntity.GetEnumValue( ENUM_CHARACTER_PROPS, -1 ) != -1 )
		//{
		//	//testing
		//	track.Anim = sharedAnimPack.Find( "fall" )
		//}
		//else
			track.Anim = sharedAnimPack.Find( animPrefix + "_fire_" + animSuffix )
			
		track.BlendIn = 0.1
		track.BlendOut = 0.0
		
		track.Push( Stack )
	}
	
	function Reload( params )
	{
		local prevTimeOverride = timeOverride
		timeOverride = reloadTimeOverride
		
		if( usePitchReloadTracks )
		{
			local time = PushPitchTrack( params, animPrefix + "_reload_low_" + animSuffix, animPrefix + "_reload_high_" + animSuffix )
			timeOverride = prevTimeOverride
			return time
		}
		else
		{
			local track = Anim.KeyFrameTrack( )
			track.Anim = sharedAnimPack.Find( animPrefix + "_reload_" + animSuffix )
			track.BlendIn = 0.1
			track.BlendOut = 0.0
			
			track.Flags = ANIM_TRACK_CLAMP_TIME;
			track.Push( Stack )
		
			local trackLen = track.Anim.OneShotLength
			if( timeOverride )
			{
				track.TimeScale = track.Anim.OneShotLength / timeOverride
				trackLen = timeOverride
			}
			
			timeOverride = prevTimeOverride
			return trackLen
		}
	}
	
	function RandomAnim( params )
	{		
		return 0.0
	}
		
	// Override RandomAnim and call this
	function PushRandomIdle( params )
	{		
		local track = Anim.KeyFrameTrack( )
		local randChance = ObjectiveRand.Int( 0, 100 )
		
		if( randChance < 25 )
		{
			track.Anim = sharedAnimPack.Find( animPrefix + "_idle_random_" + animSuffix )
			track.BlendIn = 0.1
			track.BlendOut = 0.1
			track.Push( Stack )
			track.Flags = ANIM_TRACK_CLAMP_TIME;
				
			return track.Anim.OneShotLength
		}
		else
			return 0.0
	}	

	function Fall( params )
	{		
		local track = Anim.KeyFrameTrack( )
		track.Anim = oldAnimPack.Find( "fall" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0

		track.Push( Stack )
	}

	function Land( params )
	{
		return 0.0
	}
	
	function Repair( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( animPrefix + "_idle_" + animSuffix )
		track.BlendIn = idleBlendIn
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_RESUME_TIME
		track.Push( Stack )
	}
	
	function Upgrade( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( animPrefix + "_idle_" + animSuffix )
		track.BlendIn = idleBlendIn
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_RESUME_TIME
		track.Push( Stack )
	}
	
	
	// Vehicle Stuff
	function Startup( params )
	{
	}
	
	function Shutdown( params )
	{
	}
	
	function Forward( params )
	{
	}
	
	function DeathFall( params )
	{
	}
	
	function StandardDeath( params )
	{
		print( "Warning, no death anims for inconsequential crewman" )

		return 1.0
	}

}