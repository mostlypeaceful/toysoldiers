// Text Choice with Arrows

// Requires
sigimport "gui/scripts/controls/animatingcanvas.nut"
sigimport "gui/scripts/controls/animatingtext.nut"

// Resources
sigimport "gui/textures/frontend/arrow_left_g.png"
sigimport "gui/textures/frontend/arrow_right_g.png"

class TextChoice extends AnimatingCanvas
{
	// Display
	displayText = null // AnimatingText
	displayTextOld = null //AnimatingText
	leftArrow = null // AnimatingCanvas with Gui.TexturedQuad child
	rightArrow = null // AnimatingCanvas with Gui.TexturedQuad child
	oldLocString = null
	
	// Data
	currentIndex = null
	highestIndex = null
	locStrings = null
	looping = null
	
	constructor( options, initial = 0, highest = -1, showArrows = null, width = null, looping_ = false )
	{
		::AnimatingCanvas.constructor( )
		currentIndex = 0
		highestIndex = highest
		width = (width == null)? 100: width;
		looping = looping_
		
		locStrings = [ ]
		foreach( option in options )
		{
			if( typeof option == "string" )
				locStrings.push( ::GameApp.LocString( option ) )
			else if( typeof option == "instance" ) // Assume it's a LocString
				locStrings.push( option )
			else if( typeof option == "integer" || typeof option == "float" )
				locStrings.push( ::LocString.FromCString( option.tostring( ) ) )
		}
		
		// Setup text
		displayText = ::AnimatingText( )
		
		if( locStrings.len( ) == 0 )
			displayText.BakeCString( "", TEXT_ALIGN_CENTER )
		else
		{
			oldLocString = locStrings[ currentIndex ]
			displayText.BakeLocString( oldLocString, TEXT_ALIGN_CENTER )
		}
		
		displayText.SetPosition( 0, -displayText.Height * 0.5, 0 )
		AddChild( displayText )
		
		displayTextOld = ::AnimatingText( )
		displayTextOld.SetPosition( 0, -displayText.Height * 0.5, 0 )
		AddChild( displayTextOld )

		// Setup arrows
		leftArrow = ::AnimatingCanvas( )
			local leftArrowImage = ::Gui.TexturedQuad( )
			leftArrowImage.SetTexture( "gui/textures/frontend/arrow_left_g.png" )
			leftArrow.AddChild( leftArrowImage )
			leftArrowImage.CenterPivot( )
			leftArrowImage.SetPosition( 0, 0, 0 )
		leftArrow.SetPosition( -width / 2, 0, 0 )
		AddChild( leftArrow )
		
		rightArrow = ::AnimatingCanvas( )
			local rightArrowImage = ::Gui.TexturedQuad( )
			rightArrowImage.SetTexture( "gui/textures/frontend/arrow_right_g.png" )
			rightArrow.AddChild( rightArrowImage )
			rightArrowImage.CenterPivot( )
			rightArrowImage.SetPosition( 0, 0, 0 )
		rightArrow.SetPosition( width / 2, 0, 0 )
		AddChild( rightArrow )
		
		if( locStrings.len( ) <= 1 || highest <= 0 )
			showArrows = false
		
		SetLooping( looping )
		ShowArrows( showArrows )
		GoToIndexInstant( initial )
	}
	
	function SetLooping( loop )
	{
		looping = loop
		if( looping )
		{
			leftArrow.SetAlpha( 1 )
			rightArrow.SetAlpha( 1 )
		}
		else if( !IsIndexValid( currentIndex + 1 ) )
			rightArrow.SetAlpha( 0 )
		else if( !IsIndexValid( currentIndex - 1 ) )
			leftArrow.SetAlpha( 0 )
	}
	
	function ShowArrows( show )
	{
		leftArrow.Invisible = !show
		rightArrow.Invisible = !show
	}
	
