// Earned Screen

// Requires
sigimport "gui/scripts/endgamescreens/baseendgamescreen.nut"
sigimport "gui/scripts/utility/achievementscripts.nut"
sigimport "gui/scripts/utility/goldenarcadescripts.nut"

// Resources
sigimport "gui/textures/score/score_decoration_g.png"
sigimport "gui/textures/frontend/rank_star_01_g.png"
sigimport "gui/textures/frontend/rank_star_02_g.png"
sigimport "gui/textures/frontend/rank_star_03_g.png"
sigimport "gui/textures/frontend/rank_star_04_g.png"

class EarnedItemDisplay extends AnimatingCanvas
{
	function constructor( type, value, mini = false, fontOverride = null )
	{
		::AnimatingCanvas.constructor( )
		
		local font = FONT_FANCY_MED
		if( fontOverride != null )
			font = fontOverride
		
		// Type
		local typeLocIds = [ "Earned_Decoration", "Earned_Rank", "Earned_Achievement", "Earned_AvatarAward", "Earned_GoldenArcade", "Earned_Jetpack" ]
		local typeText = ::Gui.Text( )
		typeText.SetFontById( FONT_SIMPLE_SMALL )
		typeText.SetRgba( 0.6, 0.6, 0.6, 1.0 )
		typeText.BakeLocString( ::GameApp.LocString( typeLocIds[ type ] ) )
		typeText.SetUniformScale( 0.8 )
		typeText.SetPosition( 0, 0, 0 )
		AddChild( typeText )
		
		// Get data
		local data = { name = null, desc = null, image = null, imageScale = 1.0, imageWidth = 96 }
		local levelIndex = ::GameApp.CurrentLevel.LevelNumber
		local levelInfo = ::GameApp.CurrentLevelLoadInfo
		
		switch( type )
		{
			case EARNED_ITEM_TYPE_DECORATION:
				if( value == 0 || value == 1 )
				{
					data.name = ::RationTicketNameLocString( levelIndex, value )
					data.desc = ::RationTicketDescLocString( levelIndex, value )
					data.image = ::RationTicketImagePath( levelIndex, value )
					data.imageScale = ( ( mini )? 0.4 : 0.6 )
					data.imageWidth = ( ( mini )? 48 : 72 )
				}
				else
				{
					::LogWarning( 0, "Earned a decoration with invalid index: " + value.tostring( ), true )
					::BreakPoint( )
				}
			break
			
			case EARNED_ITEM_TYPE_RANK:
				if( value >= 0 && value < 4 )
				{
					local rankValue = ( value + 1 )
					data.name = ::GameApp.LocString( "Earned_RankName" ).Replace( "i", rankValue )
					data.desc = ::GameApp.LocString( "Earned_RankDescFormat" ).Replace( "desc", levelInfo.RankDesc ).Replace( "threshold", levelInfo.RankThreshold( value ) )
					data.image = "gui/textures/frontend/rank_star_0" + rankValue.tostring( ) + "_g.png"
					data.imageWidth = 72
				}
				else
				{
					::LogWarning( 0, "Earned a rank with invalid index: " + value.tostring( ), true )
					::BreakPoint( )
				}
			break
			
			case EARNED_ITEM_TYPE_ACHIEVEMENT:
				if( value >= 0 && value < ACHIEVEMENTS_COUNT )
				{
					data.name = ::GameApp.LocString( ::AchievementName( value ) )
					data.desc = ::GameApp.LocString( ::AchievementDesc( value ) )
					data.image = ::AchievementImagePath( value )
					data.imageWidth = 64
				}
				else
				{
					::LogWarning( 0, "Earned an achievement with invalid index: " + value.tostring( ), true )
					::BreakPoint( )
				}
			break
			
			case EARNED_ITEM_TYPE_AVATAR_AWARD:
				if( value >= 0 && value < AVATAR_AWARDS_DIVIDER )
				{
					data.name = ::GameApp.LocString( ::AvatarAwardName( value ) )
					data.desc = ::GameApp.LocString( ::AvatarAwardDesc( value ) )
					data.image = ::AvatarAwardImagePath( value )
					data.imageWidth = 64
				}
				else if( value >= AVATAR_AWARDS_COUNT )
				{
					::LogWarning( 0, "Earned an avatar award with invalid index: " + value.tostring( ), true )
					::BreakPoint( )
				}
				else
				{
					::LogWarning( 0, "Earned an gamer pic (" + value.tostring( ) + "). Shouldn't have added it to the earned items list.", true )
				}
			break
			
			case EARNED_ITEM_TYPE_GOLDEN_ARCADE:
				if( value >= 0 && value <= 1 )
				{
					typeText.BakeLocString( ::GameApp.LocString( typeLocIds[ EARNED_ITEM_TYPE_DECORATION ] ) )
					data.name = ::GoldenRationTicketNameLocString( value )
					data.desc = ::GoldenRationTicketDescLocString( value )
					data.image = ::GoldenRationTicketImagePath( value )
					data.imageScale = ( ( mini )? 0.4 : 0.6 )
					data.imageWidth = ( ( mini )? 48 : 72 )
				}
				else if( value < 0 )
				{
					data.imageWidth = 88
					
					switch( value )
					{
						case -1:
							typeText.BakeLocString( ::GameApp.LocString( "Earned_GoldenArcade" ) )
							
							data.name = ::GameApp.LocString( "Golden_Arcade_Destroyed" )
							data.desc = ::GameApp.LocString( "Earned_GoldenArcadeDesc" )
							data.image = ::GoldenArcadeIconTexture( )
						break
					
						case -2:
							typeText.BakeLocString( ::GameApp.LocString( "Earned_GoldenBabushka" ) )
							
							data.name = ::GameApp.LocString( "Golden_Babushka_All_Found" )
							data.desc = ::GameApp.LocString( "Earned_GoldenBabushkaDesc" )
							data.image = ::GoldenBabushkaIconTexture( )
						break
						
						case -3:
							typeText.BakeLocString( ::GameApp.LocString( "Earned_GoldenDogTag" ) )
							
							data.name = ::GameApp.LocString( "Golden_DogTag_All_Found" )
							data.desc = ::GameApp.LocString( "Earned_GoldenDogTagDesc" )
							data.image = ::GoldenDogTagIconTexture( )
						break
					}
				}
			break
			
			case EARNED_ITEM_TYPE_JET_PACK:
				data.name = ::GameApp.LocString( "JetpackEarnedName" )
				data.desc = ::GameApp.LocString( "JetpackEarnedDesc" )
				data.image = "gui/textures/misc/unlock_jetpack_g.png"
				data.imageWidth = 64
			break
		}
		
		if( data.image )
		{
			// Image
			local image = ::Gui.AsyncTexturedQuad( )
			image.SetTexture( data.image )
			image.SetUniformScale( data.imageScale )
			image.SetPosition( -data.imageWidth * 0.5 - 44 - 5, 0, 0 )
			if( mini )
			{
				image.SetUniformScale( data.imageScale * 0.75 )
				image.SetPosition( -data.imageWidth * 0.5 * 0.75 - 22 - 5, 4, 0 )
			}
			AddChild( image )
		}
		
		if( data.name )
		{
			// Name
			local nameText = ::Gui.Text( )
			nameText.SetFontById( font )
			nameText.SetRgba( COLOR_CLEAN_WHITE )
			nameText.BakeLocString( data.name, TEXT_ALIGN_LEFT )
			nameText.SetPosition( 0, typeText.LocalRect.Height - 5, 0 )
			AddChild( nameText )
			
			local w = ( ( mini )? 300: 400 )
			nameText.Compact( w )
			
			if( data.desc && !mini )
			{
				// Line
				local line = ::Gui.TexturedQuad( )
				line.SetTexture( "gui/textures/score/score_decoration_g.png" )
				line.SetPosition( -5, nameText.GetYPos( ) + nameText.LineHeight, 0 )
				line.SetRect( ::Math.Vec2.Construct( w + 10, line.TextureDimensions( ).y ) )
				AddChild( line )

				if( !mini )
				{
					// Description
					local descText = ::Gui.Text( )
					descText.SetFontById( FONT_SIMPLE_SMALL )
					descText.SetRgba( COLOR_CLEAN_WHITE )
					descText.BakeBoxLocString( w, data.desc, TEXT_ALIGN_LEFT )
					descText.SetPosition( 0, line.GetYPos( ) + 5, 0 )
					AddChild( descText )
				}
			}
		}
	}
}

