// Leaderboard Canvas Base

// Requires
sigimport "gui/scripts/frontend/leaderboardutility.nut"

enum LBState
{
	Loading,
	None,
	Finished,
	UnexpectedResult,
	
	TotalStates
}

class LeaderboardCanvas extends AnimatingCanvas
{
	// Display
	rows = null
	async = null
	noData = null
	
	// Data
	viewIndex = null
	selectedIndex = null
	board = null
	state = null
	currentReadIndex = null
	playerIndex = null
	levelInfo = null
	user = null
	
	// Events
	onViewChanged = null
	onFirstLoad = null
	afterFirstLoad = null
	
	// Statics
	static RowsPerPage = 14
	
	constructor( board_, errorLoc, levelInfo_, user_ )
	{
		::AnimatingCanvas.constructor( )
		rows = [ ]
		viewIndex = 0
		selectedIndex = 0
		board = board_
		state = LBState.None
		currentReadIndex = 0
		levelInfo = levelInfo_
		user = user_
		
		// Loading status
		async = ::AsyncStatusSmall( )
		async.SetPosition( 30, 30, 0 )
		AddChild( async )
		
		noData = ::Gui.Text( )
		noData.SetFontById( FONT_SIMPLE_SMALL )
		noData.SetRgba( COLOR_CLEAN_WHITE )
		noData.BakeLocString( ::GameApp.LocString( errorLoc ), TEXT_ALIGN_LEFT )
		noData.SetPosition( 30, 30, 0 )
		noData.Invisible = true
		AddChild( noData )
		
		IgnoreBoundsChange = 1
	}
	
	function Read( )
	{
		::print( "Base Read" )
		state = LBState.Loading
	}
	
	function StartReading( )
	{
		::print( "Base StartReading" )
		board.CancelRead( )
		if( state == LBState.None )
		{
			::print( "Load started" )
			state = LBState.Loading
			Read( )
		}
	}
	
	function OnTick( dt )
	{
		::AnimatingCanvas.OnTick( dt )
		
		if( state == LBState.Loading )
		{
			if( board.AdvanceRead( ) )
			{
				OnLoadingComplete( )
			}
		}
	}
	
	function OnLoadingComplete( )
	{
		local firstLoad = false
		
		::print( "Loading Complete" )
		if( rows.len( ) == 0 )
		{
			RemoveChild( async )
			
			local total = board.RowsAvailable
			
			if( total == 0 )
			{
				noData.Invisible = false
				
				viewIndex = 0
				if( onViewChanged )
					onViewChanged( viewIndex )
				
				::print( "  Zero Entries!" )
				state = LBState.Finished
				return
			}
			else
			{
				for( local i = 0; i < total; ++i )
				{
					local row = ::LeaderboardRow( i )
					row.SetPosition( 0, i * ::LeaderboardRow.Height, 0 )
					rows.push( row )
					AddChild( row )
				}
			}
			
			if( onFirstLoad )
				onFirstLoad( total )
			ChangeHighlight( 0 )
			firstLoad = true
		}
		
		local numCols = board.ColumnCount
		local numRows = board.RowsAvailable
		for( local ri = 0; ri < numRows; ++ri )
		{
			local rowItems = [ ]
			for( local cj = 0; cj < numCols; ++cj )
				rowItems.push( ::LeaderboardData( board, cj, ri, levelInfo ) )
			
			local rank = board.RowRank( ri )
			local userId = board.RowUserId( ri )
			local gamerTag = board.RowGamertag( ri )
			
			local rowIndex = ri
			if( rowIndex < rows.len( ) )
				rows[ rowIndex ].Set( rowItems, rank, userId, gamerTag )
			else
				::print( "tried to add row: " + ( rowIndex ).tostring( ) )
			
			if( gamerTag == user.GamerTag.ToCString( ) )
			{
				playerIndex = rowIndex
				rows[ playerIndex ].SetAsPlayer( )
				::print( "  Found Player! index:" + playerIndex.tostring( ) )
			}
		}
		
		if( firstLoad )
		{
			if( rows.len( ) > 0 && selectedIndex in rows )
				rows[ selectedIndex ].Highlight( true )
			if( afterFirstLoad )
				afterFirstLoad( )
		}
	}
	
	function Count( )
	{
		return rows.len( )
	}
	
	function IndexInView( index )
	{
		return ( index >= viewIndex && index < viewIndex + RowsPerPage && index < rows.len( ) )
	}
	
	function SetSelection( index )
	{
		if( rows.len( ) == 0 )
			return
		
		if( index == null )
			index = 0
			
		local prevIndex = selectedIndex
		selectedIndex = index
		
		if( prevIndex in rows )
			rows[ prevIndex ].Highlight( false )
		rows[ selectedIndex ].Highlight( true )
		
		if( !IndexInView( selectedIndex ) )
			ScrollToInstant( selectedIndex )
		
		UpdateVisibility( )
	}
	
	function UpdateVisibility( )
	{
		foreach( i, row in rows )
		{
			local viewable = IndexInView( i )
			row.SetAlpha( viewable? 1: 0 )
			row.Invisible = ( viewable? 0: 1 )
			row.Disabled = ( viewable? 0: 1 )
		}
	}
	
	function ChangeHighlight( delta )
	{
		SetSelection( ::Math.Wrap( selectedIndex + delta, 0, rows.len( ) ).tointeger( ) )
	}
	
	function ScrollToInstant( index )
	{
		local prevView = viewIndex
		local maxView = rows.len( ) - RowsPerPage
		
		if( index < viewIndex )
			viewIndex = index
		if( index >= viewIndex + RowsPerPage )
			viewIndex = index - RowsPerPage + 1
		
		if( viewIndex > maxView )
			viewIndex = maxView
		if( viewIndex < 0 )
			viewIndex = 0
		
		if( viewIndex != prevView && onViewChanged )
			onViewChanged( viewIndex )
	}
	
	function HighlightedUserId( )
	{
		if( board && selectedIndex in rows && !rows[ selectedIndex ].IsNull( ) )
			return rows[ selectedIndex ].userId
		else
			return null
	}
	
	function HighlightedGamertag( )
	{
		if( board && selectedIndex in rows && !rows[ selectedIndex ].IsNull( ) )
			return rows[ selectedIndex ].gamerTag
		else
			return null
	}
	
	function ScrollToPlayer( )
	{
		if( playerIndex == null )
		{
			ScrollToInstant( 0 )
			return
		}
		
		local rowsPerPage = ::LeaderboardCanvas.RowsPerPage
		local scrollTo = ::Math.Max( playerIndex - ( rowsPerPage / 2 ) + 1, 0 )
		if( rows.len( ) < rowsPerPage )
			scrollTo = 0
		else if( playerIndex >= rows.len( ) - ( rowsPerPage / 2 ) )
			scrollTo = rows.len( ) - rowsPerPage
		::print( "scrollTo:" + scrollTo.tostring( ) )
		SetSelection( playerIndex )
		viewIndex = scrollTo
		UpdateVisibility( )
		if( onViewChanged )
			onViewChanged( viewIndex )
	}
}