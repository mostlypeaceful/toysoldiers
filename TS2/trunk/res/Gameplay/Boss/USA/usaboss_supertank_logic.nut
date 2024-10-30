sigimport "Gameplay/mobile/common/WheeledMobileLogic.nut"
sigimport "Gameplay/mobile/common/MobileTurretLogic.nut"
sigimport "gameplay/boss/common/ussrboss_flyingtank_MasterGoal.goaml"

sigimport "Anims/bosses/blue/supertank/supertank.anipk"
sigimport "Anims/bosses/blue/supertank_turret/supertank_turret.anipk"
sigimport "Anims/bosses/blue/supertank_aa/supertank_aa.anipk"
sigimport "Anims/bosses/blue/supertank_phalanx/supertank_phalanx.anipk"

sigimport "gameplay/mobile/vehicle/usa/supervespa_cycle.sigml"
sigimport "gameplay/mobile/vehicle/usa/tank_heavy_01.sigml"
sigimport "gameplay/characters/infantry/usa/infantry_paratrooper_01.sigml"
sigimport "gui/textures/waveicons/USA/infantry_lvl1_g.png"

sigimport "Gui\Textures\WaveIcons\USA\Boss_SuperTank_g.png"
sigimport "Gui\Textures\WaveIcons\USA\vehicle_supervespa_g.png"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_Boss_Supertank_Logic( )
}

class USA_Boss_Supertank_Logic extends ScriptWheeledVehicleLogic
{
	stageNum = 0
	InAir = 0
	
	paths = { }
	realDeathed = false

