// Simple checkbox
sigimport "gui/textures/frontend/checkbox_checked_g.png"
sigimport "gui/textures/frontend/checkbox_unchecked_g.png"

class Checkbox extends Gui.CanvasFrame
{
	// Status
	checked = null // bool
	
	// Events
	onChecked = null // function( ), callback
	onUnchecked = null // function( ), callback
	onCheckStatusChanged = null // function( bool ), callback
	
	// Display
	image = null // Gui.TexturedQuad
	
	constructor( )
	{
		::Gui.CanvasFrame.constructor( )
		
		checked = false
		
		image = ::Gui.TexturedQuad( )
		image.SetTexture( "gui/textures/frontend/checkbox_unchecked_g.png" )
		AddChild( image )
	}
	
	function IsChecked( )
	{
		return checked
	}
	
	function SetCheckedStatus( checked_ )
	{
		checked = checked_
		
		if( checked )
		{
			if( onChecked ) onChecked( )
			image.SetTexture( "gui/textures/frontend/checkbox_checked_g.png" )
		}
		else
		{
			if( onUnchecked ) onUnchecked( )
			image.SetTexture( "gui/textures/frontend/checkbox_unchecked_g.png" )
		}
		
		if( onCheckStatusChanged ) onCheckStatusChanged( checked )
	}
	
	function Check( )
	{
		SetCheckedStatus( true )
	}
	
	function Uncheck( )
	{
		SetCheckedStatus( false )
	}
}