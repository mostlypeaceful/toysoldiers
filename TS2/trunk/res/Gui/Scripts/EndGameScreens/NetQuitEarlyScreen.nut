
// Requires
sigimport "gui/scripts/endgamescreens/baseendgamescreen.nut"

class NetQuitEarlyScreen extends BaseEndGameScreen
{
	constructor( userWhoQuit )
	{
		local screenRect = ::GameApp.FrontEndPlayer.User.ComputeScreenSafeRect( )
		::BaseEndGameScreen.constructor( screenRect )
		inputHook = true
		::GameApp.Pause( true, ::GameApp.FrontEndPlayer.AudioSource )
		
		//local hostQuit = ( ::GameAppSession.IsHost && userWhoQuit.IsLocal ) || ( !::GameAppSession.IsHost && !userWhoQuit.IsLocal )
		
		local screenCenter = screenRect.Center
		
		// "Who did it" Text
		local text = ::Gui.Text( )
		text.SetFontById( FONT_FANCY_LARGE )
		text.SetRgba( COLOR_CLEAN_WHITE )
		local errMsg = "Menus_NiceHostQuit"
		if( userWhoQuit.IsLocal && !::GameAppSession.IsHost )
			errMsg = "Menus_NiceClientQuit"
		else if( !userWhoQuit.IsLocal && ::GameAppSession.IsHost )
			errMsg = "Menus_NiceClientQuit"
		text.BakeBoxLocString( 700, ::GameApp.LocString( errMsg ), TEXT_ALIGN_CENTER )
		text.SetPosition( screenCenter.x, screenCenter.y + 50, 0 )
		AddChild( text )
		
		local startX = screenCenter.x + -130
		local startY = screenCenter.y + 50 + ( ( text )? -text.LocalRect.Height : 0 ) - 72
		
		// Player Image
		local gamerPicture = ::Gui.GamerPictureQuad( )
		gamerPicture.SetPosition( startX, startY, 0 )
		gamerPicture.SetTexture( ::GameApp.FrontEndPlayer.User, userWhoQuit, false )
		AddChild( gamerPicture )
		
		// Player Text
		local gamerTag = ::Gui.Text( )
		gamerTag.SetFontById( FONT_FANCY_MED )
		gamerTag.SetRgba( COLOR_CLEAN_WHITE )
		gamerTag.BakeLocString( userWhoQuit.GamerTag, TEXT_ALIGN_LEFT )
		gamerTag.SetPosition( startX + 74, startY + 32 - gamerTag.Height * 0.5, 0 )
		AddChild( gamerTag )
		
		SetControls( )
	}
	
	function SetControls( )
	{
		controls.Clear( )
		if( ::GameAppSession.IsHost )
		{
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
		
		controls.AddControl( GAMEPAD_BUTTON_X, "Menus_LeaveGameSession" )
	}
	
	function HandleInput( gamepad )
	{
		if( gamepad.ButtonDown( GAMEPAD_BUTTON_X ) )
		{
			noInput = true
			::ClearFrontEndMultiplayerRestartData( )
			::GameAppSession.SetFrontEndLoadBehavior( FRONTEND_LOAD_BEHAVIOR_DESTROY_SESSION )
			::ResetGlobalDialogBoxSystem( )
			::GameApp.LoadFrontEnd( )
		}
	}
	
	function OnPressB( )
	{
		//if we aren't the host, ignore b press
		if( !::GameAppSession.IsHost )
			return
		
		//if we are the host but client hasn't readied yet, do nothing
		if( ::GameAppSession.IsHost && !twoPlayerController.lobbyController.IsClientReady( ) )
			return
			
		::BaseEndGameScreen.OnPressB( )
	}
}
