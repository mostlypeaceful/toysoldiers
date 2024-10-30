// Pause Menu

// Requires
sigimport "Gui/Scripts/FrontEnd/common.nut"
sigimport "Gui/Scripts/FrontEnd/levelselect.nut"
sigimport "Gui/Scripts/FrontEnd/leaderboards.nut"
sigimport "Gui/Scripts/FrontEnd/LevelSelect/friendleaderboardpanel.nut"
sigimport "Gui/Scripts/FrontEnd/helpandoptions.nut"
sigimport "Gui/Scripts/EndGameScreens/TrialBuyGameScreen.nut"
sigimport "gui/scripts/pausemenu/rewind.nut"
sigimport "gui/scripts/pausemenu/rationsdump.nut"
sigimport "gui/scripts/pausemenu/pausecontentbase.nut"
sigimport "gui/scripts/pausemenu/minigamepausecontent.nut"
sigimport "gui/scripts/pausemenu/campaignpausecontent.nut"
sigimport "gui/scripts/pausemenu/survivalpausecontent.nut"
sigimport "gui/scripts/pausemenu/versuspausecontent.nut"
sigimport "gui/scripts/utility/modeutility.nut"
sigimport "gui/scripts/frontend/frontendscripts.nut"

// Resources
sigimport "gui/textures/score/score_decoration_g.png"
sigimport "gui/textures/pausemenu/pausemenu_background_g.png"

class PauseMenuStack extends VerticalMenuStack
{
	player = null
	dontPause = null
	
	constructor( player_, dontActuallyPause = false )
	{
		player = player_
		::VerticalMenuStack.constructor( player.User )
		
		local menuToPush = null		
		if( ::GameApp.AskPlayerToBuyGame )
		{
			menuToPush = ::TrialBuyGameScreen( )
			::GameApp.AskPlayerToBuyGame = 0
		}
		else
			menuToPush = ::PauseMenu( player_ )

		// If it's our net buddy, hide this menu
		if( !player.User.IsLocal )
			Invisible = true
		
		// If it's net, most likely it won't actually pause the game
		dontPause = dontActuallyPause
		if( !dontPause )
			::GameApp.Pause( true, player.AudioSource )
			
		minStackCount = 0

		PushMenu( menuToPush )
	}
	
	function HandleCanvasEvent( event )
	{
		if( "HandleCanvasEvent" in CurrentMenu( ) )
			return CurrentMenu( ).HandleCanvasEvent( event )
		return false
	}
	
	function OnTick( dt )
	{
		if( IsEmpty( ) )
		{
			if( !dontPause )
				::GameApp.Pause( false, player.AudioSource )
			DeleteSelf( )
			if( user.IsLocal )
				::GameApp.HudRoot.Invisible = false
		}
		else if( !dontPause && !::GameApp.Paused( ) && !user.RawGamepad( ).ButtonHeld( GAMEPAD_BUTTON_SELECT ) )
		{ 
			// The game was unpaused via debugging methods, remove the pause menus.
			while( menus.len( ) > 0 )
				PopMenu( )
		}
		else if( menus.len( ) == 1 )
		{
			local pad = filteredGamepad.Get( )
			if( pad.ButtonDown( GAMEPAD_BUTTON_START ) && !pad.ButtonHeld( GAMEPAD_BUTTON_SELECT ) )
			{
				PopMenu( )
			}
		}
		
		::VerticalMenuStack.OnTick( dt )
	}
}

class TiltShiftCameraMenu extends FrontEndMenuBase
{	
	player = null
	
	constructor( player_ )
	{
		player = player_
		
		::FrontEndMenuBase.constructor( null )		
		SetMenuNameText( "Menus_TiltShiftCam" )
		
		::GameApp.ForEachPlayer( PushPop.bindenv( this ), true )
	}
	
	function SetControls( )
	{
		controls.Clear( )
		controls.AddControl( GAMEPAD_BUTTON_B, "Menus_Back" )
	}
	
	function OnBackOut( )
	{
		::GameApp.ForEachPlayer( PushPop.bindenv( this ), false )
		return ::FrontEndMenuBase.OnBackOut( )
	}
	
	function PushPop( player, pushPop )
	{
		if( pushPop )
			player.PushTiltShiftCamera( )
		else
			player.PopTiltShiftCamera( )
	}
}

class PauseMenu extends FrontEndMenuBase
{
	// Data
	player = null
	noInput = null
	
	// Display
	content = null
	subMenuName = null
	
