
sigimport "Gui/Scripts/Controls/ControllerButton.nut"

sigexport function CanvasCreateWaveLaunchArrowUI( arrowUI )
{
	return WaveLaunchArrowUI( arrowUI )
}


class WaveLaunchArrowUI extends Gui.CanvasFrame
{
	arrowUIobj = null
	arrow = null

	static arrowZ = 0.6

	constructor( _arrowUIobj )
	{
		::Gui.CanvasFrame.constructor( )

		arrowUIobj = _arrowUIobj
		
		arrow = ::Gui.TexturedQuad( )
		arrow.SetTexture( "gui/textures/misc/arrow_g.png" )

		local scale = 0.125
		local size = arrow.TextureDimensions( )
		arrow.SetPivot( Math.Vec2.Construct( size.x * 0.5, size.y ) )
		
		arrow.SetPosition( 0, 0, arrowZ )
		//arrow.SetUniformScale( scale )
		
		AddChild( arrow )	
	}
	function OnTick( dt )
	{
		::Gui.CanvasFrame.OnTick( dt )
	}
	function Show( show )
	{
		SetAlpha( show ? 1 : 0 )
	}
	function Set( center, angle, alpha )
	{
		arrow.SetPosition( center.x, center.y, arrowZ )
		arrow.SetAngle( angle )
		arrow.SetAlpha( alpha )
	}
}
