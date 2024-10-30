// Slider

// Requires
sigimport "gui/scripts/controls/animatingcanvas.nut"

class Slider extends AnimatingCanvas
{
	// Display
	bg = null
	bar = null
	mark = null
	
	// Data
	currentValue = null
	width = null
	bgSize = null
	markSize = null
	barSize = null
	
	constructor( bgTexture, barTexture, markTexture )
	{
		::AnimatingCanvas.constructor( )
		currentValue = 0
		width = 0
		
		bg = ::Gui.TexturedQuad( )
		bg.SetTexture( bgTexture )
		bg.SetPosition( 0, 0, 0.002 )
		AddChild( bg )
		
		mark = ::Gui.TexturedQuad( )
		mark.SetTexture( markTexture )
		AddChild( mark )
		
		bar = ::Gui.TexturedQuad( )
		bar.SetTexture( barTexture )
		AddChild( bar )
		
		bgSize = bg.TextureDimensions( )
		markSize = mark.TextureDimensions( )
		barSize = bar.TextureDimensions( )
		mark.SetPosition( 0, ( bgSize.y * 0.5 ) - ( markSize.y * 0.5 ), 0 )
		bar.SetPosition( markSize.x * 0.5, ( bgSize.y * 0.5 ) - ( barSize.y * 0.5 ), 0.001 )
		
		width = bgSize.x - markSize.x
		
		SetPercent( 1.0 )
	}
	
	function SetPercent( value )
	{
		currentValue = ::Math.Clamp( value, 0.0, 1.0 )
		mark.SetXPos( ::Math.Lerp( 0, width, currentValue ) )
		bar.SetRect( ::Math.Vec2.Construct( mark.GetXPos( ) - bar.GetXPos( ) + markSize.x * 0.5, barSize.y ) )
	}
	
	function CurrentValue( )
	{
		return currentValue
	}
}