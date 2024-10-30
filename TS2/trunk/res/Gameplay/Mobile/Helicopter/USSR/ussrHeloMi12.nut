sigimport "Gameplay/Mobile/Helicopter/USSR/ussr_helomi12_momap.nut"
sigimport "gameplay/boss/common/ussrboss_flyingtank_MasterGoal.goaml"
sigimport "gameplay/characters/infantry/ussr/infantry_paratrooper_01.sigml"
sigimport "gui/textures/waveicons/ussr/infantry_lvl1_g.png"
sigimport "gui/textures/waveicons/ussr/boss_homer_g.png"
sigimport "gameplay/mobile/vehicle/ussr/tank_medium_01_on_palette.sigml"
sigimport "gameplay/mobile/vehicle/ussr/tank_heavy_01_on_palette.sigml"
sigimport "gameplay/mobile/vehicle/ussr/ifv_01_on_palette.sigml"

sigimport "Art/Units/Bosses/Red/mi12_homer_debris.sigml" 

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Helo_Mi12( )
}

// These are just for your reference
// set weapons
//local cannon = WeaponStation( 0 ).Bank( 0 )
//cannon.AddWeapon( "BOSS_HOMER_ARTY", "cannon" )
//
//local flare = WeaponStation( 0 ).Bank( 1 )
//flare.AddWeapon( "BOSS_HOMER_FLARE", "flare0" )
//flare.AddWeapon( "BOSS_HOMER_FLARE", "flare1" )
//flare.AddWeapon( "BOSS_HOMER_FLARE", "flare2" )

class USSR_Helo_Mi12 extends HoverVehicleLogic
{
	stageNum = 0
	InAir = 0
	paths = { }
	realDeathed = false

	constructor( )
	{
		HoverVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USSR_Helo_Mi12"

	function OnSpawn( )
	{
		stageNum 		= 0
		realDeathed 	= false
		InAir = 0
		
		local cannonBank = WeaponStation( 0 ).Bank( 0 )
//		cannonBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
//
		local cannon = cannonBank.AddWeapon( "BOSS_HOMER_ARTY", "cannon" )
//		WeaponStation( 0 ).Bank( 1 ).AddWeapon( "BOSS_HOMER_ARTY", "cannon" )
//		local cannonE = cannon.SetTurretEntityNamed( "cannon" )
//		cannonE.Logic.Animatable.MotionMap = USA_Supertank_Cannon_GunMoMap( )
//		cannonE.Logic.Animatable.ExecuteMotionState( "Idle", { Weapon = cannon } )
//		
		WeaponStation( 0 ).Bank( 1 ).AddWeapon( "BOSS_HOMER_FLARE", "flare0" )
						
		SetAlternateDebrisMesh( "Art/Units/Bosses/Red/mi12_homer_debris.sigml" )
		SetAlternateDebrisMeshParent( "root" ) //must happen before on HoverLogic.OnSpawn
				
		// base class OnSpawn
		HoverVehicleLogic.OnSpawn( )
		UseDefaultEndTransition = 0		
		AICheckForCollisions = 1
		
		LinkChildAudioLogics( 1, 0 )

		
		AddCargo( "USSR_Transport_Stop" )
		AddCargo( "USSR_MTank_Paradrop_Stop" )
		AddCargo( "USSR_HTank_Paradrop_Stop" )
		AddCargo( "USSR_IFV_Paradrop_Stop" )				
				
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
		
		GameEffects.PlayEffect( OwnerEntity, "Boss_Persistent_Homer" )
			
		// blow up the box
		//local box = ::GameApp.CurrentLevel.NamedObject( "destroyBox" )
		//if( !is_null( box ) )
		//	box.Logic.ForceDestroy( )		
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
	function SetMotionMap( ) Animatable.MotionMap = USSRBossHomerMoMap( this )

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
		
			SetStateOverride( 0, 0 )
			PreventDeath( )
			::DisplayImpactText( "BossStage2" )
			return true
		}	
		else if( stageNum == 2 )
		{
			stageNum = 3
			
			SetStateOverride( 0, 0 )
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
