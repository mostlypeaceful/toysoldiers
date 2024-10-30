// World Space Flying text

sigexport function CanvasCreateFlyingText( cppObj )
{
	return WorldSpaceFlyingText( cppObj )
}

class WorldSpaceFlyingText extends AnimatingCanvas
{
	// Display
	text = null
	
	// Statics
	static flyTime = 2.0
	
	constructor( )
	{
		::AnimatingCanvas.constructor( )
	}
	
	function SetTarget( target )
	{
		ClearActions( )
		AddAction( ::MotionTween( GetPosition( ), target, flyTime, EasingTransition.Quadratic, EasingEquation.InOut ) )
	}
	
	function SetText( textObj )
	{
		text = textObj
		AddChild( text )
	}
}