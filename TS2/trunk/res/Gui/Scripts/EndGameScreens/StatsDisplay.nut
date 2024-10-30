// Control to display a series of pages of lists of stats

// Requires
sigimport "gui/scripts/controls/animatingcanvas.nut"

// Resources
sigimport "gui/textures/endgamescreens/stats_display_g.png"

class StatsDisplayLine extends AnimatingCanvas
{
	static Height = 40 
	
	constructor( statID, value1, color1 = ::Math.Vec4.Construct( 1, 1, 1, 1 ), value2 = null, color2 = ::Math.Vec4.Construct( 1, 1, 1, 1 ) )
	{
		::AnimatingCanvas.constructor( )
		
		local width = 800
		local halfWidth = width / 2
		local green = ::Math.Vec4.Construct( 0, 1, 0, 1 )
		
		if( value2 != null )
		{
			if( GameSessionStats.MoreIsBetter( statID ) )
			{
				color1 = (value1 > value2)? green: color1
				color2 = (value2 > value1)? green: color2
			}
			else
			{
				color1 = (value1 < value2)? green: color1
				color2 = (value2 < value1)? green: color2
			}
		}
		
		local label = ::Gui.Text( )
		label.SetFontById( FONT_SIMPLE_MED )
		label.SetRgba( ::Math.Vec4.Construct( 1, 1, 1, 1 ) )
		label.BakeLocString( ::GameSessionStats.StatLocName( statID ), (value2 == null)? TEXT_ALIGN_LEFT: TEXT_ALIGN_CENTER )
		label.SetXPos( (value2 == null)? -halfWidth: 0 )
		AddChild( label )
		
		local valueDisplay1 = ::Gui.Text( )
		valueDisplay1.SetFontById( FONT_SIMPLE_MED )
		valueDisplay1.SetRgba( color1 )
		valueDisplay1.BakeLocString( ::GameSessionStats.StatLocValueString( value1, statID ), (value2 == null)? TEXT_ALIGN_RIGHT: TEXT_ALIGN_LEFT )
		valueDisplay1.SetXPos( (value2 == null)? halfWidth: -halfWidth )
		AddChild( valueDisplay1 )
		
		if( value2 != null )
		{
			local valueDisplay2 = ::Gui.Text( )
			valueDisplay2.SetFontById( FONT_SIMPLE_MED )
			valueDisplay2.SetRgba( color2 )
			valueDisplay2.BakeLocString( ::GameSessionStats.StatLocValueString( value2, statID ), TEXT_ALIGN_RIGHT )
			valueDisplay2.SetXPos( halfWidth )
			AddChild( valueDisplay2 )
		}
	}
}

class StatsDisplay extends AnimatingCanvas
{
	// Data
	lines = null // array
	currentPage = null // integer
	linesPerPage = null // integer
	pageCounter = null // Gui.Text
	spacing = null // integer
	hasSubPages = null
	Width = null
	
	constructor( )
	{
		::AnimatingCanvas.constructor( )
		hasSubPages = true
		Width = 800
		
		lines = [ ]
		currentPage = 0
		linesPerPage = 10
		spacing = ::StatsDisplayLine.Height
		
		pageCounter = ::Gui.Text( )
		pageCounter.SetFontById( FONT_SIMPLE_SMALL )
		pageCounter.SetRgba( COLOR_CLEAN_WHITE )
		pageCounter.SetYPos( spacing * linesPerPage )
		AddChild( pageCounter )
		
		local background = ::Gui.TexturedQuad( )
		background.SetTexture( "gui/textures/endgamescreens/stats_display_g.png" )
		background.SetPosition( -410, -10, 0.01 )
		background.SetRect( ::Math.Vec2.Construct( 800, background.TextureDimensions( ).y ) )
		AddChild( background )
		
		ShowPage( currentPage )
	}
	
	function NextPage( )
	{
		local pageCount = PageCount( )

		if( pageCount == 1 )
			return
		
		ShowPage( (currentPage + 1) % pageCount )
	}
	
	function PreviousPage( )
	{
		local pageCount = PageCount( )
		
		if( pageCount == 1 )
			return
		
		if( currentPage == 0 )
			ShowPage( pageCount - 1 )
		else
			ShowPage( currentPage - 1 )
	}
	
	function PageCount( )
	{
		// This is integer division.
		return ::Math.RoundUp( lines.len( ).tofloat( ) / linesPerPage.tofloat( ) )
	}
	
	function ShowPage( pageIndex )
	{
		currentPage = pageIndex 
		
		// Clear all the children
		foreach( line in lines )
		{
			line.SetAlpha( 0 )
		}
		
		local startIndex = pageIndex * linesPerPage
		local endIndex = Math.Min( ((pageIndex + 1) * linesPerPage) - 1, lines.len( ) - 1 )

		local k = 0
		for( local i = startIndex; i <= endIndex; ++i )
		{
			lines[ i ].SetYPos( spacing * k )
			lines[ i ].SetAlpha( 1 )
			k++
		}
		
		local pageCount = PageCount( )
		local pageCountString = ::GameApp.LocString( "Page" ).Replace( "current", ( currentPage + 1 ).tointeger( ) ).Replace( "total", pageCount.tointeger( ) )
		pageCounter.BakeLocString( pageCountString, TEXT_ALIGN_CENTER )
		pageCounter.SetYPos( spacing * linesPerPage )
		
		if( pageCount < 2 )
		{
			hasSubPages = false
			pageCounter.SetAlpha( 0 )
		}
		else
		{
			hasSubPages = true
			pageCounter.SetAlpha( 1 )
		}
	}
	
	function AddStat( statID, stats1, stats2, showAll )
	{
		if( stats1.StatWasSet( statID ) || (stats2 && stats2.StatWasSet( statID )) )
		{
			local white = ::Math.Vec4.Construct( 1, 1, 1, 1 )
			
			local line = ::StatsDisplayLine( statID, stats1.Stat( statID ), white, ( stats2 )? stats2.Stat( statID ): null, white )
			lines.push( line )
			line.SetAlpha( 0 )
			AddChild( line )
		}
	}
	
	function AddStandardStats( stats1, stats2 = null, showAll = false )
	{
		for( local i = 0; i < SESSION_STATS_COUNT; ++i )
		{
			if( !::GameSessionStats.StatTemp( i ) )
				AddStat( i, stats1, stats2, showAll )
		}
	}
	
	function HandleInput( gamepad )
	{
		if( gamepad.ButtonDown( GAMEPAD_BUTTON_RSHOULDER ) )
		{
			PlaySound( "Play_UI_Scroll" )
			NextPage( )
			return true
		}
		else if( gamepad.ButtonDown( GAMEPAD_BUTTON_LSHOULDER ) )
		{
			PlaySound( "Play_UI_Scroll" )
			PreviousPage( )
			return true
		}
		return false
	}
}
