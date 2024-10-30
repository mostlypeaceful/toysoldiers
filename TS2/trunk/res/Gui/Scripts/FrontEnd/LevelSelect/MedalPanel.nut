// Medal panel shows rewards for a particular level

// Requires
sigimport "gui/scripts/controls/asyncstatus.nut"
sigimport "levels/scripts/common/challengescripts.nut"

// Resources
sigimport "gui/textures/frontend/rank_star_01_g.png"
sigimport "gui/textures/frontend/rank_star_02_g.png"
sigimport "gui/textures/frontend/rank_star_03_g.png"
sigimport "gui/textures/frontend/rank_star_04_g.png"
sigimport "gui/textures/cursor/barbackground_g.png"
sigimport "gui/textures/cursor/bar_g.png"
sigimport "gui/textures/cursor/powerbar_g.png"
sigimport "gui/textures/misc/challenge_decoration_g.png"
sigimport "gui/textures/frontend/rank_star_sm_01_g.png"
sigimport "gui/textures/frontend/rank_star_sm_02_g.png"
sigimport "gui/textures/frontend/rank_star_sm_03_g.png"
sigimport "gui/textures/frontend/rank_star_sm_04_g.png"

const RANKPANEL_TYPE_NORMAL = 0
const RANKPANEL_TYPE_WIDE = 1

function RankMedalImagePath( rank, small = false ) 
{
	if( rank >= 0 && rank < 4 )
		return "gui/textures/frontend/rank_star" + ( ( small )? "_sm": "" ) + "_0" + ( rank + 1 ).tostring( ) + "_g.png"
	else
		return "gui/textures/frontend/rank_star" + ( ( small )? "_sm": "" ) + "_01_g.png"
}

class MedalPanelFriendScore extends AnimatingCanvas
{
	// Display
	star = null
	nameText = null
	scoreText = null
	
	// Data
	score = null
	
	constructor( )
	{
		::AnimatingCanvas.constructor( )
		score = 0
		
		// Star
		star = ::Gui.TexturedQuad( )
		star.SetPosition( 0, 2, 0 )
		AddChild( star )
		
		// Name
		nameText = ::Gui.Text( )
		nameText.SetFontById( FONT_SIMPLE_SMALL)
		nameText.SetRgba( COLOR_CLEAN_WHITE )
		nameText.SetPosition( 32, 0, 0 )
		AddChild( nameText )
		
		// Score
		scoreText = ::Gui.Text( )
		scoreText.SetFontById( FONT_SIMPLE_SMALL)
		scoreText.SetRgba( COLOR_CLEAN_WHITE )
		scoreText.SetPosition( 320, 0, 0 )
		AddChild( scoreText )
	}
	
	function Set( rank, name, score_, threshold )
	{
		score = score_
		
		// Star
		if( rank != -1 )
		{
			star.SetTexture( ::RankMedalImagePath( rank, true ) )
			star.SetRgba( 1, 1, 1, 1 )
		}
		else
		{
			star.SetTexture( "gui/textures/frontend/rank_star_sm_01_g.png" )
			star.SetRgba( 0, 0, 0, 0.5 )
		}
		
		// Name
		nameText.BakeLocString( name, TEXT_ALIGN_LEFT )
		
		// Score
		if( threshold == null )
			scoreText.BakeCString( score.tostring( ), TEXT_ALIGN_RIGHT )
		else
			scoreText.BakeCString( score.tostring( ) + " / " + threshold.tostring( ), TEXT_ALIGN_RIGHT )
		
		// Compact name
		nameText.Compact( 320 - scoreText.Width - 10 )
	}
}

class MedalPanel extends AnimatingCanvas
{
	// Display
	star = null
	progress = null
	bar = null
	desc = null
	prevThresh = null
	nextThresh = null
	friends = null
	friendData = null 
	prevInit = null
	stats = null
	asyncStatus = null
	noFriendsData = null

	// Data
	activeColor = null
	inactiveColor = null
	levelInfo = null
	
	// Statics
	static starSize = 72
	static padding = 7
	static rankCount = 4
	static cellWidth = 366
	
