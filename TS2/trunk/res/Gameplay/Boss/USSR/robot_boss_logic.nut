sigimport "Gameplay/mobile/common/WheeledMobileLogic.nut"
sigimport "Gameplay/mobile/common/MobileTurretLogic.nut"
sigimport "gameplay/boss/common/ussrboss_flyingtank_MasterGoal.goaml"

sigimport "gui/textures/waveicons/ussr/vehicle_tank03_g.png" 

sigimport "Anims/bosses/red/boss_beetle/boss_beetle.anipk"
sigimport "Anims/bosses/red/boss_beetle_torso/boss_beetle_torso.anipk"

sigimport "gui/textures/waveicons/ussr/vehicle_helihormone_g.png"
sigimport "Gameplay/Mobile/Helicopter/USSR/helo_hormone.sigml"

sigimport "gameplay/characters/infantry/ussr/infantry_elite_01.sigml"
sigimport "Gui/Textures/WaveIcons/USSR/infantry_lvl2_g.png"

sigimport "Effects/Entities/Boss/beetle_tread_debris.sigml"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Boss_Robot( )
}

class USSR_Boss_Robot extends ScriptWheeledVehicleLogic
{
	stageNum = 0
	InAir = 0
	
	paths = { }
	realDeathed = false
	
	teslaWeap = null
	gatlingWeap = null
	rocketBank  = null
	cannon1Weap = null
	cannon2Weap  = null

