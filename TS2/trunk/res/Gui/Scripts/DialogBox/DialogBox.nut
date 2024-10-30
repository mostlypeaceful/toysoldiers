
sigexport function CanvasCreateDialogBox( dialogBox )
{
	return DialogBox( )
}

class DialogBox extends Gui.CanvasFrame
{
	displayText = null
	acceptButton = null
	background = null
	fadeDelta = 0
	fadeInSpeed = 2.0
	fadeOutSpeed = 2.0
	holdTime = 3.0
	needsConfirmation = false

	function DefaultSetup( needsConfirm )
	{
		needsConfirmation = needsConfirm

		CenterPivot( )

		displayText = ::Gui.Text( )
		displayText.SetFontById( FONT_FANCY_LARGE )
		displayText.SetPosition( Math.Vec3.Construct( 0, displayText.LineHeight * -0.5, 0 ) )
		AddChild( displayText )

		if( needsConfirmation )
		{
			acceptButton = ControllerButton( "Gui/Textures/Gamepad/button_a_g.png", "Ok", ControllerButtonAlignment.LEFT, FONT_SIMPLE_MED )
			acceptButton.SetPosition( Math.Vec3.Construct( -acceptButton.LocalRect.Width * 0.5, displayText.LineHeight, 0 ) )
			AddChild( acceptButton )
		}
		else
		{
			holdTime = 3.0
		}

		SetAlpha( 0 )

		/*background = ::Gui.TexturedQuad( )
		background.SetTexture( "gui/textures/dialogbox/dialogboxbg_g.png" )
		background.SetPosition( Math.Vec3.Construct( 0, 0, 0.001 ) )
		background.CenterPivot( )
		AddChild( background )*/
	}
	function OnTick( dt )
	{
		local newAlpha = GetAlpha( ) + fadeDelta * dt
		SetAlphaClamp( newAlpha )

		::Gui.CanvasFrame.OnTick( dt )

		if( !needsConfirmation && holdTime >= 0 )
		{
			if( newAlpha >= 1 )
				holdTime -= dt

			if( holdTime < 0 )
				FadeOut( )
		}

		if( newAlpha < 0 && fadeDelta < 0 )
			DeleteSelf( )
	}
	function FadeIn( )
	{
		fadeDelta = fadeInSpeed

		// TODOAUDIO menu is coming up for the first time, some kind of appropriate sound
	}
	function FadeOut( )
	{
		fadeDelta = -fadeOutSpeed

		// TODOAUDIO menu is being exited, some kind of appropriate sound
	}
	function SetText( text )
	{
		displayText.BakeLocString( GameApp.LocString( text ), TEXT_ALIGN_CENTER )
	}
}
