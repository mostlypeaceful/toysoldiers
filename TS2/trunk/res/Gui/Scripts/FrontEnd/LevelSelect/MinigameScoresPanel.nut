// Minigame scores

// Requires
sigimport "gui/scripts/frontend/levelselect/levelscorespanel.nut"
sigimport "gui/scripts/controls/asyncstatus.nut"

class MinigameScoresPanel extends BaseLevelScoresPanel
{
	// Display
	score = null
	xblaRank = null
	friendsRank = null
	asyncStatus = null
	
	// Data
	active = null
	board = null
	
	constructor( startingScores, startingInfo )
	{
		::BaseLevelScoresPanel.constructor( )
		autoLaunch = true
		
		// Score
		local totalWidth = 751
		local padding = 10
		local scoreSectionWidth = 300
		local scoreLabel = ::Gui.Text( )
		scoreLabel.SetFontById( FONT_FANCY_MED )
		scoreLabel.SetUniformScale( 0.7 )
		scoreLabel.SetRgba( COLOR_CLEAN_WHITE )
		scoreLabel.BakeLocString( ::GameApp.LocString( "High_Score" ), TEXT_ALIGN_CENTER )
		scoreLabel.SetPosition( padding + scoreSectionWidth * 0.5, 1, 0 )
		AddChild( scoreLabel )
		
		local scoreY = 50
		score = ::Gui.Text( )
		score.SetFontById( FONT_FANCY_LARGE )
		score.SetRgba( COLOR_CLEAN_WHITE )
		score.SetPosition( scoreLabel.GetXPos( ), scoreY, 0 )
		AddChild( score )
		
		// XBLA Rank
		local xblaSectionWidth = 220
		local xblaLabel = ::Gui.Text( )
		xblaLabel.SetFontById( FONT_FANCY_MED )
		xblaLabel.SetUniformScale( 0.7 )
		xblaLabel.SetRgba( COLOR_CLEAN_WHITE )
		xblaLabel.BakeLocString( ::GameApp.LocString( "LIVE_Rank" ), TEXT_ALIGN_CENTER )
		xblaLabel.SetPosition( padding + scoreSectionWidth  + xblaSectionWidth * 0.5, 1, 0 )
		AddChild( xblaLabel )
		
		xblaRank = ::Gui.Text( )
		xblaRank.SetFontById( FONT_FANCY_LARGE )
		xblaRank.SetRgba( COLOR_CLEAN_WHITE )
		xblaRank.SetPosition( xblaLabel.GetXPos( ), scoreY, 0 )
		AddChild( xblaRank )
		
		// Friends Rank
		local friendsSectionWidth = 220
		local friendsLabel = ::Gui.Text( )
		friendsLabel.SetFontById( FONT_FANCY_MED )
		friendsLabel.SetUniformScale( 0.7 )
		friendsLabel.SetRgba( COLOR_CLEAN_WHITE )
		friendsLabel.BakeLocString( ::GameApp.LocString( "Friends_Rank" ), TEXT_ALIGN_CENTER )
		friendsLabel.SetPosition( padding + scoreSectionWidth  + xblaSectionWidth + friendsSectionWidth * 0.5, 1, 0 )
		AddChild( friendsLabel )
		
		friendsRank = ::Gui.Text( )
		friendsRank.SetFontById( FONT_FANCY_LARGE )
		friendsRank.SetRgba( COLOR_CLEAN_WHITE )
		friendsRank.SetPosition( friendsLabel.GetXPos( ), scoreY, 0 )
		AddChild( friendsRank )
		
		if( ::GameApp.Language == LANGUAGE_SPANISH )
		{
			xblaLabel.SetScale( ::Math.Vec2.Construct( 0.5, 0.7 ) )
			friendsLabel.SetScale( ::Math.Vec2.Construct( 0.5, 0.7 ) )
		}
		
		asyncStatus = ::AsyncStatusSmall( )
		asyncStatus.SetPosition( totalWidth * 0.75, scoreY + 20, 0 )
		AddChild( asyncStatus )
		
		SetLevelInfo( startingScores, startingInfo, null )
	}

	function SetLevelInfo( scores, info, playerUnused )
	{
		local highScore = scores.GetHighScore( 0 )
		local str = null
		local havePlayed = highScore >= 0
		if( havePlayed )
			str = ::StringUtil.AddCommaEvery3Digits( highScore.tointeger( ).tostring( ) )
		else
			str = "---"
		
		score.BakeCString( str, TEXT_ALIGN_CENTER )
		
		if( havePlayed )
		{
			// Leaderboard
			board = ::Leaderboard( )
			local boardId = ::GameAppSession.GetLevelLeaderboardId( MAP_TYPE_MINIGAME, info.LevelIndex, 0 )
			board.AddBoardToRead( boardId )
			board.SelectBoard( boardId )
			board.ReadByFriends( ::GameApp.FrontEndPlayer.User, 0 )
			SetRank( )
		}
		else
		{
			board = null
			asyncStatus.SetAlpha( 0 )
			xblaRank.SetAlpha( 0 )
			friendsRank.SetAlpha( 0 )
		}
	}
	
	function OnTick( deltaTime )
	{
		::BaseLevelScoresPanel.OnTick( deltaTime )
		
		if( board && board.AdvanceRead( ) )
		{
			SetRank( )
		}
	}
	
	function SetRank( )
	{
		asyncStatus.SetAlpha( 1 )
		xblaRank.SetAlpha( 0 )
		friendsRank.SetAlpha( 0 )
		
		if( board && board.AdvanceRead( ) && board.RowsAvailable > 0 )
		{
			asyncStatus.SetAlpha( 0 )
			
			local userRow = board.RowForUserId( ::GameApp.FrontEndPlayer.User.UserId )
			if( userRow != ~0 )
			{
				local totalRank = board.RowRank( userRow )
				xblaRank.SetAlpha( 1 )
				xblaRank.BakeCString( "#" + ::StringUtil.AddCommaEvery3Digits( totalRank.tointeger( ).tostring( ) ), TEXT_ALIGN_CENTER )
				
				local friends = [ ]
				local rankAmongFriends = 0
				local numRows = board.RowsAvailable
				for( local ri = 0; ri < numRows; ++ri )
				{
					friends.push( board.RowRank( ri ) )
				}
				
				friends.sort( )
				
				foreach( i, rank in friends )
				{
					if( rank == totalRank )
						rankAmongFriends = i + 1
				}
				
				friendsRank.SetAlpha( 1 )
				friendsRank.BakeCString( "#" + rankAmongFriends.tointeger( ).tostring( ), TEXT_ALIGN_CENTER )
			}
		}
	}

	function FillLoadInfo( info )
	{
		info.Difficulty = DIFFICULTY_NORMAL // ??? Default difficulty for minigames?
		info.ChallengeMode = 0
	}
}