
sigimport "Gui/Scripts/FrontEnd/common.nut"
sigimport "Gui/Scripts/Controls/SimpleList.nut"
sigimport "gui/textures/frontend/lobby_progress_g.png"
sigimport "gui/scripts/frontend/campaignlevelselectlobby.nut"
sigimport "Gui/Scripts/DialogBox/globalmodaldialogbox.nut"

enum FrontEndInviteLobbyState
{
	Null,
	Joining,
	Joined,
	Failed
}

class FrontEndInviteLobby extends FrontEndStaticScreen
{
	state = null
	stateTimer = null
	statusText = null
	
	constructor( )
	{
		::FrontEndStaticScreen.constructor( )
		SetTitle( "InviteLobby_JoiningSession" )
		state = FrontEndInviteLobbyState.Null
		
		local vpSafeRect = ::GameApp.FrontEndPlayer.ComputeViewportSafeRect( )
		
		SetZPos( 0.3 )
		
		local bg = Gui.TexturedQuad( )
		bg.SetTexture( "gui/textures/frontend/lobby_progress_g.png" )
		bg.CenterPivot( )
		bg.SetPosition( vpSafeRect.Center.x, vpSafeRect.Center.y, 0.001 )
		AddChild( bg )

		local textPos = ::Math.Vec3.Construct( vpSafeRect.Center.x - 176, vpSafeRect.Center.y + 74, 0 )
		statusText = AsyncStatus( "InviteLobby_JoiningSession", "Gui/Textures/Misc/loading_g.png", textPos, AsyncStatusImageLocation.Left )
		AddChild( statusText )
		
		controls.Clear( )
		controls.AddControl( "gui/textures/gamepad/button_b_g.png", "Menus_Back" )
		
		BackButtons = GAMEPAD_BUTTON_B
	
	}
	function VerticalMenuFadeIn( verticalMenuStack )
	{
		::FrontEndStaticScreen.VerticalMenuFadeIn( verticalMenuStack )
		
		if( !::GameApp.FrontEndPlayer.User.SignedInOnline )
		{
			WarnFailure( "Menus_OnDisconnect" )
			::GameApp.ClearInvite( )
			return
		}
		
		if( state == FrontEndInviteLobbyState.Null )
			JoinSession( )
		else
			AutoBackOut = true
	}
	function OnBackOut( )
	{
		::FrontEndStaticScreen.OnBackOut( )

		switch( state )
		{
		case FrontEndInviteLobbyState.Null:
			break
		case FrontEndInviteLobbyState.Joining:
			::GameAppSession.CancelSession( )
			break
		}
		
		return true
	}
	function SelectActiveIcon( )
	{
		return true
	}
	function HandleCanvasEvent( event )
	{	
		switch( event.Id )
		{
		case SESSION_CREATED:
			if( BoolEventContext.Convert( event.Context ).Bool == true )
				MoveToLobby( )
			else
				WarnFailure( "Menus_OnDisconnect" )
			return true
		case SESSION_REJECTED:
			WarnFailure( "Menus_Error07" )
			return true
		case ON_DISCONNECT:
			WarnFailure( "Menus_OnDisconnect" )
			return true
		case ON_PLAYER_SIGN_OUT:
			WarnFailure( "Menus_UserSignedOutReturnToTitleScreen2" )
			return true
		case ON_PLAYER_NO_LIVE:
			WarnFailure( "Menus_Error04" )
			return true
		}
		
		return false
	}
	function JoinSession( )
	{
		if( ::GameApp.JoinInviteSession( ) )
		{
			state = FrontEndInviteLobbyState.Joining
		}
		else
		{
			WarnFailure( "Menus_Error07" ) //rejected, returning to front end
		}
	}
	
	function WarnFailure( errMsg )
	{
		ClearChildren( )
		
		state = FrontEndInviteLobbyState.Failed
		
		local dialogBox = ::ModalInfoBox( errMsg, ::GameApp.FrontEndPlayer.User, "Accept" )
		
		dialogBox.onFadedOut = function( )
		{
			AutoBackOut = true
			::ResetGlobalDialogBoxSystem( )
		}.bindenv(this)
	}
	
	function MoveToLobby( )
	{
		state = FrontEndInviteLobbyState.Joined
		local gameMode = ::GameAppSession.GameMode
		local gameType = ::GameAppSession.GameType
		if( gameMode == CONTEXT_GAME_MODE_SURVIVAL )
		{
			PushNextMenu( ::LevelSelectLobby( MAP_TYPE_SURVIVAL ) )
			AutoAdvance = true
		}
		if( gameMode == CONTEXT_GAME_MODE_MINIGAME )
		{
			PushNextMenu( ::LevelSelectLobby( MAP_TYPE_MINIGAME ) )
			AutoAdvance = true
		}
		else if( gameMode == CONTEXT_GAME_MODE_VERSUS )
		{
			PushNextMenu( ::LevelSelectLobby( MAP_TYPE_HEADTOHEAD ) )
			AutoAdvance = true
		}
		else if( gameMode == CONTEXT_GAME_MODE_CAMPAIGN )
		{
			PushNextMenu( ::LevelSelectLobby( MAP_TYPE_CAMPAIGN, DLC_COLD_WAR ) )
			AutoAdvance = true
		}
	}
}
