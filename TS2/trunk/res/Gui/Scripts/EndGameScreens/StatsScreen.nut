// Screen to show the stats to the players

// Requires
sigimport "gui/scripts/endgamescreens/baseendgamescreen.nut"
sigimport "gui/scripts/endgamescreens/statsdisplay.nut"

// Resources
sigimport "gui/textures/score/score_decoration_g.png"
sigimport "gui/textures/gamepad/button_lshoulder_g.png"
sigimport "gui/textures/gamepad/button_rshoulder_g.png"

class StatsScreen extends AnimatingCanvas
{
	// Display
	player1Name = null
	player2Name = null
	
	// Data
	stats = null // StatsDisplay
	hasSubPages = null
	
	constructor( rect, player1, player2 = null )
	{
		::AnimatingCanvas.constructor( )
		audioSource = player1.AudioSource
		
		// Stats Text
		local text = ::Gui.Text( )
		text.SetFontById( FONT_FANCY_LARGE )
		text.SetRgba( COLOR_CLEAN_WHITE )
		text.BakeLocString( ::GameApp.LocString( "EndGame_Stats" ), TEXT_ALIGN_CENTER )
		text.SetPosition( ::Math.Vec3.Construct( rect.Center.x, rect.Top + 50, 0 ) )
		AddChild( text )
		
		// Decoration
		local decoration = ::Gui.TexturedQuad( )
		decoration.SetTexture( "gui/textures/score/score_decoration_g.png" )
		decoration.CenterPivot( )
		decoration.SetPosition( text.GetXPos( ), text.GetYPos( ) + text.Height + 5, 0 )
		AddChild( decoration )
				
		// Create Stats
		stats = ::StatsDisplay( )
		stats.audioSource = audioSource
		stats.SetPosition( rect.Center.x, text.GetYPos( ) + text.Height + 15, 0 )
		AddChild( stats )
		
		local width = stats.Width
		
		local leftPlayer = player1
		local rightPlayer = player2
		
		if( ::GameApp.GameMode.IsNet && player2 && !::GameAppSession.IsHost )
		{
			leftPlayer = player2
			rightPlayer = player1
		}
		
		if( player2 )
		{
			player1Name = ::Gui.Text( )
			player1Name.SetFontById( FONT_FANCY_MED )
			player1Name.SetRgba( COLOR_CLEAN_WHITE )
			player1Name.BakeLocString( leftPlayer.User.GamerTag, TEXT_ALIGN_LEFT )
			player1Name.SetPosition( rect.Center.x - width * 0.5, stats.GetYPos( ), 0 )
			AddChild( player1Name )
			
			player2Name = ::Gui.Text( )
			player2Name.SetFontById( FONT_FANCY_MED )
			player2Name.SetRgba( COLOR_CLEAN_WHITE )
			player2Name.BakeLocString( rightPlayer.User.GamerTag, TEXT_ALIGN_RIGHT )
			player2Name.SetPosition( rect.Center.x + width * 0.5, stats.GetYPos( ), 0 )
			AddChild( player2Name )
			
			local availableTextSpace = width * 0.5 - 10
			player1Name.Compact( availableTextSpace )
			player2Name.Compact( availableTextSpace )
			
			local newYPos = stats.GetYPos( ) + 42
			
			local line = ::Gui.TexturedQuad( )
			line.SetTexture( "gui/textures/score/score_decoration_g.png" )
			line.SetRect( ::Math.Vec2.Construct( width, decoration.TextureDimensions( ).y ) )
			line.SetPosition( rect.Center.x - width * 0.5, newYPos, 0 )
			AddChild( line )
			
			stats.SetYPos( newYPos + 4 )
		}
		
		local stats1 = leftPlayer.Stats
		local stats2 = (rightPlayer)? rightPlayer.Stats: null
		stats.AddStandardStats( stats1, stats2 )
		stats.ShowPage( 0 )
		
		hasSubPages = stats.hasSubPages
	}
	
	function HandleInput( gamepad )
	{
		return stats.HandleInput( gamepad )
	}
}
