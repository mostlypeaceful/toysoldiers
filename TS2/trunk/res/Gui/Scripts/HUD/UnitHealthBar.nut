
class UnitHealthBar extends Gui.CanvasFrame
{
	barFg = null // Gui.ColoredQuad
	barBg = null // Gui.ColoredQuad

	barWidth = 200
	barHeight = 25
	barOutline = 2
	barFgColor = Math.Vec4.Construct( 1.0, 0.0, 0.0, 1.0 )
	barBgColor = Math.Vec4.Construct( 0.0, 0.0, 0.0, 0.7 )
	
	constructor( )
	{
		::Gui.CanvasFrame.constructor( )
		
		barFg = Gui.ColoredQuad( )
		barFg.SetRect( Math.Vec2.Construct( barWidth, barHeight ) )
		barFg.SetRgba( barFgColor )
		barFg.SetPosition( Math.Vec3.Construct( barOutline, barOutline, 0.0 ) )
		AddChild( barFg )
		
		barBg = Gui.ColoredQuad( )
		barBg.SetRect( Math.Vec2.Construct( barWidth + (barOutline * 2), barHeight + (barOutline * 2) ) )
		barBg.SetRgba( barBgColor )
		barBg.SetPosition( Math.Vec3.Construct( 0.0, 0.0, 0.1 ) )
		AddChild( barBg )
		
		CenterPivot( )
	}
	
	function SetHealthPercent( value ) // value: number [0:1]
	{
		value = Math.Clamp( value, 0.0, 1.0 )
		barFg.SetRect( Math.Vec2.Construct( barWidth * value, barHeight ) )
	}
	
	
	function SetColor( color ) // value: vec4
	{
		barFgColor = color
		barFg.SetRgba( barFgColor )
	}
}