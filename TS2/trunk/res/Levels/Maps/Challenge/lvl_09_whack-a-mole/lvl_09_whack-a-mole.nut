sigvars Atmosphere
@[ParticleColor] { "Particle Color", (1.0, 1.0, 1.0, 0.9), [ 0.0:2.0 ], "RGBA Particle Tint" }
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

sigimport "Gameplay/Preplaced/Breakables/whac_clock.sigml"
sigimport "Gameplay/Preplaced/Breakables/whac_doll.sigml"
sigimport "Gameplay/Preplaced/Breakables/whac_pig.sigml"

sigexport function EntityOnCreate( entity )
{
	RegisterLevelLogic( entity, LevelLogic_Challenge_lvl09_whackamole( ) )
}

class LevelLogic_Challenge_lvl09_whackamole extends GameStandardLevelLogic
{
	items = [ ]
	spawnPts = [ ]
	
	minigameCombo = null
	
	spawnDelayStart = null
	spawnDelayCurrent = null
	spawnDelayDecay = null
	currentTime = null
	
	playerUnit = null
	
	digits = [ ]
	
	constructor( )
	{
		GameStandardLevelLogic.constructor( )
		
		items = [ ]
		spawnPts = [ ]
		digits = [ ]
	}
	
	function OnSpawn( )
	{		
		GoalDriven.MasterGoal = ShootingGalleryMasterGoal( this, { } )
		ConfigureMiniGame( )
		
		::GameStandardLevelLogic.OnSpawn( )
		
		//turn off un-needed HUD elements
		ShowEnemiesAliveList( false )
		ShowWaveListUI( false )
		ControllingPlayer( ).ShowScoreUI( false )
		
		items.push( "Gameplay/Preplaced/Breakables/whac_pig.sigml" )
		items.push( "Gameplay/Preplaced/Breakables/whac_clock.sigml" )
		items.push( "Gameplay/Preplaced/Breakables/whac_doll.sigml" )
		
		// collect spawn points
		for( local i = 0; i < 15; ++i )
		{
			local pt = OwnerEntity.FirstChildWithName( "spawnPt" + (i+1).tostring( ) )
			if( !is_null( pt ) )
				spawnPts.push( pt )
		}		
				
		digits = [ ]
		for( local i = 0; i < 3; ++i )
		{
			local name = "digit_" + (i).tostring( )
			
			local d = OwnerEntity.FirstChildWithName( name )
			if( !is_null( d ) )
			{
				::StateableEntity.StateMaskEnableRecursive( d, 0 )
				digits.push( d )
			}
		}	
				
		spawnDelayStart = 2.5
		spawnDelayCurrent = spawnDelayStart
		spawnDelayDecay = 0.1
		currentTime = 0.0
		
		//LevelIntro2P( "introCam", "introCam", null )
		//onLevelIntroComplete = LevelIntroComplete
		LevelIntroComplete( )
		
		TutForceWeaponOvercharge = 1 //stop the turret from having to reload
		TutDisableBarrage = 1 //turn off barrages
		TutDisableOvercharge = 1 //turn off overcharge
		TutContinuousCombo = 1
		DisableOverkills = 1
		UseMinigameScoring = 1
		TutHideKillHoverText = 1 // we spawn the text manually
		
		::GameApp.ForEachPlayer( function( player, data )
		{
			playerUnit = player.LockInUnit( "turret" )
			CommonMiniGamePlayerSetup( player )
		}.bindenv( this ) , null )
		
		minigameCombo = ::MinigameComboUI( ControllingPlayer( ) )
		
		TimerCallback( );
	}
	
	function RandomThing( )
	{			
		if( !miniGameActive )
			return
		
		// keep track of time
		currentTime += spawnDelayCurrent
		
		local subList = [ ]
		
		// find empty holes
		for( local i = 0; i < spawnPts.len( ); ++i )
			if( !is_null( spawnPts[ i ] ) && spawnPts[ i ].ChildCount == 0 )
				subList.push( spawnPts[ i ] )
			
		if( subList.len( ) )
		{
			// only spawn something if there's not already a child
			
			local pRandom = ObjectiveRand.Int( 0, subList.len( ) - 1 )
			local spawnPt = subList[ pRandom ]
			local itemId = 0 //defaults to pig
		
			// find out what item we want to spawn.
			local pigChance = 0.75
			if( ObjectiveRand.Float( 0, 1.0 ) >= pigChance )
			{
				//determine clock to babuska ratio
				local ratio = currentTime / 60.0
				if( ratio > 1.0 ) 
					ratio = 1.0
				
				ratio *= 0.95 // preserves a 5% change of a clock
				//print( ratio )
				
				if( ObjectiveRand.Float( 0, 1.0 ) > ratio )
					itemId = 1 //clock
				else
					itemId = 2 //doll
			}
			
			spawnPt.SpawnChild( items[ itemId ] )	
			
			// speed things up
			spawnDelayCurrent = spawnDelayStart - currentTime * spawnDelayDecay
			if( spawnDelayCurrent < 0.5 )
				spawnDelayCurrent = 0.5
		}
		
		::GameApp.AddPausableTimedCallback( spawnDelayCurrent, RandomThing.bindenv( this ) ) 
	}
	
