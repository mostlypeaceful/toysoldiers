sigvars Atmosphere
@[ParticleColor] { "Particle Color", (0.9, 0.85, 0.7, 0.8), [ 0.0:2.0 ], "RGBA Particle Tint" }
@[Fog_MinDist] { "Fog Min Dist", 75.0, [ 0:1000 ], "Fog starts at this distance (from the camera)" }
@[Fog_FadeDist] { "Fog Fade Dist", 300.0, [ 0:1000 ], "Fog starts at this distance (from the camera)" }
@[Fog_Clamp] { "Fog Min Dist", (0.0, 0.175), [ 0.0:1.0 ], "Fog clamp (min, max)" }
@[Fog_Color] { "Fog Color", (0.5, 0.6, 0.7), [ 0.0:1.0 ], "RGB Fog Tint" }
@[Shadows_Amount] { "Shadows Amount", 0.6, [ 0:1000 ], "" }
@[Shadows_Dist] { "Shadows Dist", 500.0, [ 0:1000 ], "" }
@[Shadows_Near] { "Shadows Near", 0.0, [ 0:1000 ], "" }
@[Shadows_Far] { "Shadows Far", 1000.0, [ 0:2000 ], "" }
@[Shadows_Width] { "Shadows Width", 600.0, [ 0:1000 ], "" }
@[Shadows_Height] { "Shadows Height", 600.0, [ 0:1000 ], "" }
@[Post_Saturation] { "PostFx Saturation", (1.0, 1.0, 1.0), [ 0.0:4.0 ], "" }
@[Post_Contrast] { "PostFx Contrast", (1.0, 1.0, 1.0), [ 0.0:4.0 ], "" }
@[Post_Exposure] { "PostFx Exposure", (1.0, 1.0, 1.0), [ 0.0:4.0 ], "" }

sigimport "Levels/Scripts/Common/game_standard.nut"
sigimport "Gui/Textures/WaveIcons/USSR/vehicle_helitransport_g.png"

sigimport "gameplay/characters/infantry/usa/infantry_basic_01.sigml"
sigimport "gameplay/characters/captains/usa/captain_01.sigml"

sigimport "gameplay/mobile/vehicle/ussr/pechora2mrescue.sigml"
sigimport "gameplay/mobile/helicopter/usa/helo_transport_choplifter_01.sigml"
sigimport "gameplay/turrets/antiair/ussr/turret_aa_01.sigml"
sigimport "gameplay/turrets/antiair/ussr/turret_aa_02.sigml"
sigimport "gameplay/turrets/antiair/ussr/turret_aa_03.sigml"

sigexport function EntityOnCreate( entity )
{
	RegisterLevelLogic( entity, LevelLogic_Challenge_lvl10_helicopterrescue( ) )
}

class LevelLogic_Challenge_lvl10_helicopterrescue extends GameStandardLevelLogic
{
	wave_allies_01 = null
	wave_allies_02 = null
	wave_allies_03 = null
	wave_enemies = null
	
	all_waves = null
	onBoardCount = null
	playerUnit = null
	
	enemyTurrets = null
	allyTurrets = null
	allyTurretTypes = null
	newTurrets = null
	startedOnce = false
	passengerUI = null
	atDropSite = null
	
	unitValuesByLogic = null
	unitValuesByDropOff = null

	constructor( )
	{
		enemyTurrets = [ ]
		allyTurrets = [ ]
		allyTurretTypes = [ ]
		newTurrets = [ ]
		startedOnce = false
		atDropSite = false
		unitValuesByLogic = { }
		unitValuesByDropOff = 0
		
		::GameStandardLevelLogic.constructor( )
	}
	
