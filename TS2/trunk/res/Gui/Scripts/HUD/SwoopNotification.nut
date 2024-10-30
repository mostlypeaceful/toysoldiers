// Swoop notification 

// Requires
sigimport "gui/scripts/controls/animatingcanvas.nut"

SWOOP_LEFT <- ( -1 )
SWOOP_RIGHT <- ( 1 )

class SwoopNotification extends AnimatingCanvas
{
	// Display
	swoopingChildren = null
	
	// Data
	swoopDirection = null
	swooping = null
	shouldPause = null
	paused = null
	swoopInTime = null
	showTime = null
	swoopOutTime = null
	swoopInDistance = null
	swoopOutDistance = null
	
	// Events
	onSwoopInStart = null
	onSwoopInFinish = null
	onPause = null
	onContinue = null
	onSwoopOut = null
	onSwoopOutFinish = null
	onCleared = null
	
	constructor( swoopDir = SWOOP_LEFT, pause = false )
	{
		::AnimatingCanvas.constructor( ) 
		swoopingChildren = [ ]
		swoopDirection = swoopDir
		swooping = false
		shouldPause = pause
		paused = false
		swoopInTime = 0.2
		showTime = 3.0
		swoopOutTime = 0.5
		swoopInDistance = 120
		swoopOutDistance = 200
		
		SetAlpha( 0 )
	}
	
	function SwoopIn( )
	{
		if( onSwoopInStart )
			onSwoopInStart( )
		
		swooping = true
		AddAction( ::AlphaTween( 0.0, 1.0, swoopInTime, EasingTransition.Quadratic, EasingEquation.Out ) )
		AddAction( ::XMotionTween( GetXPos( ), GetXPos( ) + swoopDirection * swoopInDistance, swoopInTime, EasingTransition.Quadratic, EasingEquation.Out, null, function( canvas )
		{
			if( onSwoopInFinish )
				onSwoopInFinish( )
			if( shouldPause )
			{
				if( onPause )
					onPause( )
				paused = true
			}
			else
				AddAction( ::CanvasAction( showTime, null, null, function( canvas ) { SwoopOut( ) }.bindenv( this ) ) )
		}.bindenv( this ) ) )
		
		foreach( child in swoopingChildren )
			child.onAnimate( )
	}
	
	function SwoopOut( )
	{		
		if( onSwoopOut )
			onSwoopOut( )
			
		swooping = true
		AddAction( ::XMotionTween( GetXPos( ), GetXPos( ) + swoopDirection * swoopOutDistance, swoopOutTime, EasingTransition.Quadratic, EasingEquation.InOut ) )
		AddAction( ::AlphaTween( 1.0, 0.0, swoopOutTime - 0.1, EasingTransition.Quadratic, EasingEquation.In, null,
		function( canvas )
		{
			if( onSwoopOutFinish )
				onSwoopOutFinish( )
			DeleteSelf( )
		}.bindenv( this ) ) )
	}
	
	function Swoop( delay = null )
	{
		if( swooping )
			return
		
		if( delay )
			AddAction( ::CanvasAction( delay, null, null, function( canvas ) { SwoopIn( ) }.bindenv( this ) ) )
		else
			SwoopIn( )
	}
	
	function Continue( )
	{
		if( paused )
		{
			if( onContinue )
				onContinue( )
			paused = false
			SwoopOut( )
		}
	}
	
	function Clear( time = null )
	{
		if( onCleared )
			onCleared( )
		ClearActions( )
		if( time )
			swoopOutTime = time
		SwoopOut( )
	}
	
	function AddSwoopingChild( canvas, delay, distIn, distOut )
	{
		local anim = ::SwoopingChild( canvas )
		anim.onAnimate = function( ):(delay,swoopDirection,distIn,distOut,showTime)
		{
			AddDelayedAction( delay, ::XMotionTween( 0, swoopDirection * distIn, 0.3, EasingTransition.Quadratic, EasingEquation.Out ) )
			AddDelayedAction( delay, ::AlphaTween( 0.0, 1.0, 0.2 ) )
			AddDelayedAction( delay + 0.3, ::XMotionTween( swoopDirection * distIn, swoopDirection * (distIn + distOut), showTime + 0.5, EasingTransition.Quadratic, EasingEquation.Out ) )
		}.bindenv( anim )
		AddChild( anim )
		swoopingChildren.push( anim )
	}
	
