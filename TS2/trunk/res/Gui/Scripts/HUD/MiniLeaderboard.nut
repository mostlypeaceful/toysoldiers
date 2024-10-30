// Mini Leaderboard for the Trial

// Requires
sigimport "gui/scripts/controls/animatingcanvas.nut"
sigimport "gui/scripts/endgamescreens/scoreutility.nut"

// Resources
sigimport "gui/textures/score/score_decoration_g.png"
sigimport "gui/textures/trial/minileaderboard_glow_g.png"

class JitterName extends AnimatingCanvas
{
	name = null	
	jitterCanvas = null
	isPlayer = false	
	nameString = null
	
	constructor( name_, isPlayer_ )
	{
		nameString = name_
		::AnimatingCanvas.constructor( )
		
		isPlayer = isPlayer_
		
		SetName( name_ )
	}
	
	function SetName( name_ )
	{
		if( name )
			name.DeleteSelf( )
		if( jitterCanvas )
			jitterCanvas.DeleteSelf( )
			
		nameString = name_
			
		local col = 155
		
		jitterCanvas = ::AnimatingCanvas( )
		
		name = ::Gui.Text( )
		name.SetFontById( FONT_SIMPLE_SMALL )
		name.SetRgba( COLOR_CLEAN_WHITE )
		name.SetPosition( ::Math.Vec3.Construct( 0, 0, 0 ) )
		name.BakeLocString( name_, TEXT_ALIGN_LEFT )
		
		local textScale = name.CompactGetScale( col + 20 )

		jitterCanvas.AddChild( name )
		jitterCanvas.ShiftPivot( ::Math.Vec2.Construct( name.Width * textScale * 0.5, name.Height * 0.5 ) )
		jitterCanvas.SetPosition( col - name.Width * textScale * 0.5, name.Height * 0.5, 0 )
		AddChild( jitterCanvas )
		
		if( isPlayer )
		{
			local yellow = ::Math.Vec4.Construct( 1.0, 1.0, 0.0, 1.0 )
			name.SetRgba( yellow )
		}
	}
}

class MiniLeaderboardEntry extends AnimatingCanvas
{
	// Display
	jitterName = null
	scoreText = null
	glow = null
	
	// Data
	score = null
	isPlayer = null
	jitterFactor = null
	jittering = null
	
	nameString = null

	initialScale = 1.0		//the player has a slightly higher value
	
	constructor( name_, score_, isPlayer_ )
	{
		::AnimatingCanvas.constructor( )
		
		score = score_
		isPlayer = isPlayer_
		jitterFactor = 0
		jittering = false
		nameString = name_
		
		local col = 155
		
		jitterName = ::JitterName( name_, isPlayer_ )
		AddChild( jitterName )
		
		scoreText = ::Gui.Text( )
		scoreText.SetFontById( FONT_SIMPLE_SMALL )
		scoreText.SetRgba( COLOR_CLEAN_WHITE )
		scoreText.SetPosition( ::Math.Vec3.Construct( col + 10, 0, 0 ) )
		AddChild( scoreText )
		SetScore( score_ )
		
		if( isPlayer )
		{
			glow = ::AnimatingCanvas( )
				local glowImage = ::Gui.TexturedQuad( )
				glowImage.SetTexture( "gui/textures/trial/minileaderboard_glow_g.png" )
				glowImage.SetPosition( -glowImage.LocalRect.Width, -glowImage.LocalRect.Height * 0.5, 0 )
				glow.AddChild( glowImage )
			glow.SetPosition( glowImage.LocalRect.Width + 10, glowImage.LocalRect.Height * 0.5 - 3, 0.01 )
			glow.SetAlpha( 0.5 )
			AddChild( glow )
		}
		
		if( isPlayer )
		{
			local yellow = ::Math.Vec4.Construct( 1.0, 1.0, 0.0, 1.0 )
			scoreText.SetRgba( yellow )
			scoreText.SetUniformScale( initialScale )
		}
	}
	
