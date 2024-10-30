// Base end game screen

// Requires
sigimport "gui/scripts/controls/animatingcanvas.nut"
sigimport "gui/scripts/controls/controllerbuttoncontainer.nut"
sigimport "Gui/Scripts/FrontEnd/common.nut"
sigimport "gui/scripts/controls/gamepad.nut"


class EndGameScreenStack extends VerticalMenuStack
{	
	sessionEventHandler = null
	
	constructor( user_ )
	{
		::VerticalMenuStack.constructor( user_, false )
		minStackCount = 0 // let the trial screen back out
		SetZPos( 0.5 )
		
		sessionEventHandler = ::InGameSessionEventHandler( this )
	}
	
	function OnTick( dt )
	{
		::VerticalMenuStack.OnTick( dt )
	}
	
	function AcceptInvite( userIdx )
	{
		local dg = ::ModalConfirmationBox( "Quit_Confirm", ::GameApp.GetLocalUser( userIdx ), "Accept", "Cancel" )
		
		dg.onAPress = function( )
		{
			::ResetGlobalDialogBoxSystem( )
			::ClearFrontEndMultiplayerRestartData( ) //a player accepted an invite to a game on another box. ensure ::FrontEndStartMenuData.UserId == null to prevent wierd shit from happening when TwoPlayerMenuController calls ::GameApp.SetSecondLocalUser
			::GameAppSession.SetFrontEndLoadBehavior( FRONTEND_LOAD_BEHAVIOR_DESTROY_SESSION )
			::GameApp.LoadFrontEnd( )
			
		}.bindenv( this )
		
		dg.onBPress = function( )
		{
			::GameApp.ClearInvite( );
		}
	}
	
	function HandleCanvasEvent( event )
	{		
		if( sessionEventHandler.HandleCanvasEvent( event ) )
			return true
			
		else if( event.Id == SESSION_INVITE_ACCEPTED )
		{
			local context = IntEventContext.Convert( event.Context )
			
			AcceptInvite( context.Int )
			return true
		}
			
		return ::VerticalMenuStack.HandleCanvasEvent( event )
	}
}

class BaseEndGameScreen extends VerticalMenu
{
	// Display
	controls = null // ControllerButtonContainer
	twoPlayerController = null
	
	// Data
	noInput = null
	buyPushed = false
	
	constructor( rect, noLobby = false )
	{
		::VerticalMenu.constructor( )
		inputHook = true
		
		noInput = false
		buyPushed = false
		
		ForwardButtons = GAMEPAD_BUTTON_A
		BackButtons = GAMEPAD_BUTTON_B
		
		controls = ::ControllerButtonContainer( FONT_SIMPLE_SMALL )
		controls.SetPosition( rect.Left, rect.Bottom - 12, 0 )
		AddChild( controls )
		
		// Fade out bg
		local fade = ::Gui.ColoredQuad( )
		fade.SetRgba( 0.0, 0.0, 0.0, 0.5 )
		fade.SetRect( ::Math.Vec2.Construct( 1280, 720 ) )
		fade.SetPosition( 0, 0, 0.06 )
		AddChild( fade )
		
		// Hide HUD
		::GameApp.HudRoot.Invisible = true
		
		// Two Player Controller
		if( ::GameApp.GameMode.IsNet && !noLobby )
		{
			twoPlayerController = ::TwoPlayerMenuController( ::GameApp.FrontEndPlayer.User, ::GameAppSession.IsHost )
			twoPlayerController.SetPosition( rect.Left, rect.Top, 0 )
			if( ::GameAppSession.IsHost )
				twoPlayerController.onClientDropped = HandleClientDropped.bindenv( this )
			AddChild( twoPlayerController )
			
			if( !::GameAppSession.IsQuickMatch )
			{
				if( ::GameAppSession.IsHost )
				{
					local clientStatus = ::ClientStateChange( )
					clientStatus.Set( CLIENTSTATE_READYREQUEST )
					::GameAppSession.SendClientStateChange( clientStatus )
				}
				else
					twoPlayerController.display.ProfileBadge( 1 ).EnableControl( true )
			}
		}
	}
	
	function QueryUser( )
	{
		return user
	}
	
