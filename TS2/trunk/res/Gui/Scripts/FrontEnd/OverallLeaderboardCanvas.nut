// Block Leaderboard Canvas

// Requires
sigimport "gui/scripts/frontend/leaderboardcanvas.nut"

class OverallLeaderboardCanvas extends LeaderboardCanvas
{
	// Data
	selectedIndexAfterLoad = null
	
	// Statics
	static MaxReadCount = 100
	
	constructor( board, levelInfo, user )
	{
		::LeaderboardCanvas.constructor( board, "Rank_NoData", levelInfo, user )
		selectedIndexAfterLoad = 0
	}
	
	function ChangeHighlight( delta )
	{
		if( state == LBState.Loading )
			return
			
		local nextIndex = selectedIndex + delta
		
		if( board.RowsTotal <= MaxReadCount )
		{
			::LeaderboardCanvas.ChangeHighlight( delta )
		}
		else if( nextIndex < 0 )
		{
			ReadPrevious( )
			selectedIndexAfterLoad = -1
		}
		else if( nextIndex >= rows.len( ) )
		{
			ReadNext( )
			selectedIndexAfterLoad = 0
		}
		else
		{
			::LeaderboardCanvas.ChangeHighlight( delta )
		}
	}

	function Read( )
	{
		::print( "Overall Read" )
		board.ReadByRank( currentReadIndex + 1, MaxReadCount )
		::LeaderboardCanvas.Read( )
	}
	
	function OnLoadingComplete( )
	{
		::LeaderboardCanvas.OnLoadingComplete( )
		state = LBState.Finished
		
		if( selectedIndexAfterLoad >= 0 )
			SetSelection( selectedIndexAfterLoad )
		else // Go to the bottom
			SetSelection( rows.len( ) - 1 )
	}
	
	function Count( )
	{
		return board.RowsTotal
	}
	
	function ReadPrevious( )
	{
		Reset( )
		currentReadIndex = currentReadIndex - MaxReadCount
		if( currentReadIndex < 0 )
			currentReadIndex = board.RowsTotal - ( board.RowsTotal % MaxReadCount )
		Read( )
	}
	
	function ReadNext( )
	{
		Reset( )
		currentReadIndex = currentReadIndex + MaxReadCount
		if( currentReadIndex > board.RowsTotal - 1 )
			currentReadIndex = 0
		Read( )
	}
	
	function Reset( )
	{
		AddChild( async )
		
		foreach( row in rows )
		{
			RemoveChild( row )
			row.DeleteSelf( )
		}
		rows.clear( )
	}
}