	function SetName( name_ )
	{
		nameString = name_
		jitterName.SetName( name_ )
	}
	
	function SetScore( score_ )
	{
		score = score_
		scoreText.BakeCString( ::StringUtil.AddCommaEvery3Digits( score.tointeger( ).tostring( ) ), TEXT_ALIGN_LEFT )
		scoreText.Compact( 100 )
	}
	
	function SetJitter( jitter )
	{
		jitterFactor = jitter
		if( jitterFactor == null || jitterFactor == 0 )
		{
			if( jittering )
			{
				jittering = false
				jitterName.jitterCanvas.ClearActions( )
				jitterName.jitterCanvas.SetAngle( 0 )
				jitterName.jitterCanvas.SetUniformScale( initialScale )
				jitterName.jitterCanvas.SetAlpha( 1 )
				
				if( glow )
				{
					glow.ClearActions( )
					glow.SetUniformScale( 1.0 )
					glow.SetAlpha( 0.5 )
				}
			}
		}
		else if( !jittering )
		{
			jittering = true
			DoJitter( )
			if( glow )
			{
				glow.ClearActions( )
				glow.SetUniformScale( 1.0 )
				glow.AddAction( ::AlphaPulse( 0.5, 1.0, 0.2 + ( 0.5 * ( 1 - jitterFactor ) ) ) )
			}
		}
	}
	
	function DoJitter( )
	{
		local angleRange = ::Math.Vec2.Construct( -MATH_PI / 20, MATH_PI / 20 )
		local scaleRange = ::Math.Vec2.Construct( -0.1, 0.2 )
		local timeRange = ::Math.Vec2.Construct( 0.01, 0.09 )
		
		local newAngle = ::SubjectiveRand.Float( angleRange.x, angleRange.y ) * jitterFactor
		local newScale = initialScale + ( ::SubjectiveRand.Float( scaleRange.x, scaleRange.y ) * jitterFactor )
		local time = ::Math.Lerp( timeRange.x, timeRange.y, ( 1 - jitterFactor ) )
		time = ::SubjectiveRand.Float( time - 0.02, time + 0.02 )
		
		jitterName.jitterCanvas.AddAction( ::AngleTween( jitterName.jitterCanvas.GetAngle( ), newAngle, time, EasingTransition.Linear, null, null, 
			function( canvas ) { DoJitter( ) }.bindenv(this) ) )
		jitterName.jitterCanvas.AddAction( ::UniformScaleTween( jitterName.jitterCanvas.GetScale( ).x, newScale, time ) )
	}
}

class MiniLeaderboard extends AnimatingCanvas
{
	// Display
	entries = null // array of MiniLeaderboardEntry objects
	playerEntry = null // MiniLeaderboardEntry
	previousBest = null // MiniLeaderboardEntry
	liveBest = null
	
	// Data
	user = null // User
	friendsScores = null // array
	leaderboard = null // Leaderboard object
	xblaLeaderboard = null // Leaderboard for XBLA users
	hasScores = null // bool, false if we haven't retrieved leaderboard scores
	bestXblaScore = null
	bestFriendsScore = null
	leaderboardId = null
	playerIndex = 0
		
	// Callbacks
	onNewPreviousBest = null
	
	timeSinceLastRead = 0
	leaderboardReadTime = 300 // 5minutes * 60seconds/minute
	playerToChangeTo = null
	
	static rowStart = 5
	static rowSpacing = 24
	
