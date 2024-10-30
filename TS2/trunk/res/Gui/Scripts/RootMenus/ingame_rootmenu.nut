
sigimport "Gui/Scripts/PauseMenu/PauseMenu.nut"
sigimport "Gui/Scripts/DialogBox/globalmodaldialogbox.nut"
sigimport "Gui/Scripts/Utility/InGameSessionEventHandler.nut"

class BaseInGameRootMenu extends Gui.CanvasFrame
{
	menuStacks = null
	sessionEventHandler = null
	currentLevel = null
	mapType = null
	inviteWaitingOnBuyScreen = null

	constructor( )
	{
		::Gui.CanvasFrame.constructor( )
		menuStacks = { }
		sessionEventHandler = ::InGameSessionEventHandler( this )
		
		currentLevel = ::GameApp.CurrentLevel
		mapType = currentLevel.MapType
		inviteWaitingOnBuyScreen = false
	}
	
	function OnTick( dt )
	{
		::Gui.CanvasFrame.OnTick( dt )
	}
	
	function AutoHandlePauseMenu( dt )
	{
		local playersWithMenus =  { }
		local localPlayerMenuOpen = false
		
		// Check to see if there is a local menu stack open
		local toRemove = [ ]
		foreach( p, stack in menuStacks )
		{
			playersWithMenus[ p ] <- ( p )
			if( !stack.HasParent( ) )
				toRemove.push( p )
			
			local player = ::GameApp.GetPlayer( p )
			localPlayerMenuOpen = ( localPlayerMenuOpen || player.User.IsLocal )
		}
		
		foreach( p in toRemove )
			delete menuStacks[ p ]
		
		if( !is_null( currentLevel ) && !( currentLevel.VictoryOrDefeat ) )
		{
			local player = GetPlayerOpeningPauseMenu( currentLevel, mapType )
			
			if( ( ::GameApp.GameMode.IsLocal ) && ( ::GameApp.AskPlayerToBuyGame ) )
				player = ::GameApp.FrontEndPlayer

			if( ( player != null ) && ::Player.IsValid( player ) )
			{
				local playerIndex = ::GameApp.WhichPlayer( player.User )
				if( !( playerIndex in playersWithMenus ) && ( !player.User.IsLocal || !localPlayerMenuOpen ) )
					ShowPauseMenu( player, !::GameApp.GameMode.IsLocal )
			}
			
			// Rewind menu
			if( ::GameApp.RewindEnabled )
			{
				if( ::GameApp.GameMode.IsNet )
					player = ::GameApp.HostPlayerOpeningRewindMenu( )
				else				
					player = ::GameApp.AnyPlayerOpeningRewindMenu( )
					
				if( player && ::Player.IsValid( player ) )
				{					
					local playerIndex = ::GameApp.WhichPlayer( player.User )
					if( !( playerIndex in playersWithMenus ) && ( !player.User.IsLocal || !localPlayerMenuOpen ) )
					{
						ShowRewindMenu( player, ::GameApp.GameMode.IsLocal )
					}
				}
			}
		}
	}
	
	function GetPlayerOpeningPauseMenu( currentLevel, mapType )
	{
		if( ::GameApp.GameAppSession.LoadingLevel )
			return null
		/*if( mapType == MAP_TYPE_MINIGAME )
		{
			local player = currentLevel.ControllingPlayer( )
			local gamePad = player.Gamepad
			if( gamePad.ButtonDown( GAMEPAD_BUTTON_START ) && !gamePad.ButtonHeld( GAMEPAD_BUTTON_SELECT ) )
				return player
		}
		else
		{*/
			return ::GameApp.AnyPlayerOpeningPauseMenu( )
		/*}
		
		return null*/
	}
	
	function ShowPauseMenu( player, dontActuallyPause = false )
	{
		local playerIndex = ::GameApp.WhichPlayer( player.User )
		if( playerIndex in menuStacks )
			KillMenuStack( playerIndex )
			
		// If a debug camera paused the game, the menuStack will by null.
		//  So just unpause the game rather than pushing a menu.
		if( ::GameApp.Paused( ) )
		{
			::print( "Game unpaused via less-desirable debug means." )
			::GameApp.Pause( false, player.AudioSource )
		}
		else
		{
			if( player.User.IsLocal )
				::GameApp.HudRoot.Invisible = true
			
			// regular intended functionality
			menuStacks[ playerIndex ] <- ::PauseMenuStack( player, dontActuallyPause )
			AddChild( menuStacks[ playerIndex ] )
		}
	}
	
	function ShowRewindMenu( player, pauseGame )
	{
		local playerIndex = ::GameApp.WhichPlayer( player.User )
		if( playerIndex in menuStacks )
			KillMenuStack( playerIndex )
		
		menuStacks[ playerIndex ] <- ::RewindMenuStack( player.User )
		menuStacks[ playerIndex ].PushMenu( RewindMenu( ) )
		AddChild( menuStacks[ playerIndex ] )
		
		if( pauseGame )
			::GameApp.Pause( true, player.AudioSource )
		
		if( player.User.IsLocal )
			::GameApp.HudRoot.Invisible = true
	}
	
