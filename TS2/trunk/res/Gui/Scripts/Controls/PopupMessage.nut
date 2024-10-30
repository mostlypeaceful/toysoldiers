// Simple message pop-up for the player

// Resources
sigimport "gui/textures/trial/tutorial_bg_g.png"

class PopupMessage extends Gui.CanvasFrame
{
	// Display
	text = null // Gui.Text
	
	// Behaviour
	fadeDelta = null // number, value for controlling fade in/out
	fadeInSpeed = null // number, how fast to fade in
	fadeOutSpeed = null // number, how fast to fade out
	waitTimer = null // number, Seconds before fade out, null if no fade out
	minTimer = 0.0
	wantsFadeOut = false
	
	// Events
	onFadeOut = null
	
	constructor( locID, color = null, font = null, align = null, offset = null, user = null, waitTime = 5.0, minTime = -100.0 )
	{
		::Gui.CanvasFrame.constructor( )
		
		fadeDelta = 0.0
		fadeInSpeed = 4.0
		fadeOutSpeed = 2.0
		if( waitTime )
			waitTimer = waitTime + 1 / fadeInSpeed
		else
			waitTimer = null
		minTimer = minTime + 1 / fadeInSpeed
		
		local vpRect
		
		// Set defaults
		if( color == null )
			color = COLOR_CLEAN_WHITE
		if( font == null )
			font = FONT_FANCY_LARGE
		if( align == null )
			align = TEXT_ALIGN_CENTER
		if( offset == null )
			offset = ::Math.Vec3.Construct( 0, 0, 0 )
		if( user == null )
			vpRect = GameApp.FrontEndPlayer.User.ComputeScreenSafeRect( )
		else
			vpRect = user.ComputeViewportSafeRect( )
		
		// Get position info
		local center = vpRect.Center
		local pos = ::Math.Vec3.Construct( center.x, center.y, 0.25 )
		
		// Create Text
		text = ::Gui.Text( )
		text.SetFontById( font )
		text.SetRgba( color )
		text.BakeBoxLocString( vpRect.Width - 40, GameApp.LocString( locID ), align )
		text.SetPosition( offset )
		AddChild( text )
		
		// Create Background
		local bg = ::Gui.TexturedQuad( )
		bg.SetTexture( "gui/textures/trial/tutorial_bg_g.png" )
		bg.SetPosition( -640, offset.y - 10, 0.01 )
		bg.SetRect( ::Math.Vec2.Construct( 1280, text.Height + 20 ) )
		AddChild( bg )
		
		// Automatically add to the HUD root
		SetPosition( pos )
		::GameApp.HudRoot.AddChild( this )
		
		// Hide and then fade in
		SetAlpha( 0.0 )
		FadeIn( )
	}
	
	function OnTick( dt )
	{
		local newAlpha = GetAlpha( ) + fadeDelta * dt
		SetAlphaClamp( newAlpha )

		::Gui.CanvasFrame.OnTick( dt )
		
		if( minTimer <= 0.0 )
		{
			if( wantsFadeOut )
				fadeDelta = -fadeOutSpeed
		}
		else
			minTimer -= dt
		
		if( waitTimer != null )
		{
			if( waitTimer > 0 )
				waitTimer -= dt
			else
				FadeOut( )
		}

		if( newAlpha < 0 && fadeDelta < 0 )
		{
			// Invoke event
			if( onFadeOut )
				onFadeOut( )
				
			// Remove self from HUD root after fadeout
			DeleteSelf( )
		}
	}
	
	function FadeIn( )
	{
		wantsFadeOut = false
		fadeDelta = fadeInSpeed
	}
	
	function FadeOut( )
	{
		wantsFadeOut = true
	}
}

class PopupMessageWithButtons extends PopupMessage
{
	constructor( locID, color = null, font = null, align = null, offset = null, user = null, waitTime = 5.0, minTime = -100.0, buttons = null )
	{
		::PopupMessage.constructor( locID, color, font, align, offset, user, waitTime, minTime )
		if( buttons != null && buttons.len( ) > 0 )
		{
			local buttonContainer = ::ControllerButtonContainer( )
			
			foreach( button in buttons )
			{
				print( button.Button.tostring )
				local control = ::ControllerButton( button.Button, button.Text, null, null, "Comma" )
				control.AddAction( ::RgbPulse( ::Math.Vec3.Construct( 0.8, 0.8, 0.8 ), ::Math.Vec3.Construct( 1.5, 1.5, 1.5 ), 1.0 ) )
				buttonContainer.AddControlObject( control )
			}
			
			buttonContainer.SetPosition( -buttonContainer.WorldRect.Width * 0.5, text.GetYPos( ) + text.Height + 25, text.GetZPos( ) )
			AddChild( buttonContainer )
		}
	}
}