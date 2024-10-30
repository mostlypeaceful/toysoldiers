// Level Load Test

// Requires
sigimport "gui/scripts/loadscreens/default_level_load.nut"
sigimport "gui/scripts/controls/verticalmenu.nut"
sigimport "gui/scripts/frontend/settings.nut"
sigimport "gui/scripts/utility/modeutility.nut"

// Resources
sigimport "gui/textures/frontend/lobby_progress_g.png"

class LevelLoadTestMenu extends VerticalMenu
{
	// Display
	levelName = null
	proTips = null
	
	mapTypeChoice = null
	mapChoice = null
	modeChoice = null
	proTipChoice = null
	
	// Data
	levelInfos = null
	currentMapType = null
	currentMap = null
	currentMode = null
	
	// Statics
	static choiceSpacing = 230
	static choiceWidth = 240
	
	constructor( )
	{
		::VerticalMenu.constructor( )
		local vpRect = ::GameApp.ComputeScreenSafeRect( )
		menuPositionOffset = ::Math.Vec3.Construct( vpRect.Left + 20, vpRect.Top + 20, 0 )
		ForwardButtons = 0
		
		// Background
		local blackBg = ::Gui.ColoredQuad( )
		blackBg.SetRect( ::Math.Vec2.Construct( 1280, 720 ) )
		blackBg.SetPosition( 0, 0, 0.06 )
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

		local image = ::Gui.AsyncTexturedQuad( )
		image.SetPosition( 0, 0, 0.05 )
		image.SetTexture( imagePaths[ SubjectiveRand.Int( 0, imagePaths.len( ) - 1 ) ] )
		AddChild( image )

		local loading = ::LoadingAnimation( )
		loading.SetPosition( vpRect.Left + 16, vpRect.Bottom - 16, 0 )
		AddChild( loading )
		
		local bg2 = ::Gui.TexturedQuad( )
		bg2.SetTexture( "gui/textures/frontend/lobby_progress_g.png" )
		bg2.SetPosition( vpRect.Left, vpRect.Top, 0.04 )
		AddChild( bg2 )
		
		// Level Data
		GetLevelInfos( )
		currentMap = levelInfos[ MAP_TYPE_CAMPAIGN ][ 0 ]
		currentMode = DIFFICULTY_CASUAL
		
		levelName = ::Gui.Text( )
		levelName.SetFontById( FONT_SIMPLE_SMALL )
		levelName.SetRgba( COLOR_CLEAN_WHITE )
		levelName.SetPosition( vpRect.Left + 42, vpRect.Bottom - levelName.LineHeight, 0 )
		AddChild( levelName )
		
		loading.SetYPos( vpRect.Bottom - levelName.LineHeight * 0.5 )
		
		SetLoadingText( currentMode, currentMap )
		
		// Protips
		local screenSafeRect = ::GameApp.FrontEndPlayer.User.ComputeScreenSafeRect( )
		proTips = ::ProTipLoadScreenAnimation( 8.0, screenSafeRect.Bottom - levelName.LineHeight, false )
		proTips.SetPosition( screenSafeRect.Right, screenSafeRect.Bottom - levelName.LineHeight, 0 )
		proTips.DoTip( 0 )
		AddChild( proTips )
		
		// Menu choices
		// Map Type
		local mapTypes = [ "Menus_Campaign", "Menus_ViewSurvivalMenu", "Menus_ViewMinigameMenu", "Menus_ViewVersusMenu" ]
		mapTypeChoice = ::SettingItem_Choice( "Debug_MapType", mapTypes, 0, 0, OnMapTypeChange.bindenv( this ), choiceSpacing, choiceWidth )
		
		SetMapChoiceAndModeChoice( MAP_TYPE_CAMPAIGN )
		
		// Protip
		local proTips = [ ]
		for( local i = 0; i < PROTIP_COUNT; ++i )
			proTips.push( i )
		proTipChoice = ::SettingItem_Choice( "Debug_ProTip", proTips, 0, 0, OnProTipChange.bindenv( this ), choiceSpacing, choiceWidth )
	}
	
	function FinalizeIconSetup( )
	{
		AddIcons( )
		HighlightByIndex( 0 )
	}
	
	function OnMapTypeChange( mapType )
	{
		local types = [ MAP_TYPE_CAMPAIGN,
			MAP_TYPE_SURVIVAL,
			MAP_TYPE_MINIGAME,
			MAP_TYPE_HEADTOHEAD
		]
		currentMapType = types[ mapType ]
		SetMapChoiceAndModeChoice( currentMapType )
		AddIcons( )
		SetMap( currentMapType, 0 )
		SetMode( 0 )
	}
	