	constructor( player, leaderboardId_ )
	{
		::AnimatingCanvas.constructor( )
		
		hasScores = false
		user = player.User
		playerIndex = player.PlayerIndex
		leaderboardId = leaderboardId_
		
		// Background Image
		local background = ::Gui.TexturedQuad( )
		background.SetTexture( "gui/textures/score/score_decoration_g.png" )
		background.SetPosition( 0, 0, 0 )
		AddChild( background )
		
		local background2 = ::Gui.TexturedQuad( )
		background2.SetTexture( "gui/textures/score/score_decoration_g.png" )
		background2.SetPosition( 0, rowStart + rowSpacing * 5 + rowStart, 0 )
		AddChild( background2 )
		
		entries = [ ]
		
		// Get Leaderboard Data
		leaderboard = ::Leaderboard( )
		leaderboard.AddBoardToRead( leaderboardId )
		leaderboard.SelectBoard( leaderboardId )
		leaderboard.ReadByFriends( user, 0 )
		
		xblaLeaderboard = ::Leaderboard( )
		xblaLeaderboard.AddBoardToRead( leaderboardId )
		xblaLeaderboard.SelectBoard( leaderboardId )
		xblaLeaderboard.ReadByRank( 0, 20 )
		
		// Create the player entry
		playerEntry = ::MiniLeaderboardEntry( user.GamerTag, 0, true )
		playerEntry.SetPosition( ::Math.Vec3.Construct( 0, rowStart + rowSpacing * 0, 0 ) )
		AddChild( playerEntry )
		
		// Previous Best text
		previousBest = ::MiniLeaderboardEntry( ::GameApp.LocString( "Previous_Best" ), 0, false )
		previousBest.SetPosition( 0, rowStart + rowSpacing * 5 + rowStart * 2, 0 )
		AddChild( previousBest )
		
		// LIVE Best text
		liveBest = ::MiniLeaderboardEntry( ::GameApp.LocString( "LIVE_Best" ), 0, false )
		liveBest.SetPosition( 0, rowStart + rowSpacing * 6 + rowStart * 2, 0 )
		AddChild( liveBest )
	}
	
	function OnTick( dt )
	{
		::AnimatingCanvas.OnTick( dt )
		
		timeSinceLastRead += dt
		
		// Get leaderboard stats
		if( !hasScores )
		{
			if( leaderboard.AdvanceRead( ) && xblaLeaderboard.AdvanceRead( ) )
			{
				hasScores = true
				CreateFriendEntries( )
				
				if( playerToChangeTo )
					ChangePlayer( playerToChangeTo )
			}
		}
	}
	
	function ChangePlayer( newPlayer )
	{
		playerToChangeTo = null
			
		local levelScore = ::GetMinigameScore( newPlayer )
		if( levelScore == -1 )
			levelScore = 0
		SetPreviousBest( levelScore )
		
		local newPlayerName = newPlayer.User.GamerTag
		local newPlayerNameCString = newPlayerName.ToCString( )
		if( playerEntry.nameString.ToCString( ) == newPlayerNameCString )
			return // Don't switch to ourselves			
		
		local newPlayerIsInEntries = false
		foreach( i, entry in entries )
		{
			// Look for newPlayer in entries and swap playerEntry with newPlayer if it exists
			if( newPlayerNameCString == entry.nameString.ToCString( ) )
			{
				newPlayerIsInEntries = true
				SwapEntries( entry, playerEntry )
				break
			}
		}
		
		if( !newPlayerIsInEntries && entries.len( ) > 1 )
		{
			// If newPlayer wasn't in entries hijack lowest scoring entry, replace with old player data, and set up new player
			// this is so we keep both players on the board 
			local i;
			for( i = entries.len( ) - 1; i > 0; --i )
			{
				if( entries[ i ] != playerEntry )
				{
					SwapEntries( entries[ i ], playerEntry )
					playerEntry.SetName( newPlayerName )
					playerEntry.SetScore( 0 )
					break
				}
			}
		}
		else
			playerToChangeTo = newPlayer // Set playerToChangeTo and try to change player again after entries is populated
			
		Reposition( )
	}
	
	function SwapEntries( entryOne, entryTwo )
	{
		// Swap entry and playerEntry name and score
		local entryName = entryOne.nameString
		local entryScore = entryOne.score
		
		entryOne.SetName( entryTwo.nameString )
		entryOne.SetScore( entryTwo.score )	
					
		entryTwo.SetName( entryName )
		entryTwo.SetScore( entryScore )
	}
	
