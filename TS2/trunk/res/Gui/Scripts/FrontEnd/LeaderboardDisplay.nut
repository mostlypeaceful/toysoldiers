// Leaderboard Display

// Requires
sigimport "gui/scripts/frontend/leaderboardutility.nut"
sigimport "gui/scripts/frontend/friendleaderboardcanvas.nut"
sigimport "gui/scripts/frontend/overallleaderboardcanvas.nut"
sigimport "gui/scripts/frontend/myscoreleaderboardcanvas.nut"

// Resources
sigimport "gui/textures/score/score_decoration_g.png"

enum LBFilter
{
	FirstUnused,
	Overall,
	Friends,
	MyScore,
	
	LBFilterCount
}

class LeaderboardDisplay extends AnimatingCanvas
{
	// Display
	headerRow = null
	displays = null
	
	// Data
	name = null
	filter = null
	rowsStartY = null
	prevCount = null
	levelInfo = null
	user = null
	
	// Events
	onCountChange = null
	
	constructor( boardId, levelInfo_ = null, user_ = null )
	{
		::AnimatingCanvas.constructor( )
		filter = null
		levelInfo = levelInfo_
		user = ( ( user_ == null )? ::GameApp.FrontEndPlayer.User: user_ )
		
		local isLSP = boardId >= 1000;
		if( isLSP )
			boardId -= 1000;
		
		local board = isLSP ? ::LSPLeaderboard( ) : ::Leaderboard( )
		board.AddBoardToRead( boardId )
		board.SelectBoard( boardId )
		
		local friendsBoard = isLSP ? ::LSPLeaderboard( ) : ::Leaderboard( )
		friendsBoard.AddBoardToRead( boardId )
		friendsBoard.SelectBoard( boardId )
		
		local myScoreBoard = isLSP ? ::LSPLeaderboard( ) : ::Leaderboard( )
		myScoreBoard.AddBoardToRead( boardId )
		myScoreBoard.SelectBoard( boardId )
		
		name = board.BoardName
		
		// Headers
		local headerItems = [ ]
		local numCols = board.ColumnCount
		for( local i = 0; i < numCols; ++i )
		{
			local header = null
			if( i == 1 )
				header = ::LeaderboardHeader( ::GameApp.LocString( "Leaderboards_Gamertag" ), board.ColumnWidth( i ), board.ColumnAlignment( i ) )
			else
				header = ::LeaderboardHeader( board.ColumnName( i ), board.ColumnWidth( i ), board.ColumnAlignment( i ) )
			headerItems.push( header )
		}
		
		headerRow = ::LeaderboardRow( )
		headerRow.Set( headerItems, null, null, null )
		AddChild( headerRow )
		
		// Line
		local line = ::Gui.TexturedQuad( )
		line.SetTexture( "gui/textures/score/score_decoration_g.png" )
		line.SetPosition( 0, headerRow.LocalRect.Height + 2, 0 )
		line.SetRect( ::Math.Vec2.Construct( ::LeaderboardRow.Width, line.TextureDimensions( ).y ) )
		AddChild( line )
		
		// Rows
		rowsStartY = line.GetYPos( ) + 6
		
		local rows = ::OverallLeaderboardCanvas( board, levelInfo, user )
		rows.SetPosition( 0, rowsStartY, 0 )
		rows.Invisible = true
		AddChild( rows )
		
		local friendRows = ::FriendLeaderboardCanvas( friendsBoard, levelInfo, user )
		friendRows.SetPosition( 0, rowsStartY, 0 )
		friendRows.Invisible = true
		AddChild( friendRows )
		
		local myScoreRows = ::MyScoreLeaderboardCanvas( myScoreBoard, levelInfo, user )
		myScoreRows.SetPosition( 0, rowsStartY, 0 )
		myScoreRows.Invisible = true
		AddChild( myScoreRows )

		displays = { }
		displays[ LBFilter.Overall ] <- rows
		displays[ LBFilter.Friends ] <- friendRows
		displays[ LBFilter.MyScore ] <- myScoreRows
		
		IgnoreBoundsChange = 1
	}

	function StartReading( )
	{
		if( filter != null )
		{
			displays[ filter ].onViewChanged = OnViewChange.bindenv( this )
			displays[ filter ].StartReading( )
		}
	}
	
	function OnTick( dt )
	{
		::AnimatingCanvas.OnTick( dt )
		
		if( filter != null )
		{
			local display = displays[ filter ]
			
			if( prevCount != display.Count( ) )
			{
				if( onCountChange )
					onCountChange( )
			}
			
			prevCount = display.Count( )
		}
	}
	
	function CountText( )
	{
		local value = displays[ filter ].Count( )
		return ::GameApp.LocString( "LB_EntryCount" ).Replace( "i", ::LocString.LocalizeNumber( value ) )
	}
	
	function ChangeFilter( newFilter )
	{
		if( filter != newFilter )
		{
			if( filter != null )
			{
				displays[ filter ].onViewChanged = null
				displays[ filter ].Invisible = true
			}
			
			filter = newFilter
			
			local display = displays[ filter ]
			display.Invisible = false
			display.onViewChanged = OnViewChange.bindenv( this )
			display.StartReading( )
			
			switch( filter )
			{
				case LBFilter.Overall:
					display.SetSelection( 0 )
				break
				
				case LBFilter.Friends:
					display.SetSelection( 0 )
				break
				
				case LBFilter.MyScore:
					display.ScrollToPlayer( )
					OnViewChange( display.viewIndex )
				break
			}
		}
	}
	
	function ChangeHighlight( delta )
	{
		if( filter != null )
			displays[ filter ].ChangeHighlight( delta )
	}
	
	function OnViewChange( index )
	{
		if( filter != null )
			displays[ filter ].SetYPos( rowsStartY - index * ::LeaderboardRow.Height )
	}
	
	function ShowGamerCard( )
	{
		local userId = HighlightedUserId( )
		if( userId != null )
			user.ShowGamerCardUI( HighlightedUserId( ) )
		else
			return false
		
		return true
	}
	
	function HighlightedUserId( )
	{
		return displays[ filter ].HighlightedUserId( )
	}
	
	function HighlightedGamertag( )
	{
		return displays[ filter ].HighlightedGamertag( )
	}
}