	function IsIndexValid( index )
	{
		if( highestIndex < 0 )
			return ( index < locStrings.len( ) && index >= 0 )
		else
			return ( index < locStrings.len( ) && index >= 0 && index <= highestIndex )
	}
	
	function Next( )
	{
		return Shift( 1 )
	}
	
	function Previous( )
	{
		return Shift( -1 )
	}
	
	function Shift( offset )
	{
		if( looping )
		{
			if( currentIndex + offset >= locStrings.len( ) )
				currentIndex = 0 - offset
			else if( currentIndex + offset < 0 )
				currentIndex = locStrings.len( ) - 1 - offset
		}
		
		return GoToIndex( currentIndex + offset )
	}
	
	function GoToIndex( index )
	{
		local offset = index - currentIndex;
		if( IsIndexValid( index ) && index != currentIndex )
			currentIndex = index
		else
			return false
			
		// Sound
		if( offset < 0 )
			PlaySound( "Play_UI_Select_Left" )
		else if( offset > 0 )
			PlaySound( "Play_UI_Select_Right" )
		
		// Update Text
		local nextLocString = locStrings[ currentIndex ]
		local sign = ( (offset < 0 )? -1.0: 1.0 )
		
		local swapTime = 0.3
		local swapDistance = 60
		
		if( oldLocString )
		{
			displayTextOld.ClearActions( )
			displayTextOld.BakeLocString( oldLocString, TEXT_ALIGN_CENTER )
			displayTextOld.SetAlpha( 1.0 )
			displayTextOld.SetXPos( 0.0 )
			displayTextOld.AddAction( ::XMotionTween( 0.0, -sign * swapDistance, swapTime ) )
			displayTextOld.AddAction( ::AlphaTween( 1.0, 0.0, swapTime ) )
		}

		displayText.ClearActions( )
		displayText.BakeLocString( nextLocString, TEXT_ALIGN_CENTER )
		displayText.SetAlpha( 0.0 )
		displayText.SetXPos( sign * swapDistance )
		displayText.AddAction( ::XMotionTween( sign * swapDistance, 0.0, swapTime ) )
		displayText.AddAction( ::AlphaTween( 0.0, 1.0, swapTime ) )
		
		oldLocString = locStrings[ currentIndex ]
			
		// Pulse Arrow
		local arrow = ( (offset < 0 )? leftArrow: rightArrow )
		local max = !IsIndexValid( currentIndex + 1 )
		local min = !IsIndexValid( currentIndex - 1 )
		arrow.ClearActions( )
		arrow.AddAction( ::UniformScaleTween( 1.0, 1.3, 0.1 ) )
		arrow.AddDelayedAction( 0.1, ::UniformScaleTween( 1.3, 1.0, 0.1, null, null, null, looping? null : function( canvas ):(max, min)
		{
			// Hide arrow at ends
			if( max )
				rightArrow.SetAlpha( 0 )
			else
				rightArrow.SetAlpha( 1 )
				
			if( min )
				leftArrow.SetAlpha( 0 )
			else
				leftArrow.SetAlpha( 1 )
		}.bindenv( this ) ) )
		
		return true
	}
	
	function GoToIndexInstant( index )
	{
		local offset = index - currentIndex;
		if( IsIndexValid( index ) && index != currentIndex )
			currentIndex = index
		else
			return false
			
		oldLocString = locStrings[ currentIndex ]
		
		// Update Text
		displayText.ClearActions( )
		displayText.SetAlpha( 1.0 )
		displayText.SetXPos( 0.0 )
		displayText.BakeLocString( oldLocString, TEXT_ALIGN_CENTER )
		
		// Hide Arrows
		if( !looping )
		{
			local max = !IsIndexValid( currentIndex + 1 )
			local min = !IsIndexValid( currentIndex - 1 )
			if( max )
				rightArrow.SetAlpha( 0 )
			else
				rightArrow.SetAlpha( 1 )
				
			if( min )
				leftArrow.SetAlpha( 0 )
			else
				leftArrow.SetAlpha( 1 )
		}
	}
}