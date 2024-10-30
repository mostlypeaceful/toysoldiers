// Gui.Text wrapped by an AnimatingCanvas

// Requires
sigimport "gui/scripts/controls/animatingcanvas.nut"

class AnimatingText extends AnimatingCanvas
{
	text = null
	
	constructor( fontId = FONT_SIMPLE_SMALL, colorRgba = COLOR_CLEAN_WHITE )
	{
		::AnimatingCanvas.constructor( )
		text = ::Gui.Text( )
		text.SetFontById( fontId )
		text.SetRgba( colorRgba )
		AddChild( text )
	}
	function _get( index )
	{
		if( index == "Height" )
			return text.Height
		else
			return null
	}
	function BakeCString( cStr )
	{
		text.BakeCString( cStr )
	}
	function BakeCString( cStr, align )
	{
		text.BakeCString( cStr, align )
	}
	function BakeLocString( locString )
	{
		text.BakeLocString( locString )
	}
	function BakeLocString( locString, align )
	{
		text.BakeLocString( locString, align )
	}
}
