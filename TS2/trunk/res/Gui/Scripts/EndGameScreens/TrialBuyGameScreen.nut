// Do you want to buy this game?

// Requires
sigimport "gui/scripts/frontend/staticscreens/staticscreen.nut"
sigimport "gui/scripts/dialogbox/globalmodaldialogbox.nut"

// Resources
sigimport "gui/textures/gamepad/button_bulletpoint_g.png"

const BUYSCREEN_BULLETPOINT_COUNT = 5

class TrialBuyGameScreen extends FrontEndStaticScreen
{
	movie = null
	purchased = false
	dialogOpen = null
	backOutBehavior = null
	
	constructor( )
	{
		::FrontEndStaticScreen.constructor( )
		SetTitle( "Trial_BuyGame" )
		purchased = false
		dialogOpen = ( ::LocalGlobalDialogBoxObject != null )
		
		ForwardButtons = GAMEPAD_BUTTON_A
		BackButtons = GAMEPAD_BUTTON_B
		
		SetBackButtonText( "EndGame_Quit" )
		
		local vpSafeRect = ::GameApp.FrontEndPlayer.User.ComputeScreenSafeRect( )
		local wide = ::GameApp.IsWideLanguage( )
		
		// Fade out bg
		local fade = ::Gui.ColoredQuad( )
		fade.SetRgba( 0.0, 0.0, 0.0, 0.5 )
		fade.SetRect( ::Math.Vec2.Construct( 1280, 720 ) )
		fade.SetPosition( 0, 0, 0.06 )
		AddChild( fade )
		
		// Buy Text
		local textPos = ::Math.Vec3.Construct( vpSafeRect.Center.x, title.GetYPos( ) + 70, 0 )
		local text = ::Gui.Text( )
		text.SetFontById( FONT_FANCY_MED )
		text.SetRgba( COLOR_CLEAN_WHITE )
		text.BakeBoxLocString( 1000, ::GameApp.LocString( "Trial_BuyText" ), TEXT_ALIGN_CENTER )
		text.SetPosition( textPos )
		AddChild( text )
		
		// Bullet Points
		local bulletPointStartPos = ::Math.Vec2.Construct( vpSafeRect.Left + ( wide ? 20 : 50 ), textPos.y + 80 )
		local currentY = bulletPointStartPos.y
		local bulletSpacing = 6
		for( local i = 0; i < BUYSCREEN_BULLETPOINT_COUNT; ++i )
		{
			local bullet = ::Gui.TexturedQuad( )
			bullet.SetTexture( "gui/textures/gamepad/button_bulletpoint_g.png" )
			bullet.SetPosition( bulletPointStartPos.x, currentY + 6, 0 )
			AddChild( bullet )
			
			local bulletPoint = ::Gui.Text( )
			bulletPoint.SetFontById( FONT_SIMPLE_SMALL )
			bulletPoint.SetRgba( COLOR_CLEAN_WHITE )
			bulletPoint.SetPosition( bulletPointStartPos.x + 28, currentY, 0 )
			bulletPoint.BakeBoxLocString( ( wide ? 510 : 350 ), ::GameApp.LocString( "Trial_Bullet" + i.tostring( ) ), TEXT_ALIGN_LEFT )
			AddChild( bulletPoint )
			
			currentY += bulletPoint.Height + bulletSpacing
		}

		// Movie
		local movieScale = ( wide ? 0.4 : 0.5 )
		local movieSize = ::Math.Vec2.Construct( 1280 * movieScale, 720 * movieScale )
		local moviePos = ::Math.Vec3.Construct( vpSafeRect.Right - movieSize.x - 50, bulletPointStartPos.y + 10, 0 )
		movie = ::Gui.MovieQuad( )
		movie.SetMovie( "game:\\video\\TSColdWar_Announce_01.wmv", 1 )
		movie.SetRect( movieSize )
		movie.SetPosition( moviePos )
		AddChild( movie )
		
		// Movie Background
		local movieBG = ::Gui.ColoredQuad( )
		movieBG.SetRgba( 0, 0, 0, 1 )
		movieBG.SetRect( ::Math.Vec2.Construct( movieSize.x + 20, movieSize.y + 20 ) )
		movieBG.SetPosition( moviePos.x - 10, moviePos.y - 10, moviePos.z )
		AddChild( movieBG )
	}
	
	function SetBackButtonText( locId )
	{
		controls.Clear( )
		controls.AddControl( "gui/textures/gamepad/button_a_g.png", "Trial_BuyAndContinue" )
		controls.AddControl( "gui/textures/gamepad/button_b_g.png", locId )
	}
	
	function SelectActiveIcon( )
	{
		if( !user.SignedInOnline )
		{
			movie.Invisible = 1
			local dialog = ::ModalInfoBox( "Menus_Error08", user )
			dialog.onFadedOut = function( )
			{
				movie.Invisible = 0
			}.bindenv( this )
		}
		else
		{
			user.ShowMarketplaceUI( false )
		}
		return false 
	}
	
	function OnTick( dt )
	{
		::FrontEndStaticScreen.OnTick( dt )
		
		if( movie )
		{
			if( !dialogOpen && ::LocalGlobalDialogBoxObject != null )
				RemoveChild( movie )
			else if( dialogOpen && ::LocalGlobalDialogBoxObject == null )
				AddChild( movie )
				
			dialogOpen = ( ::LocalGlobalDialogBoxObject != null )
		}
	}
	
	function PurchaseComplete( )
	{
		if( GameApp.IsFullVersion )
		{
			if( ::GameApp.AskPlayerToBuyGameCallback )
			{
				::GameApp.AskPlayerToBuyGameCallback( )
				::GameApp.AskPlayerToBuyGameCallback = null
			}
			purchased = true
			Cleanup( )
			if( !::GameApp.GameMode.IsFrontEnd )
				AutoExit = true
		}
	}
	
	function OnBackOut( )
	{
		Cleanup( )
		
		if( purchased )
		{
			return true
		}
		else if( backOutBehavior )
		{
			if( movie )
			{
				movie.Pause( 1 )
				movie.DeleteSelf( )
			}
			return backOutBehavior( )
		}
		else
		{			
			if( ::GameApp.GameMode.IsFrontEnd )
				::GameApp.ExitGame( )
			else
			{
				AutoExit = true
				AutoBackOut = false
				::ResetGlobalDialogBoxSystem( )
				::GameApp.LoadFrontEnd( )
			}
			
			return false
		}
	}
	
	function Cleanup( )
	{
		if( movie )
		{
			movie.Pause( 1 )
			movie.DeleteSelf( )
		}
		AutoBackOut = true
	}
	
	function HandleCanvasEvent( event )
	{
		switch( event.Id )
		{
			case ON_UPGRADE_TO_FULL_VERSION:
				PurchaseComplete( )
				break;
		}
		
		return ::FrontEndStaticScreen.HandleCanvasEvent( event )
	}
}