	constructor( )
	{
		ScriptWheeledVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USA_Boss_Supertank_Logic"

	function OnSpawn( )
	{
		stageNum 		= 0
		realDeathed 	= false
		InAir = 0
		
		local cannonBank = WeaponStation( 0 ).Bank( 0 )
		cannonBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER

		local cannon = cannonBank.AddWeapon( "BOSS_SUPERTANK_CANNON", "cannon" )
		local cannonE = cannon.SetTurretEntityNamed( "cannon" )
		cannonE.Logic.Animatable.MotionMap = USA_Supertank_Cannon_GunMoMap( )
		cannonE.Logic.Animatable.ExecuteMotionState( "Idle", { Weapon = cannon } )
		
		WeaponStation( 0 ).Bank( 1 ).AddWeapon( "BOSS_SUPERTANK_LASER", "lasercannon" )
			
		
//		SetDestroyedEffect( "Medium_Vehicle_Explosion" )
		AddCargo( "USA_Motorbikes" )
		AddCargo( "USA_HeavyTanks" )
		AddCargo( "USA_Boss_Paratroops" )

		// set treads
		//SetWheelDustSigmlPath( "Effects/Entities/Boss/beetle_tread_debris.sigml" )
		
		//SetAlternateDebrisMesh( "Art/Units/Bosses/Red/mi12_homer_debris.sigml" )
		SetAlternateDebrisMeshParent( "root" ) //must happen before on base class's OnSpawn	
				
		// base class OnSpawn
		ScriptWheeledVehicleLogic.OnSpawn( )
		UseDefaultEndTransition = 0		
		AICheckForCollisions = 1
		
		LinkChildAudioLogics( 1, 0 )
				
		paths = [ ]
		paths.push( "Boss_Ground_Path_01" )
		paths.push( "Boss_Ground_Path_02" )
		paths.push( "Boss_Ground_Path_03" )
		paths.push( "Boss_Ground_Path_04" )

		AddExplicitTarget( GameApp.CurrentLevel.NamedObject( "Target_1" ) )
		AddExplicitTarget( GameApp.CurrentLevel.NamedObject( "Target_2" ) )
		AddExplicitTarget( GameApp.CurrentLevel.NamedObject( "Target_3" ) )
		AddExplicitTarget( GameApp.CurrentLevel.NamedObject( "Target_4" ) )
		AddExplicitTarget( GameApp.CurrentLevel.NamedObject( "Target_5" ) )
		AddExplicitTarget( GameApp.CurrentLevel.NamedObject( "Target_6" ) )
					

		// damage tint
		SetDamageTintColor( Math.Vec4.Construct( 1,0,0,0 ) )
		SetHealthBarColor( Math.Vec4.Construct( 1.0, 0.0, 0.0, 1.0 ) )

		SetStateOverride( 0, 0 ) //set to state zero immediately		
		
		GameEffects.PlayEffect( OwnerEntity, "Boss_Persistent" )
			
		// blow up the box
		local box = ::GameApp.CurrentLevel.NamedObject( "destroyBox" )
		//if( !is_null( box ) )
			box.Logic.ForceDestroy( )		
	}

	function ApplyAnimToChildren( contextStr )
	{
		local cont = StringEventContext( )
		cont.String = contextStr

		local ev = LogicEvent( )
		ev.Setup( GAME_EVENT_REAPPLY_MOTION_STATE, cont )

		local i
		for( i = 0; i < HitpointLinkedChildrenCount( ); ++i )
		{
			local child = HitpointLinkedChild( i ).Logic
			child.HandleLogicEvent( ev )
		}
	}		

	function SetMasterGoal( ) GoalDriven.MasterGoal = Boss_FlyingTank_MainGoal( this, { } )
	function SetMotionMap( ) Animatable.MotionMap = USA_SuperTank_MotionMap( this )

	// EVENTS
	function OnReachedEndOfPath( ) // called from Reached End of Path event in goaml
	{
		//print( " got to end of path stageNum: " + stageNum )
		
		{
			UnitPath.ClearPath( )
			UnitPath.Wait( )
			
			switch( stageNum )
			{
				case 0:
				{
					// first loop, assuming the boss has just finished intro path
					stageNum = 1
					StartNewPath( )					
				}
				break;
				case 1:
				{
					// second loop
					//stageNum = 2 //dont auto advance
					HandleLogicEvent( LogicEvent.Construct( GAME_EVENT_USER_FIRE_BEGIN ) )
				}
				break;
				case 2:
				{
					// third loop
					//stageNum = 3 //dont auto advance
					HandleLogicEvent( LogicEvent.Construct( GAME_EVENT_USER_FIRE_BEGIN ) )
				}
				break;
				case 3:
				{
					// going to goal
					//stageNum = 4 //dont auto advance
					HandleLogicEvent( LogicEvent.Construct( GAME_EVENT_USER_FIRE_BEGIN ) )
				}
				break;
				case 4:
				{
					// reached goal
					stageNum = 5
					GameApp.CurrentLevel.OnBossReachedGoal( )
				}
				break;
			}		
		}
	}
	
	function StartNewPath( )
	{
		if( stageNum == 3 && !::GameApp.IsFullVersion )
			::GameApp.DoAskPlayerToBuyGame( )
			
		local nextPath = paths[ stageNum - 1 ]
		//print( "start new path stage: " + stageNum + " path: " + nextPath )
		UnitPath.ClearPath( )
		UnitPath.ClearPathSequence( )
		UnitPath.AddSimpleVehiclePath( nextPath )
		UnitPath.LoopPaths = 0
		UnitPath.StartPathSequence( )		
	}
	
	function HitPointsChanged( ) // called from Unit Damaged event in goaml
	{
		local crit = false
		local percent = HealthPercent
		//print( "Boss Health: " + percent + " stage " + stageNum )
		
		if( InAir && percent < 0.33 )
		{
			//dont let the player reduce to less than 0.33 health during fly in stage.
			//TakesDamage = 0
			DamageModifier = percent //this is a nice soft limit
		}
		
		//if( ( stageNum == 1 && percent <= STAGE1_HEALTH ) ||
		//	( stageNum == 2 && percent <= STAGE2_HEALTH ) ||
		//	( stageNum == 3 && percent <= STAGE3_HEALTH ) )
		//{
		//	if( stageNum < 4 )
		//	{
		//		stageNum += 1				
		//		print("CRITICAL DAMAGE, moving to stage " + stageNum)
		//		
		//		//FireLevelEvent( LEVEL_EVENT_BOSS_STAGE_CHANGED )
		//		
		//		crit = true
		//	}			
		//}
		
		return crit
	}	
	
	function PreventDeath( )
	{		
		TakesDamage = 0
		
		local flashRate = 0.0
		local fillTime = 5.0
		local color = Math.Vec4.Construct( 1.0, 0.0, 0.0, 1.0 )
				
		if( stageNum == 2 )
		{
			HitPointsModifier = 1.0
			flashRate = 1.0
			color.y = 0.5
		}
		else if( stageNum == 3 )
		{
			HitPointsModifier = 1.0
			flashRate = 0.5
			color.y = 0.25
		}
		else if( stageNum == 4 )
		{
			HitPointsModifier = 0.25
			flashRate = 0.25
			color.y = 0.0
		}
		else 
			return false
		
		SetHealthBarFlashAndFill( flashRate, fillTime )
		ResetHitPoints( )
		SetHealthBarColor( color )
		
		CancelAllWeaponFire( )
		
		return true
	}

	function OnZeroHitPoints( ) // called from Unit Zero Hit Points event in goaml
	{
		//this gives us a chance to add hitpoints
		if( stageNum == 1 || stageNum == 0 ) //since there's no intro yet. includes stage 0 here. remove this when there is an intro
		{
			stageNum = 2
		
			//print("moving to stage2")
			SetStateOverride( 0, 0 )
			PreventDeath( )
			::DisplayImpactText( "BossStage2" )
			return true
		}	
		else if( stageNum == 2 )
		{
			stageNum = 3
			//print("moving to stage3")
			SetStateOverride( 1, 0 )
			PreventDeath( )
			::DisplayImpactText( "BossStage3" )
			return true
		}	
		else if( stageNum == 3 )
		{
			stageNum = 4
			SetStateOverride( 1, 0 )
			PreventDeath( )
			::DisplayImpactText( "BossFinalStage" )
			return true
		}	
		else
		{
			//dead
			stageNum = 5
			DisableAIWeaponFire( 1 )
			SetStateOverride( 2, 0 )
			::DisplayImpactText( "BossDestroyed" )
		}
		
		return false
	}
	
	function OnStateChanged( )
	{		
		return false
	}
	
	// called after Death MoState is suspended in goaml
	function RealDeath( )
	{
		if( !realDeathed )
		{
			realDeathed = true
			//print( "RealDeath" )
			ShutDown( 1 )
			ShutDown( 0 )
			ExplodeIntoAllParts( )
			//GameEffects.PlayEffect( OwnerEntity, "FlyingTankExploded" )
			//OwnerEntity.Delete( )	
			GameApp.CurrentLevel.OnBossDestroyed( )
		}
	}
	
	function SpecialEntranceStartStop( startStop )
	{
		if( startStop )
		{
			TakesDamage = 0
			SetStateOverride( 0, 0 )
			if( stageNum == 0 )
				UnitPath.ClearPath( ) //prepare our first path
			InAir = 1
		}
		else
		{
			TakesDamage = 1
			DamageModifier = 1
			InAir = 0
			rocketBank.Enabled = 1
			//SetStateOverride( 1, 0 )
			ApplyAnimToDeliveryChildren( )
		}
	}	
}


// MOTION MAPS
class USA_SuperTank_MotionMap extends VehicleMotionMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/bosses/blue/supertank/supertank.anipk" )
	}
	
	function Forward( params )
	{
		{
			local track = Anim.KeyFrameTrack( )
			local anim = "forward"
			
			//switch( Logic.stageNum )
			//{
			//	case 0: anim = "forward_1"; break;
			//	case 1: anim = "forward_1"; break;
			//	case 2: anim = "forward_2";  break;
			//	case 3: anim = "forward_3"; break;
			//	case 4: anim = "forward_4"; break;
			//}
			
			track.Anim = animPack.Find( anim  )
			track.BlendIn = 0.3
			track.BlendOut = 0.0
			
			track.Push( Stack )
		}
		
		TankTreads( params )
		
		//{
		//	local track = Anim.KeyFrameTrack( )
		//	track.Anim = animPack.Find( "fire_gun3" )
		//	track.BlendIn = 0.0
		//	track.BlendOut = 0.0
		//	
		//	track.Push( Stack )
		//}
	}
	
	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "idle" )
		track.BlendIn = 0.0
		track.BlendOut = 0.0
		
		track.Push( Stack )
		
		TankTreads( params )
	}
	
	function Death( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "death" )
		track.BlendIn = 0.1
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		track.Push( Stack )
		return track.Anim.OneShotLength
	}
	
	function Critical( params )
	{
		local anim = "critical"
		
		//switch( Logic.stageNum )
		//{
		//	case 2: anim = "stage1to2"; break;
		//	case 3: anim = "stage2to3"; break;
		//	case 4: anim = "stage3to4"; break;
		//}
		
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( anim  )
		track.BlendIn = 0.0
		track.BlendOut = 0.3
		
		track.Push( Stack )
		
		TankTreads( params )
		
		return track.Anim.OneShotLength - track.BlendOut
	}
	
	function CargoBegin( params )
	{
		local index = Logic.CurrentCargo.Index
		//print( "begin cargo drop: " + index )
		
		local track = Anim.KeyFrameTrack( )
		track.BlendIn = 0.0
		track.BlendOut = 0.00
		track.Flags = ANIM_TRACK_CLAMP_TIME

		switch( index )
		{
			case 0:
				track.Anim = animPack.Find( "deploy_ramps" )
			break;
			case 1:
				track.Anim = animPack.Find( "deploy_rear" )
			break;
			case 2:
				//track.Anim = animPack.Find( "deploy_turret" )
				return 4.0;
			break;
		}
		
		track.Push( Stack )
		return track.Anim.OneShotLength - track.BlendOut
	}
	
	function CargoIdle( params )
	{
		/*local index = Logic.CurrentCargo.Index
		
		local track = Anim.KeyFrameTrack( )
		track.BlendIn = 0.0
		track.BlendOut = 0.0
		track.Tag = "cargoidle"

		switch( index )
		{
			case 0:
				track.Anim = animPack.Find( "deploy_copters_idle" )
			break;
			case 1:
				track.Anim = animPack.Find( "deploy_plank_idle" )
			break;
			case 2:
				//track.Anim = animPack.Find( "deploy_turret_idle" )
				return;
			break;
		}
		
		track.Push( Stack )*/
	}
	
	function CargoEnd( params )
	{
		local index = Logic.CurrentCargo.Index
		
		local track = Anim.KeyFrameTrack( )
		track.BlendIn = 0.0
		track.BlendOut = 0.00
		track.Flags = ANIM_TRACK_CLAMP_TIME

		switch( index )
		{
			case 0:
				track.Anim = animPack.Find( "retract_ramps" )
			break;
			case 1:
				track.Anim = animPack.Find( "retract_rear" )
			break;
			case 2:
				//track.Anim = animPack.Find( "retract_turret" )
				return 4.03;
			break;
		}
		//Stack.RemoveTracksWithTag( "cargoidle" )
		track.Push( Stack )
		return track.Anim.OneShotLength - track.BlendOut
	}	
	
	function Entrance( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "flyin" )
		track.BlendIn = 0.2
		track.BlendOut = 0.2
		track.Flags = ANIM_TRACK_CLAMP_TIME
		track.Push( Stack )

		Logic.ApplyMotionStateToTurrets( "Entrance" )
		
		return (track.Anim.OneShotLength - 0.25)
	}
	
	function SpecialMove( params )
	{
		Logic.rocketBank.Enabled = 0
		Logic.teslaWeap.Enabled = 0
		Logic.gatlingWeap.Enabled = 0
		Logic.cannon1Weap.Enabled = 0
		Logic.cannon2Weap.Enabled = 0
		
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "smash" )
		track.BlendIn = 0.2
		track.BlendOut = 0.2
		track.Flags = ANIM_TRACK_CLAMP_TIME
		track.Push( Stack )

		return (track.Anim.OneShotLength - 0.25)
	}
	

}

