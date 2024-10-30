sigimport "Gameplay/mobile/helicopter/common/hoverVehicleLogic.nut"
sigimport "gameplay/boss/common/ussrboss_flyingtank_MasterGoal.goaml"
sigimport "Gameplay/mobile/common/MobileTurretLogic.nut"
sigimport "Anims/bosses/red/sub_typhoon/sub_typhoon.anipk"

// cargo
sigimport "gameplay/characters/infantry/ussr/infantry_basic_01.sigml"
sigimport "gameplay/mobile/vehicle/ussr/tank_medium_01.sigml"
sigimport "gui/textures/waveicons/ussr/vehicle_tank03_g.png"
sigimport "gui/textures/waveicons/ussr/infantry_lvl2_g.png"
sigimport "gui/textures/waveicons/ussr/boss_typhoon_g.png"
sigimport "gui/textures/waveicons/ussr/vehicle_helihormone_g.png"
sigimport "Gameplay/Mobile/Helicopter/USSR/helo_hormone.sigml"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Boss_Typhoon( )
}


class USSR_Boss_Typhoon extends HoverVehicleLogic
{
	stageNum = 0
	paths = { }
	realDeathed = false
	turretDeployed = 0 
	
	constructor( )
	{
		HoverVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USSR_Boss_Typhoon"

	function OnSpawn( )
	{
		stageNum 		= 1
		realDeathed 	= false
		
		local rocketBank = WeaponStation( 0 ).Bank( 1 )
		rocketBank.AddWeapon( "BOSS_TYPHOON_MISSILES", "missiles" ).SetAnimated( OwnerEntity )
		rocketBank.TriggerButton = GAMEPAD_BUTTON_LTRIGGER
		
		AddCargo( "USSR_Sub_Helos" )
		AddCargo( "USSR_Sub_Tanks" )
		AddCargo( "USSR_Sub_Troops" )
		
		//SetAlternateDebrisMesh( "Art/Units/Bosses/Red/mi12_homer_debris.sigml" )
		SetAlternateDebrisMeshParent( "root" ) //must happen before on base class's OnSpawn	

		HoverVehicleLogic.OnSpawn( )
		UseDefaultEndTransition = 0		
		//AICheckForCollisions = 1
		
		LinkChildAudioLogics( 1, 0 )
				
		paths = [ ]
		paths.push( "Boss_Ground_Path_01" )
		paths.push( "Boss_Ground_Path_02" )
		paths.push( "Boss_Ground_Path_03" )
		paths.push( "Boss_Ground_Path_04" )
		
		AddExplicitTarget( GameApp.CurrentLevel.NamedObject( "missileTarget" ) )
		AddExplicitTarget( GameApp.CurrentLevel.NamedObject( "missileTarget2" ) )
		AddExplicitTarget( GameApp.CurrentLevel.NamedObject( "missileTarget3" ) )
		AddExplicitTarget( GameApp.CurrentLevel.NamedObject( "missileTarget4" ) )
		AddExplicitTarget( GameApp.CurrentLevel.NamedObject( "missileTarget5" ) )
		AddExplicitTarget( GameApp.CurrentLevel.NamedObject( "missileTarget6" ) )
		AddExplicitTarget( GameApp.CurrentLevel.NamedObject( "missileTarget7" ) )
		AddExplicitTarget( GameApp.CurrentLevel.NamedObject( "missileTarget8" ) )
		AddExplicitTarget( GameApp.CurrentLevel.NamedObject( "missileTarget9" ) )
		
		// damage tint
		SetDamageTintColor( Math.Vec4.Construct( 1,0,0,0 ) )
		SetHealthBarColor( Math.Vec4.Construct( 1.0, 0.0, 0.0, 1.0 ) )
		
		SetStateOverride( 0, 0 ) //set to state zero immediately		
		
	}

	function SetMasterGoal( ) GoalDriven.MasterGoal = Boss_FlyingTank_MainGoal( this, { } )
	function SetMotionMap( ) Animatable.MotionMap = USSR_Boss_Typhoon_MotionMap( this )

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

	// EVENTS
	function OnReachedEndOfPath( ) // called from Reached End of Path event in goaml
	{
		print( " got to end of path stageNum: " + stageNum )
		
		{
			UnitPath.ClearPath( )
			UnitPath.Wait( )
			
			switch( stageNum )
			{
				case 0:
				{
					// first loop, assuming the boss has just finished intro path
					//stageNum = 1
					HandleLogicEvent( LogicEvent.Construct( GAME_EVENT_USER_FIRE_BEGIN ) )			
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
		local nextPath = paths[ stageNum - 1 ]
		print( "start new path stage: " + stageNum + " path: " + nextPath )
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
		print( "Boss Health: " + percent + " stage " + stageNum )
		
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
			HitPointsModifier = 0.15
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
		if( stageNum == 1 )
		{
			stageNum = 2
			SetStateOverride( 1, 0 )
			PreventDeath( )
			::DisplayImpactText( "BossStage2" )
			return true
		}	
		else if( stageNum == 2 )
		{
			stageNum = 3
			SetStateOverride( 2, 0 )
			PreventDeath( )
			::DisplayImpactText( "BossStage3" )
			return true
		}	
		else if( stageNum == 3 )
		{
			stageNum = 4
			SetStateOverride( 3, 0 )
			PreventDeath( )
			::DisplayImpactText( "BossFinalStage" )
			return true
		}	
		else
		{
			//dead
			stageNum = 5
			DisableAIWeaponFire( 1 )
			SetStateOverride( 4, 0 )
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
			print( "RealDeath" )
			ShutDown( 1 )
			ShutDown( 0 )
			ExplodeIntoAllParts( )
			//GameEffects.PlayEffect( OwnerEntity, "FlyingTankExploded" )
			//OwnerEntity.Delete( )	
			GameApp.CurrentLevel.OnBossDestroyed( )
		}
	}	
}

// MOTION MAPS
class USSR_Boss_Typhoon_MotionMap extends HoverMotionMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/bosses/red/sub_typhoon/sub_typhoon.anipk" )
	}

	function Forward( params )
	{
		if( Logic.stageNum == 4 )
			FlyingForward( params ) 
		else
			Idle( params )
	}
	
	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "idle" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.Push( Stack )
		
		if (Logic.stageNum == 1 && !Logic.turretDeployed)
			DeployTurret( params )// deploy the turret right away
		
		else if (Logic.turretDeployed)
		{
			local track = Anim.KeyFrameTrack( )
			track.Anim = animPack.Find( "deploy_turret_idle" )
			track.BlendIn = 0.0
			track.BlendOut = 0.0		
			track.Push( Stack )
		}
	}
	
	function FlyingForward( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "fly" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.Push( Stack )
		
	}
	
	function Death( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "death" )
		track.BlendIn = 0.0
		track.BlendOut = 0.01
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		track.Push( Stack )
		return track.Anim.OneShotLength
		//return 1.0
	}
	
