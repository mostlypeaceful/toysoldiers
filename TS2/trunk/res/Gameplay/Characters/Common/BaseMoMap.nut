
sigimport "Anims/Characters/Shared/Base_Soldier/infantry/infantry.anipk"
sigimport "Anims/Characters/Shared/elite/elite.anipk"

class SoldierMotionMap extends Anim.MotionMap
{
	animPack = null
	eliteAnimPack = null

	constructor( logic )
	{
		Anim.MotionMap.constructor( )
		animPack = GetAnimPack( "Anims/Characters/Shared/Base_Soldier/infantry/infantry.anipk" )
		eliteAnimPack = GetAnimPack( "Anims/Characters/Shared/elite/elite.anipk" )
	}

	function MoveForward( params )
	{
		local runAnim = "run"
		local hurry = ObjectiveRand.Float( 0.75, 1.0 )	
		local persist = Logic.CurrentPersistentEffect
		
		switch( persist )
		{
			case PERSISTENT_EFFECT_BEHAVIOR_FIRE:
				runAnim = "burning_run"
				GameEffects.PlayEffect( Logic.OwnerEntity, "Voc_OnFire" )
				break
			case PERSISTENT_EFFECT_BEHAVIOR_GAS:
				runAnim = "cough_stumble"
				GameEffects.PlayEffect( Logic.OwnerEntity, "Voc_Cough" )
				break
			case PERSISTENT_EFFECT_BEHAVIOR_STUN:
				runAnim = "flashbang_run"		// would be nice to play an actual 'stunned' anim here...eh?
				break
			default:
			{
				if( Logic.FirstWaveLaunch )
				{
					Logic.FirstWaveLaunch = 0
					GameEffects.PlayEffect( Logic.OwnerEntity, "Voc_Charge" )
				}
				
				if( Logic.InAlarmZone )
				{
					switch( ObjectiveRand.Int( 1, 2 ) )
					{
						case 1: runAnim = "highstep_run"; break;
						case 2: runAnim = "taunt_run"; break;
					}
				}
				else
				{				
					// no swimming anims
					//switch( Logic.SurfaceTypeEnum( ) )
					//{
					////case SURFACE_TYPE_WATER :
					////	runAnim = "land"; break;
					//default:
						if( Logic.IsCaptain )
							runAnim = "flag_run";
						else
						{
							switch( ObjectiveRand.Int( 1, 2 ) )
							{
								case 1: runAnim = "run"; break;
								case 2: runAnim = "run_2"; break;
							}
						}
					//}
				}
			}
		}
		
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( runAnim )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.TimeScale = hurry
		track.StartTime = ObjectiveRand.Float( 0.0, track.Anim.OneShotLength )
		
		track.Push( Stack )
		
		FollowPath( params )
	}

	function FollowPath( params )
	{
		local sourceEnt = Logic.UnitPath
		local rotateSpeed = 5.0
		
		local track = Anim.UnitPathTrack( )
		track.UnitPath = sourceEnt
		track.RotateSpeed = rotateSpeed
		track.BlendIn = 0.2
		track.BlendOut = 0.0

		track.Push( Stack )
	}
	
	function SpecialEntrance( params )
	{
		local type = Logic.OwnerEntity.GetEnumValue( ENUM_SPECIAL_ENTRANCE )
		local track = Anim.KeyFrameTrack( )
		track.BlendIn = 0.2
		track.BlendOut = 0.2
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		switch( type )
		 {
			 case SPECIAL_ENTRANCE_HELI_RAMP:		 
				track.Anim = animPack.Find( "mi12_jump" )
				break;
			 case SPECIAL_ENTRANCE_ROPE_SLIDE:		 
				track.Anim = eliteAnimPack.Find( "rope_slide" )
				break;
			 case SPECIAL_ENTRANCE_USSR_APCMG01_LEFT:
				track.Anim = animPack.Find( "apcbtr60_deploy_left" )
				break;
			 case SPECIAL_ENTRANCE_USSR_APCMG01_RIGHT:
				track.Anim = animPack.Find( "apcbtr60_deploy_right" )
				break;
			case SPECIAL_ENTRANCE_SUB_DEPLOY:
				track.Anim = eliteAnimPack.Find( "sub_deploy" )
				break;
			 case SPECIAL_ENTRANCE_USA_APCMG01_LEFT:
				track.Anim = animPack.Find( "apc_m113_deploy_left" )
				break;
			 case SPECIAL_ENTRANCE_USA_APCMG01_RIGHT:
				track.Anim = animPack.Find( "apc_m113_deploy_right" )
				break;
			 case SPECIAL_ENTRANCE_USSR_APCIFV01_LEFT:
				track.Anim = animPack.Find( "apc_bmp_deploy_left" )
				break;
			 case SPECIAL_ENTRANCE_USSR_APCIFV01_RIGHT:
				track.Anim = animPack.Find( "apc_bmp_deploy_right" )
				break;
			 case SPECIAL_ENTRANCE_SUPERTANK_DEPLOY:		 
				track.Anim = animPack.Find( "supertank_deploy" )
				break;
			default:
				return 0.0
		 }

		track.Push( Stack )		
		return track.Anim.OneShotLength - track.BlendOut
	}