	constructor( playerStats, startingInfo, startingScore, type = RANKPANEL_TYPE_NORMAL )
	{
		::AnimatingCanvas.constructor( )
		
		//clear everything out in constructor
		star = null
		progress = null
		bar = null
		desc = null
		prevThresh = null
		nextThresh = null
		friends = null
		friendData = null 
		prevInit = null
		stats = null
		asyncStatus = null
		noFriendsData = null
		activeColor = null
		inactiveColor = null
		levelInfo = null
	
		// start real construction
		activeColor = ::Math.Vec4.Construct( 1, 1, 1, 1 )
		inactiveColor = ::Math.Vec4.Construct( 0, 0, 0, 0.5 )
		prevInit = false
		stats = playerStats
		
		local centerX = cellWidth / 2
		if( type == RANKPANEL_TYPE_WIDE )
			centerX = cellWidth
		
		local label = ::Gui.Text( )
		label.SetFontById( FONT_FANCY_MED )
		label.SetPosition( centerX, 1, 0 )
		if( ::GameApp.IsAsianLanguage( ) )
			label.SetYPos( 2 )
		else
			label.SetUniformScale( 0.7 )
		label.SetRgba( COLOR_CLEAN_WHITE )
		label.BakeLocString( GameApp.LocString( "Rank" ), TEXT_ALIGN_CENTER )
		AddChild( label )
		
		local startY = 37
		local startX = 7
		local spacing = starSize + padding
		local bottomY = 207
		
		if( type == RANKPANEL_TYPE_WIDE )
		{
			startY += 6
			startX -= 24
		}
		
		// Star
		star = ::Gui.TexturedQuad( )
		star.SetTexture( "gui/textures/frontend/rank_star_01_g.png" )
		star.SetPosition( startX, startY, 0 )
		star.SetRgba( inactiveColor )
		AddChild( star )
		
		centerX = starSize + padding * 2 + ( ( cellWidth - starSize - padding * 2 ) / 2 ) - 4
		bottomY -= spacing
		spacing = 0
		
		if( type == RANKPANEL_TYPE_WIDE )
			centerX -= 12
		
		// Progress Text
		progress = ::Gui.Text( )
		progress.SetFontById( FONT_SIMPLE_SMALL )
		progress.SetRgba( COLOR_CLEAN_WHITE )
		progress.SetPosition( centerX, startY + spacing, 0 )
		AddChild( progress )
		
		// Progress Bar
		bar = ::ProgressBar( "gui/textures/cursor/barbackground_g.png", "gui/textures/cursor/bar_g.png" )
		bar.SetMode( PROGRESSBAR_MODE_TEXTURE )
		bar.SetPosition( centerX - bar.Size( ).x * 0.5, progress.GetYPos( ) + progress.LineHeight + 4, 0.001 )
		AddChild( bar )
		
		// Threshold text
		prevThresh = ::Gui.Text( )
		prevThresh.SetFontById( FONT_SIMPLE_SMALL )
		prevThresh.SetRgba( COLOR_CLEAN_WHITE )
		prevThresh.SetPosition( centerX - bar.Size( ).x * 0.5 - padding, bar.GetYPos( ) - 1, 0 )
		prevThresh.SetUniformScale( 0.8 )
		AddChild( prevThresh )
		
		nextThresh = ::Gui.Text( )
		nextThresh.SetFontById( FONT_SIMPLE_SMALL )
		nextThresh.SetRgba( COLOR_CLEAN_WHITE )
		nextThresh.SetPosition( centerX + bar.Size( ).x * 0.5 + padding, bar.GetYPos( ) - 1, 0 )
		nextThresh.SetUniformScale( 0.8 )
		AddChild( nextThresh )
		
		// Description
		desc = ::Gui.Text( )
		desc.SetFontById( FONT_SIMPLE_SMALL )
		desc.SetRgba( COLOR_CLEAN_WHITE )
		desc.SetPosition( centerX, bottomY - padding - desc.LineHeight - 4, 0 )
		AddChild( desc )
		
		local shiftX = 0
		local lineTexture = "gui/textures/score/score_decoration_g.png"
		if( type == RANKPANEL_TYPE_WIDE )
		{
			shiftX = cellWidth + 24
			bottomY = startY
			lineTexture = "gui/textures/misc/challenge_decoration_g.png"
		}
		
		// Line
		local line = ::Gui.TexturedQuad( )
		line.SetTexture( lineTexture )
		if( type == RANKPANEL_TYPE_WIDE )
			line.SetPosition( cellWidth - 2, bottomY - 10, 0 )
		else
			line.SetPosition( ( cellWidth - 256 ) / 2, bottomY - 5, 0 )
		AddChild( line )
		
		friends = [ ]
		for( local i = 0; i < 3; ++i )
		{
			local friend = ::MedalPanelFriendScore( )
			friend.SetPosition( shiftX + startX + 20, bottomY - 1 + 23 * i, 0 )
			AddChild( friend )
			friends.push( friend )
		}
		
		asyncStatus = ::AsyncStatusSmall( )
		asyncStatus.SetPosition( shiftX + cellWidth * 0.5, bottomY + 30, 0 )
		AddChild( asyncStatus )
		
		noFriendsData = ::Gui.Text( )
		noFriendsData.SetFontById( FONT_SIMPLE_SMALL )
		noFriendsData.SetRgba( COLOR_CLEAN_WHITE )
		noFriendsData.SetAlpha( 0 )
		noFriendsData.BakeLocString( ::GameApp.LocString( "Rank_NoData" ), TEXT_ALIGN_CENTER )
		noFriendsData.SetPosition( shiftX + cellWidth * 0.5, bottomY - 1, 0 )
		AddChild( noFriendsData )

		SetLevelInfo( startingInfo, startingScore )
	}
	
