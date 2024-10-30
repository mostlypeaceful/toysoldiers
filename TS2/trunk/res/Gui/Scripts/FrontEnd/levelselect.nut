
// Requires
sigimport "Gui/Scripts/FrontEnd/common.nut"
sigimport "gui/scripts/frontend/levelselectdisplay.nut"
sigimport "gui/scripts/frontend/twoplayermenucontroller.nut"
sigimport "gui/scripts/dialogbox/globalmodaldialogbox.nut"
sigimport "gui/scripts/frontend/leaderboards.nut"


function WarnIncompatibleDlc( )
{
	local user = ::GameApp.FrontEndPlayer.User
	local meNoDlc = (user.AddOnsInstalled == 0)
	
	if( meNoDlc )
	{
		local dialogBox = ::ModalConfirmationBox( "Menus_IncompatibleDlc", user, "Menus_GetDlc", "Cancel" )

		dialogBox.onAPress = function( ):(dialogBox)
		{
			user.ShowMarketplaceUI( true )
		}.bindenv(this)
	}
	else
	{
		local dialogBox = ::ModalErrorBox( "Menus_IncompatibleDlc_OtherPlayer", user )
		twoPlayerController.dialogOpen = true
	}
}

class LevelSelectMenuBase extends LevelSelectDisplay 
{
	// Display
	levelSelectDisplay = null // LevelSelectDisplay
	twoPlayerController = null // TwoPlayerMenuController
	client = null
	inviteAllowPrev = null
	restriction = null
	waitForSessionCreation = null
	waitingTimer = null
	noInput = null
	warnNoXLiveOnSessionStart = true
	hasDlcFunc = null
	
	// Statics
	static timeOut = 10.0
		
	constructor( mapType_, dlc_ = null, client_ = false, restriction_ = PlayerRestriction.None, hasDlc = null )
	{
		::LevelSelectDisplay.constructor( mapType_, dlc_, hasDlc )
		client = client_
		restriction = restriction_
		waitForSessionCreation = true
		waitingTimer = null
		noInput = false
		warnNoXLiveOnSessionStart = true
		
		BackButtons = 0
		ForwardButtons = GAMEPAD_BUTTON_A
		SetPosition( 0, 0, 0.4 )
		
		// Reset the music if they came here from the credits
		::GameApp.AudioEvent( "Stop_UI_Credits" )
	}
	
	function FinalizeIconSetup( )
	{	
		::LevelSelectDisplay.FinalizeIconSetup( )
	
		// Set up user stuff and functionality
		///////////////////////////////////////////////////////////////////////////////////////////////////////////
		local vpSafeRect = ::GameApp.ComputeScreenSafeRect( )
		
		twoPlayerController = ::TwoPlayerMenuController( user, !client, restriction )
		twoPlayerController.SetPosition( vpSafeRect.Left, vpSafeRect.Top, 0 )
		twoPlayerController.onUserChange = SetControls.bindenv( this )
		AddChild( twoPlayerController )
		
		if( !::GameApp.GameMode.IsFrontEnd )
		{
			twoPlayerController.display.ShowUser( 1, false )
		}
		
		StartXblaSession( )
		
		if( ::GameApp.GameMode.IsFrontEnd && mapType == MAP_TYPE_HEADTOHEAD )
		{
			::GameApp.InMultiplayerLobby = 1
		}
		
		if( ::GameAppSession.IsHost )
		{
			onHighlightChange = ::TwoPlayerMenuController.HandleMenuUpdate.bindenv( twoPlayerController ) 
			twoPlayerController.onAddClient = ::LevelSelectDisplay.StatusChangeNotify.bindenv( this )
			twoPlayerController.onUserChange = OnUserChange.bindenv( this )
		}
		
		if( restriction == PlayerRestriction.NetOnly )
		{
			twoPlayerController.display.ProfileBadge( 1 ).EnableControl( false )
		}
		
		inviteAllowPrev = twoPlayerController.InvitesAllowed( )
		SetControls( )
	}
	
