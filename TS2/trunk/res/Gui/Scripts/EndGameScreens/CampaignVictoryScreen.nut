// When a player wins, there is this screen!

// Requires
sigimport "gui/scripts/endgamescreens/baseendgamescreen.nut"
sigimport "gui/scripts/endgamescreens/victoryscorescreen.nut"
sigimport "gui/scripts/endgamescreens/unlockscreen.nut"
sigimport "gui/scripts/endgamescreens/statsscreen.nut"
sigimport "gui/scripts/frontend/twoplayermenucontroller.nut"
sigimport "gui/scripts/endgamescreens/earnedscreen.nut"

sigimport "effects/fx/gui/usa_smoke_flyby.fxml"
sigimport "effects/fx/gui/usa_smoke_flyby_red.fxml"

class CampaignVictoryScreen extends BaseEndGameScreen
{
	// Display
	pages = null
	userDisplay = null
	
	// Data
	currentPage = null
	victoryScoreScreen = null
	victoriousPlayer = null

	constructor( victoriousPlayer_, coopPlayer = null )
	{
		victoriousPlayer = victoriousPlayer_
		local screenRect = victoriousPlayer.User.ComputeScreenSafeRect( )
		::BaseEndGameScreen.constructor( screenRect )
		audioSource = victoriousPlayer_.AudioSource
		ForwardButtons = 0
		
		pages = [ ]
		
		// Create Pages
		victoryScoreScreen = ::VictoryScoreScreen( screenRect, victoriousPlayer, coopPlayer )
		pages.push( victoryScoreScreen )
		victoryScoreScreen.SetAlpha( 0 )
		AddChild( victoryScoreScreen )
		
		local unlockScreen = ::UnlockScreen( screenRect, victoriousPlayer )
		if( unlockScreen.HasUnlocks( ) || victoriousPlayer.BeatLevelTheFirstTime )
		{
			pages.push( unlockScreen )
			unlockScreen.SetAlpha( 0 )
			AddChild( unlockScreen )
		}
		
		local earnedScreen = ::EarnedScreen( screenRect, victoriousPlayer, coopPlayer )
		if( earnedScreen.HasEarnings( ) )
		{
			pages.push( earnedScreen )
			earnedScreen.SetAlpha( 0 )
			AddChild( earnedScreen )
		}
		
		local statsScreen = ::StatsScreen( screenRect, victoriousPlayer, coopPlayer )
		pages.push( statsScreen )
		statsScreen.SetAlpha( 0 )
		AddChild( statsScreen )
		
		SetZPos( 0.2 )
		ChangePage( 0 )
	}
	
	function OnPressB( )
	{
		return false
	}
	
	function OnPressA( )
	{
		return ::BaseEndGameScreen.OnPressA( )
	}
	
	function CheckLoadNextLevelIsValid( )
	{
		if( ::GameAppSession.IsHost )
		{
			// Check if the next level has elite/general locked
			local levelInfo = ::GameApp.CurrentLevelLoadInfo
			local difficulty = levelInfo.Difficulty
			if( difficulty == DIFFICULTY_ELITE || difficulty == DIFFICULTY_GENERAL )
			{
				local nextLevelIndex = ::GameApp.NextCampaignLevel( levelInfo.LevelIndex, levelInfo.DlcNumber )
				local nextLevelScores = victoriousPlayer.GetUserProfile( ).GetLevelScores( levelInfo.MapType, nextLevelIndex )
				if( !(nextLevelScores.GetHighScore( DIFFICULTY_CASUAL ) >= 0) && !(nextLevelScores.GetHighScore( DIFFICULTY_NORMAL ) >= 0) && !(nextLevelScores.GetHighScore( DIFFICULTY_HARD ) >= 0) )
				{
					// Not allowed
					local locId = ( ( difficulty == DIFFICULTY_ELITE ) ? "EndGame_CantPlayNextLevelWarning_Elite" : "EndGame_CantPlayNextLevelWarning_General" )
					local dialog = ::ModalConfirmationBox( locId, victoriousPlayer.User, "EndGame_QuitToLobby", "Cancel" )
					
					dialog.onAPress = function( )
					{
						noInput = true
					}.bindenv( this )
					
					dialog.onFadedOut = function( )
					{
						noInput = true
						::GameAppSession.SetFrontEndLoadBehavior( FRONTEND_LOAD_BEHAVIOR_DESTROY_SESSION )
						::GameApp.LoadFrontEnd( )
					}
					
					return false
				}
			}
		}
		return true
	}
	
	function CheckIfIsLastColdWarLevel( )
	{
		if( ::GameApp.CurrentLevelLoadInfo.DlcNumber == 0 && ::GameApp.NextCampaignLevel( ::GameApp.CurrentLevelLoadInfo.LevelIndex ) < 0 )
			return true

		return false
	}
	
