// Autosave Notice Screen

class AutosaveNoticeScreen extends AnimatingCanvas
{
	constructor( )
	{
		::AnimatingCanvas.constructor( )
		local screenRect = ::GameApp.ComputeScreenSafeRect( )
		
		// Text
		local text = ::Gui.Text( )
		text.SetFontById( FONT_FANCY_LARGE )
		text.SetRgba( COLOR_CLEAN_WHITE )
		text.BakeBoxLocString( screenRect.Width - 60, ::GameApp.LocString( "Menus_Warning01" ), TEXT_ALIGN_CENTER )
		text.SetPosition( screenRect.Center.x, screenRect.Center.y - text.Height * 0.5, 0 )
		AddChild( text )
		
		// Image
		local image = ::AsyncStatus( "Saving", "gui/textures/misc/loading_g.png", ::Math.Vec3.Construct( 540, 600, 0 ), AsyncStatusImageLocation.Left )
		image.SetPosition( screenRect.Center.x - image.LocalRect.Width * 0.5, screenRect.Center.y + text.Height * 0.5 + 50, 0 )
		AddChild( image )
	}
}