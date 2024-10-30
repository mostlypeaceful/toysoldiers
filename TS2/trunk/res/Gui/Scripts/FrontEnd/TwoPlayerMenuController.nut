// Two Player Menu Controller

// Requires
sigimport "gui/scripts/frontend/levelselect/profilebadge.nut" 
sigimport "gui/scripts/frontend/lobbycontroller.nut"

enum PlayerRestriction
{
	None,
	NetOnly,
	LocalOnly
}

class TwoPlayerUserDisplay extends AnimatingCanvas
{
	// Display
	profileBadge = null // array of ProfileBadge objects
	
	// Data
	primaryUser = null
	
	constructor( localUser, user1, user2 = null )
	{
		::AnimatingCanvas.constructor( )
		
		profileBadge = [ ]
		primaryUser = localUser
		
		local vpSafeRect = ::GameApp.FrontEndPlayer.User.ComputeScreenSafeRect( )
		
		local badge1 = ::ProfileBadge( )
		badge1.SetPosition( 0, 0, 0 )
		badge1.SetUser( primaryUser, user1 )
		badge1.EnableControl( false )
		AddChild( badge1 )
		profileBadge.push( badge1 )
		
		local badge2 = ::ProfileBadge( )
		badge2.SetPosition( ::Math.Vec3.Construct( vpSafeRect.Width - badge2.size.x, 0, 0 ) )
		badge2.SetUser( primaryUser, user2 )
		AddChild( badge2 )
		profileBadge.push( badge2 )
	}
	
	function ShowUser( index, show )
	{
		if( index in profileBadge )
			profileBadge[ index ].Invisible = !show
	}
	
	function SetUser( index, user )
	{
		if( index in profileBadge )
			profileBadge[ index ].SetUser( primaryUser, user )
	}
	
	function ClearUser( index )
	{
		if( index in profileBadge )
			profileBadge[ index ].SetInactive( index )
	}
	
	function SetStatus( index, status )
	{
		if( index in profileBadge )
			profileBadge[ index ].SetStatus( status )
	}
	
	function ProfileBadge( index )
	{
		if( index in profileBadge )
			return profileBadge[ index ]
		else
			return null
	}
	
	function EnableControls( enable )
	{
		foreach( badge in profileBadge )
			badge.EnableControl( enable )
	}
	
	function Unload( )
	{
		foreach( badge in profileBadge )
			badge.Unload( )
	}
}

class TwoPlayerMenuController extends AnimatingCanvas
{
	// Display
	display = null
	
	// Data
	user1 = null // User
	user2 = null // User
	onUserChange = null
	singlePlayer = null
	lobbyController = null
	isHost = null
	levelStarting = null
	gamepad2 = null
	dialogOpen = null
	restriction = null
	
	// Events
	onAddClient = null
	onClientDropped = null
	
	constructor( primaryUser, host = true, restriction_ = PlayerRestriction.None )
	{
		::AnimatingCanvas.constructor( )
		user1 = primaryUser
		user2 = null
		singlePlayer = false
		isHost = host
		levelStarting = false
		dialogOpen = false
		restriction = restriction_
		audioSource = ::GameApp.GetPlayerByUser( user1 ).AudioSource
		
		if( !::GameApp.GameMode.IsFrontEnd && ::GameApp.GameMode.IsSinglePlayer )
			singlePlayer = true
		
		if( !::GameApp.GameMode.IsFrontEnd && !is_null( ::GameApp.SecondaryPlayer ) )
			user2 = ::GameApp.SecondaryPlayer.User

		if( !isHost )
		{
			user1 = user2
			user2 = primaryUser
			
			// Ask host for current menu
			local clientStatus = ::ClientStateChange( )
			clientStatus.Set( CLIENTSTATE_MENUSTATEREQUEST )
			::GameAppSession.SendClientStateChange( clientStatus )
		}
		
		display = ::TwoPlayerUserDisplay( ::GameApp.FrontEndPlayer.User, user1, user2 )
		lobbyController = ::LobbyController( display, user1, user2 )
		
		if( singlePlayer )
			display.ShowUser( 1, false )
		
		AddChild( display )
	}
	
	function Unload( )
	{
		display.Unload( )
	}
	
	function InvokeOnUserChange( )
	{
		if( onUserChange )
			onUserChange( )
	}

