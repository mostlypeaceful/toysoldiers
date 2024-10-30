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

sigexport function EntityOnCreate( entity )
{
	RegisterLevelLogic( entity, LevelLogic_Campaign_lvl09( ) )
}

class LevelLogic_Campaign_lvl09 extends GameStandardLevelLogic
{
	wave_01 = null
	
	function OnSpawn( )
	{
		::GameStandardLevelLogic.OnSpawn( )
		onLevelIntroComplete = EndOfCamera
		
		//ardor: torch 30 infantry with napalm.
		//like a fox: do a barrel roll
		//rank: anti-air iii kills
		InitDecoration( 0, 30, function( event ) { return PlayerKilledWithOneOfTheseWeapons( event, UNIT_TYPE_INFANTRY, [ "USA_F14_NAPALM" ] ) }.bindenv( this ) )
		InitDecoration( 1, null, function( event ) { return event.EventID == TUTORIAL_EVENT_SPECIAL_MOVE && event.EventValue != SPECIAL_ANIM_180 }.bindenv( this ) )
		InitChallenge( function( event ) { return PlayerKilledTheseUnitsWithOneOfTheseUnits( event, [ UNIT_TYPE_INFANTRY, UNIT_TYPE_VEHICLE, UNIT_TYPE_AIR ], [ UNIT_ID_TURRET_AA_03 ] ) }.bindenv( this ) )
		
		// set do_TestBoss to 'true' to only run boss test wave
		local do_TestBoss = false		
		if( do_TestBoss )
		{
			AddWaveList( "BossWave" ).Activate( )
			SetUIWaveList( "BossWave" )
			return
		}		
		else
		{
			wave_01 = AddWaveList( "WaveList01" )
			//LevelIntro2P( "nis_intro_campath", "nis_intro_campath_02", [ wave_01 ] )
			//wave_01.Activate( )
		}
		
		local isCoop = ::GameApp.GameMode.IsCoOp
		if( isCoop || ::GameApp.FromSaveGame || ::GameApp.GetDifficulty( ) == DIFFICULTY_GENERAL )
		{
			local starLevel = NamedObject( "IntroStars" )
			starLevel.Delete( )
			RegisterAirSpace( OwnerEntity.FirstChildWithName( "FlyBox" ) )
			LevelIntro2P( "nis_intro_campath", "nis_intro_campath_02", [ wave_01 ], true, false )
		}
		else
		{
			LevelIntro2P( "nis_intro_campath", "nis_intro_campath_02", null, true, false )
			//GameApp.AddTimedCallback( 6, PutPlayerInPlane.bindenv( this ) )
		}
	}

	function SpawnPreIntroWaves( )
	{
//		wave_02.Activate( )
	}

	function EndOfCamera( )
	{
		local isCoop = ::GameApp.GameMode.IsCoOp
		if( isCoop || ::GameApp.FromSaveGame || ::GameApp.GetDifficulty( ) == DIFFICULTY_GENERAL )
		{
			
		}
		else
		{
			PutPlayerInPlane( ) 
		}
	}
	
	function PutPlayerInPlane( )
	{
		local atv = ControllingPlayer( ).LockInUnit( "playerPlane" )
		if( !is_null( atv ) )
		{
			atv.RegisterForLevelEvent( LEVEL_EVENT_UNIT_DESTROYED, PlaneDeath.bindenv( this ) )
		}
		GameApp.AddTimedCallback( 45, StartFirstWave.bindenv( this ) )
		//::PopupMessage( "Suez_MinigameInstructions", ::Math.Vec4.Construct( 1, 1, 1, 1 ), FONT_FANCY_MED, null, ::Math.Vec3.Construct( 0, -100, 0 ) )
	}
	
	function StartFirstWave( )
	{
		wave_01.Activate( )
	}
	
	function PlaneDeath( logic )
	{
		print( "plane died" )
		GameApp.CurrentLevel.RegisterAirSpace( OwnerEntity.FirstChildWithName( "FlyBox" ) )
		CommonMiniGameReset( )
		//wave_01.Activate( )
	}
	
	function ConfigureTargetMiniGame( )
	{		
		miniGameDuration = null
		miniGameLeaderBoard = GameAppSession.CurrentMiniGameLeaderBoard( )
		miniGameBeginLocKey = "beginMiniGame6"
		miniGameKillFunction = TargetMiniGameKillFunction
		miniGameStartStopFunction = TargetMiniStartStopFunction
		CommonMiniGameSetup( )
	}
	
	function TargetMiniGameKillFunction( context )
	{
		if( context.EventUnitType == UNIT_TYPE_PICKUP )
		{
			local player = ControllingPlayer( )
			player.IncrementStarShatter( player.LastKillValue, context.Entity )
			IncRankProgress( player, 1 )
			return player.LastKillValue
		}

		return 0
	}
	
	function TargetMiniStartStopFunction( start, preStart )
	{
		CommonMiniGameStartStop( start, preStart, [ ], 0 )
		
		if( start )
		{		
			if( preStart )
			{
				ResetHallway( )
			}
		}
		else
		{
			segments[ 0 ].Delete( )
			segments[ 0 ] = null
			targets = [ ]
			firstHall = OwnerEntity.SpawnChild( segmentPath )
		}
		
		CommonMiniGameStartStop( start, preStart, [ ], 1 )
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
