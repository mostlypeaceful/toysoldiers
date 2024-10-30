// Like the level intro text, a multi-stage message

// Requires
sigimport "gui/scripts/controls/animatingcanvas.nut"

class ScreenMessage extends AnimatingCanvas
{
	// Display
	text = null // Gui.Text
	
	constructor( locId, delay, verticalOffset = null, font = null, color = null, align = null )
	{
		::AnimatingCanvas.constructor( )
		
		verticalOffset = ( verticalOffset == null )? 0: verticalOffset
		font = ( font == null )? FONT_SIMPLE_MED: font
		color = ( color == null )? COLOR_CLEAN_WHITE: color
		align = ( align == null )? TEXT_ALIGN_CENTER: align
		
		text = ::Gui.Text( )
		text.SetFontById( font )
		text.SetRgba( color )
		text.BakeBoxLocString( 1280 * 0.75, ::GameApp.LocString( locId ), align )
		text.SetPosition( ::Math.Vec3.Construct( 0, verticalOffset, 0 ) )
		AddChild( text )
		
		AddDelayedAction( delay, AlphaTween( 0.0, 1.0, 1.0, EasingTransition.Quadratic, EasingEquation.Out ) )
	}
}

class MultiStageMessage extends AnimatingCanvas
{
	constructor( params = null, timeToShow = null )
	{
		::AnimatingCanvas.constructor( )
		
		local screenRectCenter = ::Math.Vec3.Construct( 1280 / 2, 720 / 2, 0 )
		
		if( type( params ) != "array" )
			return
			
		foreach( msg in params )
		{
			msg.SetPosition( screenRectCenter )
			msg.SetAlpha( 0 )
			AddChild( msg )
		}
		
		timeToShow = ( timeToShow == null )? 15.0: timeToShow
		
		AddDelayedAction( timeToShow, AlphaTween( 1.0, 0.0, 0.5, EasingTransition.Quadratic, EasingEquation.Out, null,
			function( canvas ) { canvas.DeleteSelf( ) } ) )
		
		::GameApp.HudRoot.AddChild( this )
	}
}