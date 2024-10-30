// Eng game screen for Survival Mode 

// Requires
sigimport "gui/scripts/endgamescreens/baseendgamescreen.nut"
sigimport "gui/scripts/hud/swoopnotification.nut"
sigimport "gui/scripts/endgamescreens/earnedscreen.nut"

class SurvivalEndGameScreen extends BaseEndGameScreen
{
	// Display
	scorePresenter = null
	scoreText = null
	bonusAnim = null
	scoreController = null
	earnedScreen = null
	scoreScreen = null
	
	// Data
	currentScore = null
	totalScore = null
	finished = null
	onFirstPage = null
	
	constructor( player, coopPlayer = null )
	{
		local screenRect = player.User.ComputeScreenSafeRect( )
		::BaseEndGameScreen.constructor( screenRect )
		audioSource = player.AudioSource
		
		ForwardButtons = GAMEPAD_BUTTON_A
		BackButtons = 0
		
		currentScore = 0
		finished = false
		onFirstPage = true
		
		if( twoPlayerController )
			twoPlayerController.display.EnableControls( false )
		
		// First Page
		earnedScreen = ::EarnedScreen( ::GameApp.ComputeScreenSafeRect( ), player, coopPlayer )
		
		// Second Page
		scoreScreen = ::AnimatingCanvas( )
		scoreScreen.SetPosition( screenRect.Center.x, screenRect.Top + 50, 0 )
		scoreScreen.SetAlpha( 0 )
		AddChild( scoreScreen )
		
		// Text
		local text = ::Gui.Text( )
		text.SetFontById( FONT_FANCY_LARGE )
		text.SetRgba( COLOR_CLEAN_WHITE )
		text.BakeLocString( ::GameApp.LocString( "EndGame_Survival" ), TEXT_ALIGN_CENTER )
		text.SetPosition( 0, 0, 0 )
		scoreScreen.AddChild( text )
		
		// Decoration
		local decoration = ::Gui.TexturedQuad( )
		decoration.SetTexture( "gui/textures/score/score_decoration_g.png" )
		decoration.CenterPivot( )
		decoration.SetPosition( 0, text.Height + 5, 0 )
		scoreScreen.AddChild( decoration )
		
		// Score Controller
		scoreController = ::ScoreController( player, coopPlayer, OnFinish.bindenv( this ), 20 )
		scoreController.SetPosition( 0, screenRect.Center.y - screenRect.Top - 50, 0 )
		scoreScreen.AddChild( scoreController )
		
		if( earnedScreen.HasEarnings( ) )
		{
			AddChild( earnedScreen )
		}
		else
		{
			onFirstPage = false
			earnedScreen = null
			scoreScreen.SetAlpha( 1 )
			GoToNextPage( )
		}
		
		SetControls( )
		::AnimatingCanvas.FadeIn( 0.2 )
	}
	
	function GoToNextPage( )
	{
		onFirstPage = false
		SetControls( )
		
		if( earnedScreen )
		{
			earnedScreen.FadeOut( 0.2 )
			scoreScreen.FadeIn( 0.2 )
		}
		
		if( !::GameAppSession.IsHost && twoPlayerController )
			twoPlayerController.display.ProfileBadge( 1 ).EnableControl( true )
		
		// Scores
		scoreController.DoTimeResult( "EndGame_SurvivalTotalTime", "TotalTime", "PointsFromTime", false, false )
		scoreController.DoResult( "EndGame_SurvivalKills", "Kills", "PointsFromKills", true, false, false )
		scoreController.DoResult( "EndGame_SurvivalRoundsSurvived", "Round", "PointsFromRounds", false, false, false )
		scoreController.DoBonuses( )
		scoreController.scoreEndPos = -80
		scoreController.PresentScore( )
	}
	
	function SetControls( )
	{
		controls.Clear( )
		if( onFirstPage )
		{
			controls.AddControl( GAMEPAD_BUTTON_A, "EndGame_Continue" )
		}
		else if( finished )
		{
			if( ::GameAppSession.IsHost )
			{
				controls.AddControl( GAMEPAD_BUTTON_A, "EndGame_RestartLevel" )
				controls.AddControl( GAMEPAD_BUTTON_B, "EndGame_QuitToLobby" )
			}
			else if( twoPlayerController )
			{
				if( twoPlayerController.lobbyController.IsClientReady( ) )
					controls.AddControl( GAMEPAD_BUTTON_A, "EndGame_Unready" )
				else
					controls.AddControl( GAMEPAD_BUTTON_A, "EndGame_Ready" )
			}
			
			if( ::GameApp.GameMode.IsNet )
				controls.AddControl( GAMEPAD_BUTTON_X, "Menus_LeaveGameSession" )
		}
		else
		{
			controls.AddControl( GAMEPAD_BUTTON_A, "Skip" )
		}
	}
	
	function SelectActiveIcon( )
	{
		if( noInput )
			return
			
		if( ::GameAppSession.IsHost )
		{
			return ::BaseEndGameScreen.SelectActiveIcon( )
		}
		else
		{
			if( finished )
				return ::BaseEndGameScreen.SelectActiveIcon( )
			else
				OnPressA( )
		}
		
		return true
	}
	
	function OnPressA( )
	{
		if( onFirstPage )
			GoToNextPage( )
		else if( finished )
		{
			if( ::GameAppSession.IsHost && twoPlayerController )
			{
				if( twoPlayerController.lobbyController.IsClientReady( ) )
				{
					noInput = true
					RestartCurrentLevel( )
				}
				else
					PlaySound( "Play_HUD_WeaponMenu_Error" )
			}
			else if( twoPlayerController )
			{
				// Do nothing
			}
			else
			{
				noInput = true
				RestartCurrentLevel( )
			}
		}
		else
			scoreController.Skip( )
		return true
	}
	
	function OnPressB( )
	{
		if( ::GameAppSession.IsHost )
		{
			if( !twoPlayerController )
				return ::BaseEndGameScreen.OnPressB( )
			else if( twoPlayerController.lobbyController.IsClientReady( ) )
				return ::BaseEndGameScreen.OnPressB( )
		}
		else
		{
			return ::BaseEndGameScreen.OnPressB( )
		}
		
		return false
	}
	
	function HandleInput( gamepad )
	{
		if( finished && gamepad.ButtonDown( GAMEPAD_BUTTON_X ) && ::GameApp.GameMode.IsNet )
		{
			noInput = true
			::ClearFrontEndMultiplayerRestartData( )
			::GameAppSession.SetFrontEndLoadBehavior( FRONTEND_LOAD_BEHAVIOR_DESTROY_SESSION )
			::ResetGlobalDialogBoxSystem( )
			::GameApp.LoadFrontEnd( )
		}
	}
	
	function OnFinish( )
	{
		finished = true
		BackButtons = GAMEPAD_BUTTON_B
		SetControls( )
	}
}
