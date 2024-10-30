// InGameSessionEventHandler.nut

sigimport "Gui/Scripts/DialogBox/globalmodaldialogbox.nut"

class InGameSessionEventHandler
{
	rootMenu = null
	
	constructor( rootMenu_ )
	{
		rootMenu = rootMenu_
	}
	
	function HandleCanvasEvent( event )
	{
		if( event.Id == ON_PLAYER_NO_LIVE )
		{
			if( !::GameApp.GameMode.IsNet )
				WarnNoLive( IntEventContext.Convert( event.Context ).Int );
			else
				WarnOnDisconnect( IntEventContext.Convert( event.Context ).Int, "Menus_XboxLiveConnectionLost_TitleScreen" )
				
			return true
		}
		else if( event.Id == ON_PLAYER_SIGN_OUT )
		{
			WarnPlayerSignOut( IntEventContext.Convert( event.Context ).Int )
			return true
		}
		else if( event.Id == LOBBY_CLIENT_STATE_CHANGE && !( ::GameAppSession.IsQuickMatch && ::GameApp.CurrentLevel.VictoryOrDefeat ) )
		{
			local context = ::ObjectEventContext.Convert( event.Context ).Object
				
			switch( context.State )
			{
				case CLIENTSTATE_DROPPED:
					WarnOnPlayerDrop( ::GameAppSession.IsHost )
				return true
			}
		}
		else if( event.Id == ON_DISCONNECT && !( ::GameAppSession.IsQuickMatch && ::GameApp.CurrentLevel.VictoryOrDefeat ) )
		{
			// we're either not quick match or has not ended yet.
			::GameAppSession.WriteLossForOtherPlayer( )
			WarnOnDisconnect( ::GameApp.FrontEndPlayer.PlayerIndex, null )
			return true
		}
		else if( event.Id == ON_PROFILE_STORAGE_DEVICE_REMOVED )
		{
			::ResetGlobalDialogBoxSystem( )
			::ClearFrontEndRestartData( )
			::GameAppSession.SetFrontEndLoadBehavior( FRONTEND_LOAD_BEHAVIOR_DESTROY_SESSION )
			::GameApp.LoadFrontEnd( )
			return true
		}

		return false
	}
	
	function WarnPlayerSignOut( playerIndex )
	{		
		local player = ::GameApp.GetPlayer( playerIndex )
		local dialogBox = ::ModalErrorBox( "Menus_UserSignedOutReturnToTitleScreen2", player.User, "Accept" )
		
		if( "KillMenuStack" in rootMenu )
			rootMenu.KillMenuStack( playerIndex )

		dialogBox.onFadedOut = function( )
		{
			::ResetGlobalDialogBoxSystem( )
			::ClearFrontEndRestartData( )
			::GameAppSession.SetFrontEndLoadBehavior( FRONTEND_LOAD_BEHAVIOR_DESTROY_SESSION )
			::GameApp.LoadFrontEnd( )
		}.bindenv(this)
		
		// We're a pausing box
		dialogBox.DoPause( )
	}
	
	function WarnNoLive( playerIndex )
	{	
		local player = ::GameApp.GetPlayer( playerIndex )
		local dialogBox = ::ModalInfoBox( "Menus_NoXboxLiveNoStats", player.User, "Accept" )
		
		/*dialogBox.onBPress = function( )
		{
			::ClearGlobalDialogBoxQueue( )
			::ClearFrontEndMultiplayerRestartData( )
			::GameAppSession.SetFrontEndLoadBehavior( FRONTEND_LOAD_BEHAVIOR_DESTROY_SESSION )
			::GameApp.LoadFrontEnd( )
			
		}.bindenv(this)*/
		
		// We're a pausing box
		dialogBox.DoPause( )
	}
	
	function WarnOnDisconnect( playerIndex, overrideText = null, destroy = true )
	{
		local player = ::GameApp.GetPlayer( playerIndex )
		local text = overrideText ? overrideText : "Menus_OnDisconnect"
		local dialogBox = ::ModalInfoBox( text, player.User, "Accept" )
		
		if( "KillMenuStack" in rootMenu )
			rootMenu.KillMenuStack( playerIndex )

		dialogBox.onFadedOut = function( ):( destroy )
		{
			::ResetGlobalDialogBoxSystem( )
			::ClearFrontEndRestartData( )
			::GameAppSession.SetFrontEndLoadBehavior( destroy ? FRONTEND_LOAD_BEHAVIOR_DESTROY_SESSION : FRONTEND_LOAD_BEHAVIOR_END_SESSION )
			::GameApp.LoadFrontEnd( )
		}.bindenv(this)

		// We're a pausing box
		dialogBox.DoPause( )
	}
	
	function WarnOnPlayerDrop( isHost )
	{
		local dialogBox = ::ModalInfoBox( 
			isHost ? "Menus_OnClientDrop" : "Menus_OnHostDrop", 
			::GameApp.FrontEndPlayer.User, 
			"Accept" )
			
		if( "KillMenuStack" in rootMenu )
			rootMenu.KillMenuStack( playerIndex )

		dialogBox.onFadedOut = function( )
		{
			::ResetGlobalDialogBoxSystem( )
			::ClearFrontEndMultiplayerRestartData( )
			::GameAppSession.SetFrontEndLoadBehavior( FRONTEND_LOAD_BEHAVIOR_DESTROY_SESSION )
			::GameApp.LoadFrontEnd( )
		}.bindenv(this)
		
		// We're a pausing box
		dialogBox.DoPause( )
	}
}