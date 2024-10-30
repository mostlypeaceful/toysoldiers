// Watch the Trailer

// Requires
sigimport "gui/scripts/frontend/staticscreens/staticscreen.nut"

class WatchTrailerScreen extends FrontEndStaticScreen
{
	function FinalizeIconSetup( )
	{
		SetTitle( "Menus_PAXWatchTrailer" )
		
		controls.AddControl( "gui/textures/gamepad/button_b_g.png", "Menus_Back" )
		
		local vpSafeRect = ::GameApp.FrontEndPlayer.ComputeViewportSafeRect( )
		
		local movie = ::Gui.MovieQuad( )
		movie.SetMovie( "game:\\video\\TSColdWar_Announce_01.wmv", 1 )
		movie.SetRect( 896, 504 )
		movie.SetPosition( vpSafeRect.Center.x - movie.LocalRect.Width * 0.5, vpSafeRect.Center.y - movie.LocalRect.Height * 0.5, 0 )
		AddChild( movie )
	}
}