	function TimerCallback( )
	{
		//print( "timer" ) this was to prove that this callback loop will end when the level ends
		::GameApp.AddPausableTimedCallback( 0.25, TimerCallback.bindenv( this ) ) 
		
		local time = ::Math.RoundUp( ::GameApp.CurrentLevel.MiniGameTime )
		
		if( !miniGameActive )
			time = 0
			
		local str = time.tostring( )
		local digCount = 3
		local digIndex = 0
				
		for( local i = 0; i < digCount; ++i )
		{
			local val = 0
			if( digCount - 1 - i < str.len( ) )
			{
				val = str[ digIndex ].tointeger( ) - 48 //48 is ascii for zero
				digIndex++
			}
			
			::StateableEntity.StateMaskEnableRecursive( digits[ i ], val )		
		}	
	}
	
	function OnDelete( )
	{
		if( minigameCombo )
			minigameCombo.DeleteSelf( )
		::GameStandardLevelLogic.OnDelete( )
	}

	function SpawnPreIntroWaves( )
	{
		// waves activated in minigame start stop function
		//wave_01.Activate( )
	}
	
	function LevelIntroComplete( )
	{
		HandleTutorialEvent( TUTORIAL_EVENT_BEGIN )
		SetCurrentLevelProgression( LEVEL_PROGRESSION_INTRO )
	}
	
	function ConfigureMiniGame( )
	{
		miniGameDuration = 30
		miniGameLeaderBoard = GameAppSession.CurrentMiniGameLeaderBoard( )
		miniGameBeginLocKey = "beginMiniGame9"
		miniGameKillFunction = MiniGameKillFunction
		miniGameStartStopFunction = MiniGameStartStopFunction
		CommonMiniGameSetup( )
	}
	
	function MiniGameKillFunction( context )
	{
		return ControllingPlayer( ).LastKillValue
	}
	
	function ResetMap( )
	{
		for( local i = 0; i < spawnPts.len( ); ++i )
			spawnPts[ i ].ClearChildren( )
	}
	
	function MiniGameStartStopFunction( start, preStart )
	{
		CommonMiniGameStartStop( start, preStart, [ ], 0 )
		
		if( start )
		{
			if( !preStart )
			{
				currentTime = 0.0
				spawnDelayCurrent = spawnDelayStart
				RandomThing( ) 
			}
		}
		else
		{
			ResetMap( )
		}
		
		CommonMiniGameStartStop( start, preStart, [ ], 1 )
	}
	
	function ProcessTutorialEvent( event )
	{		
		if( minigameCombo )
			minigameCombo.ProcessTutorialEvent( event )
			
		if( event.EventID == TUTORIAL_EVENT_UNIT_DESTROYED && !is_null( event.Player ) )
		{
			::GameEffects.PlayEffect( playerUnit.OwnerEntity, "killedWhack" )
			
			if( event.EventValue == UNIT_ID_WHAC_PIG )
			{
				//show regular money
				IncRankProgress( event.Player, 1 )
				event.Player.SpawnLocText( LocString.LocalizeNumber( event.Player.LastKillValue ), event.Entity.GetPosition( ), Math.Vec4.Construct( 1,1,1,1 ), 0.5 )
			}
			else if( event.EventValue == UNIT_ID_WHAC_BABUSHKA )
			{
				event.Player.SpawnLocText( ::GameApp.LocString( "mini_time_lost" ), event.Entity.GetPosition( ), Math.Vec4.Construct( 1,0,0,1 ), 1.0 )
			}
			else
			{
				event.Player.SpawnLocText( ::GameApp.LocString( "mini_time_ext" ), event.Entity.GetPosition( ), Math.Vec4.Construct( 0,1,0,1 ), 1.0 )
			}
		}
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
		PostEffects.SetDepthOfField( Math.Vec4.Construct( 0.998, 0.465, 3.9, 0.5 ) ) // x, y, z, w
	}
}