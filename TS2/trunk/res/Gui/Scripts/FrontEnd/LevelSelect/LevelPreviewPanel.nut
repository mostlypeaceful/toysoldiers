// Preview for the level

// Resources
sigimport "gui/textures/frontend/locked_level_g.png"
sigimport "gui/textures/mappreview/unknown_map_d.png"

class LevelPreviewPanel extends Gui.CanvasFrame
{
	// Display
	fadeCanvas = null // AnimatingCanvas
	image = null // Gui.TexturedQuad
	lock = null // Gui.TexturedQuad
	label = null // Gui.Text
	
	// Data
	currentInfo = null
	
	// Callbacks
	isLocked = null
	
	constructor( startingInfo, startingScores = null, lockedFunc = null )
	{
		::Gui.CanvasFrame.constructor( )
		isLocked = lockedFunc
		
		local w = 390
		local h = 220
		local padding = 10
		
		fadeCanvas = ::AnimatingCanvas( )
		AddChild( fadeCanvas )
		
		image = ::Gui.AsyncTexturedQuad( )
		//image.SetTexture( "gui/textures/mappreview/unknown_map_d.png" )
		fadeCanvas.AddChild( image )
		
		lock = ::Gui.TexturedQuad( )
		lock.SetTexture( "gui/textures/frontend/locked_level_g.png" )
		lock.SetPosition( w - lock.TextureDimensions( ).x - padding, h - lock.TextureDimensions( ).y - padding, -0.01 )
		fadeCanvas.AddChild( lock )
		
		/*label = ::Gui.Text( )
		label.SetFontById( FONT_FANCY_MED )
		label.SetUniformScale( 0.8 )
		label.SetRgba( COLOR_CLEAN_WHITE )
		label.BakeLocString( GameApp.LocString( "Map_Preview" ), TEXT_ALIGN_CENTER )
		label.SetPosition( Math.Vec3.Construct( w * 0.5, 0, -0.01 ) )
		AddChild( label )*/
		
		RawSetLevelInfo( startingInfo, startingScores )
	}
	
	function SetLevelInfo( info, scores )
	{
		if( currentInfo == null || currentInfo.MapType != info.MapType || currentInfo.LevelIndex != info.LevelIndex || currentInfo.DlcNumber != info.DlcNumber )
		{
			currentInfo = info
			
			fadeCanvas.ClearActions( )
			fadeCanvas.AddAction( ::AlphaTween( fadeCanvas.GetAlpha( ), 0.0, 0.1, null, null, null, function( canvas ):(info, scores)
			{
				RawSetLevelInfo( info, scores )
			}.bindenv(this) ) )
		}
	}
	
	function RawSetLevelInfo( info, scores = null )
	{
		local imagePath = info.GetPreviewImagePath( )
		local locked = isLocked( scores, info )
		
		lock.SetAlpha( (locked)? 1.0: 0.0 )
		
		image.OnLoaded = function( quad )
		{
			if( fadeCanvas )
				fadeCanvas.AddAction( ::AlphaTween( 0.0, 1.0, 0.1 ) )
		}.bindenv( this )
		
		if( imagePath.len( ) <= 0 )
			image.SetTexture( "gui/textures/mappreview/unknown_map_d.png" )
		else
			image.SetTexture( imagePath )
	}
	
	function Unload( )
	{
		image.Unload( )
	}
}