class USA_Supertank_Cannon_GunMoMap extends MobileTurretMoMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Bosses/Blue/supertank_turret/supertank_turret.anipk" )
	}
	
	function Recoil( params )
	{		
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "fire" )
		track.BlendIn = 0.0
		track.BlendOut = 0.1
		track.Push( Stack )
		track.Flags = ANIM_TRACK_CLAMP_TIME;
			
		//Logic.OwnerVehicle.ApplyMotionStateToSoldiers( "Recoil" )
		
		return track.Anim.OneShotLength
	}
}

class USA_Supertank_AA_GunMoMap extends TurretMotionMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Bosses/Blue/supertank_aa/supertank_aa.anipk" )
	}

	function GhostIdle( params )
	{
	}
	function Idle( params )
	{
	}
	function SpinUp( params )
	{
	}
	function SpinDown( params )
	{
	}
	function Reload( params )
	{
	}
	function FireOneShot( params )
	{
	}
	function Recoil( params )
	{
	}
	function FireLooping( params )
	{
	}
	function Repair( params )
	{
	}
	function Upgrade( params )
	{
	}
}

class USA_Supertank_Phalanx_GunMoMap extends TurretMotionMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Bosses/Blue/supertank_phalanx/supertank_phalanx.anipk" )
	}
	
	function GhostIdle( params )
	{
	}
	function Idle( params )
	{
	}
	function SpinUp( params )
	{
		return 0.1
	}
	function SpinDown( params )
	{
		return 0.1
	}
	function Reload( params )
	{
		return 5.0
	}
	function FireOneShot( params )
	{
		return 0.1
	}
	function Recoil( params )
	{
		return 0.1
	}
	function FireLooping( params )
	{
	}
	function Repair( params )
	{
		return 5.0
	}
	function Upgrade( params )
	{
		return 5.0
	}
}