	constructor( player_ )
	{
		player = player_
		::FrontEndMenuBase.constructor( null, player.User )
		local vpRect = ::GameApp.FrontEndPlayer.User.ComputeScreenSafeRect( )
		menuPositionOffset = ::Math.Vec3.Construct( vpRect.Left + 20, vpRect.Center.y - 150, 0 )
		noInput = false
		
		local fade = ::Gui.ColoredQuad( )
		fade.SetRgba( 0.0, 0.0, 0.0, 0.5 )
		fade.SetRect( ::Math.Vec2.Construct( 1280, 720 ) )
		fade.SetPosition( 0, 0, 0.02 )
		AddChild( fade )

		local levelInfo = ::GameApp.CurrentLevelLoadInfo
		local mapType = levelInfo.MapType
		
		subMenuName = Gui.Text( )
		subMenuName.SetFontById( FONT_SIMPLE_SMALL )
		subMenuName.SetRgba( COLOR_CLEAN_WHITE )
		subMenuName.SetPosition( vpRect.Right, menuName.GetYPos( ) + menuName.Height, 0 )
		AddChild( subMenuName )
		
		if( mapType == MAP_TYPE_CAMPAIGN )
			subMenuName.BakeLocString( ::GetDifficultyName( levelInfo.Difficulty ), TEXT_ALIGN_RIGHT )
		else if( mapType == MAP_TYPE_SURVIVAL )
			subMenuName.BakeLocString( ::GetSurvivalModeName( levelInfo.ChallengeMode ), TEXT_ALIGN_RIGHT )
		
		player.Stats.ClearLevelData( )
		
		local contentClasses = {
			[ MAP_TYPE_MINIGAME ] = ::MinigamePauseContent,
			[ MAP_TYPE_CAMPAIGN ] = ::CampaignPauseContent,
			[ MAP_TYPE_SURVIVAL ] = ::SurvivalPauseContent,
			[ MAP_TYPE_HEADTOHEAD ] = ::VersusPauseContent,
		}
		
		if( mapType in contentClasses )
			content = contentClasses[ mapType ]( player )

		if( content )
		{
			content.SetPosition( menuPositionOffset )
			AddChild( content )
		}
	}
	
	function FinalizeIconSetup( )
	{
		SetMenuNameText( ::GameApp.CurrentLevelDisplayName, true )
		subMenuName.SetPosition( menuName.GetXPos( ), menuName.GetYPos( ) + menuName.Height, 0 )
	
		icons.push( ::FrontEndMenuEntry( "Resume", "Resume_HelpText", Resume.bindenv(this) ) )
		
		local mapType = ::GameApp.CurrentLevel.MapType
		if( mapType != MAP_TYPE_MINIGAME )
		{
			if( ::GameApp.GameMode.IsNet )
			{
				if( mapType == MAP_TYPE_CAMPAIGN && ::GameApp.IsUserHost( user ) )
					icons.push( ::FrontEndMenuEntry( "Restart_Battle", "Restart_Battle_HelpText", ReloadCurrentLevel.bindenv(this) ) )
				else if( mapType != MAP_TYPE_CAMPAIGN )
					icons.push( ::FrontEndMenuEntry( "Surrender", "Surrender_HelpText", Surrender.bindenv(this) ) )
			}
			else if( mapType == MAP_TYPE_HEADTOHEAD )
				icons.push( ::FrontEndMenuEntry( "Surrender", "Surrender_HelpText", Surrender.bindenv(this) ) )
			else
				icons.push( ::FrontEndMenuEntry( "Restart_Battle", "Restart_Battle_HelpText", ReloadCurrentLevel.bindenv(this) ) )
		}

		PushRewindMenuIcon( )

		if( ::GameApp.GameMode.IsSinglePlayerOrCoop && !::GameApp.GameMode.IsNet ) // not allowed in net play
		{
			if( ::GameApp.IsFullVersion && !::GameApp.PAXDemoMode )
			{
				// Level Select Menus
				switch( mapType )
				{
					case MAP_TYPE_CAMPAIGN:
					case MAP_TYPE_DEVSINGLEPLAYER:
						icons.push( ::FrontEndMenuEntry( "Menus_CampaignLevelSelect", "Menus_CampaignLevelSelect_HelpText", ShowCampaignSPMenu.bindenv(this) ) )
					break
					
					case MAP_TYPE_MINIGAME:
						icons.push( ::FrontEndMenuEntry( "Menus_CampaignLevelSelect", "Menus_CampaignLevelSelect_HelpText", function( ) { return PushNextMenu( ::MinigameLevelSelectMenu( ) ) }.bindenv(this) ) )
					break
					
					case MAP_TYPE_SURVIVAL:
						icons.push( ::FrontEndMenuEntry( "Menus_CampaignLevelSelect", "Menus_CampaignLevelSelect_HelpText", function( ) { return PushNextMenu( ::SurvivalLevelSelectMenu( ) ) }.bindenv(this) ) )
					break
				}
			}
			
			// Other single player options
			if( mapType == MAP_TYPE_CAMPAIGN || mapType == MAP_TYPE_DEVSINGLEPLAYER )
			{				
				if( !::GameApp.PAXDemoMode )
				{
					local entry = ::FrontEndMenuEntry( "Menus_CampaignRationsDump", "Menus_CampaignRationsDump_HelpText", function( ) { return PushNextMenu( ::RationsDumpScreen( ::GameApp.CurrentLevel.DlcNumber ) ) }.bindenv(this) )
					icons.push( entry )
					if( ::GameApp.Language == LANGUAGE_SPANISH || ::GameApp.Language == LANGUAGE_FRENCH )
						entry.SetActiveInactiveScale( ::Math.Vec2.Construct( 0.6, 0.6 ), ::Math.Vec2.Construct( 0.7, 0.9 ) )
				}
			}
		}

		if( !player.DisableTiltShift && ::GameApp.CurrentLevel.MapType != MAP_TYPE_MINIGAME && !::GameApp.GameMode.IsNet )
		{
			local entry = ::FrontEndMenuEntry( "Menus_TiltShiftCam", "Menus_TiltShiftCam_HelpText", function( ) { return PushNextMenu( ::TiltShiftCameraMenu( player ) ) }.bindenv(this) )
			icons.push( entry )
			if( ::GameApp.Language == LANGUAGE_SPANISH || ::GameApp.Language == LANGUAGE_ITALIAN )
				entry.SetActiveInactiveScale( ::Math.Vec2.Construct( 0.6, 0.6 ), ::Math.Vec2.Construct( 0.8, 0.9 ) )
		}
		
		if( !::GameApp.PAXDemoMode )
		{
			icons.push( ::FrontEndMenuEntry( "Menus_ViewHelpAndOptions", "Menus_ViewHelpAndOptions_HelpText", function( ) { return PushNextMenu( ::FrontEndHelpAndOptionsMenu( null ) ) }.bindenv(this) ) )
			icons.push( ::FrontEndMenuEntry( "Menus_ViewLeaderboards", "Menus_ViewLeaderboardsInGame_HelpText", ShowLevelLeaderboardsMenu.bindenv(this) ) )
			icons.push( ::FrontEndMenuEntry( "Menus_ViewAchievements", "Menus_ViewAchievements_HelpText", function( ) { if( user.IsLocal ) user.ShowAchievementsUI( ); return false }.bindenv(this) ) )
		}
		
		icons.push( ::FrontEndMenuEntry( "Menus_ExitGame", "Menus_ExitGameInSession_HelpText", ExitGame.bindenv(this) ) )
		
		::VerticalMenu.FinalizeIconSetup( )
	}
	
