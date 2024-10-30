// Pro-tip animation for load screens

// Requires
sigimport "gui/scripts/utility/protipscripts.nut"

class ProTipLoadScreenAnimation extends AnimatingCanvas
{
	// Display
	tipText = null
	
	// Data
	delay = null
	startPosition = null
	order = null
	currentIndex = null
	proTipWidth = null
	
	// Statics
	static distance = 50
	static animTime = 0.3
	
	constructor( timeBetweenTips, yPos, random = true )
	{
		::AnimatingCanvas.constructor( )
		delay = timeBetweenTips
		startPosition = yPos
		currentIndex = 0
		proTipWidth = ( ::GameApp.IsAsianLanguage( ) ? 650 : 800 )
		
		tipText = ::Gui.Text( )
		tipText.SetFontById( FONT_SIMPLE_SMALL )
		tipText.SetRgba( COLOR_CLEAN_WHITE )
		tipText.SetPosition( -proTipWidth, 0, 0 )
		AddChild( tipText )
		
		// Shuffle order
		ShuffleTips( )
		
		if( random )
			DoRandomTip( )
	}
	
	function ShuffleTips( )
	{
		order = [ ]
		for( local i = 0; i < PROTIP_COUNT; ++i )
			order.push( i )
		for( local i = PROTIP_COUNT - 1; i >= 0;--i )
		{
			local temp = order[ i ]
			local swapIndex = ::SubjectiveRand.Int( 0, PROTIP_COUNT - 1 )
			order[ i ] = order[ swapIndex ]
			order[ swapIndex ] = temp
		}
		
		currentIndex = 0
	}
	
	function DoRandomTip( )
	{
		if( currentIndex >= order.len( ) )
			ShuffleTips( )
			
		DoTip( order[ currentIndex ], true )
		
		currentIndex++
	}
	
	function DoTip( index, random = false )
	{
		// Set Text
		tipText.BakeBoxLocString( proTipWidth, ::GetProTipLocString( index ), TEXT_ALIGN_RIGHT )
		
		local extraHeight = tipText.Height - tipText.LineHeight
		local start = startPosition - extraHeight
		
		// Go up
		ClearActions( )
		AddAction( ::YMotionTween( startPosition + distance, start, animTime ) )
		AddAction( ::AlphaTween( 0.0, 1.0, animTime ) )
		
		// Go down
		AddDelayedAction( delay + animTime, ::YMotionTween( start, startPosition + distance, animTime ) )
		AddDelayedAction( delay + animTime, ::AlphaTween( 1.0, 0.0, animTime, null, null, null, random? function( canvas ) { DoRandomTip( ) }.bindenv( this ) : null ) )
	}
}