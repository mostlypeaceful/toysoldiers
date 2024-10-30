
sigimport "Gui/Scripts/FrontEnd/common.nut"
sigimport "Gui/Scripts/Controls/SimpleList.nut"
sigimport "gui/textures/frontend/lobby_progress_g.png"

QuickMatchRandom <- SubjectiveRand

enum FrontEndQuickMatchState
{
	Null,
	Searching,
	Joining,
	Hosting,
}

class FrontEndQuickMatch extends FrontEndStaticScreen
{
	state = null
	stateTimer = null
	statusText = null
	connections = null

	gameMode = null
	mapType = null
	
	constructor( )
	{
		::FrontEndStaticScreen.constructor( )
		SetTitle( "Menus_ViewVersusRankedMatch" )
		::GameApp.InMultiplayerLobby = 1
		
		state = FrontEndQuickMatchState.Null
		stateTimer = ShortTimeOut( )
		
		local vpSafeRect = ::GameApp.ComputeScreenSafeRect( )
		
		local bg = Gui.TexturedQuad( )
		bg.SetTexture( "gui/textures/frontend/lobby_progress_g.png" )
		bg.CenterPivot( )
		bg.SetPosition( vpSafeRect.Center.x, vpSafeRect.Center.y, 0.001 )
		AddChild( bg )
		
		connections = ::SimpleList( )
		connections.SetPosition( vpSafeRect.Center.x - 180, vpSafeRect.Center.y - 80, 0 )
		AddChild( connections )

		local textPos = ::Math.Vec3.Construct( vpSafeRect.Center.x - 176, vpSafeRect.Center.y + 74, 0 )
		statusText = ::AsyncStatus( "QuickMatch_Searching", "Gui/Textures/Misc/loading_g.png", textPos, AsyncStatusImageLocation.Left )
		AddChild( statusText )
		
		controls.Clear( )
		controls.AddControl( "gui/textures/gamepad/button_b_g.png", "Menus_Back" )
		
		SetZPos( 0.4 )
	}
	
	function ShortTimeOut( )
	{
		return ::QuickMatchRandom.Float( 5, 15 )
	}
	
	function LongTimeOut( )
	{
		return 60*60*24// a long time
	}
	
	function VerticalMenuFadeIn( verticalMenuStack )
	{
		::FrontEndStaticScreen.VerticalMenuFadeIn( verticalMenuStack )
		
		if( !::GameApp.FrontEndPlayer.User.SignedInOnline )
		{
			statusText.SetAlpha( 0 )
			WarnNotOnline( )
			return
		}
		
		if( !::GameApp.FrontEndPlayer.User.HasPrivilege( PRIVILEGE_MULTIPLAYER ) )
		{
			statusText.SetAlpha( 0 )
			WarnNoPrivilege( )
			return
		}
		
		statusText.ChangeText( "QuickMatch_Searching" )
		state = FrontEndQuickMatchState.Searching
		::GameAppSession.BeginGameSearch( ::GameApp.FrontEndPlayer.User, CONTEXT_GAME_TYPE_RANKED, gameMode )
	}
	
	function WarnNoPrivilege( )
	{
		local dialogBox = ::ModalInfoBox( "Menus_NoXboxLivePrivilege", ::GameApp.FrontEndPlayer.User, "Accept" )

		dialogBox.onAPress = function( )
		{
			AutoBackOut = true
		}.bindenv(this)
		
		//::GameApp.HudRoot.AddChild( dialogBox )
	}
	
	function WarnNotOnline( )
	{
		local dialogBox = ::ModalConfirmationBox( "Menus_NotSignedInOnline", ::GameApp.FrontEndPlayer.User, "SignIn", "Cancel" )

		dialogBox.onBPress = function( )
		{
			AutoBackOut = true
		}.bindenv(this)
		
		dialogBox.onAPress = function( )
		{
			::GameApp.FrontEndPlayer.User.ShowSignInUI( )
		}
		
		//::GameApp.HudRoot.AddChild( dialogBox )
	}
	
	function OnBackOut( )
	{
		::FrontEndStaticScreen.OnBackOut( )
		::GameApp.InMultiplayerLobby = 0

		switch( state )
		{
		case FrontEndQuickMatchState.Null:
			break
		case FrontEndQuickMatchState.Searching:
			state = FrontEndQuickMatchState.Null
			::GameAppSession.CancelGameSearch( )
			break
		case FrontEndQuickMatchState.Joining:
		case FrontEndQuickMatchState.Hosting:
			::GameAppSession.CancelSession( )
			break;
		}
		
		return true
	}
	
	function NoGameWithNoLive( )
	{
		::ResetGlobalDialogBoxSystem( )
		local dialogBox = ::ModalInfoBox( "Menus_XboxLiveConnectionLost", user, "Accept" )
		AutoBackOut = true
	}
	