	function CurrentRankProgress( info, score )
	{
		local out = { rank = null, progress = null, previousThreshold = null, nextThreshold = null, percent = null }
		
		local progress = score.RankProgress
		local thresholds = [ info.RankThreshold( 0 ), info.RankThreshold( 1 ), info.RankThreshold( 2 ), info.RankThreshold( 3 ) ]
		local previousThreshold = 0
		foreach( i, v in thresholds )
		{
			if( progress < v )
			{
				out.rank = i - 1
				out.nextThreshold = v
				out.percent = ( progress - previousThreshold ).tofloat( ) / ( v - previousThreshold ).tofloat( )
				break
			}
			
			previousThreshold = v
		}
		
		if( progress >= thresholds[ rankCount - 1 ] )
		{
			out.rank = rankCount - 1
			out.nextThreshold = null
			out.percent = 1.0
		}
		
		out.progress = progress
		out.previousThreshold = previousThreshold
		
		return out
	}
	
	function OnTick( dt )
	{
		::AnimatingCanvas.OnTick( dt )
		
		if( friendData )
		{
			if( !prevInit && friendData.Initialized )
			{
				prevInit = true
				SetLeaderboardInfo( levelInfo )
			}
		}
	}
	
	function SetLevelInfo( info, score )
	{
		levelInfo = info
		local data = CurrentRankProgress( info, score )
		
		// Set Star
		if( data.rank == -1 )
			star.SetRgba( inactiveColor )
		else
			star.SetRgba( activeColor )
		star.SetTexture( ::RankMedalImagePath( data.rank ) )
		
		// Set Progress Text
		local progressLabelString = ::GameApp.LocString( "Challenge_ProgressLevel" ) % " " % ( data.rank + 2 ).tostring( )
		if( data.rank == 3 )
			progressLabelString = ::GameApp.LocString( "Challenge_ProgressOvertime" )
			
		if( data.nextThreshold )
			progressLabelString = progressLabelString % ( " : " + data.progress.tostring( ) + " / " + data.nextThreshold.tostring( ) )
		else
			progressLabelString = progressLabelString % ( " : " + data.progress.tostring( ) )
		
		progress.BakeLocString( progressLabelString, TEXT_ALIGN_CENTER )
			
		// Set Progress Bar
		if( data.percent >= 1.0 )
		{
			bar.meter.SetTexture( "gui/textures/cursor/powerbar_g.png" )
			bar.SetMeterHorizontal( 1.0 )
		}
		else
		{
			bar.meter.SetTexture( "gui/textures/cursor/bar_g.png" )
			bar.SetMeterHorizontal( data.percent )
		}
		
		// Set threshold text
		prevThresh.BakeCString( data.previousThreshold.tostring( ), TEXT_ALIGN_RIGHT )
		if( data.nextThreshold )
			nextThresh.BakeCString( data.nextThreshold.tostring( ), TEXT_ALIGN_LEFT )
		else
			nextThresh.BakeCString( " " )
		
		// Description
		local rankDesc = info.RankDesc
		if( rankDesc && typeof rankDesc == "string" &&rankDesc.len( ) > 0 )
			desc.BakeLocString( ::GameApp.LocString( rankDesc ), TEXT_ALIGN_CENTER )
		
		desc.CompactAndUnscale( bar.Size( ).x + 100 )
			
		// Friends Data
		friendData = stats.RequestLevelData( info.MapType, info.LevelIndex )
		if( is_null( friendData ) )
			friendData = null
		if( friendData )
			prevInit = friendData.Initialized
		
		SetLeaderboardInfo( info )
	}
	
	function ClearLeaderboardInfo( )
	{
		foreach( friend in friends )
		{
			friend.SetAlpha( 0 )
		}
		
		asyncStatus.SetAlpha( 1 )
		noFriendsData.SetAlpha( 0 )
	}
	
	function SetLeaderboardInfo( info )
	{
		ClearLeaderboardInfo( )
		
		if( friendData && friendData.Initialized )
		{
			// Friends
			local friendsInfo = [ ]
			local count = friendData.ChallengeProgressesCount( )
			local validFriends = 0
				
			if( count > 100 )
			{
				friendData = null
				::print( "WARNING: Something bad happened in friendData.ChallengeProgressesCount. Count is " + count.tostring( ) )
				count = 0
			}
			
			if( count )
			{
				for( local i = 0; i < count; ++i )
				{
					local info = friendData.GetChallengeProgresses( i )
					if( !is_null( info ) )
						friendsInfo.push( info )
					else
						::print( "data is null, i:" + i.tostring( ) )
				}
				
				friendsInfo.sort( function( a, b )
				{
					if( a.Score > b.Score )
						return -1
					else if( a.Score < b.Score )
						return 1
					else
						return 0
				} )
				
				foreach( i, data in friendsInfo )
				{
					local progress = data.Score
					local rank = ::FindRank( progress, info )
					local threshold = ::FindNextThreshold( progress, info )
					
					if( progress > 0 )
					{
						friends[ validFriends ].SetAlpha( 1 )
						friends[ validFriends ].Set( rank, data.GamerTag, progress, threshold )
						validFriends++
					}
					
					if( validFriends >= 3 )
						break
				}
			}
			
			if( count == 0 || validFriends == 0 )
				noFriendsData.SetAlpha( 1 )
			
			asyncStatus.SetAlpha( 0 )
		}
	}
}