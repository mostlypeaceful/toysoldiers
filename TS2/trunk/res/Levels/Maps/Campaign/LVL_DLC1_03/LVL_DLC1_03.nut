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
sigimport "gameplay/characters/passenger/usa/passenger_02.sigml"
sigimport "gameplay/characters/passenger/vietnam/passenger_01.sigml"


sigexport function EntityOnCreate( entity )
{
	RegisterLevelLogic( entity, LevelLogic_Campaign_lvl05( ) )
}

class LevelLogic_Campaign_lvl05 extends GameStandardLevelLogic
{
	wave_01 = null
	wave_boss = null
	wave_02 = null
	wave_03 = null
	wave_usa_lazer_tank = null
	
	constructor( )
	{
		GameStandardLevelLogic.constructor( )
		gDefaultBlueTeamPassengerSigml = "gameplay/characters/passenger/usa/passenger_02.sigml"
		gDefaultRedTeamPassengerSigml = "gameplay/characters/passenger/vietnam/passenger_01.sigml"
	}

	function OnSpawn( )
	{
		::GameStandardLevelLogic.OnSpawn( )
		
		wave_01 = AddWaveList( "WaveList01" )
		
		wave_usa_lazer_tank = AddWaveList( "WaveUSALazerTank" )
		wave_usa_lazer_tank.SetMakeSelectableUnits( 1 )
		wave_usa_lazer_tank.Saveable = 0
		wave_usa_lazer_tank.DisableAIFire = 1

		//wave_boss = AddWaveList( "BossWave" )
				
		// add starting wavelist(s) after cam path

		LevelIntro2P( "nis_intro_campath", "nis_intro_campath", null, true, false )
		
		// -Wicked: Destroy 10 tanks with the Laser Tank.
		// -Weary: Destroy 20 prone soldiers.
		// Challenge: Makeshift II kills
		InitDecoration( 0, 10, function( event ) { return PlayerKilledParticularUnitWithOneOfTheseUnits( event, [ UNIT_ID_TANK_MEDIUM_01, UNIT_ID_TANK_HEAVY_01 ], [ UNIT_ID_TANK_NUKEPROOF_01 ] ) }.bindenv( this ) )
		InitDecoration( 1, 20, function( event ) { return PlayerKillWithTargetWeaponFlag( event, UNIT_TYPE_INFANTRY, -1, KILL_FLAG_PRONE ) }.bindenv( this ) )
		InitChallenge( function( event ) { return PlayerKilledTheseUnitsWithOneOfTheseUnits( event, [ UNIT_TYPE_INFANTRY, UNIT_TYPE_VEHICLE, UNIT_TYPE_AIR ], [ UNIT_ID_TURRET_FLAME_02 ] ) }.bindenv( this ) )
	}

	function SpawnPreIntroWaves( )
	{
		wave_01.Activate( )
	}
	
	function USA_Lazer_Tank( )
	{
		wave_usa_lazer_tank.Activate( )
		PopupMessage( "Level13_objective_complete", ::Math.Vec4.Construct( 1.0, 1.0, 1.0, 1.0 ), FONT_FANCY_MED )
	}
	
	// LEVEL BOSS
	function OnRegisterBoss( boss )
	{
		SetCurrentLevelProgression( LEVEL_PROGRESSION_BOSS )
		
		//print("--> Registering Boss")
		boss.Logic.UnitPath.LoopPaths = 0
		//boss.Logic.TakesDamage = false
		//boss.Logic.RegisterForLevelEvent( LEVEL_EVENT_REACHED_END_OF_PATH, OnBossReachedEndOfPath.bindenv( this ) )
		//boss.Logic.RegisterForLevelEvent( LEVEL_EVENT_BOSS_STAGE_CHANGED, OnBossStageChanged.bindenv( this ) )
		//boss.Logic.RegisterForLevelEvent( LEVEL_EVENT_UNIT_DESTROYED, OnBossDestroyed.bindenv( this ) )	
	}
	
	function StagePath( stageNum )
	{
		local stage = stageNum
		local newpath = "path_boss_stage_" + stage
		return newpath
	}
	
	function RetreatPath( stageNum )
	{
		return "path_boss_retreat"
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
		PostEffects.SetDepthOfField( Math.Vec4.Construct( 0.994, 0.465, 1.0, 0.5 ) ) // x, y, z, w
	}
}
