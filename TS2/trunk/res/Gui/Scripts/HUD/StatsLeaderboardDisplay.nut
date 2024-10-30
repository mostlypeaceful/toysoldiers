// Stats Leaderboard Display (currently only used by PersonalBestUI)

// Requires
sigimport "gui/scripts/controls/animatingcanvas.nut"

// Resources
sigimport "gui/textures/score/score_decoration_g.png"

class StatsLeaderboardEntry extends AnimatingCanvas
{
	// Display 
	name = null
	scoreText = null
	
	// Data
	score = null
	isPlayer = null
	
	// Static
	static columnPos = 190
	
	constructor( name_, score_, statID, isPlayer_ )
	{
		::AnimatingCanvas.constructor( )
		
		score = score_
		isPlayer = isPlayer_
		
		name = ::Gui.Text( )
		name.SetFontById( FONT_SIMPLE_SMALL )
		name.SetRgba( COLOR_CLEAN_WHITE )
		name.SetPosition( ::Math.Vec3.Construct( columnPos, 0, 0 ) )
		name.BakeLocString( name_, TEXT_ALIGN_RIGHT )
		AddChild( name )
		
		scoreText = ::Gui.Text( )
		scoreText.SetFontById( FONT_SIMPLE_SMALL )
		scoreText.SetRgba( COLOR_CLEAN_WHITE )
		scoreText.SetPosition( ::Math.Vec3.Construct( columnPos + 10, 0, 0 ) )
		AddChild( scoreText )
		
		if( isPlayer )
		{
			name.SetRgba( ::Math.Vec4.Construct( 1.0, 1.0, 0.0, 1.0 ) )
			scoreText.SetRgb( ::Math.Vec3.Construct( 1.0, 1.0, 0.0 ) )
		}
		
		SetScore( score, statID )
	}
	
	function SetScore( score_, statID )
	{
		score = score_
		local valueString = ::GameSessionStats.StatLocValueString( score, statID )
		scoreText.BakeLocString( valueString, TEXT_ALIGN_LEFT )
	}
}

class StatsLeaderboard extends AnimatingCanvas
{
	// Display
	entries = null // array of StatsLeaderboardEntry objects
	playerEntry = null // StatsLeaderboardEntry
	
	// Data
	statID = null
	inGoalX = null
	outGoalX = null
	shown = null // bool, true after Show is finished, false after Hide is started

	// Statics
	static rowStart = 5
	static rowSpacing = 24
	static friendCount = 4
	
