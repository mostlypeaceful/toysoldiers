
// Requires
sigimport "Gui/Scripts/Controls/progressbar.nut"

// Resources
sigimport "gui/textures/cursor/barbackground_g.png"
sigimport "gui/textures/cursor/bar_g.png"
sigimport "gui/textures/cursor/powerbar_g.png"
sigimport "gui/textures/gamepad/button_a_g.png"

sigexport function CanvasCreateControl( worldSpaceControl )
{
	return RtsHoverText( )
}


class RtsHoverText extends Gui.CanvasFrame
{
	fadeInSpeed = 4.0
	fadeOutSpeed = 0
	fadeDelta = 0

	hoverText = null
	healthBar = null
	powerBar = null // Progressbar
	aButton = null
	
	// Statics
	static hoverTextZ = 0.49
	
	constructor( )
	{
		Gui.CanvasFrame.constructor( )

		fadeInSpeed = 4.0
		fadeOutSpeed = -2.0
		fadeDelta = fadeOutSpeed

		hoverText = ::Gui.Text( )
		hoverText.SetFontById( FONT_SIMPLE_SMALL )
		hoverText.SetPosition( 0, 0, hoverTextZ )
		AddChild( hoverText )	
		
		aButton = ::Gui.TexturedQuad( )
		aButton.SetTexture( "gui/textures/gamepad/button_a_g.png" )
		AddChild( aButton )
		
		healthBar = ProgressBar( "gui/textures/cursor/barbackground_g.png", "gui/textures/cursor/bar_g.png" )
		healthBar.SetMode( PROGRESSBAR_MODE_TEXTURE )
		healthBar.SetMeterHorizontal( 1.0 )
		healthBar.CenterPivot( )
		healthBar.SetPosition( 0, 33, hoverTextZ )
		if( GameApp.CurrentLevel.IsDisplayCase )
			healthBar.SetAlpha( 0 )
		AddChild( healthBar )
		
		powerBar = ProgressBar( "gui/textures/cursor/barbackground_g.png", "gui/textures/cursor/powerbar_g.png" )
		powerBar.SetMode( PROGRESSBAR_MODE_TEXTURE )
		powerBar.SetMeterHorizontal( 1.0 )
		powerBar.CenterPivot( )
		powerBar.SetPosition( 0, 48, hoverTextZ )
		powerBar.SetAlpha( 0.0 )
		AddChild( powerBar )

		SetAlpha( 0 )
	}
	function OnTick( dt )
	{
		local newAlpha = GetAlpha( ) + fadeDelta * dt
		SetAlphaClamp( newAlpha )

		::Gui.CanvasFrame.OnTick( dt )
	}
	function SetHoverText( text, usable = true )
	{
		hoverText.BakeLocString( text, TEXT_ALIGN_CENTER )
		//healthBar.SetMeterHorizontal( health )
		powerBar.SetAlpha( 0.0 )
		if( usable )
		{
			aButton.SetAlpha( 1.0 )
			aButton.SetPosition( -hoverText.Width * 0.5 - aButton.TextureDimensions( ).x - 4, 3, hoverTextZ )
		}
		else
			aButton.SetAlpha( 0.0 )
	}
	function SetHealth( health )
	{
		healthBar.SetMeterHorizontal( health )
	}
	function SetPowerLevel( power )
	{
		powerBar.SetAlpha( 1.0 )
		powerBar.SetMeterHorizontal( power )
	}
	function SetVisibility( visible )
	{
		if( visible )
			FadeIn( )
		else
			Hide( )
	}
	function FadeIn( )
	{
		fadeDelta = fadeInSpeed
	}
	function FadeOut( )
	{
		fadeDelta = fadeOutSpeed
	}
	function Hide( )
	{
		fadeDelta = fadeOutSpeed
		SetAlpha( 0 )
	}
}
