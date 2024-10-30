// Base Pause Menu Content

// Requires
sigimport "gui/scripts/controls/animatingcanvas.nut"

// Resources
sigimport "gui/textures/pausemenu/pausemenu_background_g.png"

class PauseContentBase extends AnimatingCanvas 
{
	// Display
	background = null
	labels = null
	
	// Data
	levelInfo = null
	player1Scores = null
	player2Scores = null
	currentPlayer = null
	origin = null
	contentSize = null
	
	constructor( player )
	{
		::AnimatingCanvas.constructor( )
		labels = { }
		currentPlayer = player
		
		// Add Background Object
		background = ::Gui.TexturedQuad( )
		background.SetPosition( -20, -49, 0.01 )
		AddChild( background )
		SetBackground( "gui/textures/pausemenu/pausemenu_background_g.png" )
		
		// Get Level data
		levelInfo = ::GameApp.CurrentLevelLoadInfo
		local userProfile1 = ::GameApp.FrontEndPlayer.GetUserProfile( )
		player1Scores = userProfile1.GetLevelScores( levelInfo.MapType, levelInfo.LevelIndex )
		if( ::Player.IsValid( ::GameApp.SecondaryPlayer ) )
		{
			local userProfile2 = ::GameApp.SecondaryPlayer.GetUserProfile( )
			player2Scores = userProfile2.GetLevelScores( levelInfo.MapType, levelInfo.LevelIndex )
		}
		
		origin = ::Math.Vec3.Construct( 340, -47, 0 )
		contentSize = ::Math.Vec2.Construct( 796, 407 )
	}
	
	function SetBackground( texture )
	{
		background.SetTexture( texture )
	}
	
	function SetLabel( labelId, locId )
	{
		if( typeof locId == "string" )
			locId = ::GameApp.LocString( locId )
		if( labelId in labels )
			labels[ labelId ].BakeLocString( locId, TEXT_ALIGN_CENTER )
	}
	
	function AddLabel( id, x, y, small = false )
	{
		local label = ::Gui.Text( )
		if( ::GameApp.IsAsianLanguage( ) )
		{
			label.SetFontById( FONT_FANCY_LARGE )
			if( small )
				label.SetUniformScale( 0.9 )
		}
		else
		{
			label.SetFontById( FONT_FANCY_MED )
			if( small )
				label.SetUniformScale( 0.7 )
		}
		label.SetRgba( COLOR_CLEAN_WHITE )
		label.SetPosition( x, y, 0 )
		AddChild( label )
		
		labels[ id ] <- label
		return label
	}
}