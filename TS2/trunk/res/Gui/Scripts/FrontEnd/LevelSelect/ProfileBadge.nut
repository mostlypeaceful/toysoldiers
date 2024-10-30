// Profile badge
// This is the name and gamer picture of a user
// It may also have extra information, like if it's inactive

// Resources
sigimport "gui/textures/frontend/default_gamerpicture_g.png"
sigimport "gui/textures/frontend/profilebadge_background_g.png"
sigimport "gui/textures/frontend/host_g.png"
sigimport "gui/textures/frontend/ready_g.png"
sigimport "gui/textures/frontend/notready_g.png"

// Consts
const PROFILEBADGE_BLANK = 0
const PROFILEBADGE_HOST = 1
const PROFILEBADGE_READY = 2
const PROFILEBADGE_NOTREADY = 3

class ProfileBadge extends Gui.CanvasFrame
{
	// Data
	gamertag = null // LocString
	active = null // bool, whether this badge is displaying a user or not
	
	// Display
	background = null // Gui.TexturedQuad
	line1 = null // Gui.Text
	line2 = null // Gui.Text
	gamerPicture = null // Gui.TexturedQuad
	unknownGamerPicture = null
	controlButton = null // Controller Button
	statusIcon = null
	
	// Dimensions
	size = Math.Vec2.Construct( 300, 72 )
	picSize = Math.Vec2.Construct( 64, 64 )
	picPadding = Math.Vec2.Construct( 4, 4 )
	
	// Colors
	line1Color = COLOR_CLEAN_WHITE
	line2Color = COLOR_CLEAN_WHITE
	
	constructor( )
	{
		::Gui.CanvasFrame.constructor( )
		
		background = ::Gui.AsyncTexturedQuad( )
		background.SetTexture( "gui/textures/frontend/profilebadge_background_g.png" )
		background.SetPosition( 0, 0, 0.05 )
		AddChild( background )
		
		unknownGamerPicture = ::Gui.AsyncTexturedQuad( )
		unknownGamerPicture.SetTexture( "gui/textures/frontend/default_gamerpicture_g.png" )
		unknownGamerPicture.SetPosition( picPadding.x, picPadding.y, 0 )
		AddChild( unknownGamerPicture )
		
		gamerPicture = ::Gui.GamerPictureQuad( )
		gamerPicture.SetPosition( picPadding.x, picPadding.y, 0 )
		AddChild( gamerPicture )
		
		statusIcon = ::Gui.TexturedQuad( )
		statusIcon.SetTexture( "gui/textures/frontend/ready_g.png" )
		statusIcon.SetPosition( picPadding.x + picSize.y - 24, picPadding.x + picSize.y - 24, -0.001 )
		AddChild( statusIcon )
		SetStatus( PROFILEBADGE_BLANK )
		
		line1 = ::Gui.Text( )
		line1.SetFontById( FONT_SIMPLE_MED )
		line1.SetRgba( line1Color ) 
		line1.SetPosition( picPadding.x * 2 + picSize.x, picPadding.y, 0 )
		line1.BakeCString( "Gamertag", TEXT_ALIGN_LEFT )
		AddChild( line1 )
		
		line2 = ::Gui.Text( )
		line2.SetFontById( FONT_SIMPLE_SMALL )
		line2.SetRgba( line1Color )
		line2.SetPosition( picPadding.x * 2 + picSize.x, picPadding.y * 2 + line1.LineHeight, 0 )
		line2.BakeCString( "Press (X) to not suck", TEXT_ALIGN_LEFT )
		line2.SetAlpha( 0.0 )
		AddChild( line2 )
		
		controlButton = ::ControllerButton( "gui/textures/gamepad/button_a_g.png", "Player1", ControllerButtonAlignment.LEFT, FONT_SIMPLE_SMALL )
		controlButton.SetPosition( 0, size.y + controlButton.GetSize( ).Height * 0.5, 0 ) 
		controlButton.SetAlpha( 0.0 )

		AddChild( controlButton )
	}
	