	function HandleCanvasEvent( event )
	{
		
		switch( event.Id )
		{
		case ON_PLAYER_NO_LIVE:
			NoGameWithNoLive( )
			return true
		case SESSION_USERS_CHANGED:
			HandleUsersChanged( )
			return true
		case SESSION_CREATED:
			if( BoolEventContext.Convert( event.Context ).Bool == false )
			{
				HostFailure( )
				::GameAppSession.CancelSession( )
				state = FrontEndQuickMatchState.Null
			}
			return true
		case SESSION_SEARCH_COMPLETE:
			if( state == FrontEndQuickMatchState.Searching )
			{
				if( ::BoolEventContext.Convert( event.Context ).Bool == true )
				{
					local firstJoinableSession = ::GameAppSession.FirstJoinableSession( )
					if( firstJoinableSession == -1 )
					{
						// no joinable session found, start my own session
						HostSession( )
					}
					else
					{
						// found a joinable session
						JoinSession( firstJoinableSession )
					}
				}
				else
				{
					HostFailure( )
					WarnNoXboxLive( )
				}
			}
			return true
		case SESSION_FILLED:
			if( state == FrontEndQuickMatchState.Hosting )
			{
				local lvlIndex = ::GameAppSession.RandomQuickMatchLevelIndex( mapType );
				
				if( lvlIndex == -1 )
				{
					print( "had to ditch quick match player because no random level could be found." )
					HostSession( )
				}
				else
					::GameAppSession.StartQuickMatchMap( mapType, lvlIndex, function( info ) { if( mapType == MAP_TYPE_SURVIVAL ) info.GameMode.AddCoOpFlag( ) }.bindenv( this ) )
			}
			return true
		case SESSION_LOAD_LEVEL:
			AutoExit = true
			return true
		case SESSION_INVITE_ACCEPTED:
			if( state == FrontEndQuickMatchState.Searching )
			{
				::GameAppSession.CancelGameSearch( )
			}
			else if( state != FrontEndQuickMatchState.Null )
			{
				::GameAppSession.CancelSession( )
			}
			return true
		}
		
		return false
	}
	
	function HostFailure( )
	{
		statusText.ChangeText( "QuickMatch_HostFailure", 380 )
		local vpSafeRect = ::GameApp.ComputeScreenSafeRect( )
		statusText.SetYPos( vpSafeRect.Center.y )
	}
	
	function WarnNoXboxLive( )
	{
		local dialogBox = ::ModalInfoBox( "Menus_NoXbox", ::GameApp.FrontEndPlayer.User, "Cancel" )
		
		dialogBox.onAPress = function( )
		{
			AutoBackOut = true
			
		}.bindenv(this)
	}
	
	function OnTick( dt )
	{
		::FrontEndStaticScreen.OnTick( dt )
		
		if( state == FrontEndQuickMatchState.Joining ||
			state == FrontEndQuickMatchState.Hosting )
		{
			stateTimer -= dt
			if( stateTimer < 0 )
			{
				::GameAppSession.CancelSession( )
				connections.Clear( )
				stateTimer = ShortTimeOut( )
				
				// go back to searching
				::print( "restart search" )
				statusText.ChangeText( "QuickMatch_Searching" )
				state = FrontEndQuickMatchState.Searching
				::GameAppSession.BeginGameSearch( ::GameApp.FrontEndPlayer.User, CONTEXT_GAME_TYPE_RANKED, gameMode )
			}
		}
	}
	
	function CreateConnectionTextItem( gamerTag )
	{
		local text = Gui.Text( )
		text.SetFontById( FONT_FANCY_MED )
		text.SetRgba( COLOR_CLEAN_WHITE )
		text.BakeLocString( gamerTag, TEXT_ALIGN_LEFT )
		text.Compact( 360 )
		
		return text
	}
	
	function HandleUsersChanged( )
	{
		connections.Clear( )
		
		local gamerTag = null
		
		// Add local users
		local localUserCount = GameAppSession.LocalUserCount
		for( local i = 0; i < localUserCount; ++i )
		{
			local user = GameAppSession.LocalUser( i )
			gamerTag = user.GamerTag
			connections.AddItem( CreateConnectionTextItem( gamerTag ) )
		}
	
		// Add remote users
		local remoteUserCount = GameAppSession.RemoteUserCount
		for( local i = 0; i < remoteUserCount; ++i )
		{
			local remoteUser = GameAppSession.RemoteUser( i )
			gamerTag = remoteUser.GamerTag
			connections.AddItem( CreateConnectionTextItem( gamerTag ) )
		}
		
		if( connections.ItemCount( ) == 1 )
			stateTimer = ShortTimeOut( )
		else
			stateTimer = LongTimeOut( )
	}
	
	function JoinSession( sessionId )
	{
		statusText.ChangeText( "QuickMatch_Joining" )
		state = FrontEndQuickMatchState.Joining
	}
	
	function HostSession( )
	{
		statusText.ChangeText( "QuickMatch_Hosting" )
		state = FrontEndQuickMatchState.Hosting
	}
}

class FrontEndChallengeQuickMatch extends FrontEndQuickMatch
{
	constructor( )
	{
		::FrontEndQuickMatch.constructor( )
		gameMode = CONTEXT_GAME_MODE_SURVIVAL
		mapType = MAP_TYPE_SURVIVAL
	}
	
	function JoinSession( sessionId )
	{
		::FrontEndQuickMatch.JoinSession( sessionId )
		::GameAppSession.JoinRemoteSessionFromSearchIndex( ::GameApp.FrontEndPlayer.User, sessionId )
	}
	
	function HostSession( )
	{
		::FrontEndQuickMatch.HostSession( )
		::GameAppSession.HostGame( ::GameApp.FrontEndPlayer.User, gameMode, CONTEXT_GAME_TYPE_RANKED )
	}
}

class FrontEndVersusQuickMatch extends FrontEndQuickMatch
{
	constructor( )
	{
		::FrontEndQuickMatch.constructor( )
		gameMode = CONTEXT_GAME_MODE_VERSUS
		mapType = MAP_TYPE_HEADTOHEAD
	}
	
	function JoinSession( sessionId )
	{
		::FrontEndQuickMatch.JoinSession( sessionId )
		::GameAppSession.JoinRemoteSessionFromSearchIndex( ::GameApp.FrontEndPlayer.User, sessionId )
		::GameAppSession.IsQuickMatch = 1
	}
	
	function HostSession( )
	{
		::FrontEndQuickMatch.HostSession( )
		::GameAppSession.HostGame( ::GameApp.FrontEndPlayer.User, gameMode, CONTEXT_GAME_TYPE_RANKED )
		::GameAppSession.IsQuickMatch = 1
	}
}
