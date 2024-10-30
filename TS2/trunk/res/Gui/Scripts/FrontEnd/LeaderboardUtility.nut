// Utility Leaderboard Display classes

// Requires
sigimport "gui/scripts/utility/medalscripts.nut"
sigimport "levels/scripts/common/challengescripts.nut"
sigimport "gui/scripts/frontend/levelselect/medalpanel.nut"

// Globals
LeaderboardHighlightColor 		<- ::Math.Vec4.Construct( 1.0, 1.0, 0.0, 1.0 )
LeaderboardHighlightBarColor 	<- ::Math.Vec4.Construct( 1.0, 1.0, 0.0, 0.2 )
LeaderboardDarkBarColor 		<- ::Math.Vec4.Construct( 0.0, 0.0, 0.0, 0.2 )
LeaderboardLightBarColor 		<- ::Math.Vec4.Construct( 0.1, 0.1, 0.1, 0.2 )
LeaderboardPlayerTextColor 		<- ::Math.Vec4.Construct( 0.0, 1.0, 0.0, 1.0 )

class LeaderboardItem extends AnimatingCanvas
{
	// Data
	text = null
	colWidth = null
	colAlign = null
	
	constructor( width, align )
	{
		::AnimatingCanvas.constructor( )
		colWidth = width
		colAlign = align
	}
}

class LeaderboardData extends LeaderboardItem
{
	highlightColor = null
	normalColor = null
	
	constructor( board, column, row, levelInfo )
	{
		::LeaderboardItem.constructor( board.ColumnWidth( column ), board.ColumnAlignment( column ) )
		IgnoreBoundsChange = 1
		highlightColor = ::LeaderboardHighlightColor
		normalColor = COLOR_CLEAN_WHITE
		
		text = ::Gui.Text( )
		text.SetFontById( FONT_SIMPLE_SMALL )
		text.SetRgba( COLOR_CLEAN_WHITE )
		
		local type = board.ColumnUserData( column )
		switch( type )
		{
			case LEADERBOARD_COLUMN_TYPE_MEDAL:
				local medal = ::Gui.TexturedQuad( )
				local medalTexture = ::VictoryScreenMedalImagePath( board.Data( column, row ).ToInt( ), MEDAL_SIZE_SMALL, true )
				if( medalTexture )
				{
					medal.SetTexture( medalTexture )
					medal.SetPivot( ::Math.Vec2.Construct( medal.LocalRect.Width * 0.5, 0 ) )
					AddChild( medal )
				}
			break
			
			case LEADERBOARD_COLUMN_TYPE_CHALLENGEPROGRESS:
				if( levelInfo )
				{
					local progress = board.Data( column, row ).ToInt( )
					local threshold = ::FindNextThreshold( progress, levelInfo )
					if( threshold != null )
						text.BakeCString( progress.tostring( ) + " / " + threshold.tostring( ), colAlign )
					else
						text.BakeCString( progress.tostring( ), colAlign )
					
					local medalImage = ::Gui.TexturedQuad( ) 
					local rank = ::FindRank( progress, levelInfo )
					medalImage.SetTexture( ::RankMedalImagePath( rank, true ) )
					if( rank == -1 )
						medalImage.SetRgba( 0, 0, 0, 0.5 )
					
					local padding = 4
					if( colAlign == TEXT_ALIGN_RIGHT )
					{
						text.SetXPos( -24 )
						medalImage.SetPosition( text.GetXPos( ) + padding, 0, 0 )
					}
					else if( colAlign == TEXT_ALIGN_CENTER )
					{
						medalImage.SetPosition( text.GetXPos( ) + text.Width * 0.5 + padding, 0, 0 )
					}
					else
					{
						medalImage.SetPosition( text.GetXPos( ) + text.Width + padding, 0, 0 )
					}
					
					AddChild( medalImage )
				}
			break
			
			case LEADERBOARD_COLUMN_TYPE_MONEY:
				text.BakeCString( "$" + board.Data( column, row ).ToString( ), colAlign )
			break
			
			case LEADERBOARD_COLUMN_TYPE_TIME:
				text.BakeLocString( board.Data( column, row ).ToTimeString( ), colAlign )
			break
			
			case LEADERBOARD_COLUMN_TYPE_MINIGAME_META:
				text.BakeLocString( ::LocString.LocalizeNumber( ::GameSessionStats.LeaderBoardToStat( SESSION_STATS_MINIGAME_META_STAT, board.Data( column, row ).ToInt( ) ) ), colAlign )
			break
			
			case LEADERBOARD_COLUMN_TYPE_NUMBER:
				// FALLS THROUGH ON PURPOSE
			default:
				text.BakeCString( board.Data( column, row ).ToString( ), colAlign )
			break
		}

		AddChild( text )
	}
	
