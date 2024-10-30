// Survival Pause Content

// Requires

sigimport "gui/scripts/pausemenu/pausecontentbase.nut"

// Resources
sigimport "gui/textures/pausemenu/minipause_background_g.png" 

class SurvivalPauseContent extends PauseContentBase
{
	constructor( player )
	{
		::PauseContentBase.constructor( player )
		SetBackground( "gui/textures/pausemenu/minipause_background_g.png" )
		
		local cell1 = ::Math.Rect.ConstructEx( 0, 0, 368, 90 )
		local cell2 = ::Math.Rect.ConstructEx( 371, 0, 425, 208 )
		local cell3 = ::Math.Rect.ConstructEx( 0, 300, contentSize.x, 194 )
		local cell4 = ::Math.Rect.ConstructEx( 0, 91, 368, 207 )
		
		AddLabel( "cell1", origin.x + cell1.Center.x, origin.y + cell1.Top, true )
		SetLabel( "cell1", "High_Score" )
		
		// Scores
		local stats = player.Stats
		local killsLabel = ::Gui.Text( )
		killsLabel.SetFontById( FONT_SIMPLE_SMALL )
		killsLabel.SetRgba( COLOR_CLEAN_WHITE )
		killsLabel.BakeLocString( ::GameApp.LocString( "Survival_Kills" ), TEXT_ALIGN_LEFT )
		killsLabel.SetPosition( origin.x + 10, origin.y + cell1.Top + 30 + ( cell1.Height - 30 - killsLabel.Height ) * 0.5, 0 )
		AddChild( killsLabel )
		
		local killsLabelScale = ( ::GameApp.IsWideLanguage( ) ? 0.75 : 1.0 )
		killsLabel.SetScale( ::Math.Vec2.Construct( killsLabelScale, 1.0 ) )
		
		local killsText = ::Gui.Text( )
		killsText.SetFontById( FONT_FANCY_LARGE )
		killsText.SetRgba( COLOR_CLEAN_WHITE )
		killsText.BakeCString( ::StringUtil.AddCommaEvery3Digits( stats.Stat( SESSION_STATS_KILLS ).tointeger( ).tostring( ) ), TEXT_ALIGN_LEFT)
		killsText.SetPosition( killsLabel.GetXPos( ) + killsLabel.Width * killsLabelScale + 10, origin.y + cell1.Top + 30 + ( cell1.Height - 30 - killsText.Height ) * 0.5, 0 )
		AddChild( killsText )
		
		local timeLabel = ::Gui.Text( )
		timeLabel.SetFontById( FONT_SIMPLE_SMALL )
		timeLabel.SetRgba( COLOR_CLEAN_WHITE )
		timeLabel.BakeLocString( ::GameApp.LocString( "Survival_Time" ), TEXT_ALIGN_LEFT )
		timeLabel.SetPosition( origin.x + 190, origin.y + cell1.Top + 30 + ( cell1.Height - 30 - timeLabel.Height ) * 0.5, 0 )
		AddChild( timeLabel )
		
		local timeText = ::Gui.Text( )
		timeText.SetFontById( FONT_FANCY_LARGE )
		timeText.SetRgba( COLOR_CLEAN_WHITE )
		timeText.BakeLocString( ::LocString.ConstructTimeString( ::GameApp.CurrentLevel.SurvivalLevelTime, 0 ), TEXT_ALIGN_LEFT)
		timeText.SetPosition( timeLabel.GetXPos( ) + timeLabel.Width + 10, origin.y + cell1.Top + 30 + ( (cell1.Height - 30) - timeText.Height ) * 0.5, 0 )
		AddChild( timeText )
		
		// Leaderboards
		local friendsLeaderBoard = ::FriendLeaderboardPanel( ::GameApp.FrontEndPlayer.User.GamerTag, ::GameApp.FrontEndPlayer.Stats, levelInfo, player1Scores, levelInfo.ChallengeMode, 10, true )
		friendsLeaderBoard.SetPosition( origin.x + cell2.Left, origin.y, 0 )
		AddChild( friendsLeaderBoard )
		
		// Tips
		local tips = ::TipsPanel( levelInfo )
		tips.SetPosition( origin.x + 10, origin.y + cell3.Top, 0 )
		AddChild( tips )
		
		// Rank
		local profile = player.GetUserProfile( )
		local rank = ::MedalPanel( player.Stats, levelInfo, profile.GetLevelScores( levelInfo.MapType, levelInfo.LevelIndex ) )
		rank.SetPosition( origin.x + cell4.Left, origin.y + cell4.Top, 0 )
		AddChild( rank )
	}
}