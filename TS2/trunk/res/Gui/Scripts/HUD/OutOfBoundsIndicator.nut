// Out of bounds indicator

// Resources
sigimport "gui/textures/misc/out_of_bounds_g.png"

sigexport function CanvasCreateOutOfBoundsIndicator( cppObj )
{
	return ::OutOfBoundsIndicator( cppObj )
}

class OutOfBoundsIndicator extends AnimatingCanvas
{
	constructor( cppObj )
	{
		::AnimatingCanvas.constructor( )
		
		local text = ::Gui.Text( )
		text.SetFontById( FONT_FANCY_MED )
		text.SetRgba( 1.0, 0.0, 0.0, 1.0 )
		text.BakeLocString( ::GameApp.LocString( "b_to_exit" ), TEXT_ALIGN_CENTER )
		AddChild( text )
		
		//local image = ::Gui.TexturedQuad( )
		//image.SetTexture( "gui/textures/misc/out_of_bounds_g.png" )
		//image.CenterPivot( )
		//image.SetPosition( 0, 0, 0 )
		//AddChild( image )
				
		local user = cppObj.User
		::GameApp.HudLayer( "viewport" + user.ViewportIndex ).AddChild( this )
		local vpRect = user.ComputeViewportSafeRect( )
		SetPosition( vpRect.Center.x, vpRect.Bottom - 180, 0.4 )
		
		SetAlpha( 0 )
	}
	
	function Show( show )
	{
		ClearActions( )
		if( show )
		{
			AddAction( ::AlphaTween( GetAlpha( ), 1.0, 0.5 ) )
			AddDelayedAction( 0.5, ::AlphaPulse( 0.5, 1.0, 0.4 ) )
		}
		else
		{
			AddAction( ::AlphaTween( GetAlpha( ), 0.0, 0.5 ) )
		}
	}
}