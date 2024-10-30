
// global stuff
sigimport "Gui/Textures/FrontEnd/logo_ts2_g.png"
sigimport "Gui/Scripts/DialogBox/GlobalModalDialogBox.nut"
sigimport "Gui/Scripts/FrontEnd/titlescreen.nut"
sigimport "Gui/Scripts/FrontEnd/InviteLobby.nut"
sigimport "gui/scripts/frontend/frontendscripts.nut"

class FrontEndRootMenu extends Gui.CanvasFrame
{
	menuStack = null
	fireInviteEvent = false
	inviteWaitingOnBuyScreen = null

	constructor( startMode = FrontEndRootMenuStartMode.Normal )
	{
		fireInviteEvent = false
		inviteWaitingOnBuyScreen = false
		
		::Gui.CanvasFrame.constructor( )
		
		::GameApp.HudRoot.Invisible = false
		
		// Set all player's rich presence to NOT in a lobby
		::GameApp.InMultiplayerLobby = false
		
		// Reset the music if they came here from the credits
		::GameApp.AudioEvent( "Stop_UI_Credits" )
		
		switch( startMode )
		{
		case FrontEndRootMenuStartMode.Normal:
			CreateMenuStack( )
			break
		case FrontEndRootMenuStartMode.WarnUserSignedOut:
			WarnUserSignedOut( )
			break
		case FrontEndRootMenuStartMode.FromInvite:
			CreateMenuStack( )
			fireInviteEvent = true
			break
		}
		
		// We can't clear this data if they're signed out.
		//  We need to return them into the menu stack.
		//  If this data were cleared, the "signed out" notification would never been show.
		//  Once that notification is shown, they will be returned to the title screen and this data will be cleared.
		//if( !::GameApp.FrontEndPlayer.User.SignedIn )
		//	::FrontEndStartMenuData = null
		
		if( ::FrontEndStartMenuData )
		{
			if( ::FrontEndStartMenuData.IsClient )
			{
				local newMenu = ::LevelSelectLobby( ::FrontEndStartMenuData.MapType, ::FrontEndStartMenuData.Dlc )
				// we dont want an on-create warning that there's no live when coming back from a game
				newMenu.warnNoXLiveOnSessionStart = false
				menuStack.PushMenu( newMenu )
				
				if( ::FrontEndStartMenuData.UserId != null )
					ForceAddPlayer1( ::FrontEndStartMenuData.UserId )
			}
			else
			{
				menuStack.PushMenu( ::FrontEndMainMenu( ) )
				
				if( ::FrontEndStartMenuData.Type != "MainMenu" )
				{
					menuStack.PushMenu( ::FrontEndPlayGameMenu( ) )
					
					if( ::GameApp.IsFullVersion )
					{
						switch( ::FrontEndStartMenuData.Type )
						{
							case "RankedMatch":
								menuStack.PushMenu( ::FrontEndHeadToHeadMenu( ) )
							break
							
							case "DisplayCase":
							break
							
							case "LevelSelect":
								// Go to level select screen
								switch( ::FrontEndStartMenuData.MapType )
								{
									case MAP_TYPE_CAMPAIGN:
										menuStack.PushMenu( ::FrontEndCampaignMenu( ::FrontEndStartMenuData.Dlc ) )
										menuStack.PushMenu( ::CampaignLevelSelectMenu( ::FrontEndStartMenuData.Dlc ) )
									break
									
									case MAP_TYPE_HEADTOHEAD:
										menuStack.PushMenu( ::FrontEndHeadToHeadMenu( ) )
										menuStack.PushMenu( ::HeadToHeadLevelSelectMenu( ) )
									break
									
									case MAP_TYPE_SURVIVAL:
										menuStack.PushMenu( ::SurvivalLevelSelectMenu( ) )
									break
									
									case MAP_TYPE_MINIGAME:
										menuStack.PushMenu( ::MinigameLevelSelectMenu( ) )
									break
									
									case "OneManArmy":
										menuStack.PushMenu( ::OneManArmyLevelSelectMenu( ) )
									break
								}
								
								// Add player2
								if( ::FrontEndStartMenuData.UserId != null )
									ForceAddPlayer2( ::FrontEndStartMenuData.UserId )
							break
						}
					}
				}
			}
			
			menuStack.filteredGamepad.SetUser( ::GameApp.FrontEndPlayer.User )
			::FrontEndStartMenuData = null
		}
	}

	function ForceAddPlayer2( userId )
	{
		if( userId == "remote" && ::GameAppSession.RemoteUserCount > 0 )
			menuStack.CurrentMenu( ).AddRemoteUser2( ::GameAppSession.RemoteUser( 0 ) )
		else
			menuStack.CurrentMenu( ).AddLocalUser2( ::GameApp.GetLocalUser( userId ) )
	}
	
	function ForceAddPlayer1( userId )
	{
		if( userId == "remote" && ::GameAppSession.RemoteUserCount > 0 )
			menuStack.CurrentMenu( ).AddRemoteUser1( ::GameAppSession.RemoteUser( 0 ) )
		else
			menuStack.CurrentMenu( ).AddLocalUser1( ::GameApp.GetLocalUser( userId ) )
	}
	
