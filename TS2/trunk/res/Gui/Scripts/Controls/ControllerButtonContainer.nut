// Controller button container to control lists of controls (like at the bottom of menus or for unit controls) 

class ControllerButtonContainer extends AnimatingCanvas
{
	// Display
	controls = null // array of ControllerButton objects
	
	// Data
	padding = null // number, pixels between controls
	font = null
	buttonScale = null // number, scale of whole button
	sorted = null
	Size = null
	
	constructor( font_ = FONT_SIMPLE_MED, padding_ = 10, buttonScale_ = 1.0, isSorted = false )
	{
		::AnimatingCanvas.constructor( )
		
		controls = [ ]
		Size = ::Math.Vec2.Construct( 0, 0 )
		
		padding = padding_
		font = font_
		buttonScale = buttonScale_
		SetSorted( isSorted )
	}
	
	function SetSorted( isSorted )
	{
		sorted = isSorted
	}
	
	function Control( index )
	{
		if( index >= 0 && index < controls.len( ) )
			return controls[ index ]
		else
			return null
	}
	
	function AddControl( buttonTexture, locID, combo = false )
	{
		local control = ::ControllerButton( buttonTexture, locID, ControllerButtonAlignment.LEFT, font, combo )
		AddControlObject( control )
		return control
	}
	
	function AddControlObject( control )
	{
		control.SetScale( Math.Vec2.Construct( buttonScale, buttonScale ) )
		AddChild( control )
		
		controls.push( control )
		
		if( sorted )
			controls.sort( )
		
		Reposition( )
	}
	
	function Clear( )
	{
		foreach( c in controls )
		{
			RemoveChild( c )
			c.DeleteSelf( )
		}
		
		controls.clear( )
	}
	
	function Reposition( )
	{
		// Reposition controls
		Size = ::Math.Vec2.Construct( 0, 0 )
		local x = 0
		foreach( c in controls )
		{
			local cSize = c.GetSize( )
			c.SetPosition( Math.Vec3.Construct( x, 0, 0 ) )
			x += cSize.Width + padding
			Size.x += cSize.Width + padding
			Size.y = ::Math.Max( Size.y, cSize.Height )
		}
		
		if( controls.len( ) > 0 )
			Size.x -= padding
	}
}