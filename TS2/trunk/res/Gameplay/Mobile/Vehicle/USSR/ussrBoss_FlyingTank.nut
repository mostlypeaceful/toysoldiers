sigimport "Gameplay/mobile/common/WheeledMobileLogic.nut"
sigimport "Gameplay/mobile/common/MobileTurretLogic.nut"
sigimport "gameplay/boss/common/ussrboss_flyingtank_MasterGoal.goaml"

sigimport "gameplay/boss/antonov_aa.sigml"
sigimport "gameplay/mobile/vehicle/ussr/tank_screwtank.sigml"
sigimport "gameplay/mobile/vehicle/ussr/tank_electric_screwtank.sigml"
sigimport "gui/textures/waveicons/ussr/boss_antonov_g.png" 
sigimport "gui/textures/waveicons/ussr/vehicle_tank03_g.png" 

sigimport "Anims/Bosses/Red/antonov_cannon/antonov_cannon.anipk"
sigimport "Anims/Bosses/Red/antonov/antonov.anipk"
sigimport "gameplay/boss/antonov_airbase.sigml"

sigimport "Effects/Entities/Boss/flying_tank_treads_debris.sigml"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Boss_FlyingTank( )
}

class USSR_Boss_FlyingTank extends ScriptWheeledVehicleLogic
{
	stageNum 		= 0
	InAir = 0
	
	paths = { }
	realDeathed = false

	constructor( )
	{
		ScriptWheeledVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USSR_Boss_FlyingTank"

	function OnSpawn( )
	{
		stageNum 		= 0
		realDeathed 	= false
		InAir = 0

		// set weapons
		local cannonBank 	= WeaponStation( 0 ).Bank( 0 )
//		local gunBank 		= WeaponStation( 0 ).Bank( 1 )
//		local aaBank 		= WeaponStation( 0 ).Bank( 2 )
//		local flameBank 		= WeaponStation( 0 ).Bank( 3 )

		local cannon = cannonBank.AddWeapon( "BOSS_FLYINGTANK_CANNON", "cannon" )
		local cannonE = cannon.SetTurretEntityNamed( "cannon" )
		cannonE.Logic.Animatable.MotionMap = USSR_Boss_FlyingTank_MainGunMoMap( )
		cannonE.Logic.Animatable.ExecuteMotionState( "Idle", { Weapon = cannon } )			

//		local bombBank = WeaponStation( 0 ).Bank( 3 )
//		flameBank.AddWeapon( "BOSS_FLYINGTANK_FLAME", "FlamethrowerA" )
//		flameBank.AddWeapon( "BOSS_FLYINGTANK_FLAME", "FlamethrowerB" )
//		bombBank.FireMode = FIRE_MODE_ALTERNATE
			
//		local gun = gunBank.AddWeapon( "BOSS_FLYINGTANK_MG", "tinygun1" )
//		local gun = gunBank.AddWeapon( "BOSS_FLYINGTANK_MG", "tinygun2" )
//		local gun = gunBank.AddWeapon( "BOSS_FLYINGTANK_MG", "tinygun3" )
//		local gun = gunBank.AddWeapon( "BOSS_FLYINGTANK_MG", "tinygun4" )
//		gunBank.FireMode = FIRE_MODE_ALTERNATE
		
		AddCargo( "USSR_Screw_Tank2" )
		AddCargo( "USSR_Screw_Tank3" )
		AddCargo( "USSR_Screw_Tank4" )
		
		// set treads
		SetWheelDustSigmlPath( "Effects/Entities/Boss/flying_tank_treads_debris.sigml" )

		// set gibs for death TODO: set flying tank debris
		SetAlternateDebrisMesh( "Art/Units/Bosses/Red/mi12_homer_debris.sigml" )
		SetAlternateDebrisMeshParent( "root" ) //must happen before on base class's OnSpawn	
		
		//SetFlyingBase( "gameplay/boss/antonov_airbase.sigml" )
		
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
		
		AddExplicitTarget( GameApp.CurrentLevel.NamedObject( "antonovTarget1" ) )
		AddExplicitTarget( GameApp.CurrentLevel.NamedObject( "antonovTarget2" ) )
		AddExplicitTarget( GameApp.CurrentLevel.NamedObject( "antonovTarget3" ) )
		AddExplicitTarget( GameApp.CurrentLevel.NamedObject( "antonovTarget4" ) )
		AddExplicitTarget( GameApp.CurrentLevel.NamedObject( "antonovTarget5" ) )		
		AddExplicitTarget( GameApp.CurrentLevel.NamedObject( "antonovTarget6" ) )	
		AddExplicitTarget( GameApp.CurrentLevel.NamedObject( "antonovTarget7" ) )
		AddExplicitTarget( GameApp.CurrentLevel.NamedObject( "antonovTarget8" ) )

				
		// damage tint
		SetDamageTintColor( Math.Vec4.Construct( 1,0,0,0 ) )
		SetHealthBarColor( Math.Vec4.Construct( 1.0, 0.0, 0.0, 1.0 ) )
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
	function SetMotionMap( ) Animatable.MotionMap = USSR_Boss_FlyingTank_MotionMap( this )

	// EVENTS
	function OnReachedEndOfPath( ) // called from Reached End of Path event in goaml
	{
		print( " got to end of path stageNum: " + stageNum )
		local flyingBase = FlyingBase
		if( !is_null( flyingBase ) )
		{
			AttachFlyingBase( 0 )
			GameEffects.PlayEffect( OwnerEntity, "FlyingTankLanded" )
			print("flyingtank landed")
		}
		else
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
		DamageModifier = 1
		
		local flashRate = 0.0
		local fillTime = 1.0
		local color = Math.Vec4.Construct( 1.0, 0.0, 0.0, 1.0 )
				
		if( stageNum == 2 )
		{
			HitPointsModifier = 1
			flashRate = 1.0
			color.y = 0.5
		}
		else if( stageNum == 3 )
		{
			HitPointsModifier = 1
			flashRate = 0.5
			color.y = 0.25
		}
		else if( stageNum == 4 )
		{
			HitPointsModifier = 0.08
			flashRate = 0.1
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
			SetStateOverride( 2, 1 )
			PreventDeath( )
			::DisplayImpactText( "BossStage2" )
			return true
		}	
		else if( stageNum == 2 )
		{
			stageNum = 3
			SetStateOverride( 3, 1 )
			PreventDeath( )
			::DisplayImpactText( "BossStage3" )
			return true
		}	
		else if( stageNum == 3 )
		{
			stageNum = 4
			SetStateOverride( 4, 1 )
			PreventDeath( )
			::DisplayImpactText( "BossFinalStage" )
			return true
		}	
		else
		{
			//dead
			stageNum = 5
			DisableAIWeaponFire( 1 )
			SetStateOverride( 5, 0 )
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
			SetStateOverride( 1, 0 )
		}
	}	
}

// MOTION MAPS
class USSR_Boss_FlyingTank_MotionMap extends VehicleMotionMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Bosses/Red/antonov/antonov.anipk" )
	}

