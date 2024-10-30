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
	RegisterLevelLogic( entity, LevelLogic_Campaign_lvl03( ) )
}

class LevelLogic_Campaign_lvl03 extends GameStandardLevelLogic
{
	wave01 = null
	wave_huey_01 = null
	wave_huey_02 = null
	wave_initial_aa = null
	NVASuppliesKilled = 0
	
	constructor( )
	{
		GameStandardLevelLogic.constructor( )
		gDefaultBlueTeamPassengerSigml = "gameplay/characters/passenger/usa/passenger_02.sigml"
		gDefaultRedTeamPassengerSigml = "gameplay/characters/passenger/vietnam/passenger_01.sigml"
	}

	
	function OnSpawn( )
	{
		::GameStandardLevelLogic.OnSpawn( )

		// Main waves
		wave01 = AddWaveList( "WaveList01" )
		wave01.SetLooping( false )

		// Periodic ally huey wave
		wave_huey_01 = AddWaveList( "WaveListHueyLoop" )
		wave_huey_01.SetLooping( true )

		wave_huey_02 = AddWaveList( "WaveListHueyIntro" )
		wave_huey_02.SetLooping( false )

		// AA APCs spawned during flyin. Subsequent AA spawns take place in main wavelist.
		wave_initial_aa = AddWaveList( "WaveListAA" )
		
		LevelIntro2P( "nis_intro_campath", "nis_intro_campath_02", null, true, false )

		onLevelIntroComplete = EndOfCamera
		
		// Sky's the Limit!: Destroy all of the Soviet Anti-Air turrets.
		// None Shall Pass: Prevent all of the convoy units from reaching the Toy Box.
		// Challenge: Medium Tank Ignites
		InitDecoration( 0, 2, function( event ) { return PlayerKilledParticularUnitWithOneOfTheseUnits( event, [ UNIT_ID_TURRET_AA_01 ], -1 ) }.bindenv( this ) )
		InitDecoration( 1, null, null )
		InitChallenge( function( event ) { return PlayerIgnitedTheseUnitsWithOneOfTheseUnits( event, [ UNIT_TYPE_INFANTRY, UNIT_TYPE_VEHICLE, UNIT_TYPE_AIR ], [ UNIT_ID_TANK_MEDIUM_01 ] ) }.bindenv( this ) )
	}
	
	function PlayerIgnitedTheseUnitsWithOneOfTheseUnits( event, targetTypes, playerUnitIDs )
	{
		if( event.EventID == TUTORIAL_EVENT_UNIT_IGNITED && !is_null( event.Player ) )
		{
			local myUnit = event.PlayerKillerLogic
			
			foreach( targetType in targetTypes )
			{
				if( targetType == -1 || event.Entity.Logic.UnitType == targetType )
				{
					if( !is_null( myUnit ) )
					{
						local myUnitID = myUnit.UnitID
						for( local i = 0; i < playerUnitIDs.len( ); ++i )
						{
							if( myUnitID == playerUnitIDs[ i ] )
								return true
						}
					}
				}
			}
		}
		
		return false
	}
	
	function ProcessTutorialEvent( event )
	{
		// Decoration 2: Prevent all of the convoy units from reaching the Toy Box.
		if( event.EventID == TUTORIAL_EVENT_UNIT_REACHED_GOAL && event.Entity.Logic.UnitID == UNIT_ID_TRUCK_CONVOY_01  )
			FailRationTicket( 1 )
			
		::GameStandardLevelLogic.ProcessTutorialEvent( event )
	}
	
	function OnNoWavesOrEnemiesLeft( )
	{
		// Decoration 2: Prevent all of the convoy units from reaching the Toy Box.
		if( RationTicketActive( 1 ) )
			AwardRationTicket( 1 )
		
		::GameStandardLevelLogic.OnNoWavesOrEnemiesLeft( )
	}

	function SpawnPreIntroWaves( )
	{
		wave01.Activate( )
		wave_huey_02.Activate( )
	}

	function EndOfCamera( )
	{		
		wave_huey_01.Activate( )
	}

	function Convoy()
	{
		PopupMessage("NVAConvoy", Math.Vec4.Construct( 1.0, 1.0, 1.0, 0.7 ), FONT_FANCY_MED, null, Math.Vec3.Construct(0,-50,0))		
	}

	function OnRegisterSpecialLevelObject( specialLevelObject )
	{
		specialLevelObject.Logic.RegisterForLevelEvent( LEVEL_EVENT_UNIT_DESTROYED, OnBeachTurretDestroyed.bindenv( this ) )

		// be sure to always call the base
		::GameStandardLevelLogic.OnRegisterSpecialLevelObject( specialLevelObject )
	}

	function OnBeachTurretDestroyed( logic )
	{
		//search and destroy: destory all three vietcong supply caches.
		if( logic.OwnerEntity.GetName( ) == "NVASupply" )
		{
			NVASuppliesKilled += 1
			if (NVASuppliesKilled == 4)
			{
				RationTicketProgress( 1, 1, 3 )
				//PopupMessage("NVASuppliesDestroyed1", Math.Vec4.Construct( 1.0, 1.0, 1.0, 0.5 ), FONT_FANCY_MED, null, Math.Vec3.Construct(0,100,0))
			}
			if (NVASuppliesKilled == 8)
			{
				RationTicketProgress( 1, 2, 3 )
				//PopupMessage("NVASuppliesDestroyed2", Math.Vec4.Construct( 1.0, 1.0, 1.0, 0.5 ), FONT_FANCY_MED, null, Math.Vec3.Construct(0,100,0))
			}
			if (NVASuppliesKilled == 12)
			{
				RationTicketProgress( 1, 3, 3 )
				//PopupMessage("NVASuppliesDestroyed3", Math.Vec4.Construct( 1.0, 1.0, 1.0, 1.0 ), FONT_FANCY_MED)
			
				if( RationTicketActive( 1 ) )
				{
					//print( "ration ticket 1 GET!" )
					AwardRationTicket( 1 )
				}
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
		PostEffects.SetDepthOfField( Math.Vec4.Construct( 0.994, 0.465, 3.9, 0.5 ) ) // x, y, z, w
	}
}