	function OnTick( dt )
	{
		if( !::GameApp.Paused( ) )
			::AnimatingCanvas.OnTick( dt )
	}
}

class SwoopingChild extends AnimatingCanvas 
{
	// Data
	onAnimate = null // func
	
	constructor( child )
	{
		::AnimatingCanvas.constructor( )
		AddChild( child )
		SetAlpha( 0 )
	}
}

class StandardSwoop extends SwoopNotification
{
	// Display
	line1Text = null
	line2Text = null
	line3Text = null
	
	// Statics
	standardShowTime = 1.5
		
	constructor( line1LocString, line2LocString, line3LocString, dir = SWOOP_LEFT )
	{
		::SwoopNotification.constructor( dir )
		
		local y = 9
		
		if( line1LocString )
		{
			line1Text = ::Gui.Text( )
			line1Text.SetFontById( FONT_SIMPLE_SMALL )
			line1Text.SetRgba( 0.9, 0.9, 0.9, 1.0 )
			line1Text.SetUniformScale( 0.8 )
			line1Text.SetPosition( -dir * 200, y, 0 )
			line1Text.BakeLocString( line1LocString, TEXT_ALIGN_RIGHT )
			AddSwoopingChild( line1Text, 0.0, 200, 20 )
		}
		
		y += ( ( line1Text == null )? 0: line1Text.LineHeight )
		
		if( line2LocString )
		{
			line2Text = ::Gui.Text( )
			line2Text.SetFontById( FONT_SIMPLE_MED )
			line2Text.SetRgba( COLOR_CLEAN_WHITE )
			line2Text.BakeLocString( line2LocString, TEXT_ALIGN_RIGHT )
			line2Text.SetPosition( -dir * 200, y, 0 )
			AddSwoopingChild( line2Text, 0.0, 200, 30 )
		}
		
		y += ( ( line2Text == null )? 0: line2Text.LineHeight + 6 )
		
		local line = ::Gui.TexturedQuad( )
		line.SetTexture( "gui/textures/score/score_decoration_g.png" )
		line.CenterPivot( )
		line.SetPosition( (-dir * 375) - 75, y, 0 )
		AddSwoopingChild( line, 0.1, 320, 90 )
		
		if( line3LocString )
		{
			line3Text = ::Gui.Text( )
			line3Text.SetFontById( FONT_FANCY_LARGE )
			line3Text.SetRgba( COLOR_CLEAN_WHITE )
			line3Text.BakeLocString( line3LocString, TEXT_ALIGN_RIGHT )
			line3Text.SetPosition( -dir * 200, y - 4, 0 )
			AddSwoopingChild( line3Text, 0.2, 200, 40 )
		}
	}
}

class ResultsSwoop extends StandardSwoop
{
	// Data
	score = null
	
	constructor( label, scoreValue = null, pause = true, gamertag = null, source = null )
	{
		local scoreText = null
		if( scoreValue != null )
			scoreText = ::LocString.FromCString( scoreValue.tointeger( ).tostring( ) )
		::StandardSwoop.constructor( gamertag, label, scoreText, SWOOP_RIGHT )
		if( line3Text )
			line3Text.SetUniformScale( 0.9 )
		showTime = 1.5
		swoopInDistance = 500
		swoopOutDistance = 600
		shouldPause = pause
		
		if( scoreValue != null )
			score = scoreValue
		else
			shouldPause = false
			
		audioSource = source
		
		onSwoopInStart = function( )
		{
			PlaySound( "Play_HUD_Awesome_Into" )
		}
		
		onSwoopOut = function( )
		{
			PlaySound( "Play_HUD_Awesome_Out" )
		}
	}
	
	function Swoop( delay = null )
	{
		if( score )
		{
			AddDelayedAction( delay, ::TextCountTween( line3Text, 0.0, score, 0.8, function( text, value )
			{
				text.BakeCString( ::StringUtil.AddCommaEvery3Digits( value.tointeger( ).tostring( ) ), TEXT_ALIGN_RIGHT )
			} ) )
		}
		::StandardSwoop.Swoop( delay )
	}
}