	function StartXblaSession( )
	{
		// Start XBLA session
		if( ::GameApp.IsFullVersion && ::GameApp.GameMode.IsFrontEnd && !client )
		{
			local gameMode = CONTEXT_GAME_MODE_CAMPAIGN
			if( mapType == MAP_TYPE_SURVIVAL )
				gameMode = CONTEXT_GAME_MODE_SURVIVAL
			else if( mapType == MAP_TYPE_MINIGAME )
				gameMode = CONTEXT_GAME_MODE_MINIGAME
			else if( mapType == MAP_TYPE_HEADTOHEAD )
				gameMode = CONTEXT_GAME_MODE_VERSUS
			
			// We may have a session already from returning to the level select
			// via the end of a match or decisive exit
			if( ::GameAppSession.IsInactive )
			{
				if( user.SignedInOnline )
				{
					waitForSessionCreation = false
					waitingTimer = 0
					SetControls( )
					
					::GameAppSession.HostGame( user, gameMode, CONTEXT_GAME_TYPE_STANDARD )
					return
				}
				else if( user.IsOnlineEnabled )
				{
					waitForSessionCreation = true
					SetControls( )
					
					if( warnNoXLiveOnSessionStart )
						WarnNoLive( )
					return
				}
				else
				{
					waitForSessionCreation = true
					SetControls( )
					return
				}	
			}
		}

		waitForSessionCreation = true
	}
	
	function WarnNoLive( )
	{
		local dialogBox = ::ModalConfirmationBox( "Menus_NoXboxLiveNoStats", user, "Accept", "Cancel" )
		twoPlayerController.dialogOpen = true

		dialogBox.onAPress = function( )
		{
			twoPlayerController.dialogOpen = false
		}.bindenv(this)
		dialogBox.onBPress = function( )
		{
			twoPlayerController.dialogOpen = false
			AutoBackOut = true
		}.bindenv(this)		
	}

	function OnUserChange( )
	{
		if( ::GameApp.PlayerIncompatibilityCheck( ) )
		{
			//ReturnToRootMenu( false );
			WarnIncompatibleDlc( )
			AutoBackOut = true
			return
		}

		AllowSuspendedPlay( twoPlayerController.user2 == null )
		SetControls( )
		
		if( restriction == PlayerRestriction.NetOnly )
		{
			twoPlayerController.display.ProfileBadge( 1 ).EnableControl( false )
		}
		else
		{		
			if( twoPlayerController.user2 && !twoPlayerController.user2.IsLocal )
				twoPlayerController.display.ProfileBadge( 1 ).EnableControl( false )
			else
				twoPlayerController.display.ProfileBadge( 1 ).EnableControl( true )
		}
		
		if( mapType == MAP_TYPE_CAMPAIGN && twoPlayerController.user2 && twoPlayerController.user2.IsLocal )
		{
			local myProfile = ::GameApp.GetPlayerByUser( twoPlayerController.user2 ).GetUserProfile( )
			local myHighest = myProfile.HighestLevelReached( dlc )
			local hostProfile = ::GameApp.FrontEndPlayer.GetUserProfile( )
			local hostHighest = hostProfile.HighestLevelReached( dlc )
			
			::print( "myHighest:" + myHighest.tostring( ) + " hostHighest:" + hostHighest.tostring( ) ) 
			
			if( myHighest < hostHighest )
			{
				dialogOpen = true
				local dg = ::ModalInfoBox( 
					"Lobby_Player1Higher", 
					twoPlayerController.user2 )
				dg.onFadedOut = function( ) { dialogOpen = false }.bindenv( this )
			}
		}
		
		::GameApp.ApplyRichPresence( )
	}
	
	function TryToBackOut( )
	{
		// This should be the host
		if( ::GameAppSession.IsHost && twoPlayerController && twoPlayerController.HasTwoPlayers( ) && !twoPlayerController.user2.IsLocal )
		{
			local dialog = ::ModalConfirmationBox( "Warning_LobbyCancel", player.User, "EndGame_Continue", "Cancel" )
			dialog.onAPress = function( )
			{
				AutoBackOut = true
			}.bindenv( this )
			return false
		}
		
		return true
	}
	
	function OnBackOut( )
	{
		if( noInput )
			return
			
		noInput = true
			
		if( ::GameApp.GameMode.IsFrontEnd )
		{
			::GameAppSession.CancelSession( )
			::GameApp.InMultiplayerLobby = 0
		}
		twoPlayerController.OnBackOut( )
		return ::LevelSelectDisplay.OnBackOut( )
	}

