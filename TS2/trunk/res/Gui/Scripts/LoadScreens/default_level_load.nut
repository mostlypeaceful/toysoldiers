// Default game loading screen

// Requires
sigimport "gui/scripts/controls/asyncstatus.nut"
sigimport "gui/scripts/loadscreens/personalbest.nut"
sigimport "gui/scripts/loadscreens/protiploadscreen.nut"
sigimport "gui/scripts/loadscreens/campaignloadscreen.nut"
sigimport "gui/scripts/utility/modeutility.nut"

// Resources
sigimport "gui/textures/misc/loading_run_g.png"

sigexport function CanvasCreateLoadScreen( loadScreen )
{
	return DefaultLevelLoadScreen( loadScreen )
}

enum DefaultLevelLoadScreenState
{
	FadingIn,
	FadingOut,
	ShowingIntroText,
}

class LoadingAnimation extends AnimatingCanvas
{
	constructor( )
	{
		::AnimatingCanvas.constructor( )
		
		local image = ::Gui.AnimatedQuad( )
		image.SetTexture( "gui/textures/misc/loading_run_g.png" )
		image.SetRect( ::Math.Vec2.Construct( 64, 64 ) )
		image.Framerate = 30
		image.FrameCount = 24
		image.FrameSize = ::Math.Vec2.Construct( 64, 64 )
		AddChild( image )
		
		CenterPivot( )
	}
}

class DefaultLevelLoadScreen extends Gui.CanvasFrame
{
	loadScreen = null
	state = null
	timer = 0
	finalFadeOutTime = 0
	blackBg = null
	image = null
	loading = null
	introText = null
	personalBest = null
	proTips = null
	levelName = null
	finished = null
	lineHeight = null

	constructor( _loadScreen )
	{
		::Gui.CanvasFrame.constructor( )
		loadScreen = _loadScreen
		local vpRect = ::GameApp.ComputeScreenSafeRect( )
		local levelInfo = ::GameApp.NextLevelLoadInfo
		local mapType = levelInfo.MapType
		finished = true
		lineHeight = 24

		state = DefaultLevelLoadScreenState.FadingIn
		timer = 0.0
		finalFadeOutTime = 4.0

		blackBg = ::Gui.ColoredQuad( )
		blackBg.SetRect( ::Math.Vec2.Construct( 1280, 720 ) )
		blackBg.SetPosition( 0, 0, 0.1 )
		blackBg.SetRgba( 0.0, 0.0, 0.0, 1 )
		AddChild( blackBg )

		local imagePaths = [
			"Gui/Textures/LoadScreens/Default/load0_g.png",
			"Gui/Textures/LoadScreens/Default/load1_g.png",
			"Gui/Textures/LoadScreens/Default/load2_g.png",
			"Gui/Textures/LoadScreens/Default/load3_g.png",
			"Gui/Textures/LoadScreens/Default/load4_g.png",
			"Gui/Textures/LoadScreens/Default/load5_g.png"
		]
		
		local evilEmpireImagePaths = [
			"Gui/Textures/LoadScreens/Default/load6_g.png",
			"Gui/Textures/LoadScreens/Default/load7_g.png"
		]
		
		local napalmImagePaths = [
			"Gui/Textures/LoadScreens/Default/load8_g.png",
		]

		local mpImages = [
			"gui/textures/loadscreens/versus/mp_01_g.png",
			"gui/textures/loadscreens/versus/mp_02_g.png",
			"gui/textures/loadscreens/versus/mp_03_g.png",
			"gui/textures/loadscreens/versus/mp_05_g.png", // These are switched because of the DLC reorder
			"gui/textures/loadscreens/versus/mp_04_g.png",
		]
		
		local levelIndex = levelInfo.LevelIndex

		if( mapType == MAP_TYPE_HEADTOHEAD && levelIndex in mpImages )
		{
			image = ::Gui.AsyncTexturedQuad( )
			image.SetTexture( mpImages[ levelIndex ] )
		}
		else
		{
			if( mapType == MAP_TYPE_CAMPAIGN && !levelInfo.SkipBriefing && ::LevelHasBriefing( levelIndex ) )
			{
				finished = false
				image = ::CampaignLoadScreen( levelIndex, levelInfo.DlcNumber )
				image.onFinished = function( ) { finished = true }.bindenv( this )
			}
			else
			{
				image = ::Gui.AsyncTexturedQuad( )
				
				switch( levelInfo.DlcNumber )
				{
					case DLC_COLD_WAR:
						image.SetTexture( imagePaths[ ::SubjectiveRand.Int( 0, imagePaths.len( ) - 1 ) ] )
					break
					
					case DLC_EVIL_EMPIRE:
						image.SetTexture( evilEmpireImagePaths[ ::SubjectiveRand.Int( 0, evilEmpireImagePaths.len( ) - 1 ) ] )
					break
					
					case DLC_NAPALM:
						image.SetTexture( napalmImagePaths[ ::SubjectiveRand.Int( 0, napalmImagePaths.len( ) - 1 ) ] )
					break
				}
			}
		}
		image.SetPosition( 0, 0, 0.05 )
		AddChild( image )

		loading = ::LoadingAnimation( )
		loading.SetPosition( vpRect.Left + 16, vpRect.Bottom - 16, 0 )
		AddChild( loading )
			
		loading.SetYPos( vpRect.Bottom - lineHeight * 0.5 )
		
		levelName = null

		SetRgba( 1, 1, 1, 0 )
		SetZPos( 0.01 )
		
		// Setup Personal Best stuff
		personalBest = ::PersonalBestController( )
		personalBest.SetPosition( 1000 - 100, 500 - 120 + 4, 0 )
		if( finished )
			AddChild( personalBest )
		
		// Protips
		local screenSafeRect = ::GameApp.FrontEndPlayer.User.ComputeScreenSafeRect( )
		proTips = ::ProTipLoadScreenAnimation( 8.0, screenSafeRect.Bottom - lineHeight )
		proTips.SetPosition( screenSafeRect.Right, screenSafeRect.Bottom - lineHeight, 0 )
		if( finished )
			AddChild( proTips ) 
	}
	