	function OnTick( dt )
	{
		::AnimatingCanvas.OnTick( dt )
		
		if( ::GameAppSession.LoadingLevel || restriction == PlayerRestriction.NetOnly )
			return
		
		//::print( "isHost:" + isHost.tostring( ) + " singlePlayer:" + singlePlayer.tostring( ) + " IsFrontEnd:" + ::GameApp.GameMode.IsFrontEnd.tostring( ) + " dialogOpen:" + dialogOpen.tostring( ) )
		if( isHost && !singlePlayer && ::GameApp.GameMode.IsFrontEnd && !dialogOpen )
		{
			//::print( "user2 " + ( ( user2 )? "exists": "null" ) )
			// Check if any other users exist
			if( !user2 )
			{
				// Get user2
				local userId = ::Application.FindActiveLocalUser( GAMEPAD_BUTTON_A )
				if( userId == ~0 )
					return
				
				// Make sure it's not the first player
				if( userId == ::GameApp.WhichUser( user1 ) )
					return 
					
				::print( "new user pressed A" )
				
				local checkUser = ::GameApp.GetLocalUser( userId )
				//user2 = ::GameApp.SecondaryPlayer.User
				if( !checkUser.SignedIn )
					checkUser.ShowSignInUI( )
				else
				{
					user2 = checkUser
					AddPlayer2( )
				}
			}
			
			if( user2 && user2.IsLocal && gamepad2 && !gamepad2.IsNull( ) )
			{
				if( gamepad2.Get( ).ButtonDown( GAMEPAD_BUTTON_B ) )
				{
					DropPlayer2( )
				}
				
				else if( gamepad2.Get( ).ButtonDown( GAMEPAD_BUTTON_SELECT ) && !::GameApp.GamercardViewDisabled( ) && user1.SignedInOnline )
				{
					PlaySound( "Play_UI_Select_Forward" )
					user2.ShowGamerCardUI( user1.UserId )
				}
			}
		}
	}
	
	function AddPlayer2( )
	{
		::GameAppSession.AddLocalUserToSession( user2 )
		SetLocalPlayer2( )
	}
	
	function DropPlayer2( )
	{
		if( !is_null( user2 ) )
		{
			if( user2.IsLocal )
				::GameAppSession.RemoveLocalUserFromSession( user2 )
		}
		ClearPlayer2( )
		::GameApp.AudioEvent( "Play_HUD_MP_PlayerDrop" )
	}
	
	function HandleCanvasEvent( event )
	{
		if( event.Id == SESSION_INVITE_ACCEPTED )
		{
			::GameApp.SetPlayerFromUser( 1, null )
		}
		
		if( !singlePlayer )
		{
			if( isHost && event.Id == ON_PLAYER_SIGN_OUT  && ::GameApp.GameMode.IsFrontEnd )
			{
				if( user2 && IntEventContext.Convert( event.Context ).Int == ::GameApp.WhichPlayer( user2 ) )
				{
					HandleClientDropped( )
					return true
				}
			}
			else if( isHost && event.Id == ON_DISCONNECT && ::GameApp.GameMode.IsFrontEnd )
			{
				if( user2 )
				{
					HandleClientDropped( )
				}
			}
			else if( event.Id == SESSION_USERS_CHANGED )
			{
				if( BoolEventContext.Convert( event.Context ).Bool == true )
				{
					if( isHost && !user2 )
					{
						// Assume client player
						if( ::GameAppSession.RemoteUserCount > 0 && ::GameApp.GameMode.IsFrontEnd )
						{
							::print( "adding remote user2" )
							user2 = ::GameAppSession.RemoteUser( 0 )
							SetLocalPlayer2( )
						}
						else if( ::GameAppSession.LocalUserCount > 1 && ::GameApp.GameMode.IsFrontEnd )
						{
							local newUser = ::GameAppSession.LocalUser( 1 )
							
							if( !::GameApp.SetPlayerLocalUser( 1, newUser.LocalHwIndex ) )
							{
								LogWarning( 0, "Tried to add second player in level select screen, failed", true )
								return
							}
							
							::print( "adding local user2" )
							user2 = newUser
							SetLocalPlayer2( )
						}
						// Here we shouldn't have a second user
						else if( user2 )
						{
							HandleClientDropped( )
						}
					}
					else if( !isHost )
					{
						// Assume host player "joined"
						if( ::GameAppSession.RemoteUserCount > 0 )
						{
							::print( "adding remote user1" )
							user1 = ::GameAppSession.RemoteUser( 0 )
							SetLocalPlayer1( )
						}
					}
				}
				else if( user2 )
				{
					::GameAppSession.AddLocalZombieToSession( user2 )
					//if( user2.IsOnlineEnabled )
					//	WarnNoLive( )
				}
				return true
			}
			else if( event.Id == LOBBY_CLIENT_STATE_CHANGE )
			{
				local context = ::ObjectEventContext.Convert( event.Context ).Object
				
				if( ::GameAppSession.IsHost )
				{
					switch( context.State )
					{
						case CLIENTSTATE_DROPPED:
							HandleClientDropped( )
						break
						
						case CLIENTSTATE_READY:
							//::print( "client ready" )
							HandleClientReady( )
						return true
						
						case CLIENTSTATE_UNREADYREQUEST:
							//::print( "client unready request, levelStarting:" + levelStarting.tostring( ) )
							HandleClientRequestUnready( )
						return true
						
						case CLIENTSTATE_MENUSTATEREQUEST:
							if( onAddClient )
								onAddClient( )
						return true
					}
				}
			}
		}
		
		return false
	}
	