	function HandleInput( gamepad )
	{
		if( noInput )
			return
			
		// Handle two-player
		if( twoPlayerController && twoPlayerController.HandleInput( gamepad, !waitForSessionCreation ) )
			return true
		
		return ::LevelSelectDisplay.HandleInput( gamepad, waitForSessionCreation ) 
	}
	
	function HandleCanvasEvent( event )
	{
		if( event.Id == ON_PLAYER_NO_LIVE )
		{
			// If this is for the same player as user
			if( IntEventContext.Convert( event.Context ).Int == ::GameApp.WhichPlayer( user ) )
			{
				if( twoPlayerController.user2 && !twoPlayerController.user2.IsLocal )
					twoPlayerController.DropPlayer2( )
				
				WarnNoLive( )
				return true
			}
		}
		else if( event.Id == ON_PLAYER_YES_LIVE )
		{
			// If this is for the same player as user
			if( IntEventContext.Convert( event.Context ).Int == ::GameApp.WhichPlayer( user ) )
			{
				StartXblaSession( )
				return true
			}
		}
		else if( event.Id == ON_PARTY_MEMBERS_CHANGE )
		{
			SetControls( )
			return true
		}
		else if( event.Id == SESSION_CREATED )
		{
			waitForSessionCreation = true
			SetControls( )
			return true
		}

		if( twoPlayerController.HandleCanvasEvent( event ) )
			return true
		
		return ::LevelSelectDisplay.HandleCanvasEvent( event )
	}
	
	function OnTick( dt )
	{
		::LevelSelectDisplay.OnTick( dt )
		
		if( waitingTimer != null && !waitForSessionCreation )
		{
			waitingTimer += dt
			if( waitingTimer > timeOut )
			{
				waitingTimer = null
				WarnNoLive( )
			}
		}
		
		local invitesAllowed = twoPlayerController.InvitesAllowed( )
		if( inviteAllowPrev != invitesAllowed )
		{
			SetControls( )
			inviteAllowPrev = invitesAllowed
		}
	}
	
	function SetControls( )
	{
		local wideLanguage = ::GameApp.IsWideLanguage( )
		local asianLanguage = ::GameApp.IsAsianLanguage( )
		
		controls.Clear( )
		local controlA = null
		if( !scores || scores.active || scores.AutoLaunch( ) )
			controlA = controls.AddControl( "Gui/Textures/Gamepad/button_a_g.png", "CustomMatch_StartGame" )
		else
			controlA = controls.AddControl( "Gui/Textures/Gamepad/button_a_g.png", "Menus_Select" )
				
		if( !waitForSessionCreation )
			controlA.SetRgba( 0.5, 0.5, 0.5, 1.0 )
		
		controls.AddControl( "Gui/Textures/Gamepad/button_b_g.png", "Menus_Back" )
		
		local haveDlc = dlc != null && ( !scores || !scores.active ) && ( player.HasDLC( DLC_NAPALM ) || player.HasDLC( DLC_EVIL_EMPIRE ) )
		
		navControls.Clear( )
		if( haveDlc )
			navControls.AddControl( [ "gui/textures/gamepad/button_lshoulder_g.png", "gui/textures/gamepad/button_rshoulder_g.png" ], "Select_Campaign" )
		
		if( twoPlayerController.user2 && ( twoPlayerController.user2.IsOnlineEnabled || !twoPlayerController.user2.IsLocal ) )
			navControls.AddControl( GAMEPAD_BUTTON_SELECT, "LB_Menus_Gamer_Card" )
			
		navControls.AddControl( "gui/textures/gamepad/button_y_g.png", "Menus_ViewLeaderboards" )
		
		if( twoPlayerController.InvitesAllowed( ) && waitForSessionCreation )
		{
			local inviteControls = ( ( wideLanguage || asianLanguage ) ? navControls : controls )
			if( !twoPlayerController.user1.IsInActiveParty )
				inviteControls.AddControl( "gui/textures/gamepad/button_x_g.png", "Invite_Friend" )
			else
				inviteControls.AddControl( "gui/textures/gamepad/button_x_g.png", "Invite_Party" )	
		}
	}
	
