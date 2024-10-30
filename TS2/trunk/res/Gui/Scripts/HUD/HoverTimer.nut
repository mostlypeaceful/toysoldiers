sigimport "gui/textures/cursor/recharge_g.png"

sigexport function CanvasCreateControl( worldSpaceControl )
{
	return HoverTimer( )
}


class HoverTimer extends Gui.CanvasFrame
{
	fadeInSpeed = 4.0
	fadeOutSpeed = 0
	fadeDelta = 0

	hoverText = null
	rechargeIcon = null // Gui.TexturedQuad
	
	constructor( )
	{
		::Gui.CanvasFrame.constructor( )

		fadeInSpeed = 4.0
		fadeOutSpeed = -2.0
		fadeDelta = fadeInSpeed
		
		hoverText = null
		rechargeIcon = null // Gui.TexturedQuad

		SetAlpha( 0 )
	}
	function Setup( iconMode )
	{		
		if( iconMode )
		{
			rechargeIcon = ::Gui.TexturedQuad( )
			rechargeIcon.SetTexture( "gui/textures/cursor/recharge_g.png" )
			rechargeIcon.SetZPos( 0.6 )
			rechargeIcon.CenterPivot( )
			AddChild( rechargeIcon )
		}
		else
		{
			hoverText = ::Gui.Text( )
			hoverText.SetFontById( FONT_SIMPLE_SMALL )
			AddChild( hoverText )	
		}
	}
	function OnTick( dt )
	{
		local newAlpha = GetAlpha( ) + fadeDelta * dt
		SetAlphaClamp( newAlpha )

		::Gui.CanvasFrame.OnTick( dt )
	}
	function ScaleIcon( scale ) 
	{
		// Scale recharge icon based on distance
		rechargeIcon.SetUniformScale( scale )
	}
	function SetText( text )
	{
		hoverText.BakeBoxLocString( 400, text, TEXT_ALIGN_CENTER )
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
	function FadedOut( )
	{
		return fadeDelta < 0 && GetAlpha( ) < 0.001
	}
}

