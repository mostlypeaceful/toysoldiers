// Clock countdown timer

// Requires
sigimport "gui/scripts/controls/animatingcanvas.nut"

// Resources
sigimport "gui/textures/trial/watch_face_g.png"
sigimport "gui/textures/trial/watch_hour_g.png"
sigimport "gui/textures/trial/watch_minute_g.png"
sigimport "gui/textures/trial/watch_second_g.png"

class ClockTimer extends AnimatingCanvas
{
	// Display
	face = null
	hourHand = null
	minuteHand = null
	secondHand = null
	
	// Data
	currentTime = null
	direction = null
	pause = null
	
	timeScale = 1.0		//1 second is one second
	
	constructor( startTime_ = 0, direction_ = 0 )
	{
		::AnimatingCanvas.constructor( )
		
		direction = direction_
		pause = false
		
		local width = 64
		local height = 64
		
		// Create Display Elements
		face = ::Gui.TexturedQuad( )
		face.SetTexture( "gui/textures/trial/watch_face_g.png" )
		AddChild( face )
		
		local centerPos = ::Math.Vec2.Construct( width / 2, height / 2 )
		
		hourHand = ::Gui.TexturedQuad( )
		hourHand.SetTexture( "gui/textures/trial/watch_hour_g.png" )
		hourHand.CenterPivot( )
		hourHand.SetPosition( ::Math.Vec3.Construct( centerPos.x, centerPos.y, -0.01 ) )
		AddChild( hourHand )
		
		minuteHand = ::Gui.TexturedQuad( )
		minuteHand.SetTexture( "gui/textures/trial/watch_minute_g.png" )
		minuteHand.CenterPivot( )
		minuteHand.SetPosition( ::Math.Vec3.Construct( centerPos.x, centerPos.y, -0.02 ) )
		AddChild( minuteHand )
		
		secondHand = ::Gui.TexturedQuad( )
		secondHand.SetTexture( "gui/textures/trial/watch_second_g.png" )
		secondHand.CenterPivot( )
		secondHand.SetPosition( ::Math.Vec3.Construct( centerPos.x, centerPos.y, -0.03 ) )
		AddChild( secondHand )
		
		secondHand.SetAngle( MATH_3_PI_OVER_2 )
		SetTime( startTime_ )
	}
	
	function OnTick( dt )
	{
		dt *= timeScale
		::AnimatingCanvas.OnTick( dt )
		
		if( ::GameApp.Paused( ) || pause )
			return
		
		// Rotate main hands
		if( direction == 0 )
		{
			if( currentTime < 0 )
			{
				secondHand.SetAngle( MATH_3_PI_OVER_2 )
				SetTime( 0 )
			}
			else if( currentTime > 0 )
			{
				secondHand.SetAngle( ::Math.Wrap( secondHand.GetAngle( ) - dt * MATH_2_PI, 0, MATH_2_PI ) )
				SetTime( currentTime - dt )
			}
		}
		else
		{
			secondHand.SetAngle( ::Math.Wrap( secondHand.GetAngle( ) + dt * MATH_2_PI, 0, MATH_2_PI ) )
			
			if( currentTime > 4 * 60 )
			{
				SetTime( 0 )
			}
			else
				SetTime( currentTime - dt )
		}
	}
	
	function SetTime( time )
	{
		if( currentTime == time )
			return
			
		currentTime = time
		
		local hourHandPercentage = time / ( 4 * 60 )
		local minuteHandPercentage = ( time - ( ::Math.RoundDown( time / 60 ) * 60) ) / 60
		
		local hourHandAngle = ::Math.Wrap( ::Math.Lerp( 0, MATH_2_PI, hourHandPercentage ) + MATH_3_PI_OVER_2, 0, MATH_2_PI )
		local minuteHandAngle = ::Math.Wrap( ::Math.Lerp( 0, MATH_2_PI, minuteHandPercentage ) + MATH_3_PI_OVER_2, 0, MATH_2_PI )
		
		hourHand.SetAngle( hourHandAngle )
		minuteHand.SetAngle( minuteHandAngle )
	}
	
	function Reset( startTime_ = 0, direction_ = 0 )
	{
		SetTime( startTime_ )
		direction = direction_
	}
	
	function Pause( p )
	{
		pause = p
	}
}