	function CanProceedToNewLevel( )
	{
		return ( finished ? 1 : 0 )
	}
	
	function OnTick( dt )
	{
		switch( state )
		{
		case DefaultLevelLoadScreenState.FadingIn:
			{
				SetAlphaClamp( timer )

				if( timer >= 1.0 )
					loadScreen.SetCanvasIsFadedIn( )

				timer += dt

				if( loadScreen.NewLevelIsLoaded( ) && finished )
				{
					loadScreen.SetCanProceedToNewLevel( )
					state = DefaultLevelLoadScreenState.FadingOut
					timer = 1.0
				}
			}
			break;
		case DefaultLevelLoadScreenState.FadingOut:
			{
				image.SetAlphaClamp( timer )
				loading.SetAlphaClamp( timer )
				personalBest.SetAlphaClamp( timer )
				proTips.SetAlphaClamp( timer )
				if( levelName )
					levelName.SetAlphaClamp( timer )

				if( timer <= 0.0 )
				{
					state = DefaultLevelLoadScreenState.ShowingIntroText
					timer = introText ? finalFadeOutTime : 1.0
				}
				
				if( loadScreen.CanSpawn( ) )
					timer -= dt
			}
			break;
		case DefaultLevelLoadScreenState.ShowingIntroText:
			{
				SetAlphaClamp( timer )

				if( timer <= 0.0 )
					loadScreen.SetCanvasIsFadedOut( )
					
				if( loadScreen.CanSpawn( ) )
					timer -= dt
			}
			break;
		}
		
		if( !levelName && ::GameApp.LevelNameReadyToRead )
		{					
			local info = ::GameApp.CurrentLevelLoadInfo
			local mapName = ::GameApp.GetLocalLevelName( info.MapType, info.LevelIndex )
			
			local vpRect = ::GameApp.ComputeScreenSafeRect( )
		
			local modeName = null
			if( info.MapType == MAP_TYPE_CAMPAIGN )
			{
				local difficultyNames = [
					::GameApp.LocString( "Difficulty_Casual" ),
					::GameApp.LocString( "Difficulty_Normal" ),
					::GameApp.LocString( "Difficulty_Hard" ),
					::GameApp.LocString( "Difficulty_Elite" ),
					::GameApp.LocString( "Difficulty_General" )
				]
				modeName = difficultyNames[ info.Difficulty ]
			}
			else if( info.MapType == MAP_TYPE_SURVIVAL )
			{
				modeName = ::SurvivalModeNames[ info.ChallengeMode ]
			}
			
			levelName = ::Gui.Text( )
			levelName.SetFontById( FONT_SIMPLE_SMALL )
			levelName.SetRgba( COLOR_CLEAN_WHITE )
			local locString = null
			if( modeName )
				locString = ::GameApp.LocString( "Loading_LoadScreen" ).Replace( "levelName", mapName ).Replace( "difficulty", modeName )
			else
				locString = ::GameApp.LocString( "Loading_LoadScreen_NoDiff" ).Replace( "levelName", mapName )
			levelName.BakeLocString( locString, TEXT_ALIGN_LEFT )
			levelName.SetPosition( vpRect.Left + 42, vpRect.Bottom - levelName.LineHeight, 0 )
			AddChild( levelName )
		}
		
		// always clear this, not just when we grab a name
		::GameApp.LevelNameReadyToRead = 0

		::Gui.CanvasFrame.OnTick( dt )
	}
}

