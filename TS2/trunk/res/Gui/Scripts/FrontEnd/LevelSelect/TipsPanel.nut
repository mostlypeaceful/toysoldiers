// Tips Panel

class TipsPanel extends AnimatingCanvas
{
	// Display
	tips = null
	
	constructor( startingLevelInfo )
	{
		::AnimatingCanvas.constructor( )
		
		local title = ::Gui.Text( )
		title.SetFontById( ::GameApp.IsAsianLanguage( ) ? FONT_FANCY_LARGE : FONT_FANCY_MED )
		title.SetUniformScale( 0.7 )
		title.SetRgba( COLOR_CLEAN_WHITE )
		title.BakeLocString( ::GameApp.LocString( "LevelSelect_Tips" ), TEXT_ALIGN_CENTER )
		title.SetPosition( 375, 1, -0.01 )
		if( ::GameApp.IsAsianLanguage( ) )
			title.SetYPos( 2 )
		else
			title.SetUniformScale( 0.7 )
		AddChild( title )
		
		// Do tips
		tips = [ ]
		local spacing = 28
		for( local i = 0; i < 2; ++i )
		{
			local tipText = ::Gui.Text( )
			tipText.SetFontById( FONT_SIMPLE_SMALL )
			tipText.SetRgba( COLOR_CLEAN_WHITE )
			tipText.SetPosition( 0, 38 + i * spacing, 0 )
			AddChild( tipText )
			tips.push( tipText )
		}
		
		SetLevelInfo( startingLevelInfo )
	}
	
	function SetLevelInfo( info )
	{
		for( local i = 0; i < 2; ++i )
			tips[ i ].BakeBoxLocString( 720, ::GameApp.LocString( info.DescriptionLocId + "_Tip" + i.tostring( ) ), TEXT_ALIGN_LEFT )
	}
}