// Difficulty selector image that goes behind the difficulty being selected

// Requires
sigimport "gui/scripts/controls/animatingcanvas.nut"

// Resources
sigimport "gui/textures/frontend/difficultyselector_g.png"

class DifficultySelector extends AnimatingCanvas
{
	// Display
	background = null // Gui.TexturedQuad
	
	// Behaviour
	time = null
	flashSpeed = null
	show = false
	fadeOut = null
	
	constructor( height )
	{
		::Gui.CanvasFrame.constructor( )
		
		time = 0
		flashSpeed = 4.0
		fadeOut = 0.8
		
		background = ::Gui.TexturedQuad( )
		background.SetTexture( "gui/textures/frontend/difficultyselector_g.png" )
		background.SetRect( ::Math.Vec2.Construct( background.TextureDimensions( ).x, height ) )
		AddChild( background )
		
		CenterPivot( )
	}
	
	function OnTick( dt )
	{
		if( show )
		{
			time += dt
			SetAlphaClamp( ((Math.Sin( time * flashSpeed ) + 1) * 0.25) + 0.5 )
		}
		else
		{
			SetAlpha( GetAlpha( ) * fadeOut )
		}
	}
	
	function Show( visible )
	{
		if( !show ) time = 0
		show = visible
	}
}