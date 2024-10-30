// Minigame Pause Menu Content

// Requires
sigimport "gui/scripts/frontend/levelselect/tipspanel.nut"
sigimport "gui/scripts/frontend/levelselect/medalpanel.nut"
sigimport "gui/scripts/pausemenu/pausecontentbase.nut"

// Resources
sigimport "gui/textures/pausemenu/minipause_background_g.png"

class MinigamePauseContent extends PauseContentBase
{
	constructor( player )
	{
		::PauseContentBase.constructor( player )
		SetBackground( "gui/textures/pausemenu/minipause_background_g.png" )
		
		local cell1 = ::Math.Rect.ConstructEx( 0, 0, 368, 90 )
		local cell2 = ::Math.Rect.ConstructEx( 371, 0, 425, 208 )
		local cell3 = ::Math.Rect.ConstructEx( 0, 300, contentSize.x, 194 )
		local cell4 = ::Math.Rect.ConstructEx( 0, 91, 368, 207 )
		
		AddLabel( "cell1", origin.x + cell1.Center.x, origin.y, true )
		SetLabel( "cell1", "High_Score" )
		
		// Scores
		local stats = player.Stats
		local currentScore = ::Gui.Text( )
		currentScore.SetFontById( FONT_FANCY_LARGE )
		currentScore.SetRgba( COLOR_CLEAN_WHITE )
		currentScore.SetPosition( origin.x + cell1.Center.x, origin.y + cell1.Top + 30, 0 )
		currentScore.BakeCString( ::StringUtil.AddCommaEvery3Digits( ::GameApp.CurrentLevel.MiniGameCurrentScore.tointeger( ).tostring( ) ), TEXT_ALIGN_CENTER )
		AddChild( currentScore )
		
		// Rank
		local profile = player.GetUserProfile( )
		local rank = ::MedalPanel( player.Stats, levelInfo, profile.GetLevelScores( levelInfo.MapType, levelInfo.LevelIndex ) )
		rank.SetPosition( origin.x + cell4.Left, origin.y + cell4.Top, 0 )
		AddChild( rank )
		
		// Leaderboards
		local friendsLeaderBoard = ::FriendLeaderboardPanel( ::GameApp.FrontEndPlayer.User.GamerTag, ::GameApp.FrontEndPlayer.Stats, levelInfo, player1Scores, levelInfo.Difficulty )
		friendsLeaderBoard.SetPosition( origin.x + cell2.Left, origin.y, 0 )
		friendsLeaderBoard.AddExtraItem( ::GameApp.LocString( "Minigame_CurrentScore" ), ::GameApp.CurrentLevel.MiniGameCurrentScore.tointeger( ), null, COLOR_SUSPENDED_BLUE )
		friendsLeaderBoard.SetLevelInfo( levelInfo, player1Scores, levelInfo.Difficulty )
		AddChild( friendsLeaderBoard )
		
		// Tips
		local tips = ::TipsPanel( ::GameApp.CurrentLevelLoadInfo )
		tips.SetPosition( origin.x + 20, origin.y + cell3.Top, 0 )
		AddChild( tips )
	}
}