	function OnProTipChange( index )
	{
		proTips.DoTip( index )
	}
	
	function OnMapChange( index )
	{
		SetMap( currentMapType, index )
	}
	
	function AddIcons( )
	{
		icons.clear( )
		icons = [ ]
		finalized = false
		
		icons.push( mapTypeChoice )
		icons.push( mapChoice )
		if( modeChoice )
			icons.push( modeChoice )
		icons.push( proTipChoice )
		
		foreach( icon in icons )
			icon.SetLooping( true )
		
		local prevHighlight = highlightIndex
		::VerticalMenu.FinalizeIconSetup( )
		HighlightByIndex( prevHighlight )
	}
	
	function SetMapChoiceAndModeChoice( mapType )
	{
		currentMapType = mapType
		
		// Map
		local mapNames
		
		if( mapChoice )
		{
			RemoveChild( mapChoice )
			mapChoice.DeleteSelf
			mapChoice = null
		}
		if( modeChoice )
		{
			RemoveChild( modeChoice )
			modeChoice.DeleteSelf
			modeChoice = null
		}
		
		local mapNames = [ ]
		foreach( map in levelInfos[ mapType] )
			mapNames.push( map.MapDisplayName )
		mapChoice = ::SettingItem_Choice( "Debug_Map", mapNames, 0, 0, OnMapChange.bindenv( this ), choiceSpacing, choiceWidth )
		
		// Mode
		local modeNames = GetModeNames( mapType )
		if( modeNames )
			modeChoice = ::SettingItem_Choice( "Debug_Mode", modeNames, 0, 0, SetMode.bindenv( this ), choiceSpacing, choiceWidth )
	}
	
	function GetLevelInfos( )
	{
		levelInfos = {
			[ MAP_TYPE_CAMPAIGN ] = [ ],
			[ MAP_TYPE_SURVIVAL ] = [ ],
			[ MAP_TYPE_MINIGAME ] = [ ],
			[ MAP_TYPE_HEADTOHEAD ] = [ ],
		}
		
		foreach( mapType, list in levelInfos )
		{
			local numLevels = ::GameApp.NumLevelsInTable( mapType )
			for( local i = 0; i < numLevels; ++i )
			{
				local levelInfo = ::GameApp.GetLevelLoadInfo( mapType, i )
				list.push( levelInfo )
			}
			
			// Sort level info
			list.sort( function( a, b )
			{
				if( a.DlcNumber > b.DlcNumber )
					return 1
				else if( a.DlcNumber < b.DlcNumber )
					return -1
				else
				{
					if( a.LevelIndex > b.LevelIndex )
						return 1
					else if( a.LevelIndex < b.LevelIndex )
						return -1
					else
						return 0
				}
			} )
		}
	}
	
	function SetMode( mode )
	{
		currentMode = mode
		SetLoadingText( currentMode, currentMap )
	}
	
	function SetMap( mapType, index )
	{
		currentMap = levelInfos[ mapType ][ index ]
		SetLoadingText( currentMode, currentMap )
	}
	
	function SetLoadingText( mode, map )
	{
		local modeName = GetModeName( mode, map.MapType )
		
		/*if( modeName )
			levelName.BakeLocString( ::GameApp.LocString( "Loading" ) % ": " % map.MapDisplayName % " (" % modeName % ")" )
		else
			levelName.BakeLocString( ::GameApp.LocString( "Loading" ) % ": " % map.MapDisplayName )*/
		
		local locString = null
		if( modeName )
			locString = ::GameApp.LocString( "Loading_LoadScreen" )
		else
			locString = ::GameApp.LocString( "Loading_LoadScreen_NoDiff" )
			
		locString.Replace( "levelName", map.MapDisplayName )
		
		if( modeName )
			locString.Replace( "difficulty", modeName )
		
		levelName.BakeLocString( locString, TEXT_ALIGN_LEFT )
	}
	
	function GetModeName( mode, mapType )
	{
		local modeName = null
		local names = GetModeNames( mapType )
		if( names )
			modeName = names[ mode ]
		
		return modeName
	}
	
	function GetModeNames( mapType )
	{
		local names = null
		if( mapType == MAP_TYPE_CAMPAIGN )
		{
			names = [
				::GameApp.LocString( "Difficulty_Casual" ),
				::GameApp.LocString( "Difficulty_Normal" ),
				::GameApp.LocString( "Difficulty_Hard" ),
				::GameApp.LocString( "Difficulty_Elite" ),
				::GameApp.LocString( "Difficulty_General" )
			]
		}
		else if( mapType == MAP_TYPE_SURVIVAL )
		{
			names = ::SurvivalModeNames
		}
		
		return names
	}
}