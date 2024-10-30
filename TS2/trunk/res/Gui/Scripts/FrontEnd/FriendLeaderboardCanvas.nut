// Friend Leaderboard Canvas

// Requires
sigimport "gui/scripts/frontend/leaderboardcanvas.nut"

class FriendLeaderboardCanvas extends LeaderboardCanvas
{
	constructor( board, levelInfo, user )
	{
		::LeaderboardCanvas.constructor( board, "LB_No_FriendsData", levelInfo, user )
	}
	
	function Read( )
	{
		::print( "Friends Read" )
		currentReadIndex = 0
		board.ReadByFriends( user, 0 )
		::LeaderboardCanvas.Read( )
	}
	
	function OnLoadingComplete( )
	{
		::print( "Friends OnLoadingComplete" )
		::LeaderboardCanvas.OnLoadingComplete( )
		state = LBState.Finished
	}
}