	function OnSpawn( )
	{
		GoalDriven.MasterGoal = ShootingGalleryMasterGoal( this, { } )
		ConfigureMiniGame( )
		
		::GameStandardLevelLogic.OnSpawn( )
		
		// Initialize wavelist variables
		
		wave_allies_01 = AddWaveList( "WaveListAllies01" )
		wave_allies_02 = AddWaveList( "WaveListAllies02" )
		wave_allies_03 = AddWaveList( "WaveListAllies03" )
		wave_enemies = AddWaveList( "WaveListEnemies" )
		
		all_waves = [ wave_allies_01, wave_allies_02, wave_allies_03, wave_enemies ]
		
		ClearCameras( true )
		LevelIntro2P( "nis_intro_campath", "nis_intro_campath", null, null )
		onLevelIntroComplete = EndOfCamera		
		
		playerUnit = null
		
		passengerUI = ::MinigamePassengerUI( ControllingPlayer( ) )
		
		// after on spawn			
		::GameApp.ForEachPlayer( function( player, data )
		{
			CommonMiniGamePlayerSetup( player )
		}.bindenv( this ) , null )
		
		SetOnBoardCount( 0 )
	}
	
	function OnDelete( )
	{
		if( passengerUI )
			passengerUI.DeleteSelf( )
		::GameStandardLevelLogic.OnDelete( )
	}
	
	function SetOnBoardCount( value )
	{
		onBoardCount = value
		//if( passengerUI )
			passengerUI.SetValue( value )
	}

	function SpawnPreIntroWaves( )
	{
	}
	
	function EndOfCamera( )
	{
		HandleTutorialEvent( TUTORIAL_EVENT_BEGIN )
		SetCurrentLevelProgression( LEVEL_PROGRESSION_INTRO )
	}
	
	function PutPlayerInHeli( )
	{	
		ClearCameras( false )
		
		local player = ControllingPlayer( )
		playerUnit = player.LockInUnit( "playerHelo" )
		player.UnlockFromUnit( 0 )

		if( !is_null( playerUnit ) )
			playerUnit.RegisterForLevelEvent( LEVEL_EVENT_UNIT_DESTROYED, VehicleMiniGameDestroyed.bindenv( this ) )
	}
	
	function ClearCameras( tiltShift )
	{
		::GameApp.ForEachPlayer( function( player, data )
		{
			player.ClearCameraStack( )
		}.bindenv( this ) , null )
		
		local player = ControllingPlayer( )
		
		if( tiltShift )
			player.PushTiltShiftCamera( )
		else
			player.PushRTSCamera( )
	}
	
	function VehicleMiniGameDestroyed( logic )
	{
		local player = ControllingPlayer( )
		player.PushTiltShiftCamera( )
		
		HandleTutorialEvent( TUTORIAL_EVENT_MINIGAME_DEFEAT )
		
		playerUnit = null		
	}
	
	function ConfigureMiniGame( )
	{
		miniGameDuration = 60
		miniGameLeaderBoard = GameAppSession.CurrentMiniGameLeaderBoard( )
		miniGameBeginLocKey = "beginMiniGame8"
		miniGameKillFunction = MiniGameKillFunction
		miniGameStartStopFunction = MiniGameStartStopFunction
		TutDisableVehicleInput = 1
		
		CommonMiniGameSetup( )
		
		TutDisableRTSRings = 0 // this has to be after common setup
		TutDisableLaunchArrows = 0 // this has to be after common setup
		TutAllyLaunchArrows = 1
		TutDisablePlaceMenu = 1
	}
	
	function MiniGameKillFunction( context )
	{
		return 0
	}
	

	
	function MiniGameStartStopFunction( start, preStart )
	{
		CommonMiniGameStartStop( start, preStart, all_waves, 0 )
				
		if( start )
		{
			if( preStart )
			{
				PutPlayerInHeli( )				
				StartWaves( )
			}
			else
				TutDisableVehicleInput = 0
		}
		else
		{
			if( playerUnit )
			{
				playerUnit.ForceDestroy( )
				playerUnit = null
			}
			
			TutDisableVehicleInput = 1
			SetOnBoardCount( 0 )
			atDropSite = false
			unitValuesByLogic = { }
		}
		
		CommonMiniGameStartStop( start, preStart, all_waves, 1 )
	}
	
