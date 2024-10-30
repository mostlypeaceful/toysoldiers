// My Score Leaderboard Canvas

// Requires
sigimport "gui/scripts/frontend/leaderboardcanvas.nut"

class MyScoreLeaderboardCanvas extends LeaderboardCanvas
{
	constructor( board, levelInfo, user )
	{
		::LeaderboardCanvas.constructor( board, "LB_No_Data", levelInfo, user )
	}
	
	function Read( )
	{
		::print( "MyScore Read" )
		currentReadIndex = 0
		board.ReadByRankAround( user, 100 )
		::LeaderboardCanvas.Read( )
	}
	
	function OnLoadingComplete( )
	{
		::print( "MyScore OnLoadingComplete" )
		::LeaderboardCanvas.OnLoadingComplete( )
		state = LBState.Finished
		ScrollToPlayer( )
	}
}