	function HandleCanvasEvent( event )
	{
		if( !::GameAppSession.IsHost && twoPlayerController )
		{
			switch( event.Id )
			{
				case SESSION_LOAD_LEVEL:
				case SESSION_DELETED:
					AutoExit = true
				return true
				
				case ON_DISCONNECT:
					HandleHostDropped( )
				return true
				
				case LOBBY_CLIENT_STATE_CHANGE:
					local context = ::ObjectEventContext.Convert( event.Context ).Object
					
					switch( context.State )
					{
						case CLIENTSTATE_ALLOWUNREADY:
							HandleHostAllowsUnready( )
						return true
						
						case CLIENTSTATE_READYREQUEST:
							// Just send the current status of the client
							local clientStatus = ::ClientStateChange( )
							if( twoPlayerController.lobbyController.IsClientReady( ) )
								clientStatus.Set( CLIENTSTATE_READY )
							else
								clientStatus.Set( CLIENTSTATE_UNREADYREQUEST )

							::GameAppSession.SendClientStateChange( clientStatus )
						return true
						
						case CLIENTSTATE_QUIT:
							::print( "host quitting back to main menu" )
							local levelInfo = ::GameApp.CurrentLevelLoadInfo
							::SetFrontEndLevelSelectRestart( levelInfo.MapType, levelInfo.DlcNumber, "remote", true )
							::GameAppSession.SetFrontEndLoadBehavior( FRONTEND_LOAD_BEHAVIOR_END_SESSION )
							CloseEndGameScreen( )
							::ResetGlobalDialogBoxSystem( )
							::GameApp.LoadFrontEnd( )
						return true
					}
				break
			}
		}
		
		if( twoPlayerController && twoPlayerController.HandleCanvasEvent( event ) )
			return true
		
		return false
	}
	
	function HandleClientDropped( )
	{
		::GameApp.AudioEvent( "Play_HUD_MP_PlayerDrop" )
		SetControls( )
	}
	
	function HandleHostDropped( )
	{
		::GameApp.AudioEvent( "Play_HUD_MP_PlayerDrop" )
		if( twoPlayerController )
			twoPlayerController.ClearPlayer1( )
		SetControls( )
	}
		
	function HandleHostAllowsUnready( )
	{
		if( twoPlayerController )
			twoPlayerController.lobbyController.SetClientReady( false )
		SetControls( )
	}
	
	function SelectActiveIcon( )
	{
		if( noInput )
			return
			
		if( ::GameAppSession.IsHost )
		{
			if( !buyPushed )
			{		
				PlaySound( "Play_UI_Select_Forward" )
				return OnPressA( )
			}
			else
			{
				buyPushed = false
				return true //buy was pushed "accept" the menu change
			}
		}
		else
		{
			if( !::GameAppSession.IsQuickMatch && twoPlayerController && twoPlayerController.user2 )
			{
				// Send Ready/Unready message
				local clientStatus = ::ClientStateChange( )
				
				if( twoPlayerController.lobbyController.IsClientReady( ) )
				{
					clientStatus.Set( CLIENTSTATE_UNREADYREQUEST )
				}
				else
				{
					twoPlayerController.lobbyController.SetClientReady( true )
					clientStatus.Set( CLIENTSTATE_READY )
				}
				SetControls( )
				::GameAppSession.SendClientStateChange( clientStatus )
			}
		}
		return false
	}
	
	function OnBackOut( )
	{
		if( noInput )
			return
		PlaySound( "Play_UI_Select_Backward" )
		return OnPressB( )
	}
	
	function CloseEndGameScreen( )
	{
		AutoExit = true
	}

	function QuitToMainMenu( )
	{
		// Send a drop message
		if( ::GameApp.GameMode.IsNet && twoPlayerController )
		{
			local clientStatus = ::ClientStateChange( )
			
			if( ::GameAppSession.IsHost )
				clientStatus.Set( CLIENTSTATE_QUIT )
			else
				clientStatus.Set( CLIENTSTATE_DROPPED )
			
			::GameAppSession.SendClientStateChange( clientStatus )
			
			if( ::GameAppSession.IsHost && twoPlayerController.user2 )
			{
				local levelInfo = ::GameApp.CurrentLevelLoadInfo
				local userId = ( ( twoPlayerController.user2.IsLocal )? twoPlayerController.user2.UserId: "remote" )
				::SetFrontEndLevelSelectRestart( levelInfo.MapType, levelInfo.DlcNumber, userId, false )
				::GameAppSession.SetFrontEndLoadBehavior( FRONTEND_LOAD_BEHAVIOR_END_SESSION )
			}
		}
		
		CloseEndGameScreen( )
		::ResetGlobalDialogBoxSystem( )
		::GameApp.LoadFrontEnd( )
	}
	
	function RestartCurrentLevel( )
	{
		if( ::GameApp.GameMode.IsNet )
		{
			if( ::GameAppSession.IsHost )
			{
				if( twoPlayerController && twoPlayerController.lobbyController.IsClientReady( ) )
				{
					CloseEndGameScreen( )
					::GameAppSession.RestartMap( )
				}
				else
					PlaySound( "Play_HUD_WeaponMenu_Error" )
			}
		}
		else
		{
			CloseEndGameScreen( )
			::GameApp.ReloadCurrentLevel( )
		}
	}
	
	function OnPressA( ) 
	{ 
		return true 
	}
	
	function OnPressB( )
	{				
		if( ::GameApp.IsFullVersion )
		{
			noInput = true
			QuitToMainMenu( )
			return true
		}
		else
		{
			PushNextMenu( ::TrialBuyGameScreen( ) )
			buyPushed = true // pushing the trial screen will select the 
			AutoAdvance = true
			return false
		}
	}
	
	function SetControls( ) {}
}