class EarnedItemsDisplayCanvas extends AnimatingCanvas
{
	// Display
	header = null
	icons = null
	columns = null

	constructor( player, forceMini = false, fontOverride = null, headerLocStr = null )
	{
		::AnimatingCanvas.constructor( )
		icons = [ ]
		columns = 0
		
		// Header maybe
		if( headerLocStr )
		{
			header = ::Gui.Text( )
			header.SetFontById( FONT_FANCY_MED )
			header.SetRgba( COLOR_CLEAN_WHITE )
			header.BakeLocString( headerLocStr, TEXT_ALIGN_CENTER )
			header.SetPosition( 80, -header.Height - 4, 0 )
			AddChild( header )
			
			// Decoration
			local decoration = ::Gui.TexturedQuad( )
			decoration.SetTexture( "gui/textures/score/score_decoration_g.png" )
			decoration.CenterPivot( )
			decoration.SetPosition( header.GetXPos( ), -4, 0 )
			AddChild( decoration )
		}
		
		// Sort data
		local earnedItems = player.EarnedItems
		local sortedData = [ ]
		foreach( key, item in earnedItems )
			sortedData.push( item )
		sortedData.sort( function( a, b )
		{
			if( a.Type > b.Type ) 
				return 1
			else if( a.Type < b.Type ) 
				return -1
			else
			{
				if( a.Value > b.Value ) 
					return 1
				else if( a.Value < b.Value ) 
					return -1
				return 0
			}
		} )
		
		//sortedData.push( { Type = EARNED_ITEM_TYPE_DECORATION, Value = 0, Deferred = 0 } )
		//sortedData.push( { Type = EARNED_ITEM_TYPE_GOLDEN_ARCADE, Value = 0, Deferred = 0 } )
		//sortedData.push( { Type = EARNED_ITEM_TYPE_JET_PACK, Value = 0, Deferred = 0 } )
		
		//sortedData.push( { Type = EARNED_ITEM_TYPE_ACHIEVEMENT, Value = ACHIEVEMENTS_A_JOB_WELL_DONE, Deferred = 0 } )
		/*for( local i = 0; i < 3; ++i )
		{
			sortedData.push( { Type = EARNED_ITEM_TYPE_ACHIEVEMENT, Value = i, Deferred = 0 } )
		}*/
		
		local mini = ( sortedData.len( ) > 8 ) || forceMini
		
		// Earned Icons
		local itemsPerColumn = 4
		if( sortedData.len( ) > 4 )
			itemsPerColumn = ::Math.RoundUp( sortedData.len( ) / 2.0 ).tointeger( )
		if( sortedData.len( ) > 14 )
			itemsPerColumn = ::Math.RoundUp( sortedData.len( ) / 3.0 ).tointeger( )
		if( forceMini )
			itemsPerColumn = 20
		local itemWidth = 500
		local startX = -150
		local startY = 0
		local spacing = 96 + 20
		
		if( mini )
		{
			spacing = 65
			itemWidth = 360
			startX = 0
		}
		
		if( sortedData.len( ) > itemsPerColumn )
			startX = -itemWidth + ( ( mini )? 40: 96 )
		if( sortedData.len( ) > itemsPerColumn * 2 )
			startX *= 1.5
		
		foreach( i, item in sortedData )
		{
			local type = item.Type
			local value = item.Value

			local display = ::EarnedItemDisplay( type, value, mini, fontOverride )
			local xPos = startX + itemWidth * ( i / itemsPerColumn )
			local yPos = startY + spacing * ( icons.len( ) - itemsPerColumn * ( i / itemsPerColumn ) )

			display.SetPosition( xPos, yPos, 0 )
			AddChild( display )
			
			icons.push( display )
			
			if( type == EARNED_ITEM_TYPE_ACHIEVEMENT && item.Deferred )
				player.AwardAchievement( value )
		}
		
		if( sortedData.len( ) > 0 )
			columns = ( ( sortedData.len( ).tointeger( ) - 1 ) / itemsPerColumn ) + 1
		else
			columns = 0
	}
	