	function UserGroundMove( params )
	{
		
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "run" )
		track.BlendIn = 0.2
		track.BlendOut = 0.2

		track.Push( Stack )
		
		local track2 = Anim.KeyFrameTrack( )
		track2.Anim = animPack.Find( "run_left" )
		track2.BlendIn = 0.2
		track2.BlendOut = 0.2
		track2.BlendStrength = 0.5

		track2.Push( Stack )
		
//		local move = Anim.CharacterMoveTrackFPS( )
//		move.RunAnim = animPack.Find( "run" )
//		move.IdleAnim = animPack.Find( "idle_lax" )		
//		move.BlendIn = 0.2
//		move.BlendOut = 0.0	
//		move.Character = Logic
//
//		move.PushBeforeTag( Stack, "UpperBody" )		
			
		/// Old IK stuff, not relevant to current blending issue, yet.
		/*local ik = Anim.IKLimbTrack( )
		ik.Anim = animPack.Find( "ik_left_foot" )
		ik.Owner = Logic
		ik.BlendIn = 0.2
		ik.BlendOut = 0.0
		ik.TargetCallback = Logic.IKLegs
		ik.TargetChannel = 0
		ik.TargetGroup = 0

		ik.Push( Stack, "UpperBody" )
		
		local ikr = Anim.IKLimbTrack( )
		ikr.Anim = animPack.Find( "ik_right_foot" )
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
		local aim = Anim.CharacterAimTrack( )
		aim.HighAnim = animPack.Find( "idle_aim_up" )
		aim.LowAnim = animPack.Find( "idle_aim_down" )		
		aim.BlendIn = 0.1
		aim.BlendOut = 0.0	
		aim.Character = Logic
		aim.Tag = "UpperBody"

		aim.Push( Stack )
		
		/*local ik = Anim.IKCCDTrack( )
		ik.Anim = animPack.Find( "ik_left_arm" )
		ik.Owner = Logic
		ik.BlendIn = 0.2
		ik.BlendOut = 0.0
		//ik.TargetCallback = Logic.IKLegs
		ik.TargetChannel = 0
		ik.TargetGroup = 1
		ik.Tag = "UpperBody"

		ik.Push( Stack )
		
		local ikr = Anim.IKCCDTrack( )
		ikr.Anim = animPack.Find( "ik_right_arm" )
		ikr.Owner = Logic
		ikr.BlendIn = 0.2
		ikr.BlendOut = 0.0
		//ikr.TargetCallback = Logic.IKLegs
		ikr.TargetChannel = 1
		ikr.TargetGroup = 1
		ikr.Tag = "UpperBody"

		ikr.Push( Stack )*/
	}

	function UserGroundAndAim( params )
	{
		UserGroundMove( params )
		Aim( params )
	}
	
	function RandomAnim( params )
	{
		local track = Anim.KeyFrameTrack( )
		
		local randChance = ObjectiveRand.Int(0, 100)
		if (randChance < 10)
		{
			track.Anim = animPack.Find( "trip_and_fall" )
			GameEffects.PlayEffect( Logic.OwnerEntity, "Voc_TripFall" )
		}
		else if (randChance < 35)
		{
			local propId = Logic.OwnerEntity.GetEnumValue( ENUM_CHARACTER_PROPS, -1 )
			
			if( Logic.WaveDisabledAIFire )
			{
				track.Anim = animPack.Find( "flag_charge" )
				GameEffects.PlayEffect( Logic.OwnerEntity, "Voc_Rally" )
			}
			else if( propId != -1 )
			{
				switch( propId )
				{
					case CHARACTER_PROPS_ROCKET_LAUNCHER:
						track.Anim = animPack.Find( "stand_fire_01" )
						GameEffects.PlayEffect( Logic.OwnerEntity, "Voc_Combat" )
						break
					case CHARACTER_PROPS_PISTOL:
						track.Anim = animPack.Find( "fire_rpg_low01" )
						GameEffects.PlayEffect( Logic.OwnerEntity, "Voc_Combat" )
						break
					case CHARACTER_PROPS_FLAG:
						track.Anim = animPack.Find( "flag_charge" )
						GameEffects.PlayEffect( Logic.OwnerEntity, "Voc_Rally" )
						break	
					default:				
						return 0.0
				}
			}
			else
			{
				track.Anim = animPack.Find( "stand_fire_01" )
				GameEffects.PlayEffect( Logic.OwnerEntity, "Voc_Combat" )
			}
		}
		else 
			return 0.0
				
		track.BlendIn = 0.1
		track.BlendOut = 0.1
		track.Push( Stack )
		return track.Anim.OneShotLength - track.BlendOut
	}

	function VictoryDance( params )
	{
		local track = Anim.KeyFrameTrack( )
		
		local danceAnim
		
		switch( ObjectiveRand.Int( 1, 3 ) )
		{
			case 1: danceAnim = "dance_raisetheroof"
			break;

			case 2: danceAnim = "dance_backflip"
			break;

			case 3: danceAnim = "dance_airhump"
			break;
	
			default:
			break;			
		}
		
		track.Anim = animPack.Find( danceAnim )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_CLAMP_TIME

		track.Push( Stack )
		GameEffects.PlayEffect( Logic.OwnerEntity, "Voc_Dance" )
		GameEffects.PlayEffect( Logic.OwnerEntity, "SoldierAcrossTheGoalFx" )

		return track.Anim.OneShotLength
	}

	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "idle_lax" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.StartTime = ObjectiveRand.Float( 0.0, track.Anim.OneShotLength )

		track.Push( Stack )
		
		if( Logic.CurrentPersistentEffect == PERSISTENT_EFFECT_BEHAVIOR_FIRE )
		{
			local track = Anim.KeyFrameTrack( )
			track.Anim = animPack.Find( "onfire" )
			track.BlendIn = 0.2
			track.BlendOut = 0.0
			track.Push( Stack )			
		}
	}

	function Fall( params )
	{
		local anim = "fall"
		
		if( Logic.Parachuting )
			anim = "chute_idle"
		
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( anim )
		track.BlendIn = 0.2
		track.BlendOut = 0.0

		track.PushBeforeTag( Stack, "UpperBody" )
	}

	function DeathFall( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "explosion_loop" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.StartTime = ObjectiveRand.Float( 0.0, track.Anim.OneShotLength )
		
		track.PushBeforeTag( Stack, "UpperBody" )
		GameEffects.PlayEffect( Logic.OwnerEntity, "Voc_Death_Fly" )
	}

	function Land( params )
	{
		local anim = "land"
		
		if( Logic.Parachuting )
			anim = "chute_land"
		
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( anim )
		track.BlendIn = 0.2
		track.BlendOut = 0.2

		track.PushBeforeTag( Stack, "UpperBody" )

		return track.Anim.OneShotLength - 0.2
	}

	function StandardDeath( params )
	{
		local deathAnim = "death_armshot1"
		
		switch( ObjectiveRand.Int( 1, 8 ) )
		{
		case 1:
			deathAnim = "death_armshot1"
			break;
		case 2:
			deathAnim = "death_headshot1"
			break;
		case 3:
			deathAnim = "death_toy_blowback1"
			break;
		case 4:
			deathAnim = "death_toy_blowback2"
			break;
		case 5:
			deathAnim = "death_toy_blowback3"
			break;
		case 6:
			deathAnim = "death_toyspin1"
			break;
		case 7:
			deathAnim = "death_toyspin2"
			break;
		case 8:
			deathAnim = "death_toyspin3"
			break;
		}
		
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( deathAnim )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_CLAMP_TIME

		track.Push( Stack )

		Logic.VocDeath( )
		return track.Anim.OneShotLength - 2 * track.BlendOut
	}
	
	function Reload( params )
	{	
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "reload" )
		track.BlendIn = 0.2
		track.BlendOut = 0.2
		track.Tag = "UpperBody"

		track.Push( Stack )

		return track.Anim.OneShotLength - 0.2
	}
	
	function FireOneShot( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "recoil" )
		track.BlendIn = 0.05
		track.BlendOut = 0.05
		
		track.Push( Stack )
		
		//return 0.25
		return track.Anim.OneShotLength - 0.1
		
	}
	
	function FireLooping( params )
	{
		//local track = Anim.KeyFrameTrack( )
		//track.Anim = animPack.Find( "recoil" )
		//track.BlendIn = 0.2
		//track.BlendOut = 0.0
		//track.Tag = "FireLooping"

		//track.Push( Stack )
	}
	
	function Throw( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "throwgrenade" )
		track.BlendIn = 0.2
		track.BlendOut = 0.2
		track.Tag = "UpperBody"

		track.Push( Stack )

		return track.Anim.OneShotLength - 0.2
	}
	
	function Melee( params )
	{
		local timeScale = 1.0
		
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "rifle_swing" )
		track.BlendIn = 0.08
		track.BlendOut = 0.08
		track.TimeScale = timeScale
		track.Tag = "UpperBody"

		track.Push( Stack )

		return track.Anim.OneShotLength / timeScale - 0.2
	}
	
	function Stand2Crawl( params )
	{
		local timeScale = 1.0
		
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "stand2crawl" )
		track.BlendIn = 0.2
		track.BlendOut = 0.2
		track.TimeScale = timeScale

		track.Push( Stack )

		return track.Anim.OneShotLength / timeScale - 0.2
	}
	
	function ContextJump( params )
	{
		local timeScale = 1.0
		
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "jump" )
		track.BlendIn = 0.2
		track.BlendOut = 0.2
		track.TimeScale = timeScale

		track.Push( Stack )

		return track.Anim.OneShotLength / timeScale - 0.2
	}
	
	function Crawl2Stand( params )
	{
		local timeScale = 1.0
		
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "crawl2stand" )
		track.BlendIn = 0.2
		track.BlendOut = 0.2
		track.TimeScale = timeScale

		track.Push( Stack )

		return track.Anim.OneShotLength / timeScale - 0.2
	}
	
	function Crawl( params )
	{
		local timeScale = 1.0
		
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "crawl" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.TimeScale = timeScale

		track.Push( Stack )
		
		FollowPath( params )
	}
	
	function AlignToTarget( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "idle_lax" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0

		track.Push( Stack )
		
		local track = Anim.UnitContextAlignTrack( )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.UnitPath = Logic.UnitPath
		track.RotateSpeed = 10.0

		track.Push( Stack )
	}
	
	function MoveToLadder( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "run" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0

		track.Push( Stack )
		
		local sourceEnt = Logic.UnitPath
		local rotateSpeed = 5.0
		
		local track = Anim.UnitPathTrack( )
		track.UnitPath = sourceEnt
		track.RotateSpeed = rotateSpeed
		track.BlendIn = 0.2
		track.BlendOut = 0.0

		track.Push( Stack )
	}
	
	function AlignToLadder( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "idle_lax" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0

		track.Push( Stack )
		
		PushContextAlign( )
	}
	
	function ClimbingLadder( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "climb" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0

		track.Push( Stack )
		
		local sourceEnt = Logic.UnitPath
		
		local track = Anim.UnitInterpolatePathTrack( )
		track.UnitPath = sourceEnt
		track.BlendIn = 0.2
		track.BlendOut = 0.0

		track.Push( Stack )
	}
	
	function PushContextAlign( rotateSpeed = 10 )
	{		
		local track = Anim.UnitContextAlignTrack( )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.UnitPath =  Logic.UnitPath
		track.RotateSpeed = rotateSpeed

		track.Push( Stack )
	}
	
	function StepUp( params )
	{
		//print( "step height = " + params.Height )
		
		local timeScale = 1.0
		
		local track = Anim.KeyFrameTrack( )
		
		if( params.Height < 1.0 )
			track.Anim = animPack.Find( "step_low" )
		else
			track.Anim = animPack.Find( "step_high" )
			
		track.BlendIn = 0.2
		track.BlendOut = 0.2
		track.TimeScale = timeScale

		track.Push( Stack )

		local duration = track.ScaledOneShotLength - 0.2
		
		PushContextAlign( )
		
		return duration
	}	

	function WallClimb( params )
	{
		//print( "wall height = " + params.Height )
		
		local timeScale = 1.0
		
		local track = Anim.KeyFrameTrack( )
		
		if( params.Height < 1.0 )
			track.Anim = animPack.Find( "wallclimb_100" )
		else if( params.Height < 1.5 )
			track.Anim = animPack.Find( "wallclimb_150" )
		else if( params.Height < 2.0 )
			track.Anim = animPack.Find( "wallclimb_200" )
		else
			track.Anim = animPack.Find( "wallclimb_300" )
		
		track.BlendIn = 0.2
		track.BlendOut = 0.2
		track.TimeScale = timeScale

		track.Push( Stack )

		local duration = track.ScaledOneShotLength - 0.2
		
		PushContextAlign( )
		
		return duration
	}	

	function VaultOver( params )
	{
		//print( "vault height = " + params.Height )
		
		local timeScale = 1.0
		
		local track = Anim.KeyFrameTrack( )
		
		if( params.Height < 2 )
			track.Anim = animPack.Find( "vaultover" )
		else
			track.Anim = animPack.Find( "vaultover_high" )
			
		track.BlendIn = 0.2
		track.BlendOut = 0.2
		track.TimeScale = timeScale

		track.Push( Stack )

		local duration = track.ScaledOneShotLength - 0.2
		
		PushContextAlign( )
		
		return duration
	}	
		
	function CaptainIdle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "flag_wave" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0

		track.Push( Stack )

		return track.Anim.OneShotLength - track.BlendOut
	}
		
	function CaptainWaveLaunch( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "flag_charge" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0

		track.Push( Stack )

		GameEffects.PlayEffect( Logic.OwnerEntity, "Voc_Rally" )
		return track.Anim.OneShotLength - track.BlendOut
	}
	

}