	function Forward( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "forward" )
		track.BlendIn = 0.0
		track.BlendOut = 0.0
		
		track.Push( Stack )
		
		TankTreads( params )
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
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "critical" )
		track.BlendIn = 0.0
		track.BlendOut = 0.3
		
		track.Push( Stack )
		
		TankTreads( params )
		
		return track.Anim.OneShotLength - track.BlendOut
	}
	
	function CargoBegin( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "open_hatch" )
		track.BlendIn = 0.1
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		track.Push( Stack )

		return ( track.Anim.OneShotLength )
	}
	
	function CargoIdle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "open_hatch_idle" )
		track.BlendIn = 0.1
		track.BlendOut = 0.0
		
		track.Push( Stack )
	}
	
	function CargoEnd( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "close_hatch" )
		track.BlendIn = 0.1
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		track.Push( Stack )

		return ( track.Anim.OneShotLength )
	}	
	
	function Entrance( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "fly_in" )
		track.BlendIn = 0.2
		track.BlendOut = 0.2
		
		track.Push( Stack )

		return (track.Anim.OneShotLength - 0.25)
	}
}

class USSR_Boss_FlyingTank_MainGunMoMap extends MobileTurretMoMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Bosses/Red/antonov_cannon/antonov_cannon.anipk" )
	}
	
	function Recoil( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( "fire" )
		track.BlendIn = 0.01
		track.BlendOut = 0.01
		track.Flags = ANIM_TRACK_CLAMP_TIME
		
		track.Push( Stack )

		return (track.Anim.OneShotLength )
	}
}