	function TryGetUpdatedLeaderboards( )
	{
		// if it has been 5 minutes since last leaderboard read
		if( timeSinceLastRead > leaderboardReadTime )
		{
			timeSinceLastRead = 0
			hasScores = false
			
			::GameAppSession.FlushStats( )
			
			// Get Leaderboard Data
			leaderboard = ::Leaderboard( )
			leaderboard.AddBoardToRead( leaderboardId )
			leaderboard.SelectBoard( leaderboardId )
			leaderboard.ReadByFriends( user, 0 )
			
			xblaLeaderboard = ::Leaderboard( )
			xblaLeaderboard.AddBoardToRead( leaderboardId )
			xblaLeaderboard.SelectBoard( leaderboardId )
			xblaLeaderboard.ReadByRank( 0, 20 )
		}
	}
	
	function CreateFriendEntries( )
	{
		// Delete entries
		foreach( i, entry in entries )
		{
			if( entry != playerEntry )
				entry.DeleteSelf( )
		}
		
		// Erase the list of entries
		entries.clear( )
		
		// Get the names and scores from the leaderboard
		local names = [ ]
		local scores = [ ]
		local numFriends = leaderboard.RowsAvailable
		local numXbla = xblaLeaderboard.RowsAvailable
		local userNameCString = user.GamerTag.ToCString( )
		local friendNames = [ ]
		
		for( local i = 0; i < numFriends; ++i )
		{
			local gamerTag = leaderboard.Data( LEADER_BOARD_COLUMN_GAMER_NAME, i ).ToString( )
			local score = leaderboard.DataById( ::GameAppSession.LEADERBOARD_MINIGAME_SCORE_ID, i )

			// Reformat score to actual numbers
			if( score.IsSet )
				score = score.ToInt( )
			else
				score = -1
			
			// Filter out friends who haven't played the minigame
			if( score == -1 )
			{
				continue
			}
			
			local statScore = GameSessionStats.LeaderBoardToStat( SESSION_STATS_MINIGAME_META_STAT, score )
			
			// Filter out the current user, and use it to set the current value
			if( userNameCString == gamerTag  )
			{
				SetPreviousBest( statScore )
				continue
			}
			
			if( !bestFriendsScore || statScore > bestFriendsScore )
				bestFriendsScore = statScore
			
			friendNames.push( gamerTag )
			names.push( LocString.FromCString( gamerTag ) )
			scores.push( statScore )
			
			if( names.len( ) >= 4 )
				break
		}
		
		// Add more if there arent enough
		for( local i = 0; i < numXbla; ++i )
		{
			local gamerTag = xblaLeaderboard.Data( LEADER_BOARD_COLUMN_GAMER_NAME, i ).ToString( )
			local score = xblaLeaderboard.DataById( ::GameAppSession.LEADERBOARD_MINIGAME_SCORE_ID, i )
			
			// Reformat score to actual numbers
			if( score.IsSet )
				score = score.ToInt( )
			else
				score = -1
			
			// Filter out the current user
			if( userNameCString == gamerTag  )
				continue
			
			// Filter out XBLA users who haven't played the minigame
			if( score == -1 )
				continue
				
			// Filter out XBLA users who are already on the list...
			local friendIsOnList = false
			foreach( name in friendNames )
			{
				if( name == gamerTag )
					friendIsOnList = true
			}
			
			local statScore = GameSessionStats.LeaderBoardToStat( SESSION_STATS_MINIGAME_META_STAT, score )
			if( !bestXblaScore || statScore > bestXblaScore )
				bestXblaScore = statScore
				
			if( friendIsOnList )
				continue
			
			if( names.len( ) < 4 )
				names.push( LocString.FromCString( gamerTag ) )
			
			scores.push( statScore )			
			
			if( names.len( ) >= 4 )
				break
		}
		
		SetLiveBest( bestXblaScore )
		
		// if the leaderboards fail fall back to the saved score
		// scratch that always do it
		//if( !previousBest.score )
		{
			local levelScore = ::GetMinigameScore( ::GameApp.GetPlayer( playerIndex ) )
			if( levelScore == -1 )
				levelScore = 0
			SetPreviousBest( levelScore )
		}
		
		// Add the entries for friends
		for( local i = 0; i < names.len( ); ++i )
		{
			local entry = ::MiniLeaderboardEntry( names[ i ], scores[ i ], false )
			entry.SetPosition( ::Math.Vec3.Construct( 0, rowStart + rowSpacing * i, 0 ) )
			AddChild( entry )
			entries.push( entry )
		}
		
		// Add the entry for the player
		entries.push( playerEntry )
		playerEntry.SetScore( 0 )
		
		// Sort Entries
		entries.sort( function( a, b )
			{
				if( a.score < b.score ) return 1
				else if( a.score > b.score ) return -1
				else return 0
			} )

		// Set up positions
		local rowStart = 5
		local rowSpacing = 24
		foreach( i, entry in entries )
		{
			entry.SetYPos( rowStart + rowSpacing * i )
		}
	}	
	