	function KillMenuStack( playerIndex = null )
	{
		if( !playerIndex )
		{
			foreach( p, stack in menuStacks )
				stack.DeleteSelf( )
			menuStacks.clear( )
		}
		else if( menuStacks && playerIndex in menuStacks )
		{
			menuStacks[ playerIndex ].DeleteSelf( )
			delete menuStacks[ playerIndex ]
		}
	}
	
	function AddMenuStack( player, stack )
	{
		local playerIndex = ::GameApp.WhichPlayer( player.User )
		if( playerIndex in menuStacks )
			KillMenuStack( playerIndex )
		
		menuStacks[ playerIndex ] <- stack
		AddChild( menuStacks[ playerIndex ] )
		
		if( player.User.IsLocal )
			::GameApp.HudRoot.Invisible = true
	}
	
	function WarnUserSignedOut( )
	{
		//user.SetToGamepadCaptureMode( GAMEPAD_BUTTON_A )
		local dialogBox = ::ModalErrorBox( "Menus_UserSignedOutReturnToTitleScreen2", null )
		dialogBox.onFadedOut = function( )
		{
			::ResetGlobalDialogBoxSystem( )
			::ClearFrontEndRestartData( )
			::GameAppSession.SetFrontEndLoadBehavior( FRONTEND_LOAD_BEHAVIOR_DESTROY_SESSION )
			::GameApp.LoadFrontEnd( )
		}.bindenv(this)
	}
	
	function WarnNoLive( user )
	{
		::ModalInfoBox( "Menus_Error03", user, "Accept" )
	}
	
	function AcceptInvite( userIdx )
	{
		local user = ::GameApp.GetLocalUser( userIdx )
		local dialog = ::ModalConfirmationBox( "AcceptInvite_Confirm", user, "Accept", "Cancel" )
		
		
		dialog.onAPress = function( )
		{
			//::print( "Pressed A on invite" )
			
			::ResetGlobalDialogBoxSystem( )
			::ClearFrontEndRestartData( )
			::GameApp.LoadFrontEnd( )
		}
		
		dialog.onBPress = function( )
		{
			//::print( "Pressed B on invite" )
			::GameApp.ClearInvite( )
		}
	}
	
	function HandleCanvasEvent( event )
	{		
		if( event.Id == SESSION_INVITE_ACCEPTED )
		{
			local context = IntEventContext.Convert( event.Context )
			AcceptInvite( context.Int )
			return true
		}
		else if( event.Id == SESSION_INVITE_NEED_FULL_VERISON )
		{
			::print("FRONTEND - NEED FULL VER TO JOIN XBOXLIVE GAME")
			local userIndex = IntEventContext.Convert( event.Context ).Int
			local user = ::GameApp.GetLocalUser( userIndex )
			local dialog = ::ModalConfirmationBox( "Menus_Error09", user, "Trial_BuyAndContinue", "Cancel" )
			dialog.onAPress = function( ) : ( user )
			{
				inviteWaitingOnBuyScreen = true
				user.ShowMarketplaceUI( false )
			}.bindenv( this )
			dialog.onBPress = function( )
			{
				::GameApp.ClearInvite( )
			}
			return true
		}
		else if( event.Id == ON_SYSTEM_UI_CHANGE )
		{
			//have to do this forA the case that the buy screen comes up and is dismissed. Otherwise the SetGamePurchasedCallback may continue to be set after the screen goes away
			if( inviteWaitingOnBuyScreen && !BoolEventContext.Convert( event.Context ).Bool )
			{
				inviteWaitingOnBuyScreen = false
				if( !::GameApp.IsFullVersion )
				{
					::GameApp.ClearInvite( )
				}
			}
			else if( BoolEventContext.Convert( event.Context ).Bool )
			{
				// Don't show the menu if the game is already paused
				if( ::GameApp.GameMode.IsLocal && !::GameApp.Paused( ) )
					ShowPauseMenu( ::GameApp.FrontEndPlayer )
			}
		}
		else if( event.Id == ON_PLAYER_LOSE_INPUT )
		{
			if( !::GameApp.GameMode.IsNet )
			{
				local hwIndex = IntEventContext.Convert( event.Context ).Int
				::print("lost connection to controller " + hwIndex)
				local playerWithoutController = ::GameApp.GetPlayerByHwIndex( hwIndex )
				local gamePaused = ::GameApp.Paused( )
				if( !is_null( playerWithoutController ) && !gamePaused )
					ShowPauseMenu( playerWithoutController )
			}
		}
		else if( event.Id == ON_UPGRADE_TO_FULL_VERSION )
		{
				EndTrialToFullInviteDialog( )
				::Gui.CanvasFrame.HandleCanvasEvent( event )
		}
		else if( event.Id == SESSION_STATS_LOST )
		{
			local dialog = ::ModalErrorBox( "error_failedLeaderboardWrite", null )
			::ClearAllGlobalDialogBoxQueuesOfTextId( dialog.textId )
			return true
		}
		else if( ::Gui.CanvasFrame.HandleCanvasEvent( event ) )
			return true
			
		return sessionEventHandler.HandleCanvasEvent( event )
	}
	
	function EndTrialToFullInviteDialog( )
	{
		if( ::GameApp.IsFullVersion )
		{
			::GameApp.FireInviteEvent( )
		}
	}
}