	constructor( statID_, playerScore, player )
	{
		::AnimatingCanvas.constructor( )
		statID = statID_
		shown = false
		
		local statLocName = ::GameSessionStats.StatLocName( statID )
		local stats = player.Stats
		local moreIsBetter = ::GameSessionStats.MoreIsBetter( statID )
		
		// Text
		local nameText = ::Gui.Text( )
		nameText.SetFontById( FONT_FANCY_MED )
		nameText.SetRgba( COLOR_CLEAN_WHITE )
		nameText.BakeLocString( statLocName, TEXT_ALIGN_LEFT )
		nameText.SetPosition( 0, -nameText.LineHeight, 0 )
		AddChild( nameText )
		
		/*local label = ::Gui.Text( )
		label.SetFontById( FONT_SIMPLE_SMALL )
		label.SetRgba( 0.8, 0.8, 0.8, 1.0 )
		label.SetPosition( 0, -( nameText.LineHeight + label.LineHeight ), 0 )
		label.BakeLocString( ::GameApp.LocString( "PersonalBest" ), TEXT_ALIGN_LEFT )
		AddChild( label )*/

		// Background Image
		local background = ::Gui.TexturedQuad( )
		background.SetTexture( "gui/textures/score/score_decoration_g.png" )
		background.SetPosition( 0, 0, 0 )
		AddChild( background )
		
		local background2 = ::Gui.TexturedQuad( )
		background2.SetTexture( "gui/textures/score/score_decoration_g.png" )
		background2.SetPosition( 0, rowStart + rowSpacing * (friendCount + 1) + rowStart, 0 )
		AddChild( background2 )
		
		SetAlpha( 0 )
		
		entries = [ ]
		
		// Create the player entry
		playerEntry = ::StatsLeaderboardEntry( player.User.GamerTag, playerScore, statID, true )
		playerEntry.SetPosition( ::Math.Vec3.Construct( 0, rowStart + rowSpacing * 0, 0 ) )
		AddChild( playerEntry )
		entries.push( playerEntry )
		
		// Get the names and scores from the leaderboard
		local names = [ ]
		local scores = [ ]
		if( stats.FriendDataInitialized )
		{
			local friendStart = stats.GetFirstRelativeFriend( statID, friendCount )
			local numFriends = stats.GetRelativeFriendCount( statID, friendCount )
			
			for( local i = 0; i < numFriends; ++i )
			{
				local friendData = stats.FriendStat( statID, friendStart + i )
				if( is_null( friendData ) )
					break
				names.push( friendData.GamerTag )
				scores.push( friendData.Stat )
			}
			
			if( numFriends == 0 )
				RemoveChild( background2 )
		}
		else
		{
			RemoveChild( background2 )
		}
		
		// Add the entries for friends
		for( local i = 0; i < names.len( ); ++i )
		{
			local entry = ::StatsLeaderboardEntry( names[ i ], scores[ i ], statID, false )
			entry.SetPosition( ::Math.Vec3.Construct( 0, rowStart + rowSpacing * i, 0 ) )
			AddChild( entry )
			entries.push( entry )
		}
		
		// Sort Entries
		SortEntries( moreIsBetter )
		
		// Set up positions
		local rowStart = 5
		local rowSpacing = 24
		foreach( i, entry in entries )
		{
			entry.SetYPos( rowStart + rowSpacing * i )
		}
	}
	
	function SortEntries( moreIsBetter )
	{
		entries.sort( function( a, b ):(moreIsBetter)
		{
			if( a.score < b.score ) return (moreIsBetter? 1: -1)
			else if( a.score > b.score ) return (moreIsBetter? -1: 1)
			else return 0
		} )
	}
	
	function SetPlayerScore( score )
	{
		// Change the player's score
		playerEntry.SetScore( score, statID )
		
		// Resort entries based on score
		SortEntries( ::GameSessionStats.MoreIsBetter( statID ) )
		
		// Reposition all entries
		Reposition( )
	}
	
	function Reposition( )
	{
		foreach( i, entry in entries )
		{
			local currentY = entry.GetYPos( )
			local goalY = rowStart + rowSpacing * i
			
			//if( currentY != goalY )
			//{
				entry.ClearActions( )
				entry.AddAction( ::YMotionTween( currentY, goalY, 0.3, EasingTransition.Quadratic, EasingEquation.Out ) )
			//}
		}
	}
	
	function SetGoalPositions( inGoal, outGoal )
	{
		inGoalX = inGoal
		outGoalX = outGoal
	}
	
	function Show( )
	{
		if( !shown )
		{
			shown = true
			ClearActions( )
			AddAction( ::XMotionTween( GetXPos( ), inGoalX, 0.4 ) )
			AddAction( ::AlphaTween( GetAlpha( ), 1.0, 0.4 ) )
		}
	}
	
	function Hide( onEnd = null )
	{
		if( shown )
		{
			shown = false
			ClearActions( )
			AddAction( ::XMotionTween( GetXPos( ), outGoalX, 0.3, null, EasingEquation.In, null, onEnd ) )
			AddAction( ::AlphaTween( GetAlpha( ), 0.0, 0.3 ) )
		}
	}
	
	function HideAndDie( )
	{
		Hide( function( canvas ) { canvas.DeleteSelf( ) } )
	}
}