// Yellow flashing highlight for level select screen

// Requires
sigimport "gui/scripts/controls/animatingcanvas.nut"

class PanelHighlight extends AnimatingCanvas
{
	constructor( size, highlightWidth )
	{
		::AnimatingCanvas.constructor( )
		local yellow = ::Math.Vec4.Construct( 1, 1, 0, 1 )
		
		local top = ::Gui.ColoredQuad( )
		top.SetRect( ::Math.Vec2.Construct( size.x, highlightWidth ) )
		top.SetRgba( yellow )
		AddChild( top )
		
		local bottom = ::Gui.ColoredQuad( )
		bottom.SetRect( ::Math.Vec2.Construct( size.x, highlightWidth ) )
		bottom.SetRgba( yellow )
		bottom.SetPosition( 0, size.y - highlightWidth, 0 )
		AddChild( bottom )
		
		local left = ::Gui.ColoredQuad( )
		left.SetRect( ::Math.Vec2.Construct( highlightWidth, size.y - highlightWidth * 2 ) )
		left.SetRgba( yellow )
		left.SetPosition( 0, highlightWidth, 0 )
		AddChild( left )
		
		local right = ::Gui.ColoredQuad( )
		right.SetRect( ::Math.Vec2.Construct( highlightWidth, size.y - highlightWidth * 2 ) )
		right.SetRgba( yellow )
		right.SetPosition( size.x - highlightWidth, highlightWidth, 0 )
		AddChild( right )
	}
	
	function SetActive( active )
	{
		ClearActions( )
		
		if( active )
			AddAction( AlphaPulse( 0.2, 1.0, 0.5 ) )
		else
			AddAction( AlphaTween( GetAlpha( ), 0.0, 0.2 ) )
	}
}