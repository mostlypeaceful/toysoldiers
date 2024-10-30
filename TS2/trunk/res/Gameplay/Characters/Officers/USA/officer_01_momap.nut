
sigimport "Anims/Characters/Shared/Base_Soldier/infantry/infantry.anipk"
sigimport "Anims/Characters/Shared/commando/commando.anipk"
sigimport "Gameplay/Characters/Common/basemomap.nut"

class USA_Officer_01_Momap extends SoldierMotionMap
{
	newAnimPack = null
	forBarrage = false

	constructor( logic, forBarrage_ )
	{
		forBarrage = forBarrage_
		SoldierMotionMap.constructor( logic )
		newAnimPack = GetAnimPack( "Anims/Characters/Shared/commando/commando.anipk" )
	}

	function UserGroundMove( params )
	{
		local move = Anim.CharacterMoveTrackFPS( )
		move.RunForwardAnim = newAnimPack.Find( "run" )
		move.RunBackwardAnim = newAnimPack.Find( "run_back" )
		move.RunLeftAnim = newAnimPack.Find( "run_left" )
		move.RunRightAnim = newAnimPack.Find( "run_right" )
		move.RunLeftBackAnim = newAnimPack.Find( "run_back_left" )
		move.RunRightBackAnim = newAnimPack.Find( "run_back_right" )
		move.IdleAimUpAnim = newAnimPack.Find( "idle_aim_high" )		
		move.IdleAimDownAnim = newAnimPack.Find( "idle_aim_low" )	
		move.RunAimUpAnim = newAnimPack.Find( "run_aim_high" )		
		move.RunAimDownAnim = newAnimPack.Find( "run_aim_low" )		
		move.IdleAnim = newAnimPack.Find( "idle" )				
		move.SprintAnim = newAnimPack.Find( "sprint" )	
		move.BlendIn = 0.3
		move.BlendOut = 0.0	
		move.Character = Logic

		move.PushBeforeTag( Stack, "UpperBody" )
		
//		local track = Anim.KeyFrameTrack( )
//		track.Anim = newAnimPack.Find( "run" )
//		track.BlendIn = 0.2
//		track.BlendOut = 0.0
//
//		track.Push( Stack )
		
//		local track2 = Anim.KeyFrameTrack( )
//		track2.Anim = newAnimPack.Find( "run_left" )
//		track2.BlendIn = 0.2
//		track2.BlendOut = 0.0
//		track2.BlendScale = 0.5
//
//		track2.Push( Stack )
		
//		local move = Anim.CharacterMoveTrackFPS( )
//		move.RunAnim = newAnimPack.Find( "run" )
//		move.IdleAnim = newAnimPack.Find( "idle_lax" )		
//		move.BlendIn = 0.2
//		move.BlendOut = 0.0	
//		move.Character = Logic
//
//		move.PushBeforeTag( Stack, "UpperBody" )		
			
		/// Old IK stuff, not relevant to current blending issue, yet.
		/*local ik = Anim.IKLimbTrack( )
		ik.Anim = newAnimPack.Find( "ik_left_foot" )
		ik.Owner = Logic
		ik.BlendIn = 0.2
		ik.BlendOut = 0.0
		ik.TargetCallback = Logic.IKLegs
		ik.TargetChannel = 0
		ik.TargetGroup = 0

		ik.Push( Stack, "UpperBody" )
		
		local ikr = Anim.IKLimbTrack( )
		ikr.Anim = newAnimPack.Find( "ik_right_foot" )
		ikr.Owner = Logic
		ikr.BlendIn = 0.2
		ikr.BlendOut = 0.0
		ikr.TargetCallback = Logic.IKLegs
		ikr.TargetChannel = 1
		ikr.TargetGroup = 0

		ikr.Push( Stack, "UpperBody" )*/
	}
	
	function Aim( params )
	{		
//		local aim = Anim.CharacterAimTrack( )
//		aim.HighAnim = newAnimPack.Find( "aim_high" )
//		aim.LowAnim = newAnimPack.Find( "aim_low" )		
//		aim.BlendIn = 0.1
//		aim.BlendOut = 0.0	
//		aim.Character = Logic
//		aim.Tag = "UpperBody"
//
//		aim.Push( Stack )
		
		/*local ik = Anim.IKCCDTrack( )
		ik.Anim = newAnimPack.Find( "ik_left_arm" )
		ik.Owner = Logic
		ik.BlendIn = 0.2
		ik.BlendOut = 0.0
		//ik.TargetCallback = Logic.IKLegs
		ik.TargetChannel = 0
		ik.TargetGroup = 1
		ik.Tag = "UpperBody"

		ik.Push( Stack )
		
		local ikr = Anim.IKCCDTrack( )
		ikr.Anim = newAnimPack.Find( "ik_right_arm" )
		ikr.Owner = Logic
		ikr.BlendIn = 0.2
		ikr.BlendOut = 0.0
		//ikr.TargetCallback = Logic.IKLegs
		ikr.TargetChannel = 1
		ikr.TargetGroup = 1
		ikr.Tag = "UpperBody"

		ikr.Push( Stack )*/
	}
	