	function SetPlayerScore( score )
	{
		// Change the player's score
		playerEntry.SetScore( score )
		
		// Set Jitter using next player's score
		local nextScore = -1
		
		// Resort entries based on score
		foreach( i, entry in entries )
		{
			if( entry == playerEntry )
			{
				entries.remove( i )
				break
			}
		}
		
		local playerHasLowestScore = true
		foreach( i, entry in entries )
		{
			if( entry.score > playerEntry.score )
			{
				nextScore = entry.score
				continue
			}
			
			playerHasLowestScore = false
			entries.insert( i, playerEntry )
			break
		}
		
		// Player has lowest score, insert last
		if( playerHasLowestScore )
			entries.insert( entries.len( ), playerEntry )
		
		if( nextScore > -1 )
		{
			local scoreThreshold = nextScore * 0.5	//when we get within 50% of the next highest score...start jittering!
			if( score > scoreThreshold && nextScore > score )
				playerEntry.SetJitter( ::Math.Remap( scoreThreshold, nextScore, score ) )
			else
				playerEntry.SetJitter( null )
		}
		else
			playerEntry.SetJitter( null )
		
		// Reposition all entries
		Reposition( )
	}
	
	function SetPreviousBest( score )
	{
		if( score < 0 )
			previousBest.SetAlpha( 0 )
		else
		{
			{
				if( onNewPreviousBest )
					onNewPreviousBest( previousBest.score, score )
				previousBest.SetScore( score )
			}
		}
	}
	
	function SetLiveBest( score )
	{
		liveBest.SetAlpha( 1 )
		if( score != null && score > 0 )
			liveBest.SetScore( score )
		else
			liveBest.SetAlpha( 0 )
	}
	
	function BetterThanTheRest( )
	{
		if( entries.len( ) <= 1 )
			return false
		else if( ( !bestXblaScore && playerEntry.score > 0 ) || ( bestXblaScore != null && playerEntry.score > bestXblaScore ) )
			return true
		else
			return false
	}
	
	function FriendsBest( )
	{
		if( entries.len( ) <= 1 )
			return false
		else if( ( !bestFriendsScore && playerEntry.score > 0 ) || ( bestFriendsScore != null && playerEntry.score > bestFriendsScore ) )
			return true
		else
			return false
	}
	
	function Reposition( )
	{
		local rowStart = 5
		local rowSpacing = 24
		
		foreach( i, entry in entries )
		{
			local currentY = entry.GetYPos( )
			local goalY = rowStart + rowSpacing * i
			
			if( currentY != goalY )
			{
				entry.ClearActions( )
				entry.AddAction( ::YMotionTween( currentY, goalY, 0.3 ) )
			}
		}
	}
}