	function HandleCanvasEvent( event )
	{
		switch( event.Id )
		{
			case ON_UPGRADE_TO_FULL_VERSION:
				RefinalizeIconSetup( )
				break;
		}
		
		return ::FrontEndMenuBase.HandleCanvasEvent( event )
	}
	
	function SelectActiveIcon( )
	{
		if( noInput )
			return false
		else
			return ::FrontEndMenuBase.SelectActiveIcon( )
	}
	
	function Resume( )
	{
		noInput = true
		nextAction = VerticalMenuAction.PopMenu
		return true
	}
	
	function ReloadCurrentLevel( )
	{
		local dialog = ::ModalConfirmationBox( "Restart_Confirm", player.User, "Ok", "Cancel" )
		dialog.onFadedOut = ActuallyRestart.bindenv( this )
		//dialog.SetZPos( -0.1 )
		//AddChild( dialog )
		return false
	}
	
	function ActuallyRestart( )
	{
		if( ::GameApp.GameMode.IsNet )
		{
			if( ::GameAppSession.IsHost )
			{
				noInput = true
				AutoExit = true
				::GameAppSession.RestartMap( )
			}
		}
		else
		{
			noInput = true
			AutoExit = true
			::GameApp.ReloadCurrentLevel( )
		}
	}
	
	function Surrender( )
	{
		if( ::GameApp.CurrentLevel.MapType == MAP_TYPE_MINIGAME || !player )
			return false
		
		local dialog = ::ModalConfirmationBox( "Surrender_Confirm", player.User, "Ok", "Cancel" )
		dialog.onFadedOut = function( )
		{
			noInput = true
			
			::GameApp.CurrentLevel.DefeatedPlayer = player
			::GameApp.CurrentLevel.DelayedGameEnd( 2.0 )			
			if( ::GameAppSession.IsQuickMatch )
				::GameAppSession.SetFrontEndLoadBehavior( FRONTEND_LOAD_BEHAVIOR_DESTROY_SESSION )
				
			AutoExit = true
		}.bindenv( this )
		return true
	}
	
