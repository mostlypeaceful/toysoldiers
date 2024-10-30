

class PulsingButton extends Gui.CanvasFrame
{
	image = null
	text = null
	pulseSpeed = 4.0
	timer = 0

	constructor( imagePath )
	{
		::Gui.CanvasFrame.constructor( )
		image = ::Gui.TexturedQuad( )
		image.SetTexture( imagePath )
		AddChild( image )
	}
	function OnTick( dt )
	{
		SetAlpha( Math.Lerp( 0.25, 1.25, Math.Abs( Math.Sin( pulseSpeed * timer ) ) ) )
		timer += dt
		::Gui.CanvasFrame.OnTick( dt )
	}
}