	function Highlight( highlightOn )
	{
		if( text )
		{
			if( highlightOn )
				text.SetRgba( highlightColor )
			else
				text.SetRgba( normalColor )
		}
	}
	
	function SetAsPlayer( )
	{
		highlightColor = ::LeaderboardPlayerTextColor
		normalColor = ::LeaderboardPlayerTextColor
		Highlight( false )
	}
}

class LeaderboardHeader extends LeaderboardItem
{
	constructor( loc, width, align )
	{
		::LeaderboardItem.constructor( width, align )
		
		text = ::Gui.Text( )
		text.SetFontById( FONT_FANCY_MED )
		text.SetRgba( COLOR_CLEAN_WHITE )
		text.BakeLocString( loc, align )
		AddChild( text )
		
		IgnoreBoundsChange = 1
	}
}

class LeaderboardRow extends AnimatingCanvas
{
	// Display
	items = null
	async = null
	bar = null
	barColor = null
	
	// Data
	rank = null
	userId = null
	gamerTag = null
	
	// Statics
	static Width = 1112
	static Height = 30
	
	constructor( index = null )
	{
		::AnimatingCanvas.constructor( )
		
		// Loading status
		async = ::AsyncStatusSmall( )
		async.SetPosition( 30, Height * 0.5, 0 )
		AddChild( async )
		
		// Bar
		if( index != null )
		{
			/*local num = ::Gui.Text( )
			num.SetFontById( FONT_SIMPLE_SMALL )
			num.SetRgba( COLOR_CLEAN_WHITE )
			num.BakeCString( index.tostring( ), TEXT_ALIGN_RIGHT )
			num.SetXPos( -2 )
			AddChild( num )*/
		
			bar = ::Gui.ColoredQuad( )
			bar.SetRect( ::Math.Vec2.Construct( Width, Height ) )
			barColor = ( ( index % 2 == 0 )? LeaderboardDarkBarColor: LeaderboardLightBarColor )
			bar.SetRgba( barColor )
			bar.SetPosition( -1, -1, 0.0001 )
			AddChild( bar )
		}
		
		Disabled = 1
	}
	
	function IsNull( )
	{
		return items == null
	}
	
	function Set( items_, rank_, userId_, gamerTag_ )
	{
		items = items_
		rank = rank_
		userId = userId_
		gamerTag = gamerTag_
		
		local nextX = 0
		local nextY = 0
		foreach( item in items )
		{
			// Set position based on alignment
			item.SetPosition( nextX, nextY, 0 )
			if( item.colAlign == TEXT_ALIGN_RIGHT )
				item.SetXPos( nextX + item.colWidth )
			else if( item.colAlign == TEXT_ALIGN_CENTER )
				item.SetXPos( nextX + item.colWidth * 0.5 )
			
			// Scale if text is too wide
			local availableSpace = item.colWidth - 10
			local text = item.text
			local width = text.Width
			if( width != 0 && width > availableSpace )
				item.text.SetScale( availableSpace / width, 1.0 )
			
			AddChild( item )
			
			nextX += item.colWidth
		}
		
		RemoveChild( async )
	}
	
	function UnSet( )
	{
		if( !items )
			return
			
		foreach( item in items )
		{
			RemoveChild( item )
			item.DeleteSelf( )
		}
		items.clear( )
		items = null
		
		AddChild( async )
	}
	
	function SetAsPlayer( )
	{
		foreach( item in items )
			item.SetAsPlayer( )
	}
	
	function Highlight( highlight )
	{
		if( bar )
			bar.SetRgba( highlight? LeaderboardHighlightBarColor: barColor )
		if( items )
		{
			foreach( item in items )
				item.Highlight( highlight )
		}
	}
}