	function LoadFrontEnd( )
	{
		if( user.IsLocal )
		{
			noInput = true
			::ResetGlobalDialogBoxSystem( )
			::GameApp.LoadFrontEnd( )
			return true
		}
		return false
	}
	
	function ShowCampaignSPMenu( )
	{
		if( ::GameApp.CurrentLevel.ExtraMode )
		{
			nextMenu = ExtraLevelSelectMenu( false )
			nextAction = VerticalMenuAction.PushMenu
		}
		else
		{
			nextMenu = ::CampaignLevelSelectMenu( ::GameApp.CurrentLevel.DlcNumber )
			nextAction = VerticalMenuAction.PushMenu
		}
		return true
	}
	
	function BuyGame( )
	{
		if( !user.SignedInOnline )
			::ModalInfoBox( "Menus_Error08", user )
		else
			user.ShowMarketplaceUI( false )
		return false
	}
	
	function ExitGame( )
	{
		if( !::GameApp.IsFullVersion )
		{
			PushNextMenu( ::TrialBuyGameScreen( ) )
			return true
		}
		else
		{
			local dialog = ::ModalConfirmationBox( "Quit_Confirm", player.User, "Ok", "Cancel" )
			if( ::GameApp.GameMode.IsNet )
			{
				noInput = true
				
				local userWhoPressedTheMagicButton = player.User
				dialog.onAPress = function( ):(userWhoPressedTheMagicButton)
				{	
					::GameApp.CurrentLevel.OnNetPlayerQuitEarly( userWhoPressedTheMagicButton )
					
				}.bindenv( this )
				
				dialog.onBPress = function( )
				{
					noInput = false
					
				}.bindenv( this )
			}
			else
			{
				noInput = true
				
				dialog.onBPress = function( )
				{
					noInput = false
					
				}.bindenv( this )
				
				dialog.onFadedOut = function( )
				{
					LoadFrontEnd( )
				}.bindenv( this )
			}
			return false
		}
	}
	
	function PushRewindMenuIcon( )
	{
		if( !::GameApp.RewindEnabled )
			return
			
		//if it is a net game, only host can rewind ( NOTE: host must be determined by user to prevent a desync )
		if( ::GameApp.GameMode.IsNet && !::GameApp.IsUserHost( user ) )
			return
			
		local rewind = ::FrontEndMenuEntry( "Menus_Rewind", "Menus_Rewind_HelpText", function( ) { return PushNextMenu( ::RewindMenu( ) ) }.bindenv(this) )
		if( rewind != null )
		{
			icons.push( rewind )
			if( ::GameApp.Language == LANGUAGE_SPANISH || ::GameApp.Language == LANGUAGE_FRENCH )
				rewind.SetActiveInactiveScale( ::Math.Vec2.Construct( 0.6, 0.6 ), ::Math.Vec2.Construct( 0.8, 0.9 ) )
		}
	}
	
	function ShowLevelLeaderboardsMenu( )
	{
		local levelInfo = ::GameApp.CurrentLevelLoadInfo
		
		// Create a new leaderboard object
		local levelIndex = levelInfo.LevelIndex
		local mapType = levelInfo.MapType
		local count = 1
		if( mapType == MAP_TYPE_CAMPAIGN ) 
			count = DIFFICULTY_COUNT
		else if( mapType == MAP_TYPE_MINIGAME )
			count = DIFFICULTY_ELITE
		else if( mapType == MAP_TYPE_SURVIVAL )
		{
			count = CHALLENGE_MODE_HARDCORE + 1
			if( player.HasDLC( DLC_EVIL_EMPIRE ) )
				count = CHALLENGE_MODE_TRAUMA + 1
			if( player.HasDLC( DLC_NAPALM ) )
				count = CHALLENGE_MODE_COMMANDO + 1
		}
		
		local initial = 0
		if( mapType == MAP_TYPE_CAMPAIGN )
			initial = levelInfo.Difficulty
		else if( mapType == MAP_TYPE_SURVIVAL )
			initial = levelInfo.ChallengeMode
		
		local boards = [ ]
		for( local mode = 0; mode < count; ++mode )
		{
			local id = ::GameAppSession.GetLevelLeaderboardId( mapType, levelIndex, mode )
			if( id != ~0 )
				boards.push( id )
		}
		
		if( boards.len( ) > 0 )
			return PushNextMenu( ::FrontEndLeaderboardsMenu( boards, levelInfo, initial, this ) )
		else
		{
			::print( "Warning, tried to load a leaderboard menu that has no leaderboards. MapType:" + mapType.tostring( ) + " LevelIndex:" + levelIndex.tostring( ) )
			return false
		}
	}
}