class EliteMotionMap extends SoldierMotionMap
{	

	function MoveForward( params )
	{
		local runAnim = null
		local hurry = ObjectiveRand.Float( 0.75, 1.0 )
		local track = Anim.KeyFrameTrack( )
		track.Anim = eliteAnimPack.Find( "run_rpg01" )
		
		local persist = Logic.CurrentPersistentEffect
		switch( persist )
		{
			case PERSISTENT_EFFECT_BEHAVIOR_FIRE:
				track.Anim = animPack.Find( "burning_run" )
				GameEffects.PlayEffect( Logic.OwnerEntity, "Voc_OnFire" )
				break
			case PERSISTENT_EFFECT_BEHAVIOR_STUN:
				track.Anim = animPack.Find( "flashbang_run" )
				break;
			case PERSISTENT_EFFECT_BEHAVIOR_GAS:
				//immune
				break;
			default:
			{
				if( Logic.FirstWaveLaunch )
				{
					Logic.FirstWaveLaunch = 0
					GameEffects.PlayEffect( Logic.OwnerEntity, "Voc_Charge" )
				}
			}
		}
		
		
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.TimeScale = hurry
		track.StartTime = ObjectiveRand.Float( 0.0, track.Anim.OneShotLength )
		
		track.Push( Stack )
		
		FollowPath( params )
	}
	