	constructor( )
	{
		ScriptWheeledVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USSR_Boss_Robot"

	function OnSpawn( )
	{
		stageNum 		= 0
		realDeathed 	= false
		InAir = 1

		// set weapons
		rocketBank = WeaponStation( 0 ).Bank( 4 )
		local rocketWeap = 	rocketBank.AddWeapon( "BOSS_TANK_ROCKETS", "rocketsL" )
		rocketBank.AddWeapon( "BOSS_TANK_ROCKETS", "rocketsR" )
		
		gatlingWeap = WeaponStation( 0 ).Bank( 0 ).AddWeapon( "BOSS_TANK_MG", "gun1" )
		cannon1Weap = WeaponStation( 0 ).Bank( 1 ).AddWeapon( "BOSS_TANK_AUTOGUN", "gun2" )
		cannon2Weap = WeaponStation( 0 ).Bank( 2 ).AddWeapon( "BOSS_TANK_CANNON", "gun3" )
		teslaWeap   = WeaponStation( 0 ).Bank( 3 ).AddWeapon( "BOSS_TANK_TESLA", "cannon" )					
		
		local torsoEnt = rocketWeap.SetTurretEntityNamed( "torso" )
		torsoEnt.Logic.Animatable.MotionMap = USSR_Boss_Robot_MainGunMoMap( this )
		torsoEnt.Logic.Animatable.ExecuteMotionState( "Idle", { } )	
		
		gatlingWeap.SetTurretEntity( torsoEnt )
		cannon2Weap.SetTurretEntity( torsoEnt )
		cannon1Weap.SetTurretEntity( torsoEnt )
		teslaWeap.SetTurretEntity( torsoEnt )
		
		rocketWeap.SetAnimated( torsoEnt )
		gatlingWeap.SetAnimated( torsoEnt )
		cannon1Weap.SetAnimated( torsoEnt )
		cannon2Weap.SetAnimated( torsoEnt )
		teslaWeap.SetAnimated( torsoEnt )
		
		rocketBank.Enabled = 0
		teslaWeap.Enabled = 0
		gatlingWeap.Enabled = 0
		cannon1Weap.Enabled = 0
		cannon2Weap.Enabled = 0

		AddCargo( "USSR_Elite_Robot_Boss" )
		AddCargo( "USSR_RobotCopters" )
		AddCargo( "USSR_Robot_Tanks" )

		// set treads
		SetWheelDustSigmlPath( "Effects/Entities/Boss/beetle_tread_debris.sigml" )
		
		SetAlternateDebrisMesh( "Art/Units/Bosses/Red/mi12_homer_debris.sigml" )
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

		
		AddExplicitTarget( GameApp.CurrentLevel.NamedObject( "missileTarget1" ) )
		AddExplicitTarget( GameApp.CurrentLevel.NamedObject( "missileTarget2" ) )
		AddExplicitTarget( GameApp.CurrentLevel.NamedObject( "missileTarget3" ) )
		AddExplicitTarget( GameApp.CurrentLevel.NamedObject( "missileTarget4" ) )
		AddExplicitTarget( GameApp.CurrentLevel.NamedObject( "missileTarget5" ) )
		AddExplicitTarget( GameApp.CurrentLevel.NamedObject( "missileTarget6" ) )
		AddExplicitTarget( GameApp.CurrentLevel.NamedObject( "missileTarget7" ) )
		AddExplicitTarget( GameApp.CurrentLevel.NamedObject( "missileTarget8" ) )
		AddExplicitTarget( GameApp.CurrentLevel.NamedObject( "missileTarget9" ) )						
		AddExplicitTarget( GameApp.CurrentLevel.NamedObject( "missileTarget10" ) )						
						
		// damage tint
		SetDamageTintColor( Math.Vec4.Construct( 1,0,0,0 ) )
		SetHealthBarColor( Math.Vec4.Construct( 1.0, 0.0, 0.0, 1.0 ) )

		SetStateOverride( 0, 0 ) //set to state zero immediately		
		
		GameEffects.PlayEffect( OwnerEntity, "Boss_Persistent" )
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

	function ApplyAnimToDeliveryChildren( )
	{
		local ev = LogicEvent.Construct( GAME_EVENT_REAPPLY_ONESHOT_MOTION_STATE )
		
		//print( "Deplying delivery helos" )

		local i
		for( i = 0; i < HitpointLinkedChildrenCount( ); ++i )
		{
			local child = HitpointLinkedChild( i )
			if( child.GetName( ) == "BeetleHeloTransportMoMap_A" )
				child.Logic.HandleLogicEvent( ev )
			else if( child.GetName( ) == "BeetleHeloTransportMoMap_B" )
				child.Logic.HandleLogicEvent( ev )
			else if( child.GetName( ) == "BeetleHeloTransportMoMap_C" )
				child.Logic.HandleLogicEvent( ev )
			else if( child.GetName( ) == "BeetleHeloTransportMoMap_D" )
				child.Logic.HandleLogicEvent( ev )
			else if( child.GetName( ) == "BeetleRopeMoMap_A" )
				child.Logic.HandleLogicEvent( ev )
			else if( child.GetName( ) == "BeetleRopeMoMap_B" )
				child.Logic.HandleLogicEvent( ev )
			else if( child.GetName( ) == "BeetleRopeMoMap_C" )
				child.Logic.HandleLogicEvent( ev )
			else if( child.GetName( ) == "BeetleRopeMoMap_D" )
				child.Logic.HandleLogicEvent( ev )
		}
	}		

	function SetMasterGoal( ) GoalDriven.MasterGoal = Boss_FlyingTank_MainGoal( this, { } )
	function SetMotionMap( ) Animatable.MotionMap = USSR_Boss_Robot_MotionMap( this )

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
			
			rocketBank.Enabled = 0
			teslaWeap.Enabled = 0
			gatlingWeap.Enabled = 1
			cannon1Weap.Enabled = 1
			cannon2Weap.Enabled = 1
		
			SetStateOverride( 0, 0 )
			PreventDeath( )
			::DisplayImpactText( "BossStage2" )
			return true
		}	
		else if( stageNum == 2 )
		{
			stageNum = 3
			
			rocketBank.Enabled = 0
			teslaWeap.Enabled = 1
			gatlingWeap.Enabled = 1
			cannon1Weap.Enabled = 1
			cannon2Weap.Enabled = 1
			
			SetStateOverride( 1, 0 )
			PreventDeath( )
			::DisplayImpactText( "BossStage3" )
			return true
		}	
		else if( stageNum == 3 )
		{
			stageNum = 4
			
			rocketBank.Enabled = 0
			teslaWeap.Enabled = 0
			gatlingWeap.Enabled = 0
			cannon1Weap.Enabled = 0
			cannon2Weap.Enabled = 0
			
			SetStateOverride( 2, 0 )
			PreventDeath( )
			::DisplayImpactText( "BossFinalStage" )
			return true
		}	
		else
		{
			//dead
			stageNum = 5
			DisableAIWeaponFire( 1 )
			SetStateOverride( 3, 0 )
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
class USSR_Boss_Robot_MotionMap extends VehicleMotionMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/bosses/red/boss_beetle/boss_beetle.anipk" )
	}
	
	function Forward( params )
	{
		{
			local track = Anim.KeyFrameTrack( )
			local anim = "forward_4"
			
			switch( Logic.stageNum )
			{
				case 0: anim = "forward_1";
					//Logic.rocketBank.Enabled = 0
					//Logic.teslaWeap.Enabled = 0
					//Logic.gatlingWeap.Enabled = 0
					//Logic.cannon1Weap.Enabled = 0
					//Logic.cannon2Weap.Enabled = 0
				break;
				case 1: anim = "forward_1";
					//Logic.rocketBank.Enabled = 0
					//Logic.teslaWeap.Enabled = 0
					//Logic.gatlingWeap.Enabled = 0
					//Logic.cannon1Weap.Enabled = 0
					//Logic.cannon2Weap.Enabled = 0
				break;
				case 2: anim = "forward_2"; 
					Logic.rocketBank.Enabled = 0
					Logic.teslaWeap.Enabled = 0
					Logic.gatlingWeap.Enabled = 1
					Logic.cannon1Weap.Enabled = 1
					Logic.cannon2Weap.Enabled = 1
				break;
				case 3: anim = "forward_3"; 
					Logic.rocketBank.Enabled = 0
					Logic.teslaWeap.Enabled = 1
					Logic.gatlingWeap.Enabled = 1
					Logic.cannon1Weap.Enabled = 1
					Logic.cannon2Weap.Enabled = 1
				break;
				case 4: anim = "forward_4"; break;
					//Logic.rocketBank.Enabled = 0
					//Logic.teslaWeap.Enabled = 0
					//Logic.gatlingWeap.Enabled = 0
					//Logic.cannon1Weap.Enabled = 0
					//Logic.cannon2Weap.Enabled = 0
			}
			
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
		
		switch( Logic.stageNum )
		{
			case 2: anim = "stage1to2"; break;
			case 3: anim = "stage2to3"; break;
			case 4: anim = "stage3to4"; break;
		}
		
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
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "deploy" )
		track.BlendIn = 0.1
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		track.Push( Stack )

		return ( track.Anim.OneShotLength )
	}
	
	function CargoIdle( params )
	{
		//local track = Anim.KeyFrameTrack( )
		//track.Anim = animPack.Find( "open_hatch_idle" )
		//track.BlendIn = 0.1
		//track.BlendOut = 0.0
		//
		//track.Push( Stack )
	}
	
	function CargoEnd( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "retract" )
		track.BlendIn = 0.1
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		track.Push( Stack )

		return ( track.Anim.OneShotLength )
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

class USSR_Boss_Robot_MainGunMoMap extends MobileTurretMoMap
{
	owner = null
	
	constructor( owner_ )
	{
		MobileTurretMoMap.constructor( )
		owner = owner_
	}
	
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/bosses/red/boss_beetle_torso/boss_beetle_torso.anipk" )
	}

	function PushMuzzleTrack( params, low, high, lowAngle, highAngle )
	{
		local track = Anim.PitchBlendMuzzleTrack( )
		track.LowAnim = animPack.Find( low )
		track.HighAnim = animPack.Find( high )
		track.LowAngle = lowAngle
		track.HighAngle = highAngle
		track.BlendIn = 0.25
		track.BlendOut = 0.0
		track.Weapon = params.Weapon
		track.Flags = ANIM_TRACK_PARTIAL
		track.Push( Stack )
	}
	
	function Idle( params )
	{		
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "idle" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		
		track.Push( Stack )
	}
	function Entrance( params )
	{		
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "flyin" )
		track.BlendIn = 0.2
		track.BlendOut = 0.2
		track.Flags = ANIM_TRACK_CLAMP_TIME

		track.Push( Stack )
		
		return (track.Anim.OneShotLength - 0.25)
	}

