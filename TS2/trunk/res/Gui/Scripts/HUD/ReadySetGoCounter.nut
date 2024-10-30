// Special Counter for "Ready, Set, Go!" type events

// Requires
sigimport "gui/scripts/controls/animatingcanvas.nut"

class ReadySetGoText extends AnimatingCanvas
{
	// Display
	text = null
	
	constructor( value )
	{
		::AnimatingCanvas.constructor( )
		
		text = ::Gui.Text( )
		text.SetFontById( FONT_FANCY_LARGE )
		text.SetRgba( COLOR_CLEAN_WHITE )
		AddChild( text )
		
		if( type( value ) == "integer" )
			text.BakeCString( value.tostring( ), TEXT_ALIGN_CENTER )
		else
			text.BakeLocString( ::GameApp.LocString( value ), TEXT_ALIGN_CENTER )
		
		text.SetYPos( -text.Height * 0.5 )
		
		SetAlpha( 0.0 )
		SetPosition( ::Math.Vec3.Construct( 0, 0, 0 ) )
	}
	
	function Pulse( delay, callback )
	{
		AddDelayedAction( delay, AlphaTween( 0.0, 1.0, 0.5, EasingTransition.Quadratic, EasingEquation.Out ) )
		AddDelayedAction( delay, UniformScaleTween( 1.0, 2.0, 0.5, EasingTransition.Quadratic, EasingEquation.In ) )
		AddDelayedAction( delay + 0.5, AlphaTween( 1.0, 0.0, 0.3, EasingTransition.Quadratic, EasingEquation.In ) )
		AddDelayedAction( delay + 0.5, UniformScaleTween( 2.0, 1.5, 0.3, EasingTransition.Quadratic, EasingEquation.Out,
			function( canvas ):(callback) // OnStart
			{
				if( callback )
					callback( )
			},
			function( canvas ) // OnEnd
			{
				canvas.DeleteSelf( )
			}
		) )
	}
}

class ReadySetGoCounter extends AnimatingCanvas
{
	// Display
	text3 = null // Gui.Text
	text2 = null // Gui.Text
	text1 = null // Gui.Text
	go = null // Gui.Text
	
	// Callbacks
	onPulse = null // function
	onGo = null // function
	
	constructor( useNumbers = false )
	{
		::AnimatingCanvas.constructor( )
		
		// Create the displays
		local delay = 0.0
		if( useNumbers )
		{
			text3 = ::ReadySetGoText( 3 )
			text3.Pulse( delay, CallOnPulse.bindenv( this ) )
			AddChild( text3 )
			delay += 1.0
		}
		
		text2 = ::ReadySetGoText( useNumbers? 2: "Ready" )
		text2.Pulse( delay, CallOnPulse.bindenv( this ) )
		AddChild( text2 )
		delay += 1.0
		
		text1 = ::ReadySetGoText( useNumbers? 1: "Set" )
		text1.Pulse( delay, CallOnPulse.bindenv( this ) )
		AddChild( text1 )
		delay += 1.0
		
		go = ::ReadySetGoText( "Go" )
		go.Pulse( delay, CallOnGo.bindenv( this ) )
		AddChild( go )
		
		CenterPivot( )
		SetAlpha( 0 )
		FadeIn( 0.5 )
		::GameApp.HudRoot.AddChild( this )
	}
	
	function OnTick( dt )
	{
		if( !::GameApp.Paused( ) )
			::AnimatingCanvas.OnTick( dt )
	}
	
	function CallOnPulse( )
	{
		PlaySound( "Play_HUD_CountDown_Tick" )
		if( onPulse )
			onPulse( )
	}
	
	function CallOnGo( )
	{
		PlaySound( "Play_HUD_CountDown_Done" )
		if( onGo )
			onGo( )
		FadeOutAndDie( 0.5 )
	}
}

class MinigameReadySetGoCounter extends ReadySetGoCounter
{
	// Display
	gamerPicture = null
	gamerTag = null
	
	constructor( player, useNumbers = false )
	{
		::ReadySetGoCounter.constructor( useNumbers )
		audioSource = player.AudioSource
		
		local startX = -150
		local startY = -180
		
		// Player Image
		gamerPicture = ::Gui.GamerPictureQuad( )
		gamerPicture.SetPosition( startX, startY, 0 )
		gamerPicture.SetTexture( ::GameApp.FrontEndPlayer.User, player.User, false )
		AddChild( gamerPicture )
		
		// Player Text
		gamerTag = ::Gui.Text( )
		gamerTag.SetFontById( FONT_FANCY_MED )
		gamerTag.SetRgba( COLOR_CLEAN_WHITE )
		gamerTag.BakeLocString( player.User.GamerTag, TEXT_ALIGN_LEFT )
		gamerTag.SetPosition( startX + 74, startY + 32 - gamerTag.Height * 0.5, 0 )
		AddChild( gamerTag )
	}
}