	function StartWaves( )
	{
		startedOnce = true
		
		local waves = all_waves
		
		for( local i = 0; i < waves.len( ); ++i )
		{
			waves[ i ].Activate( )
		}
		
		for( local i = 0; i < allyTurrets.len( ); ++i )
		{
			local pos = allyTurrets[ i ]
			local t = OwnerEntity.SpawnChild( allyTurretTypes[ i ] )
			t.MoveToXForm( pos )
			t.SetEnumValue( ENUM_SPECIAL_LEVEL_OBJECT, SPECIAL_LEVEL_OBJECT_DEFAULT )
			t.SetName( "ally_turret" )
		}
		
		for( local i = 0; i < enemyTurrets.len( ); ++i )
		{
			local pos = enemyTurrets[ i ]
			local t = OwnerEntity.SpawnChild( "gameplay/turrets/antiair/ussr/turret_aa_01.sigml" )
			t.MoveToXForm( pos )
			t.SetEnumValue( ENUM_SPECIAL_LEVEL_OBJECT, SPECIAL_LEVEL_OBJECT_DEFAULT )
			t.SetName( "enemy_turret" )
		}
		
		for( local i = 0; i < newTurrets.len( ); ++i )
		{
			newTurrets[ i ].DeleteImmediate( )
		}
		newTurrets = [ ]
	}
	
	function ProcessTutorialEvent( event )
	{
					
		if( event.EventID == TUTORIAL_EVENT_BEGIN_CHARGING )
		{
			local charger = event.Entity.GetName( )
			if( charger == "site_1" )
				ReleaseWave( wave_allies_01 )
			else if( charger == "site_2" )
				ReleaseWave( wave_allies_02 )
			else if( charger == "site_3" )
				ReleaseWave( wave_allies_03 )
			else if( charger == "base" )
			{
				atDropSite = 1
			}
		}
		else if( event.EventID == TUTORIAL_EVENT_USE_UNIT && event.EventValue == 1 )
		{
			//we just entered vehicle
			atDropSite = 0			
		}
	}
	
	function VehShutdown( logic )
	{
		if( atDropSite )
		{
			logic.ClearCargo( )
			logic.AddSimpleCargo( UNIT_ID_INFANTRY_BASIC_01, onBoardCount, 0.5 )
			logic.DroppingEnabled = 1
			logic.DropCargo( 0 )
			logic.HitPoints += logic.MaxHitPoints * 0.25
			logic.ReapplyChangeState( )
			
			ControllingPlayer( ).SpawnLocText( ::GameApp.LocString( "HealthAdded" ),logic.OwnerEntity.GetPosition( ), Math.Vec4.Construct( 0,1,0,1 ), 1.0 )
			
			unitValuesByDropOff = onBoardCount
		}
	}
	
	function InfantryReachedDeadEnd( logic )
	{
		if( logic.Team == ControllingPlayer( ).Team )
		{
			local pathName = logic.UnitPath.LastStartedPathName
			
			if( pathName == "last_path" )
			{
				// reached safe base
				local mult = 1
				local points = 1000
				
				if( VehicleLogic.PtrToInt( logic ) in unitValuesByLogic )
				{
					mult = unitValuesByLogic[ VehicleLogic.PtrToInt( logic ) ] + 1
					delete unitValuesByLogic[ VehicleLogic.PtrToInt( logic ) ]
				}
				
				ExtraMiniGamePoints( points * mult )	
				logic.OwnerEntity.Delete( )
				
				if( miniGameActive )
					IncRankProgress( ControllingPlayer( ), 1 )
					
				ControllingPlayer( ).AudioEvent( "Play_UI_Score" )
				
				return
			}
			else if( pathName == "tochopper" )
			{
				if( playerUnit && !is_null( playerUnit ) && MeshEntity.CombinedWorldSpaceBox( playerUnit.OwnerEntity ).Contains( logic.OwnerEntity.GetPosition( ) ) )
				{
					// reached the helo
					SetOnBoardCount( onBoardCount + 1 )
					ControllingPlayer( ).SpawnLocText( ::LocString.LocalizeNumber( onBoardCount ), logic.OwnerEntity.GetPosition( ), ::Math.Vec4.Construct( 1,1,1,1 ), 0.5 )	
					logic.OwnerEntity.Delete( )
					ControllingPlayer( ).AudioEvent( "Play_Mini_Choplifter_Embark_DLC" )
					return
				}
			}
			else
			{
				if( playerUnit && !is_null( playerUnit ) )
				{
					// send them to helo:
					logic.UnitPath.StartSimplePointPath( playerUnit.OwnerEntity.GetPosition( ), "tochopper" )
					return //dont delete
				}				
			}
		}
		
		logic.ExplodeIntoParts( )
		logic.OwnerEntity.Delete( )
	}
	
