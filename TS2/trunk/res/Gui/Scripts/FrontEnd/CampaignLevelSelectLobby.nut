// Campaign Level Select Lobby

// Requires
sigimport "gui/scripts/frontend/levelselect.nut" 

class LevelSelectLobby extends LevelSelectMenuBase 
{
	// Data
	clientDropped = null
	currentHasDlcMask = null
	hasDlc = null
	
	constructor( mapType_, dlc_ = null )
	{
		if( mapType_ != MAP_TYPE_CAMPAIGN )
			dlc_ = null
		::LevelSelectMenuBase.constructor( mapType_, dlc_, true, PlayerRestriction.None, this.HasDlc.bindenv( this ) )
		inputHook = true
		clientDropped = false
		ForwardButtons = 0
		BackButtons = 0
		
		currentHasDlcMask = null
		hasDlc = [ true, false, false ]
	}
	
	function FinalizeIconSetup( )
	{
		::LevelSelectMenuBase.FinalizeIconSetup( )
		twoPlayerController.display.EnableControls( true )
		twoPlayerController.onUserChange = OnUserChange.bindenv( this )
	}
	
	function OnTick( dt )
	{
		::LevelSelectMenuBase.OnTick( dt )
	}
	
	function OnUserChange( )
	{
		if( ::GameApp.PlayerIncompatibilityCheck( ) )
		{
			//ReturnToRootMenu( false );
			twoPlayerController.dialogOpen = true
			WarnIncompatibleDlc( )
			AutoBackOut = true
			return
		}
		
		if( mapType == MAP_TYPE_CAMPAIGN )
		{
			local hostPlayer = ::GameApp.GetPlayer( 1 )
			if( !is_null( hostPlayer ) )
			{
				local hostProfile = hostPlayer.GetUserProfile( )
				if( !is_null( hostProfile ) )
				{
					local myProfile = ::GameApp.FrontEndPlayer.GetUserProfile( )
					local myHighest = myProfile.HighestLevelReached( dlc )
					local hostHighest = hostProfile.HighestLevelReached( dlc )
					
					if( myHighest < hostHighest && mapType == MAP_TYPE_CAMPAIGN )
						::ModalInfoBox( "Lobby_HostHigher", ::GameApp.FrontEndPlayer.User )
					
					hostPlayer = null
				}
			}
		}
		
		SetControls( )
	}
	
	function ChangeHorizontalHighlight( delta )
	{
		// Can't manually control
	}
	
	function ChangeHighlight( indexDelta, force = false )
	{
		// Can't manually control
	}
	
	function HandleCanvasEvent( event )
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
			
			case ON_PLAYER_NO_LIVE:
				if( IntEventContext.Convert( event.Context ).Int == ::GameApp.WhichPlayer( user ) )
				{
					::ResetGlobalDialogBoxSystem( )
					local dialogBox = ::ModalInfoBox( "Menus_Error04", user, "Accept" )
					twoPlayerController.dialogOpen = true

					dialogBox.onAPress = function( )
					{
						twoPlayerController.dialogOpen = false
						ReturnToRootMenu( true )
					}.bindenv(this)
					
					return true
				}
				else
					return false
			break
			
			
			case LOBBY_CLIENT_STATE_CHANGE:
				local context = ::ObjectEventContext.Convert( event.Context ).Object
				
				switch( context.State )
				{
					case CLIENTSTATE_ALLOWUNREADY:
						HandleHostAllowsUnready( )
					return true
				}
			break
			
