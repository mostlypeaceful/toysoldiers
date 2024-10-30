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
	RegisterLevelLogic( entity, LevelLogic_Challenge_lvl10( ) )
}

class LevelLogic_Challenge_lvl10 extends GameStandardLevelLogic
{
	targets = [ ]
	segments = [ ]
	segmentPath = null
	segmentLength = null
	segmentZero = null
	firstHall = null
	
	constructor( )
	{
		GameStandardLevelLogic.constructor( )
		targets = [ ]
		segments = [ ]
		segmentPath = "nopathfound"
		segmentLength = null
		segmentZero = null
		firstHall = null
	}		
	
	function OnSpawn( )
	{		
		GoalDriven.MasterGoal = ShootingGalleryMasterGoal( this, { } )
		ConfigureTargetMiniGame( )
		
		::GameStandardLevelLogic.OnSpawn( )
			
		firstHall = SceneRefEntity.Convert( NamedObject( "gallery" ) )
		segmentPath = firstHall.ResourcePath
		segmentLength = firstHall.CombinedWorldSpaceBox.ComputeDiagonal.z - 2000 //todo tweak this to make the seam look nice
		segmentZero = firstHall.GetPosition( ).z
		
		
		if( false ) // set this true to kill all the obstructions, for debugging
		{
			for( local i = 0; i < targets.len( ); ++i )
				targets[ i ].Delete( )
		}
		
		//LevelIntro2P( "nis_intro_campath", "nis_intro_campath", null, null )
		LevelIntroComplete( )
		
		ShowEnemiesAliveList( false )
		ShowWaveListUI( false )
		UseMinigameScoring = 1
		TutHideKillHoverText = 1
		
		::GameApp.ForEachPlayer( function( player, data )
		{
			player.LockInUnit( "turret" )
			CommonMiniGamePlayerSetup( player )
		}.bindenv( this ) , null )
	}
	
	function LevelIntroComplete( )
	{
		HandleTutorialEvent( TUTORIAL_EVENT_BEGIN )
		SetCurrentLevelProgression( LEVEL_PROGRESSION_INTRO )
	}		

	function SpawnPreIntroWaves( )
	{
		//wave_01.Activate( )
		
	}
	
	function AddTarget( ent )
	{
		targets.push( ent )
	}
	
	function GetSegmentZero( )
		return segmentZero
		
	function GetSegmentLength( )
		return segmentLength
	
	function GetTargetCount( )
	{
		return targets.len( )
	}
	
	function GetTarget( index )
	{
		return targets[ index ]
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
	
	function ResetHallway( )
	{
		for( local i = 0; i < segments.len( ); ++i )
		{
			if( segments[ i ] )
				segments[ i ].Delete( )
		}
		
		segments = [ ]	
		segments.push( firstHall )
		firstHall = null	
	}
	
	function SpawnSegment( )
	{	
		local ith = segments.len( )
		local newSeg = OwnerEntity.SpawnChild( segmentPath )
		local pos = segmentLength * ith
		newSeg.SetPosition( Math.Vec3.Construct( 0, 0, pos ) )
		
		local deleteIth = ith - 2
		if( deleteIth >= 1 )
		{
			segments[ deleteIth ].Delete( )
			segments[ deleteIth ] = null
		}
		
		segments.push( newSeg )
		
		targets = [ ] //the rocket will have already read the targets, no need to keep accumulating them
	}
	
	//function NewDirection( ent )
	//{
	//	local event = LogicEvent.Construct( GAME_EVENT_REAPPLY_MOTION_STATE )
	//	ent.Logic.HandleLogicEvent( event )
	//}
	
	
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
		//PostEffects.SetDepthOfField( Math.Vec4.Construct( 0.994, 0.465, 3.9, 0.5 ) ) // x, y, z, w
		PostEffects.SetDepthOfField( Math.Vec4.Construct( 0, 0, 0, 0 ) ) // x, y, z, w
	}
}
