// Menu for displaying the levels available in the selected DLC

// Requires
sigimport "gui/scripts/controls/animatingcanvas.nut"
sigimport "gui/scripts/frontend/levelselect/panelhighlight.nut"

class LevelDisplay extends Gui.CanvasFrame
{
	// Display
	highlight = null // Gui.TexturedQuad
	
	constructor( startingFront, startingDlc )
	{
		::Gui.CanvasFrame.constructor( )

		highlight = ::PanelHighlight( ::Math.Vec2.Construct( 395, 223 ), 3 )
		highlight.SetPosition( ::Math.Vec3.Construct( 0, 0, -0.02 ) )
		AddChild( highlight )
		
		SetActive( true )
	}
	
	function SetActive( active )
	{
		highlight.SetActive( active )
	}
}