	function FireOneShot( params )
	{
		local propId = Logic.OwnerEntity.GetEnumValue( ENUM_CHARACTER_PROPS, -1 )
		
		if( propId != -1 )
		{
			local track = Anim.KeyFrameTrack( )
			
			switch( propId )
			{
				case CHARACTER_PROPS_ROCKET_LAUNCHER:
					track.Anim = eliteAnimPack.Find( "fire_rpg_low01" )		
					GameEffects.PlayEffect( Logic.OwnerEntity, "Voc_Combat" )				
					break
				case CHARACTER_PROPS_PISTOL:
					track.Anim = eliteAnimPack.Find( "fire_gun01" )
					GameEffects.PlayEffect( Logic.OwnerEntity, "Voc_Combat" )
					break
				case CHARACTER_PROPS_FLAG:
					//falls through on purpose, no elite flag anim
				default:				
					return 0.0
			}
			
			track.BlendIn = 0.1
			track.BlendOut = 0.1
			track.Push( Stack )
			
		
			local track2 = Anim.UnitContextAlignTrack( )
			track2.BlendIn = 0.2
			track2.BlendOut = 0.0
			track2.UnitPath = Logic.UnitPath
			track2.RotateSpeed = 10.0
			track2.Push( Stack )
		
			return track.Anim.OneShotLength - track.BlendOut					
		}

	}
	
	function RandomAnim( params )
	{
		local track = Anim.KeyFrameTrack( )
		
		local randChance = ObjectiveRand.Int(0, 100)
		if (randChance < 10)
		{
			track.Anim = animPack.Find( "trip_and_fall" )
			GameEffects.PlayEffect( Logic.OwnerEntity, "Voc_TripFall" )
		}
		else 
			return 0.0
			
		// Elite firing is done through the goaml and the FireOneShot state above.
				
		track.BlendIn = 0.1
		track.BlendOut = 0.1
		track.Push( Stack )
		return track.Anim.OneShotLength - track.BlendOut
	}

	
}