	function IsCoop( )
	{
		local twoPlayer = twoPlayerController.HasTwoPlayers( )
		local coOp = twoPlayer && ( mapType != MAP_TYPE_HEADTOHEAD )
		return coOp
	}
	
	function FillLevelLoadInfo( info )
	{
		if( scores )
			scores.FillLoadInfo( info )
		
		if( IsCoop( ) ) 
			info.GameMode.AddCoOpFlag( )
	}		
		
	function LoadLevel( front, levelIndex )
	{
		local twoPlayer = twoPlayerController.HasTwoPlayers( )
		local allowedToStart = twoPlayerController.AllowedToStart( )
		local coOp = IsCoop( )
		
		if( !allowedToStart )
			return false
			
		if( mapType == MAP_TYPE_HEADTOHEAD && !twoPlayer )
			return false
		
		if( scores && scores.ModeLocked( ) )
			return false
			
		noInput = true
		
		if( twoPlayer )
		{
			if( !twoPlayerController.user2.IsLocal )
				::GameAppSession.StartMap( front, levelIndex, FillLevelLoadInfo.bindenv( this )  )
			else
				::GameApp.LoadLevel( front, levelIndex, FillLevelLoadInfo.bindenv( this ) )
		}
		else
		{
			::GameApp.LoadLevel( front, levelIndex, FillLevelLoadInfo.bindenv( this ) )
		}

		if( twoPlayer && twoPlayerController.user2.IsLocal )
		{
			//::print( "local coop game starting" )
			SetLevelStarting( ::GameApp.WhichUser( twoPlayerController.user2 ) )
		}
		else if( twoPlayer && !twoPlayerController.user2.IsLocal )
		{
			//::print( "net coop game starting" )
			SetLevelStarting( "remote" )
		}
		else
		{
			//::print( "local singleplayer game starting" )
			SetLevelStarting( null )
		}
		return true
	}
	
	function AllowedToForceAdd( user, remoteAdd = false )
	{
		if( is_null( user ) )
		{
			::print( "user is null" )
			return false
		}
		if( !remoteAdd && !user.IsLocal )
		{
			::print( "user is not local (tried to add local user)" )
			return false
		}
		else if( remoteAdd && user.IsLocal )
		{
			::print( "user is local (tried to add remote user)" )
			return false
		}
		if( !remoteAdd && !user.SignedIn )
		{
			::print( "user is not signed in" )
			return false
		}
		
		return true
	}
	
	function AddLocalUser2( user )
	{
		if( !AllowedToForceAdd( user, false ) )
			return
			
		twoPlayerController.ForceAddUser2( user )
	}
	
	function AddRemoteUser2( user )
	{
		if( !AllowedToForceAdd( user, true ) )
			return
		
		twoPlayerController.ForceAddUser2( user )
	}
	
	function AddLocalUser1( user )
	{
		if( !AllowedToForceAdd( user, false ) )
			return
			
		twoPlayerController.ForceAddUser1( user )
	}
	
	function AddRemoteUser1( user )
	{
		if( !AllowedToForceAdd( user, true ) )
			return
		
		twoPlayerController.ForceAddUser1( user )
	}
}

class CampaignLevelSelectMenu extends LevelSelectMenuBase
{
	constructor( dlc, client = false )
	{
		if( !::GameApp.IsFullVersion )
			dlc = null
		LevelSelectMenuBase.constructor( MAP_TYPE_CAMPAIGN, dlc, client )
	}
}

class SurvivalLevelSelectMenu extends LevelSelectMenuBase
{
	constructor( client = false )
	{
		LevelSelectMenuBase.constructor( MAP_TYPE_SURVIVAL, null, client )
	}
}

class MinigameLevelSelectMenu extends LevelSelectMenuBase
{
	constructor( client = false )
	{
		LevelSelectMenuBase.constructor( MAP_TYPE_MINIGAME, null, client )
	}
}

class HeadToHeadLevelSelectMenu extends LevelSelectMenuBase
{
	constructor( client = false, restriction = PlayerRestriction.None )
	{
		LevelSelectMenuBase.constructor( MAP_TYPE_HEADTOHEAD, null, client, restriction )
	}
}