	function Critical( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "critical" )
		track.BlendIn = 0.0
		track.BlendOut = 0.2
		track.Flags = ANIM_TRACK_CLAMP_TIME
		track.Push( Stack )
		
		return track.Anim.OneShotLength - track.BlendOut
		//return 1.0
	}
	
	function CargoBegin( params )
	{
		local index = Logic.CurrentCargo.Index
		print( "begin cargo drop: " + index )
		
		local track = Anim.KeyFrameTrack( )
		track.BlendIn = 0.0
		track.BlendOut = 0.00
		track.Flags = ANIM_TRACK_CLAMP_TIME

		switch( index )
		{
			case 0:
				track.Anim = animPack.Find( "deploy_copters" )
			break;
			case 1:
				track.Anim = animPack.Find( "deploy_plank" )
			break;
			case 2:
				//track.Anim = animPack.Find( "deploy_turret" )
				return 4.0;
			break;
		}
		
		track.Push( Stack )
		return track.Anim.OneShotLength - track.BlendOut
	}
	
	function DeployTurret( params )
	{
		Logic.turretDeployed = 1
		local track = Anim.KeyFrameTrack( )
		track.BlendIn = 0.0
		track.BlendOut = 0.0	
		track.Anim = animPack.Find( "deploy_turret" )
		track.Flags = ANIM_TRACK_CLAMP_TIME

		track.Push( Stack )
		
		return track.Anim.OneShotLength - track.BlendOut
	}
	function CargoIdle( params )
	{
		local index = Logic.CurrentCargo.Index
		
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
		
		track.Push( Stack )
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
				track.Anim = animPack.Find( "retract_copters" )
			break;
			case 1:
				track.Anim = animPack.Find( "retract_plank" )
			break;
			case 2:
				//track.Anim = animPack.Find( "retract_turret" )
				return 4.03;
			break;
		}
		Stack.RemoveTracksWithTag( "cargoidle" )
		track.Push( Stack )
		return track.Anim.OneShotLength - track.BlendOut
	}	
	
	function Entrance( params )
	{
		return 1.0
	}
	
	function Recoil( params )
	{
		local anim = "fire_1"
		switch( params.MuzzleID )
		{
			case 1:	anim = "fire_2"; break;
			case 2:	anim = "fire_3"; break;
			case 3:	anim = "fire_4"; break;
			case 4:	anim = "fire_5"; break;
			case 5:	anim = "fire_6"; break;
			case 6:	anim = "fire_7"; break;
			case 7:	anim = "fire_8"; break;
			case 8:	anim = "fire_9"; break;
			case 9:	anim = "fire_10"; break;
		}
			
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( anim )
		track.BlendIn = 0.01
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		track.Push( Stack )
	}
}
