// Panel to display friends' high scores for levels ( 425 x 210 )

// Requires
sigimport "gui/scripts/utility/medalscripts.nut"
sigimport "gui/scripts/controls/asyncstatus.nut"

class FriendLeaderboardItem extends AnimatingCanvas
{
	// Display
	name = null
	score = null
	medal = null
	
	// Data
	scoreValue = null
	isPlayer = null
	
	// Statics
	static scoreColumn = 180
	static medalColumn = 280
	
	constructor( name_, score_, medal_, isPlayer_ = false )
	{
		::AnimatingCanvas.constructor( )
		scoreValue = score_
		isPlayer = isPlayer_
		
		local color = ( isPlayer? ::Math.Vec4.Construct( 1, 1, 0, 1 ): COLOR_CLEAN_WHITE )
		
		name = ::Gui.Text( )
		name.SetFontById( FONT_SIMPLE_SMALL )
		name.SetRgba(color  )
		name.BakeLocString( name_, TEXT_ALIGN_LEFT )
		AddChild( name )
		
		name.Compact( scoreColumn - 10 )
		
		score = ::Gui.Text( )
		score.SetFontById( FONT_SIMPLE_SMALL )
		score.SetRgba( color )
		local str = null
		local havePlayed = scoreValue >= 0
		if( havePlayed )
			str = ::StringUtil.AddCommaEvery3Digits( scoreValue.tostring( ) )
		else
			str = "---"
		score.BakeCString( str, TEXT_ALIGN_LEFT )
		score.SetPosition( scoreColumn, 0, 0 )
		AddChild( score )
		
		if( havePlayed && ( medal_ != null ) )
		{
			medal = ::Gui.TexturedQuad( )
			local medalTexture = ::VictoryScreenMedalImagePath( medal_, MEDAL_SIZE_SMALL, true )
			if( medalTexture )
			{
				medal.SetTexture( medalTexture )
				medal.SetPosition( medalColumn, 0, 0 )
				AddChild( medal )
			}
		}
	}
	
	function _cmp( other )
	{
		if( scoreValue > other.scoreValue )
			return -1
		else if( scoreValue < other.scoreValue )
			return 1
		else
		{
			if( isPlayer )
				return -1
			else
				return 0
		}
	}
}

class FriendLeaderboardPanel extends AnimatingCanvas
{
	// Display
	label = null // Gui.Text
	difficultyLabel = null
	items = null
	asyncStatus = null
	
	// Data
	data = null
	stats = null
	playerName = null
	levelInfo = null
	scores = null
	difficulty = null
	prevInit = null
	noMedals = null
	displayCount = null
	extraItems = null
	noTitle = null
	
	// Statics
	static cellWidth = 364
		
	constructor( gamertag, playerStats, startingLevelInfo, startingScores, startingDifficulty, displayCount_ = 6, noTitle_ = false )
	{
		::AnimatingCanvas.constructor( )
		items = [ ]
		stats = playerStats
		playerName = gamertag
		prevInit = false
		noMedals = false
		displayCount = displayCount_
		extraItems = [ ]
		noTitle = noTitle_
		
		label = ::Gui.Text( )
		label.SetFontById( FONT_FANCY_MED )
		label.SetPosition( cellWidth * 0.5, 1, 0 )
		if( ::GameApp.IsAsianLanguage( ) )
			label.SetYPos( 2 )
		else
			label.SetUniformScale( 0.7 )
		label.SetRgba( COLOR_CLEAN_WHITE )
		label.BakeLocString( ::GameApp.LocString( "Friend_Scores" ), TEXT_ALIGN_CENTER )
		AddChild( label )
		
		if( ::GameApp.IsWideLanguage( ) || ::GameApp.IsAsianLanguage( ) )
			label.SetScale( 0.7 * label.GetScale( ).x, label.GetScale( ).y )
		
		difficultyLabel = ::Gui.Text( )
		difficultyLabel.SetFontById( FONT_SIMPLE_SMALL )
		difficultyLabel.SetRgba( COLOR_CLEAN_WHITE )
		difficultyLabel.SetPosition( cellWidth + 17, 1, 0 )
		AddChild( difficultyLabel )
		
		if( startingLevelInfo.MapType != MAP_TYPE_CAMPAIGN )
			noMedals = true
			
		asyncStatus = ::AsyncStatusSmall( )
		asyncStatus.SetPosition( cellWidth * 0.5, 118, 0 )
		AddChild( asyncStatus )

		SetLevelInfo( startingLevelInfo, startingScores, startingDifficulty )
	}
	