	function HandleInput( gamepad )
	{
		if( noInput )
			return
			
		// If the page handled the input, don't do the rest of this input
		if( ("HandleInput" in pages[ currentPage ]) && (pages[ currentPage ].HandleInput( gamepad )) )
		{
			::print( "page handled input" )
			return
		}
		
		if( !noInput )
		{
			if( ::GameAppSession.IsHost )
			{
				// Let A continue to the next page
				if( gamepad.ButtonDown( GAMEPAD_BUTTON_A ) )
				{
					PlaySound( "Play_UI_Select_Forward" )
					if( IsNotLastPage( ) )
						ChangePage( currentPage + 1 )
					else if( IsLastPage( ) )
					{
						if( (!::GameApp.IsFullVersion && ::GameApp.CurrentLevel.IsTrial) || ::GameApp.PAXDemoMode )
							QuitToMainMenu( )
						else if( CheckIfIsLastColdWarLevel( ) )
						{
							if( ::GameApp.GameMode.IsNet )
							{
								QuitToMainMenu( )
							}
							else
							{
								// Play credits
								PushNextMenu( ::FrontEndCreditsMenu( true ) )
								AutoAdvance = true
							}
						}
						else if( CheckLoadNextLevelIsValid( ) )
							LoadNextLevel( )
					}
				}
				if( gamepad.ButtonDown( GAMEPAD_BUTTON_X ) )
				{
					PlaySound( "Play_UI_Select_Backward" )
					if( IsLastPage( ) )
						RestartCurrentLevel( )
				}
			}
			else
			{
				if( gamepad.ButtonDown( GAMEPAD_BUTTON_A ) )
				{
					if( IsNotLastPage( ) )
					{
						PlaySound( "Play_UI_Select_Forward" )
						ChangePage( currentPage + 1 )
					}
					else if( IsLastPage( ) )
						::BaseEndGameScreen.SelectActiveIcon( )
				}
			}

			// Last page can quit to the main menu
			if( gamepad.ButtonDown( GAMEPAD_BUTTON_B ) )
			{
				PlaySound( "Play_UI_Select_Backward" )
				
				if( IsLastPage( ) )
				{
					PlaySound( "Play_UI_Select_Backward" )
					local dialog = ::ModalConfirmationBox( "Quit_ConfirmDisplayCase", user, "Ok", "Cancel" )
					dialog.onAPress = function( )
					{
						noInput = true
					}.bindenv( this )
					dialog.onFadedOut = function( )
					{
						::BaseEndGameScreen.OnPressB( )
					}.bindenv( this )
				}
			}
			
			if( twoPlayerController )
				twoPlayerController.HandleInput( gamepad )
		}
	}
	
	function IsNotLastPage( )
	{
		return ( currentPage < (pages.len( ) - 1) )
	}
	
	function IsLastPage( )
	{
		return ( currentPage == (pages.len( ) - 1) )
	}
	
	function LoadNextLevel( )
	{
		if( ::GameApp.GameMode.IsNet )
		{
			if( twoPlayerController && twoPlayerController.lobbyController.IsClientReady( ) )
			{
				noInput = true
				CloseEndGameScreen( )
				::GameAppSession.StartNextMap( )
			}
		}
		else
		{
			noInput = true
			CloseEndGameScreen( )
			::GameApp.CurrentLevel.LoadNextLevel( )
		}
	}
	
	function SetControls( )
	{
		controls.Clear( )
		if( currentPage == (pages.len( ) - 1) )
		{
			if( ::GameAppSession.IsHost )
			{
				if( !::GameApp.PAXDemoMode )
					controls.AddControl( GAMEPAD_BUTTON_A, "EndGame_NextLevel" )
				controls.AddControl( GAMEPAD_BUTTON_X, "EndGame_ReplayLevel" )
			}
			else
			{
				if( twoPlayerController && twoPlayerController.user2 )
				{
					if( twoPlayerController.lobbyController.IsClientReady( ) )
						controls.AddControl( GAMEPAD_BUTTON_A, "EndGame_Unready" )
					else
						controls.AddControl( GAMEPAD_BUTTON_A, "EndGame_Ready" )
				}
			}
			
			controls.AddControl( GAMEPAD_BUTTON_B, "EndGame_Quit" )
		}
		else
		{
			controls.AddControl( "gui/textures/gamepad/button_a_g.png", "EndGame_Continue" )
		}
		
		local currentPageObj = pages[ currentPage ]
		if( "hasSubPages" in currentPageObj && currentPageObj.hasSubPages )
			controls.AddControl( [ "gui/textures/gamepad/button_lshoulder_g.png", "gui/textures/gamepad/button_rshoulder_g.png" ], "LB_Pages" )
		
		if( twoPlayerController && twoPlayerController.CanViewGamercard( ) )
			controls.AddControl( GAMEPAD_BUTTON_SELECT, "LB_Menus_Gamer_Card" )
	}
	
	function ChangePage( pageIndex )
	{
		if( pageIndex == 1 )
		{
			victoryScoreScreen.FadeOutMedalEffects( )
			
			local screenRect = victoriousPlayer.User.ComputeScreenSafeRect( )
			local screenWidth = screenRect.Right
			local screenHeight = screenRect.Bottom
			
			local whiteandblue = ::Gui.ScreenSpaceFxSystem( )
			whiteandblue.SetSystem( "effects/fx/gui/usa_smoke_flyby.fxml", 1, true )	//path, playcount(-1=loop), localSystem
			whiteandblue.SetPosition( screenWidth / 2, screenHeight / 2, 0.725 )
			whiteandblue.SetDelay( 0.0 )
			AddChild( whiteandblue )
			
			local justred = ::Gui.ScreenSpaceFxSystem( )
			justred.SetSystem( "effects/fx/gui/usa_smoke_flyby_red.fxml", 1, true )	//path, playcount(-1=loop), localSystem
			justred.SetPosition( screenWidth / 2, screenHeight / 2, 0.775 )
			justred.SetDelay( 0.0 )
			AddChild( justred )
		}
		
		if( pageIndex != currentPage )
		{
			// Change Page
			if( currentPage in pages )
				pages[ currentPage ].FadeOutAnd( 0.5, function( canvas )
				 {
					 if( "OnDelete" in canvas )
						canvas.OnDelete( )
				 })
			pages[ pageIndex ].FadeIn( 0.5 )
			currentPage = pageIndex
			
			// Set Up Controls
			SetControls( )
		}
	}
}
