
sigimport "Gui/Scripts/FrontEnd/common.nut"
sigimport "Gui/Scripts/Controls/SimpleList.nut"

// I THINK THIS MENU IS COMPLETELY DEPRECATED

enum FrontEndPrivateMatchState
{
	Null,
	StartingHost,
	Hosting,
	Destroying,
	Joining
}

class FrontEndPrivateMatch extends VerticalMenu
{
	state = null
	stateTimer = null
	tempImage = null
	statusText = null
	
	gameType = null
	gameMode = null
	mapType = null
	invited = false
	
	constructor( _invited = false )
	{
		VerticalMenu.constructor( )
		inputHook = true
		
		invited = _invited
		state = FrontEndPrivateMatchState.Null
		
		tempImage = Gui.TexturedQuad( )
		//tempImage.SetTexture( "gui/textures/frontend/coop_lobby_mockup_g.png" )
		tempImage.SetPosition( Math.Vec3.Construct( 0, 0, 0.2 ) )
		AddChild( tempImage )

		local bgDims = tempImage.TextureDimensions( )

		statusText = AsyncStatus( 
			"PrivateMatch_StartHost",
			"Gui/Textures/Misc/loading_g.png",
			Math.Vec3.Construct( 200, bgDims.y * 0.8, 0 ),
			AsyncStatusImageLocation.Left )
		AddChild( statusText )
	}
	function VerticalMenuFadeIn( verticalMenuStack )
	{
		VerticalMenu.VerticalMenuFadeIn( verticalMenuStack )
		statusText.ChangeText( "PrivateMatch_StartHost" )
		
		if( !invited )
			HostSession( )
		else
			JoinSession( )
	}
	function OnBackOut( )
	{
		VerticalMenu.OnBackOut( )

		switch( state )
		{
		case FrontEndPrivateMatchState.Null:
			break
		case FrontEndPrivateMatchState.StartingHost:
		case FrontEndPrivateMatchState.Hosting:
		case FrontEndPrivateMatchState.Joining:
			statusText.ChangeText( "QuickMatch_DeletingSession" )
			state = FrontEndPrivateMatchState.Destroying
			GameAppSession.CancelSession( )
			break
		}
		
		return true
	}
	function HandleCanvasEvent( event )
	{		
		switch( event.Id )
		{
			case SESSION_LOAD_LEVEL:
			case SESSION_DELETED:
				AutoExit = true
			return true
		}
		return false
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
			clientStatus.Set( CLIENTSTATE_READY )
			::GameAppSession.SendClientStateChange( clientStatus )
		}
	}
	function HandleCanvasEvent( event )
	{		
		switch( event.Id )
		{
			case SESSION_LOAD_LEVEL:
			case SESSION_DELETED:
				AutoExit = true
			return true
		}
		return false
	}
	function OnTick( dt )
	{
		VerticalMenu.OnTick( dt )
	}
	
	function HostSession( )
	{
		statusText.ChangeText( "PrivateMatch_StartHost" )
		state = FrontEndPrivateMatchState.StartingHost
	}
	
	function JoinSession( )
	{
		statusText.ChangeText( "PrivateMatch_Join" )
		state = FrontEndPrivateMatchState.Joining
	}
}

class FrontEndCoopPrivateMatch extends FrontEndPrivateMatch
{
	constructor( _gameType, _invited = false )
	{
		::FrontEndPrivateMatch.constructor( _invited )
		
		gameType = _gameType
		gameMode = CONTEXT_GAME_MODE_SURVIVAL
		mapType = MAP_TYPE_SURVIVAL
	}
	
	function HostSession( )
	{
		::FrontEndPrivateMatch.HostSession( )
		::GameAppSession.HostGame( ::GameApp.FrontEndPlayer.User, gameMode, gameType )
	}
	
}

class FrontEndHeadToHeadPrivateMatch extends FrontEndPrivateMatch
{
	constructor( _gameType, _invited = false )
	{
		::FrontEndPrivateMatch.constructor( _invited )
		
		gameType = _gameType
		gameMode = CONTEXT_GAME_MODE_VERSUS
		mapType = MAP_TYPE_HEADTOHEAD
	}
	
	function HostSession( )
	{
		::FrontEndPrivateMatch.HostSession( )
		::GameAppSession.HostGame( ::GameApp.FrontEndPlayer.User, gameMode, gameType )
	}	
}

class FrontEndCampaignPrivateMatch extends FrontEndPrivateMatch
{
	constructor( _gameType, _invited = false )
	{
		::FrontEndPrivateMatch.constructor( _invited )
		
		gameType = _gameType
		gameMode = CONTEXT_GAME_MODE_CAMPAIGN
		mapType = MAP_TYPE_CAMPAIGN
	}
}