	function SetLevelInfo( levelInfo_, scores_, difficulty_ )
	{
		levelInfo = levelInfo_
		scores = scores_
		if( levelInfo.MapType == MAP_TYPE_HEADTOHEAD )
			difficulty = DIFFICULTY_NORMAL
		else if( levelInfo.MapType == MAP_TYPE_MINIGAME )
			difficulty = 0
		else
			difficulty = difficulty_
		
		data = stats.RequestLevelData( levelInfo.MapType, levelInfo.LevelIndex )
		if( is_null( data ) )
			data = null
		
		if( data )
			prevInit = data.Initialized
		
		asyncStatus.SetAlpha( 1 )
		SetItems( )
	}
	
	function SetItems( )
	{
		foreach( item in items )
		{
			item.DeleteSelf( )
			RemoveChild( item )
		}
		items.clear( )
		
		local friendData = [ ]
		if( data && data.Initialized )
		{
			local count = data.ScoreCount( difficulty )
			for( local i = 0; i < count; ++i )
			{
				friendData.push( data.GetScore( difficulty, i ) )
			}
			
			asyncStatus.SetAlpha( 0 )
		}
		
		local validFriends = 0
		foreach( d in friendData )
		{
			if( d.Score > 0 )
			{
				validFriends++
				local item = ::FriendLeaderboardItem( d.GamerTag, d.Score, d.OverallMedal )
				AddChild( item )
				items.push( item )
			}
			
			if( validFriends >= displayCount )
				break
		}
		
		// Push Player's scores
		local playerScore = scores.GetHighScore( difficulty )
		local medal = scores.MedalProgress( difficulty, MEDAL_TYPE_OVERALL )
		local item = ::FriendLeaderboardItem( playerName, playerScore, noMedals? null: medal, true )
		AddChild( item )
		items.push( item )
		
		// Extra items
		foreach( extra in extraItems )
		{
			local item = ::FriendLeaderboardItem( extra.Name, extra.Score, noMedals? null: extra.Medal, false )
			AddChild( item )
			items.push( item )
			
			if( extra.Color )
				item.SetRgba( extra.Color )
		}
		
		// Sort
		items.sort( )
		
		// Position
		local startY = 30
		local spacing = 25
		foreach( i, item in items )
			item.SetPosition( 10, startY + spacing * i, 0 )
		
		if( !noTitle )
			SetTitle( difficulty )
	}
	
	function AddExtraItem( name, score, medal = null, color = null )
	{
		extraItems.push( { Name = name, Score = score, Medal = medal, Color = color } )
	}
	
	function SetTitle( mode )
	{
		local names = null
		
		if( levelInfo.MapType == MAP_TYPE_CAMPAIGN )
		{
			names = [
				"Difficulty_Casual",
				"Difficulty_Normal",
				"Difficulty_Hard",
				"Difficulty_Elite",
				"Difficulty_General"
			]
		}
		else if( levelInfo.MapType == MAP_TYPE_SURVIVAL )
		{
			names = [
				"ChallengeMode_Survival",
				"ChallengeMode_Lockdown",
				"ChallengeMode_Hardcore",
				"ChallengeMode_Trauma",
				"ChallengeMode_Lockcore"
			]
		}
		
		if( names != null && mode != null && mode in names )
			difficultyLabel.BakeLocString( ::GameApp.LocString( names[ mode ] ), TEXT_ALIGN_RIGHT )
		else
			difficultyLabel.BakeCString( " " )
	}
	
	function OnTick( dt )
	{
		::AnimatingCanvas.OnTick( dt )
		
		if( data )
		{
			if( !prevInit && data.Initialized )
				SetItems( )
			
			prevInit = data.Initialized
		}
	}
}