	function Forward( params )
	{		
		local anim = "forward_4"
		
		switch( owner.stageNum )
		{
			case 0: anim = "forward_1"; break;
			case 1: anim = "forward_1"; break;
			case 2: anim = "forward_2"; break;
			case 3: anim = "forward_3"; break;
			case 4: anim = "forward_4"; break;
		}
		
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( anim  )
		track.BlendIn = 0.0
		track.BlendOut = 0.0
		
		track.Push( Stack )
		
		if( owner.teslaWeap.Enabled )
		{
			params.Weapon = owner.teslaWeap
			PushMuzzleTrack( params, "aim_down_tesla", "aim_up_tesla", -85, 85 )
		}
		
		if( owner.cannon1Weap.Enabled )
		{
			params.Weapon = owner.cannon1Weap
			PushMuzzleTrack( params, "aim_down_guns", "aim_up_guns", -85, 85 )
		}
	}
	
	function Recoil( params )
	{
		local track = Anim.KeyFrameTrack( )
		
		if( params.BankID == 0 )
			track.Anim = animPack.Find( "fire_tesla" )
		else if( params.BankID == 1 )
			track.Anim = animPack.Find( "fire_gun1" )
		else if( params.BankID == 2 )
			track.Anim = animPack.Find( "fire_gun2" )
		else if( params.BankID == 3 )
			track.Anim = animPack.Find( "fire_gun3" )
		else
			return 0.0
			
		track.BlendIn = 0.01
		track.BlendOut = 0.02
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		track.Push( Stack )

		return (track.Anim.OneShotLength - track.BlendOut)
	}
	
	function Critical( params )
	{		
		local anim = "critical"
		
		switch( owner.stageNum )
		{
			case 2: anim = "stage1to2"; break;
			case 3: anim = "stage2to3"; break;
			case 4: anim = "stage3to4"; break;
		}
		
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( anim  )
		
		track.BlendIn = 0.01
		track.BlendOut = 0.02
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		track.Push( Stack )

		return (track.Anim.OneShotLength - track.BlendOut)
	}
	
	function SpecialMove( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "smash" )
		track.BlendIn = 0.2
		track.BlendOut = 0.2
		track.Flags = ANIM_TRACK_CLAMP_TIME		
		track.Push( Stack )

		return (track.Anim.OneShotLength - 0.25)
	}
}