	function MiniGameTurretDestroyed( logic )
	{
		if( logic.OwnerEntity.GetName( ) == "enemy_turret" )
		{
			local upgradeID = logic.UpgradeIDVar
			if( upgradeID == UNIT_ID_NONE )
				upgradeID = logic.UnitID
			
			logic.HitPoints = 1
			logic.DoUpgrade( upgradeID )
		}
		
		return false
	}
	
	function MiniGameCargoDropSpawn( logic, index )
	{
		if( onBoardCount > 0 )
		{
			local seconds = 5
			AddMiniGameTime( seconds )
				
			//print( "spawn: " + index + " guid: " + VehicleLogic.PtrToInt( logic ) )
			unitValuesByLogic[ VehicleLogic.PtrToInt( logic ) ] <- index
			
			SetOnBoardCount( onBoardCount - 1 )
			ControllingPlayer( ).SpawnLocText( ::LocString.LocalizeNumber( onBoardCount ), logic.OwnerEntity.GetPosition( ), ::Math.Vec4.Construct( 1,1,1,1 ), 0.5 )
			
			ControllingPlayer( ).AudioEvent( "Play_Mini_Choplifter_Disembark_DLC" )
		}
	}
	
	function ReleaseWave( wave )
	{
		wave.ReleaseTrenched( )
		wave.Pause( )
		::GameApp.AddPausableTimedCallback( 25.0, function( ):( wave )
		{
			wave.KillEveryone( )
			wave.Reset( )
			wave.Activate( )
		}.bindenv( this ) ) 
	}

	function OnRegisterSpecialLevelObject( specialLevelObject )
	{
		local name = specialLevelObject.GetName( )
		if( name == "ally_turret" )
		{
			newTurrets.push( specialLevelObject )
			if( !startedOnce )
			{
				allyTurrets.push( specialLevelObject.GetMatrix( ) )			
				allyTurretTypes.push( SceneRefEntity.Convert( specialLevelObject ).ResourcePath )
			}
		}
		else if( name == "enemy_turret" )
		{
			newTurrets.push( specialLevelObject )
			
			if( !startedOnce )
				enemyTurrets.push( specialLevelObject.GetMatrix( ) )
		}
		
		GameStandardLevelLogic.OnRegisterSpecialLevelObject( specialLevelObject )
	}
	
	function SetAtmosphere( )
	{
		Gfx.SetFlatParticleColor( Math.Vec4.Construct( @[ParticleColor].x, @[ParticleColor].y, @[ParticleColor].z, @[ParticleColor].w ) )
		Gfx.SetFog(
			Math.Vec4.Construct( @[Fog_MinDist], @[Fog_FadeDist], @[Fog_Clamp].x, @[Fog_Clamp].y ),
			Math.Vec3.Construct( @[Fog_Color].x, @[Fog_Color].y, @[Fog_Color].z ) )
		Gfx.SetShadows( 
			@[Shadows_Amount],
			@[Shadows_Dist],
			@[Shadows_Near],
			@[Shadows_Far],
			@[Shadows_Width],
			@[Shadows_Height] )
		PostEffects.SetSaturation( Math.Vec3.Construct( @[Post_Saturation].x, @[Post_Saturation].y, @[Post_Saturation].z ) ) // r, g, b
		PostEffects.SetContrast( Math.Vec3.Construct( @[Post_Contrast].x, @[Post_Contrast].y, @[Post_Contrast].z ) ) // r, g, b
		PostEffects.SetExposure( Math.Vec3.Construct( @[Post_Exposure].x, @[Post_Exposure].y, @[Post_Exposure].z ) ) // r, g, b
		PostEffects.SetDepthOfField( Math.Vec4.Construct( 0.994, 0.465, 3.9, 0.5 ) ) // x, y, z, w
	}
}







	



	
	