			case LOBBY_MENU_STATE_CHANGE:
				local context = ::ObjectEventContext.Convert( event.Context ).Object
				SetAvailableDlc( context.AvailableDlc )
				Set( context.SelectedLevel, context.SelectingMode, context.SelectedMode, context.CurrentDlc )
			return true
		}
		
		return ::LevelSelectMenuBase.HandleCanvasEvent( event )
	}
	
	function SetAvailableDlc( mask )
	{
		if( currentHasDlcMask == mask )
			return
			
		currentHasDlcMask = mask
		
		foreach( i, value in hasDlc )
		{
			hasDlc[ i ] = ( ( 1 << i & mask ) ? true : false )
		}
		
		ForceUpdateLists( )
		scores.ResetHeadingsAndScores( )
	}
	
	function HasDlc( player, dlc )
	{
		//::print( "  using host's dlc" )
		if( dlc in hasDlc )
			return hasDlc[ dlc ]
		else
			return false
	}
	
	function HandleHostDropped( )
	{
		::GameApp.AudioEvent( "Play_HUD_MP_PlayerDrop" )
		
		local dialog = ::ModalInfoBox( "Menus_OnHostDrop", ::GameApp.FrontEndPlayer.User )
		dialog.onAPress = function( )
		{
			::ResetGlobalDialogBoxSystem( )
			
			AutoExit = true
			ReturnToRootMenu( true )
		}.bindenv( this )
		
		//dialog.SetPosition( dialog.GetPosition( ) - GetWorldPosition( ) )
		
		//AddChild( dialog )
	}
	
	function ReturnToRootMenu( cancelSession = true )
	{
		local rootMenu = ::GameApp.CurrentLevel.GetRootMenu( )
		rootMenu.KillMenuStack( )
		rootMenu.CreateMenuStack( )
			
		::GameApp.InMultiplayerLobby = 0
		
		if( cancelSession )
			::GameAppSession.CancelSession( )
	}
		
	function HandleHostAllowsUnready( )
	{
		twoPlayerController.lobbyController.SetClientReady( false )
		SetControls( )
	}
	
	function QueryUser( )
	{
		return user
	}
	
	function HandleInput( gamepad )
	{
		if( gamepad.ButtonDown( GAMEPAD_BUTTON_A ) )
		{
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
		else if( gamepad.ButtonDown( GAMEPAD_BUTTON_B ) )
		{
			if( TryToBackOut( ) )
				AutoBackOut = true
		}
		else if( gamepad.ButtonDown( GAMEPAD_BUTTON_Y ) )
		{
			// Show Leaderboards
			ShowLeaderboards( )
		}
		
		if( twoPlayerController )
			twoPlayerController.HandleInput( gamepad )
	}
	
	function TryToBackOut( )
	{
		// This should be the client
		local dialog = ::ModalConfirmationBox( "Warning_LobbyBackOut", player.User, "EndGame_Continue", "Cancel" )
		dialog.onAPress = function( )
		{
			AutoBackOut = true
		}.bindenv( this )
		//dialog.SetZPos( -0.1 )
		//AddChild( dialog )
		return false
	}
	
	function OnBackOut( )
	{
		// Skip LevelSelectBase.BackOut
		::LevelSelectDisplay.OnBackOut( )	
		
		// Send a drop message
		local clientStatus = ::ClientStateChange( )
		clientStatus.Set( CLIENTSTATE_DROPPED )
		::GameAppSession.SendClientStateChange( clientStatus )
		
		ReturnToRootMenu( )
		
		return true
	}
	
	function LoadLevel( front, levelIndex )
	{
		// Do nothing, wait for StartMap
	}
	
	function SetControls( )
	{
		controls.Clear( )
		if( twoPlayerController.lobbyController.IsClientReady( ) )
			controls.AddControl( GAMEPAD_BUTTON_A, "EndGame_Unready" )
		else
			controls.AddControl( GAMEPAD_BUTTON_A, "EndGame_Ready" )
		controls.AddControl( GAMEPAD_BUTTON_B, "EndGame_Quit" )
		
		navControls.Clear( )
		if( twoPlayerController && twoPlayerController.CanViewGamercard( ) )
			navControls.AddControl( GAMEPAD_BUTTON_SELECT, "LB_Menus_Gamer_Card" )
		navControls.AddControl( "gui/textures/gamepad/button_y_g.png", "Menus_ViewLeaderboards" )
	}
}