	function ForceAddUser1( newUser )
	{
		if( user1 )
		{
			//SHIT
			::print("ForceAddUser1  when there is already a user1!!!");
			DumpCallstack( )
			//BreakPoint( )
		}
		user1 = newUser
		SetLocalPlayer1( )
	}
	
	function ForceAddUser2( newUser )
	{
		if( user2 )
		{
			//WTF
			::print("ForceAddUser2  when there is already a user2222222!!!");
			DumpCallstack( )
			//BreakPoint( )
		}
		user2 = newUser
		
		if( user2.SignedInOnline && user2.HasPrivilege( PRIVILEGE_MULTIPLAYER ) )
		{
			::print( "force add local gold user" )
			if( user2.IsLocal && ::GameAppSession.LocalUserCount < 2 )
				::GameAppSession.AddLocalUserToSession( user2 )
		}
		else
		{
			if( user2.IsLocal && ::GameAppSession.LocalUserCount < 2 )
			{
				::print( "force add local zombie" )
				::GameAppSession.AddLocalZombieToSession( user2 )
			}
		}
		SetLocalPlayer2( )
	}
	
	function HandleClientDropped( )
	{
		::print( "client dropped" )
		::GameApp.AudioEvent( "Play_HUD_MP_PlayerDrop" )
		lobbyController.DropClient( )
		ClearPlayer2( )
		if( onClientDropped )
			onClientDropped( )
	}
	
	function HandleClientReady( )
	{
		if( ::GameAppSession.IsHost && user2 && !user2.IsLocal )
		{
			lobbyController.SetClientReady( true )
		}
		else
		{
			::print( "Client was set ready... but this isn't the host or the user is not valid" )
		}
	}
	
	function HandleClientRequestUnready( )
	{
		if( !levelStarting )
		{
			local clientStatus = ::ClientStateChange( )
			clientStatus.Set( CLIENTSTATE_ALLOWUNREADY )
			::GameAppSession.SendClientStateChange( clientStatus )
			
			lobbyController.SetClientReady( false )
		}
	}
	
	function GetDlcMask( player )
	{
		local ret = 1
		if( player.HasDLC( DLC_EVIL_EMPIRE ) )
			ret = ret | 2
		if( player.HasDLC( DLC_NAPALM ) )
			ret = ret | 4
		return ret
	}
	
	function HandleMenuUpdate( highlight, scoresActive, scoresHighlight, extra, player )
	{
		if( ::GameAppSession.IsHost && ::GameApp.GameMode.IsFrontEnd )
		{
			local menuStatus = ::LevelSelectStatus( )
			local hasDlcMask = GetDlcMask( player )
			//::print( "HasDlcMask:" + hasDlcMask.tostring() )
			menuStatus.Set( highlight, scoresActive, scoresHighlight, extra, hasDlcMask )
			::GameAppSession.SendLevelSelectStatus( menuStatus )
			
			//::print( "highlight:" + highlight.tostring( ) + " scoresActive:" + scoresActive.tostring( ) + " scoresHighlight:" + scoresHighlight.tostring( ) + " extra:" + extra.tostring( ) )
		}
	}
	
	function WarnNoLive( )
	{	
		local dialogBox = ::ModalConfirmationBox( "Menus_NoXboxLiveNoStats", user1, "Accept", "Kick_Player" )
		dialogOpen = true

		dialogBox.onBPress = function( )
		{
			dialogOpen = false

			DropPlayer2( )
		}.bindenv(this)
		
		dialogBox.onAPress = function( )
		{
			dialogOpen = false
		}.bindenv(this)
	}
	
	function InvitesAllowed( )
	{
		if( !::GameAppSession.IsOnline )
			return false
		if( !::GameAppSession.IsHost )
			return false
		if( singlePlayer )
			return false
		if( !::GameApp.GameMode.IsFrontEnd )
			return false
		if( user2 )
			return false
		if( ::GameApp.PAXDemoMode )
			return false
		if( ::GameApp.E3Mode )
			return false
		if( !::GameApp.IsFullVersion )
			return false
		if( restriction == PlayerRestriction.LocalOnly )
			return false
			
		return true
	}
	
