
// Imports are in usa_barrage_imports and ussr_barrage_imports

sigexport function CanvasCreateControl( worldSpaceControl )
{
	return InUseIndicator( )
}

class InUseIndicator extends Gui.CanvasFrame
{
	// Display
	indicatorIcon = null
	
	constructor( )
	{
		::Gui.CanvasFrame.constructor( )
		
		indicatorIcon = ::Gui.TexturedQuad( )
		SetIndicator( null, 0 )
		AddChild( indicatorIcon )
		
		SetZPos( 0.7 )
	}
	
	function SetIndicator( playerControl, country )
	{
		if( !is_null( playerControl ) )
		{
			if( playerControl.Country == COUNTRY_USA )
				indicatorIcon.SetTexture( "gui/textures/cursor/inuseindicatorUSA_g.png" )
			else
				indicatorIcon.SetTexture( "gui/textures/cursor/inuseindicatorUSSR_g.png" )
		}
		else
		{
			if( country == COUNTRY_USA )
				indicatorIcon.SetTexture( "gui/textures/cursor/enemyindicatorusa_g.png" )
			else 
				indicatorIcon.SetTexture( "gui/textures/cursor/enemyIndicator_g.png" )
		}
		
		local size = indicatorIcon.TextureDimensions( )
		indicatorIcon.SetPosition( -size.x * 0.5, -size.y * 0.5, 0 )
	}
	
	function Show( )
	{
		if( ::GameApp.SingleScreenCoop )
			Hide( )
		else
			indicatorIcon.SetAlpha( 1.0 )
	}
	
	function Hide( )
	{
		indicatorIcon.SetAlpha( 0.0 )
	}
}