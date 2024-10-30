// Static Screen (How-to, settings, etc...)

// Requires
sigimport "gui/scripts/controls/verticalmenu.nut"
sigimport "gui/scripts/controls/controllerbuttoncontainer.nut"

// Resources
sigimport "gui/textures/score/score_decoration_g.png"

class FrontEndStaticScreen extends VerticalMenu
{
	// Display
	title = null // Gui.Text
	subtitle = null // Gui.Text
	decoration = null // Gui.TexturedQuad
	controls = null // ControllerButtonContainer
	secondaryControls = null // ControllerButtonContainer
	descriptor = null
	
	constructor( )
	{
		::VerticalMenu.constructor( )
		ForwardButtons = 0 // Can't continue
		
		local vpSafeRect = ::GameApp.ComputeScreenSafeRect( )
		
		// Title
		title = ::Gui.Text( )
		title.SetFontById( FONT_FANCY_LARGE )
		title.SetRgba( COLOR_CLEAN_WHITE )
		title.BakeCString( "Static Screen Title", TEXT_ALIGN_CENTER )
		title.SetPosition( vpSafeRect.Center.x, vpSafeRect.Top + 10, 0 )
		AddChild( title )
		
		// Decoration
		decoration = ::Gui.TexturedQuad( )
		decoration.SetTexture( "gui/textures/score/score_decoration_g.png" )
		decoration.CenterPivot( )
		decoration.SetPosition( title.GetXPos( ), title.GetYPos( ) + title.Height + 5, 0 )
		AddChild( decoration )
		
		subtitle = ::Gui.Text( )
		subtitle.SetFontById( FONT_FANCY_MED )
		subtitle.SetRgba( COLOR_CLEAN_WHITE )
		subtitle.SetPosition( vpSafeRect.Center.x, decoration.GetYPos( ), 0 )
		AddChild( subtitle )
		
		// Description Text
		descriptor = Gui.Text( )
		descriptor.SetFontById( FONT_SIMPLE_SMALL )
		descriptor.SetRgba( COLOR_CLEAN_WHITE )
		descriptor.SetPosition( vpSafeRect.Right, vpSafeRect.Bottom - descriptor.LineHeight, 0 )
		AddChild( descriptor )
		
		// Controls
		controls = ::ControllerButtonContainer( FONT_SIMPLE_SMALL, 20 )
		controls.SetPosition( vpSafeRect.Left, vpSafeRect.Bottom - 12, 0 )
		AddChild( controls )
		
		secondaryControls = ::ControllerButtonContainer( FONT_SIMPLE_SMALL, 15 )
		secondaryControls.SetPosition( controls.GetXPos( ), controls.GetYPos( ) - 26, 0 )
		AddChild( secondaryControls )
		
		SetZPos( 0.4 )
	}
	
	function SetDescriptorText( textId )
	{
		descriptor.SetAlpha( 1 )
		if( typeof textId == "string" )
			descriptor.BakeLocString( ::GameApp.LocString( textId ), TEXT_ALIGN_RIGHT )
		else if( typeof textId == "instance" )
			descriptor.BakeLocString( textId, TEXT_ALIGN_RIGHT )
		else
			descriptor.SetAlpha( 0 )
	}
	
	function SetTitle( titleLocId )
	{
		_settext( title, titleLocId )
	}
	
	function SetSubtitle( locId )
	{
		_settext( subtitle, locId )
	}
	
	function _settext( text, locid )
	{
		local locString = null
		if( typeof locid == "string" )
			locString = ::GameApp.LocString( locid )
		else
			locString = locid
		text.BakeLocString( locString, TEXT_ALIGN_CENTER )
	}
}