	function OnTick( dt )
	{
		::Gui.CanvasFrame.OnTick( dt )
			
		// Fire the invite event if we haven't yet
		if( fireInviteEvent )
		{
			fireInviteEvent = false
			::GameApp.FireInviteEvent( )
		}
			
	}
	function HandleCanvasEvent( event )
	{
		// If we've started up intending to handle an invite
		// then we need to ensure that the invite is the first event we handle.
		if( !::GameApp.HasInvite || event.Id == SESSION_INVITE_ACCEPTED )
			fireInviteEvent = false
			
		if( fireInviteEvent )
		{
			fireInviteEvent = false
			::GameApp.FireInviteEvent( )
		}
		
		switch( event.Id )
		{
		case ON_PLAYER_SIGN_OUT:
			{
				if( menuStack )
				{
					local playerIndex = IntEventContext.Convert( event.Context ).Int
					if( playerIndex == ::GameApp.FrontEndPlayer.PlayerIndex )
					{
						local shouldWarn = ( !menuStack.CurrentMenu( ) || !menuStack.CurrentMenu( ).IgnoreSignOutEvent( ) )
						
						KillMenuStack( )
						
						if( shouldWarn )
							WarnUserSignedOut( )
						else
							CreateMenuStack( )
						return true
					}
				}
			}
			break;
		case SESSION_INVITE_ACCEPTED:
			{
				if( ::GameApp.HasInvite )
				{
					::print( "Accepted invite" )
					// Forward the event to our children
					::Gui.CanvasFrame.HandleCanvasEvent( event )
					
					local context = IntEventContext.Convert( event.Context )
					
					// If the hw index doesn't match the defaut user
					if( context.Int != ::GameApp.FrontEndPlayer.User.LocalHwIndex )
					{
						// Change default to new hw index
						::GameApp.SetPlayerLocalUser( 0, context.Int )
					}
					
					if( ::GameApp.FrontEndPlayer.NeedsToChooseSaveDevice )
						::GameApp.FrontEndPlayer.ChooseSaveDeviceId( 0 )
					
					::print( "pushing invite lobby" )
					::ResetGlobalDialogBoxSystem( )
					KillMenuStack( )
					CreateMenuStack( )
					menuStack.PushMenu( ::FrontEndInviteLobby( ) )
					menuStack.filteredGamepad.SetUser( ::GameApp.FrontEndPlayer.User )
				}
				
				return true;
			}
			break;
		case SESSION_INVITE_NEED_FULL_VERISON:
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
			break;
		case SESSION_INVITE_REJECTED:
			{
				local userIndex = IntEventContext.Convert( event.Context ).Int
				local user = ::GameApp.GetLocalUser( userIndex )
				
				local dialog = ::ModalErrorBox( "Menus_Error07_NoAction", user, "Accept" )
				return true
			}
		case ON_SYSTEM_UI_CHANGE:
			{
				//have to do this for the case that the buy screen comes up and is dismissed. Otherwise the SetGamePurchasedCallback may continue to be set after the screen goes away
				if( inviteWaitingOnBuyScreen && !BoolEventContext.Convert( event.Context ).Bool )
				{
					inviteWaitingOnBuyScreen = false
					if( !::GameApp.IsFullVersion )
					{
						::GameApp.ClearInvite( )
					}
				}
			}
			break;
		case ON_PROFILE_STORAGE_DEVICE_REMOVED:
			{
				::print( "frontend - OnProfileStorageDeviceRemoved" )
				if( menuStack )
				{
					local menuCount = menuStack.MenuCount( )
					if( menuCount > 1 )
					{
						local userIndex = IntEventContext.Convert( event.Context ).Int
						
						if( userIndex  == ::GameApp.FrontEndPlayer.User.LocalHwIndex )
						{
							KillMenuStack( )
							::ResetGlobalDialogBoxSystem( )
							CreateMenuStack( )
							return true
						}
					}
				}
			}
			break;
		case ON_UPGRADE_TO_FULL_VERSION:
			EndTrialToFullInviteDialog( )
			break;
		case SESSION_STATS_LOST:
			{
				local dialog = ::ModalErrorBox( "error_failedLeaderboardWrite", null )
				::ClearAllGlobalDialogBoxQueuesOfTextId( dialog.textId )
			}
			return true
		}
		
		if( menuStack && menuStack.CurrentMenu( ) && menuStack.CurrentMenu( ).HandleCanvasEvent( event ) )
			return true
			
		return ::Gui.CanvasFrame.HandleCanvasEvent( event )
	}
	
	function EndTrialToFullInviteDialog( )
	{
		if( ::GameApp.IsFullVersion )
		{
			::GameApp.FireInviteEvent( )
		}
	}
	
	function CreateMenuStack( )
	{		
		menuStack = ::VerticalMenuStack( ::GameApp.FrontEndPlayer.User )
		menuStack.PushMenu( ::FrontEndTitleScreen( ) )
		AddChild( menuStack )
	}
	
	function KillMenuStack( )
	{
		// If the menu stack is killed reset any session that we may have been in
		::GameAppSession.CancelSession( )
		
		if( menuStack )
			menuStack.DeleteSelf( )
		
		menuStack = null
	}
	
	function WarnUserSignedOut( )
	{
		local dialogBox = ::ModalErrorBox( "Menus_UserSignedOutReturnToTitleScreen1", null )
		
		dialogBox.onAPress = function( )
		{
			::ResetGlobalDialogBoxSystem( )
			CreateMenuStack( )
			
		}.bindenv( this )
	}
}



