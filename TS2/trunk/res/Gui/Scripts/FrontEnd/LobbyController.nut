// Lobby Controller

// Requires
sigimport "gui/scripts/frontend/levelselect/profilebadge.nut" 

class LobbyController extends Gui.CanvasFrame 
{
	// Data
	userDisplay = null
	hostUser = null
	clientUser = null
	
	// Flags
	clientReady = null
	
	// Callbacks
	onClientReady = null
	onClientRequestUnready = null
	onHostAckUnready = null
	onClientQuit = null
	onHostQuit = null
	
	constructor( display, host, client = null )
	{
		::Gui.CanvasFrame.constructor( )
		userDisplay = display
		if( host )
		{
			hostUser = host
		}
		if( client )
		{
			SetClientJoin( client )
		}
	}
	
	function HandleCanvasEvent( event )
	{
		return false
	}
	
	function OnTick( dt )
	{
		::Gui.CanvasFrame.OnTick( dt )
	}
		
	function SetClientJoin( client )
	{
		clientUser = client
		userDisplay.SetUser( 1, clientUser )
		userDisplay.EnableControls( false )
		SetClientReady( false )
	}
	
	function IsClientReady( )
	{
		return ( clientUser != null && clientReady == true )
	}
	
	function SetClientReady( ready )
	{
		clientReady = ready
		if( userDisplay )
			userDisplay.SetStatus( 1, ready? PROFILEBADGE_READY: PROFILEBADGE_NOTREADY )
		
		if( ready )
			::GameApp.AudioEvent( "Play_HUD_MP_PlayerReady" )
			
		if( onClientReady )
			onClientReady( ready )
	}
	
	function DropClient( )
	{
		if( clientUser && onClientQuit )
			onClientQuit( )
			
		clientUser = null
		clientReady = false
		if( userDisplay )
			userDisplay.ClearUser( 1 )
	}
}