// Versus Stats

// Requires
sigimport "gui/scripts/frontend/staticscreens/staticscreen.nut"
sigimport "gui/scripts/endgamescreens/statsdisplay.nut"
sigimport "gui/scripts/endgamescreens/baseendgamescreen.nut"
sigimport "gui/scripts/endgamescreens/statsscreen.nut"

class VersusStatsScreen extends BaseEndGameScreen
{
	// Display
	stats = null
	secondaryControls = null
	
	constructor( )
	{
		local vpSafeRect = ::GameApp.ComputeScreenSafeRect( )
		::BaseEndGameScreen.constructor( vpSafeRect )
		audioSource = ::GameApp.FrontEndPlayer.AudioSource
		
		stats = ::StatsScreen( vpSafeRect, ::GameApp.FrontEndPlayer, ::GameApp.SecondaryPlayer )
		AddChild( stats )
		
		// Controls
		secondaryControls = ::ControllerButtonContainer( FONT_SIMPLE_SMALL )
		secondaryControls.SetPosition( controls.GetXPos( ), controls.GetYPos( ) - 26, 0 )
		AddChild( secondaryControls )
		
		SetControls( )
		
		if( ::GameAppSession.IsQuickMatch )
		{
			BackButtons = 0
			ForwardButtons = 0
			twoPlayerController.display.SetStatus( 1, PROFILEBADGE_BLANK )
			twoPlayerController.display.ProfileBadge( 1 ).EnableControl( false )
		}
	}
	
	function OnPressA( )
	{
		if( !::GameAppSession.IsHost )
			return false
			
		if( !::GameApp.GameMode.IsNet )
		{
			RestartCurrentLevel( )
			return true
		}
		else if( twoPlayerController && twoPlayerController.lobbyController.IsClientReady( ) )
		{
			if( !::GameAppSession.IsQuickMatch && ::GameAppSession.IsHost )
			{
				if( twoPlayerController.user2.IsLocal || twoPlayerController.lobbyController.IsClientReady( ) )
				{
					RestartCurrentLevel( )
					return true
				}
				
				PlaySound( errorSound )
				return false
			}
		}
		else
			return false
	}
	
	function OnPressB( )
	{
		if( ::GameAppSession.IsQuickMatch )
		{
			noInput = true
			PlaySound( "Play_UI_Select_Backward" )
			::ClearFrontEndMultiplayerRestartData( )
			::GameAppSession.SetFrontEndLoadBehavior( FRONTEND_LOAD_BEHAVIOR_DESTROY_SESSION )
			::ResetGlobalDialogBoxSystem( )
			::GameApp.LoadFrontEnd( )
		}
		else if( ::GameAppSession.IsHost && !twoPlayerController )
			::BaseEndGameScreen.OnPressB( )
		else if( ::GameAppSession.IsHost && twoPlayerController && twoPlayerController.lobbyController.IsClientReady( ) )
			::BaseEndGameScreen.OnPressB( )
		
		return false
	}
	
	function HandleInput( gamepad )
	{
		if( noInput )
			return
			
		if( ::GameApp.GameMode.IsNet && gamepad.ButtonDown( GAMEPAD_BUTTON_X ) )
		{
			noInput = true
			::ClearFrontEndMultiplayerRestartData( )
			::GameAppSession.SetFrontEndLoadBehavior( FRONTEND_LOAD_BEHAVIOR_DESTROY_SESSION )
			::ResetGlobalDialogBoxSystem( )
			::GameApp.LoadFrontEnd( )
		}
		
		stats.HandleInput( gamepad )
		
		if( twoPlayerController && twoPlayerController.HandleInput( gamepad, true ) )
			return true
	}
	
	function SetControls( )
	{
		controls.Clear( )
		
		if( !::GameAppSession.IsQuickMatch )
		{
			if( ::GameAppSession.IsHost )
			{
				if( !twoPlayerController || twoPlayerController.user2 )
					controls.AddControl( GAMEPAD_BUTTON_A, "EndGame_VersusRematch" )
				controls.AddControl( GAMEPAD_BUTTON_B, "EndGame_QuitToLobby" )
			}
			else
			{
				if( twoPlayerController && twoPlayerController.user1 )
				{
					if( twoPlayerController.lobbyController.IsClientReady( ) )
						controls.AddControl( GAMEPAD_BUTTON_A, "EndGame_Unready" )
					else
						controls.AddControl( GAMEPAD_BUTTON_A, "EndGame_Ready" )
				}
			}
		}
		
		if( ::GameApp.GameMode.IsNet )
			controls.AddControl( GAMEPAD_BUTTON_X, "Menus_LeaveGameSession" )
		
		secondaryControls.Clear( )
		if( stats.hasSubPages )
			secondaryControls.AddControl( [ "gui/textures/gamepad/button_lshoulder_g.png", "gui/textures/gamepad/button_rshoulder_g.png" ], "LB_Pages" )
		
		if( ::GameAppSession.IsHost && twoPlayerController && twoPlayerController.user2 )
			controls.AddControl( GAMEPAD_BUTTON_SELECT, "LB_Menus_Gamer_Card" )
		else if( twoPlayerController && twoPlayerController.user1 )
			controls.AddControl( GAMEPAD_BUTTON_SELECT, "LB_Menus_Gamer_Card" )
	}
}