	function HasEarnings( )
	{
		return ( icons.len( ) > 0 )
	}
}

class EarnedScreen extends AnimatingCanvas
{
	// Display
	display = null
	display2 = null
	
	constructor( rect, player1, player2 = null )
	{
		::AnimatingCanvas.constructor( )
		
		// Earned Text
		local text = ::Gui.Text( )
		text.SetFontById( FONT_FANCY_LARGE )
		text.SetRgba( COLOR_CLEAN_WHITE )
		text.BakeLocString( ::GameApp.LocString( "EndGame_Earned" ), TEXT_ALIGN_CENTER )
		text.SetPosition( rect.Center.x, rect.Top + 50, 0 )
		AddChild( text )
		
		// Decoration
		local decoration = ::Gui.TexturedQuad( )
		decoration.SetTexture( "gui/textures/score/score_decoration_g.png" )
		decoration.CenterPivot( )
		decoration.SetPosition( text.GetXPos( ), text.GetYPos( ) + text.Height + 5, 0 )
		AddChild( decoration )
		
		if( ::GameApp.CurrentLevelLoadInfo.MapType == MAP_TYPE_CAMPAIGN )
			player2 = null
		
		// Display
		if( player2 == null )
			display = ::EarnedItemsDisplayCanvas( player1 )
		else
			display = ::EarnedItemsDisplayCanvas( player1, true, FONT_SIMPLE_MED, player1.User.GamerTag )
		display.SetPosition( rect.Center.x, decoration.GetYPos( ) + 20, 0 )
		AddChild( display )
		
		if( player2 != null )
		{
			display.SetPosition( rect.Center.x - 260, decoration.GetYPos( ) + 40, 0 )
			
			display2 = ::EarnedItemsDisplayCanvas( player2, true, FONT_SIMPLE_MED, player2.User.GamerTag )
			display2.SetPosition( rect.Center.x + 60, decoration.GetYPos( ) + 40, 0 )
			AddChild( display2 )
		}
	}
	
	function HasEarnings( )
	{
		if( display2 )
			return display.HasEarnings( ) && display2.HasEarnings( )
		else
			return display.HasEarnings( )
	}
}