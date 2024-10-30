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
@[Post_Exposure] { "PostFx Exposure", (0.93, 0.93, 0.93), [ 0.0:4.0 ], "" }

sigimport "gui/Scripts/frontend/displaycase.nut"
sigimport "Levels/Scripts/Common/game_standard.nut"
sigimport "gameplay/characters/passenger/usa/passenger_02.sigml"
sigimport "gameplay/characters/passenger/ussr/passenger_01.sigml"

sigexport function EntityOnCreate( entity )
{
	RegisterLevelLogic( entity, LevelLogic_ShowCase( ) )
}

class LevelLogic_ShowCase extends GameStandardLevelLogic
{
	infantryWave = null
	carWave = null
	airWave = null
	
	allWaves = [ ]
	
	screenStack = null
	
	constructor( )
	{
		GameStandardLevelLogic.constructor( )
		gDefaultBlueTeamPassengerSigml = "gameplay/characters/passenger/usa/passenger_02.sigml"
		gDefaultRedTeamPassengerSigml = "gameplay/characters/passenger/ussr/passenger_01.sigml"
		
		TutDisableSell = 1
		TutDisableUpgrade = 1
		TutDisableRandomPickups = 1
		TutDisableBarrage = 1
		TutHideKillHoverText = 1
		TutDisableLaunchArrows = 1
		TutDisablePlaceMenu = 1
		TutDisablePropMoney = 1
		TutDisableQuickSwitch = 1
		IsDisplayCase = 1
		TutDisableBonusHoverText = 1
		TutDisableComboText = 1
		
		infantryWave = null
		carWave = null
		airWave = null
		
		allWaves = [ ]
	}	
	
	function OnSpawn( )
	{
		infantryWave = AddWaveList( "WaveList01" )		
		infantryWave.SetLooping( true )
		carWave = AddWaveList( "WaveList02" )		
		carWave.SetLooping( true )
		airWave = AddWaveList( "WaveList03" )		
		airWave.SetLooping( true )

		allWaves.push( infantryWave )
		allWaves.push( carWave )
		allWaves.push( airWave )
		
		::GameStandardLevelLogic.OnSpawn( )
		
		local user = ControllingPlayer( ).User
		local screen = ::FrontEndDisplayCase( ControllingPlayer( ) )
		screenStack = ::VerticalMenuStack( user, false )
		screenStack.minStackCount = 0
		MoveToGuiScreen( screenStack )
		screenStack.SetUser( user )		
		screenStack.PushMenu( screen )
		
		
		SpawnShowcaseTurrets( )
		
		ShowEnemiesAliveList( false )
		ShowWaveListUI( false )
		ControllingPlayer( ).ShowScoreUI( false )
		ControllingPlayer( ).ShowMiniMap( false )
		ControllingPlayer( ).ShowOverchargeMeter( false )
	}
	
	function StartInfantryWave( )
		StartWaveList( infantryWave )
	
	function StartCarWave( )
		StartWaveList( carWave )
	
	function StartAirWave( )
		StartWaveList( airWave )
	
	function StartWaveList( wave )
	{
		StopWaves( )		
		wave.Activate( )
		wave.FinishReadying( )
	}
	
	function StopWaves( )
	{
		for( local i = 0; i < allWaves.len( ); ++i )
		{
			allWaves[ i ].KillEveryone( )
			allWaves[ i ].Pause( )
			allWaves[ i ].Reset( )
		}
	}
	
	function ProcessTutorialEvent( event )
	{			
		if( event.EventID == TUTORIAL_EVENT_USE_UNIT )
		{
			if( event.EventValue == 1 )
			{
				local id = event.CurrentUnitID
				
				if( Player.IsAntiAir( id ) )
					GameApp.CurrentLevel.StartAirWave( )
				else if( Player.IsAntiTank( id ) )
					GameApp.CurrentLevel.StartCarWave( )
				else
					GameApp.CurrentLevel.StartInfantryWave( )
			}
			else
				GameApp.CurrentLevel.StopWaves( )
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