	function ShowUser1Gamercard( )
	{
		if( user1.SignedInOnline )
			::GameApp.FrontEndPlayer.User.ShowGamerCardUI( user1.UserId )
	}
	
	function ShowUser2Gamercard( )
	{
		if( user2.SignedInOnline )
			::GameApp.FrontEndPlayer.User.ShowGamerCardUI( user2.UserId )
	}
	
	function CanViewGamercard( )
	{
		if( isHost )
			return ( user2 != null )
		else
			return ( user1 != null )
	}
	
	function HostUser( )
	{
		if( isHost )
			return user1
		else
			return user2
	}
	
	function HandleInput( gamepad, noInvites = false )
	{
		if( InvitesAllowed( ) && !noInvites && gamepad.ButtonDown( GAMEPAD_BUTTON_X ) )
		{
			PlaySound( "Play_UI_Select_Forward" )
			user1.ShowInviteUI( )
		}
		
		if( gamepad.ButtonDown( GAMEPAD_BUTTON_SELECT ) && HostUser( ).SignedInOnline && !::GameApp.GamercardViewDisabled( ) )
		{
			::print( "pressed back" )
			if( isHost && user2 )
			{
				PlaySound( "Play_UI_Select_Forward" )
				ShowUser2Gamercard( )
			}
			else if( !isHost && user1 )
			{
				PlaySound( "Play_UI_Select_Forward" )
				ShowUser1Gamercard( )
			}
		}
		
		return false
	}
	
	function HasTwoPlayers( )
	{
		return ( user1 && user2 )
	}
	
	function AllowedToStart( )
	{
		if( user2 && !user2.IsLocal )
			return lobbyController.IsClientReady( )
		else
			return true
	}
	
	function SetLocalPlayer1( )
	{	
		display.SetUser( 0, user1 )
		::GameApp.SetPlayerFromUser( 1, user1 )
		
		InvokeOnUserChange( )
		if( !user1.IsLocal )
		{
			display.ProfileBadge( 0 ).SetStatus( PROFILEBADGE_HOST )
		}
		else
		{
			display.ProfileBadge( 0 ).SetStatus( PROFILEBADGE_BLANK )
			display.ProfileBadge( 1 ).SetStatus( PROFILEBADGE_BLANK )
		}
	}
	
	function ClearPlayer1( )
	{
		user1 = null
		//::GameApp.SetPlayerFromUser( 0, null )
		display.ClearUser( 0 )
		InvokeOnUserChange( )
		display.ProfileBadge( 0 ).SetStatus( PROFILEBADGE_BLANK )
		display.ProfileBadge( 1 ).SetStatus( PROFILEBADGE_BLANK )
	}
	
	function SetLocalPlayer2( )
	{	
		::GameApp.AudioEvent( "Play_HUD_MP_PlayerEntered" )
		display.SetUser( 1, user2 )
		::GameApp.SetPlayerFromUser( 1, user2 )
		InvokeOnUserChange( )
		if( !user2.IsLocal )
		{
			lobbyController.SetClientJoin( user2 )
			display.ProfileBadge( 0 ).SetStatus( PROFILEBADGE_HOST )
			display.ProfileBadge( 1 ).SetStatus( PROFILEBADGE_NOTREADY )
		}
		else
		{
			if( gamepad2 )
			{
				gamepad2.Release( )
				gamepad2 = null
			}
			gamepad2 = ::FilteredGamepad( user2, false )
			display.ProfileBadge( 0 ).SetStatus( PROFILEBADGE_BLANK )
			display.ProfileBadge( 1 ).SetStatus( PROFILEBADGE_BLANK )
		}
	}
	
	function ClearPlayer2( )
	{
		if( gamepad2 )
		{
			gamepad2.Release( )
			gamepad2 = null
		}
		user2 = null
		::GameApp.SetPlayerFromUser( 1, null )
		display.ClearUser( 1 )
		InvokeOnUserChange( )
		display.ProfileBadge( 0 ).SetStatus( PROFILEBADGE_BLANK )
		display.ProfileBadge( 1 ).SetStatus( PROFILEBADGE_BLANK )
		display.ProfileBadge( 1 ).EnableControl( true )
		if( lobbyController )
			lobbyController.DropClient( )
	}
	
	function OnBackOut( )
	{
		if( gamepad2 )
		{
			gamepad2.Release( )
			gamepad2 = null
		}
		::GameApp.SetPlayerFromUser( 1, null )
	}
}