	function SetUser( primaryUser, badgeUser )
	{
		if( badgeUser == null || primaryUser == null )
		{
			SetInactive( )
			return 
		}
		
		SetLine1Gamertag( badgeUser.GamerTag )
		//ClearLine2( ) // For now... later, set Rank/Specialty
		gamerPicture.SetTexture( primaryUser, badgeUser, false )
		unknownGamerPicture.SetAlpha( 0 )
		if( ( badgeUser != ::GameApp.FrontEndPlayer.User ) && badgeUser.IsLocal )
			SetControl( "gui/textures/gamepad/button_b_g.png", "Quit" )
		else
			controlButton.SetAlpha( 0 )
	}
	
	function EnableControl( enable )
	{
		controlButton.Invisible = !enable
	}
	
	function SetLine1( locID )
	{
		BakeLine1( ::GameApp.LocString( locID ) )
	}
	
	function SetLine1Gamertag( locGamertag )
	{
		BakeLine1( locGamertag )
	}
	
	function BakeLine1( locString )
	{
		line1.SetFontById( FONT_SIMPLE_MED )
		line1.BakeLocString( locString )
		local availableSpace = 228
		if( line1.Width != 0 && line1.Width > availableSpace )
		{
			line1.SetFontById( FONT_SIMPLE_SMALL )
			line1.BakeLocString( locString )
			if( line1.Width != 0 && line1.Width > availableSpace )
				line1.SetScale( availableSpace / line1.Width, 1.0 )
		}
	}
	
	function SetLine2( locID )
	{
		line2.SetAlpha( 1.0 )
		line2.BakeLocString( GameApp.LocString( locID ) )
	}
	
	function SetControl( buttonPath, locID )
	{
		controlButton.Set( buttonPath, locID, ControllerButtonAlignment.LEFT, FONT_SIMPLE_SMALL )
		controlButton.Reposition( )
		controlButton.SetAlpha( 1.0 )
	}
	
	function SetInactive( playerIndex = 1 )
	{
		local unknownPlayerNames = [ "Player1", "Player2" ]
		if( playerIndex < 0 || playerIndex >= unknownPlayerNames.len( ) )
			playerIndex = 1
		SetLine1( unknownPlayerNames[ playerIndex ] )
		SetControl( "gui/textures/gamepad/button_a_g.png", "Join" )
		gamerPicture.UnsetTexture( )
		unknownGamerPicture.SetAlpha( 1 )
		SetStatus( PROFILEBADGE_BLANK )
	}
	
	function SetStatus( status )
	{
		statusIcon.Invisible = false
		background.SetRgba( 1.0, 1.0, 1.0, 1.0 )
		switch( status )
		{
			case PROFILEBADGE_HOST:
				statusIcon.SetTexture( "gui/textures/frontend/host_g.png" )
				controlButton.SetAlpha( 0.0 )
			break
			
			case PROFILEBADGE_READY:
				statusIcon.SetTexture( "gui/textures/frontend/ready_g.png" )
				background.SetRgba( 0.0, 1.0, 0.0, 1.0 )
				SetControl( "gui/textures/gamepad/button_a_g.png", "EndGame_Unready" )
				//controlButton.Invisible = false
			break
			
			case PROFILEBADGE_NOTREADY:
				statusIcon.SetTexture( "gui/textures/frontend/notready_g.png" )
				background.SetRgba( 1.0, 0.0, 0.0, 1.0 )
				SetControl( "gui/textures/gamepad/button_a_g.png", "EndGame_Ready" )
				//controlButton.Invisible = false
			break
			
			default:
				statusIcon.Invisible = true
			break
		}
	}
	
	function ClearLine2( )
	{
		line2.SetAlpha( 0.0 )
		//controlButton.SetAlpha( 0.0 )
	}
	
	function Unload( )
	{
		background.Unload( )
		gamerPicture.UnsetTexture( )
	}
}