	function MoveForward( params )
	{
		local hurry = ObjectiveRand.Float( 0.75, 1.0 )
		
		if( Logic.ShellShocked ) 
			hurry = 0.1
		
		local runAnim
		
		switch( Logic.SurfaceTypeEnum( ) )
		{
		case SURFACE_TYPE_WATER :
			runAnim = "land"; break;
		default:
			switch( ObjectiveRand.Int( 1, 2 ) )
			{
			case 1: runAnim = "run"; break;
			case 2: runAnim = "run"; break;
			}
		}
		
		local track = Anim.KeyFrameTrack( )
		track.Anim = newAnimPack.Find( runAnim )
		track.BlendIn = 0.2
		track.BlendOut = 0.1
		track.TimeScale = hurry

		track.Push( Stack )
		
		FollowPath( params )
		
		return track.Anim.OneShotLength
	}

	function UserGroundAndAim( params )
	{
		UserGroundMove( params )
		Aim( params )
	}
	
	function UserFall( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = newAnimPack.Find( "fall" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0

		track.PushBeforeTag( Stack, "UpperBody" )
		
		local aim = Anim.CharacterAimTrack( )
		aim.HighAnim = newAnimPack.Find( "fall_aim_high" )
		aim.LowAnim = newAnimPack.Find( "fall_aim_low" )		
		aim.BlendIn = 0.25
		aim.BlendOut = 0.0	
		aim.Character = Logic

		aim.Push( Stack )
	}
	
	function UserLand( params )
	{
		local move = Anim.CharacterMoveTrackFPS( )
		move.RunForwardAnim = newAnimPack.Find( "land_forward" )
		move.RunBackwardAnim = newAnimPack.Find( "land_back" )
		move.RunLeftAnim = newAnimPack.Find( "land_left" )
		move.RunRightAnim = newAnimPack.Find( "land_right" )
		move.RunLeftBackAnim = newAnimPack.Find( "land_back_left" )
		move.RunRightBackAnim = newAnimPack.Find( "land_back_right" )
		move.IdleAimUpAnim = newAnimPack.Find( "idle_aim_high" )		
		move.IdleAimDownAnim = newAnimPack.Find( "idle_aim_low" )	
		move.RunAimUpAnim = newAnimPack.Find( "run_aim_high" )		
		move.RunAimDownAnim = newAnimPack.Find( "run_aim_low" )		
		move.IdleAnim = newAnimPack.Find( "land" )				
		move.SprintAnim = newAnimPack.Find( "land_sprint" )	
		move.BlendIn = 0.1
		move.BlendOut = 0.3	
		move.Character = Logic

		move.PushBeforeTag( Stack, "UpperBody" )
		
		GameEffects.PlayEffect( Logic.OwnerEntity, "Play_Character_Commando_Land" )

		//return move.RunForwardAnim.OneShotLength - 0.3
		return 0.3
	}

	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = newAnimPack.Find( "idle" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0

		track.Push( Stack )
	}

	function Fall( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = newAnimPack.Find( "fall" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0

		track.PushBeforeTag( Stack, "UpperBody" )
	}

	function DeathFall( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = newAnimPack.Find( "fall" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0

		track.PushBeforeTag( Stack, "UpperBody" )
	}

	function Land( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = newAnimPack.Find( "land" )
		track.BlendIn = 0.2
		track.BlendOut = 0.2

		track.PushBeforeTag( Stack, "UpperBody" )

		return track.Anim.OneShotLength - 0.2
	}

	function EnterFall( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = newAnimPack.Find( "enter_fall" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0

		track.PushBeforeTag( Stack, "UpperBody" )
	}

	function EnterLand( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = newAnimPack.Find( "enter_land" )
		track.BlendIn = 0.2
		track.BlendOut = 0.2

		track.PushBeforeTag( Stack, "UpperBody" )

		return track.Anim.OneShotLength - 0.2
	}
	

	function StandardDeath( params )
	{
		local track = Anim.KeyFrameTrack( )
		if( forBarrage )
			track.Anim = newAnimPack.Find( "ending" )
		else
			track.Anim = newAnimPack.Find( "death_headshot" )
			
		track.BlendIn = 0.2
		track.BlendOut = 0.1
		track.Flags = ANIM_TRACK_CLAMP_TIME

		track.Push( Stack )

		return track.Anim.OneShotLength - 0.2
	}
	
	function Reload( params )
	{	
		local track = Anim.KeyFrameTrack( )
		track.Anim = newAnimPack.Find( "reload" )
		track.BlendIn = 0.2
		track.BlendOut = 0.2
		track.Tag = "UpperBody"

		track.Push( Stack )

		return track.Anim.OneShotLength - 0.2
	}
	
	function FireOneShot( params )
	{
//		local track = Anim.KeyFrameTrack( )
//		track.Anim = newAnimPack.Find( "fire_rocket" )
//		track.BlendIn = 0.05
//		track.BlendOut = 0.1
//		
//		track.Push( Stack )
//
//		return track.Anim.OneShotLength - 0.1		
		return 0
	}
	
	function Recoil( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = newAnimPack.Find( "fire_rocket" )
		track.BlendIn = 0.05
		track.BlendOut = 0.1
		
		track.Push( Stack )

		return track.Anim.OneShotLength - 0.1		
	}
	
	function FireLooping( params )
	{
		Stack.RemoveTracksWithTag( "FireLooping" )
		
		local track = Anim.KeyFrameTrack( )
		track.Anim = newAnimPack.Find( "fire_m60" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.Tag = "FireLooping"

		track.Push( Stack )
	}
	
	function Throw( params )
	{
//		local track = Anim.KeyFrameTrack( )
//		track.Anim = newAnimPack.Find( "throwgrenade" )
//		track.BlendIn = 0.2
//		track.BlendOut = 0.2
//		track.Tag = "UpperBody"
//
//		track.Push( Stack )
//
//		return track.Anim.OneShotLength - 0.2
		return 0
	}
	
	function Melee( params )
	{
//		local timeScale = 1.0
//		
//		local track = Anim.KeyFrameTrack( )
//		track.Anim = newAnimPack.Find( "rifle_swing" )
//		track.BlendIn = 0.08
//		track.BlendOut = 0.08
//		track.TimeScale = timeScale
//		track.Tag = "UpperBody"
//
//		track.Push( Stack )
//
//		return track.Anim.OneShotLength / timeScale - 0.2
		return 0
	}
	
	function JetPackTakeoff( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = newAnimPack.Find( "jetpack_takeoff" )
		track.BlendIn = 0.2
		track.BlendOut = 0.1
		
		track.PushBeforeTag( Stack, "UpperBody" )
		
		return track.Anim.OneShotLength - 0.1

	}
	
	function Push4BlendTrack( animPack, frontLeft, frontRight, backLeft, backRight )
	{
		local track = Anim.VehiclePassengerTrack( )
		track.FrontLeftAnim = animPack.Find( frontLeft )
		track.FrontRightAnim = animPack.Find( frontRight )
		track.BackLeftAnim = animPack.Find( backLeft )
		track.BackRightAnim = animPack.Find( backRight )
		
		track.BlendIn = 0.2
		track.BlendOut = 1.0
		track.Acc = Logic.ParentSprungMass( )
		track.PushBeforeTag( Stack, "UpperBody" )
	}
	
	function JetPack( params )
	{
		// push his idle to clear the stack
		local track = Anim.KeyFrameTrack( )
		track.Anim = newAnimPack.Find( "jetpack_idle" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.PushBeforeTag( Stack, "UpperBody" )
		
		// push his blend
		Push4BlendTrack( newAnimPack, "jetpack_fly_frontleft", "jetpack_fly_frontright", "jetpack_fly_backleft", "jetpack_fly_backright" )
		
		JetPackTakeoff( params )
		
		//Stack.RemoveTracksWithTag( "jetpack_aim" )
		
		local aim = Anim.CharacterAimTrack( )
		aim.HighAnim = newAnimPack.Find( "jetpack_aim_high" )
		aim.LowAnim = newAnimPack.Find( "jetpack_aim_low" )		
		aim.BlendIn = 0.25
		aim.BlendOut = 0.0	
		aim.Character = Logic
		aim.Tag = "UpperBody"

		aim.Push( Stack )
	}

}

