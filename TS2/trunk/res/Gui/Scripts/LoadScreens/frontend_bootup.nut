
// Requires
sigimport "gui/scripts/controls/asyncstatus.nut"
sigimport "gui/scripts/loadscreens/autosavenoticescreen.nut"
sigimport "gui/scripts/loadscreens/default_level_load.nut"

sigexport function CanvasCreateLoadScreen( loadScreen )
{
	return FrontEndBootUpLoadScreen( loadScreen )
}

class VideoBootScreen extends AnimatingCanvas
{
	movie = null

	function Start( )
	{
		movie = ::Gui.MovieQuad( )
		movie.SetMovie( "game:\\video\\signalstudios_logo.wmv", 0 )
		movie.SetRect( ::Math.Vec2.Construct( 1280, 720 ) )
		movie.SetPosition( 0, 0, 0 )
		AddChild( movie )
	}
	
	function DeleteSelf( )
	{
		//::print( "movie deleted" )
		movie.DeleteSelf( )
		RemoveChild( movie )
		::AnimatingCanvas.DeleteSelf( )
	}
}

class FrontEndBootUpLoadScreen extends Gui.CanvasFrame
{
	// Display
	images = null
	loading = null
	levelName = null
	
	// data
	loadScreen = null
	timer = null
	imageIndex = null
	finalFadeOut = null
	finished = null

	constructor( _loadScreen )
	{
		::Gui.CanvasFrame.constructor( )
		local vpRect = ::GameApp.ComputeScreenSafeRect( )
		loadScreen = _loadScreen
		timer = 0
		imageIndex = 0
		finalFadeOut = false
		finished = false

		// for the bootup screen, it's safe to do this immediately
		loadScreen.SetCanvasIsFadedIn( )
		loadScreen.SetCanProceedToNewLevel( )

		local imagePaths = [ "gui/textures/loadscreens/bootup/xbla_logo_g.png" ]
		
		if( ::GameApp.Region == REGION_NORTH_AMERICA )
			imagePaths.push( "gui/textures/loadscreens/bootup/esrb_rating_g.png" )

		local bg = ::Gui.ColoredQuad( )
		bg.SetRect( ::Math.Vec2.Construct( 1280, 720 ) )
		bg.SetPosition( 0, 0, 0.10 );
		bg.SetRgba( 0.0, 0.0, 0.0, 1 )
		AddChild( bg )

		images = [ ]
		foreach( path in imagePaths )
		{
			local image = ::Gui.AsyncTexturedQuad( )
			image.SetPosition( 0, 0, 0.05 )
			image.SetTexture( path )
			image.SetAlphaClamp( 0 )
			image.BlockForLoad( )
			images.push( image )
		}
		
		// Video Logo
		local videoLogo = ::VideoBootScreen( )
		videoLogo.SetPosition( 0, 0, 0.05 )
		videoLogo.SetAlphaClamp( 0 )
		images.push( videoLogo )
		
		// Autosave notice
		local autosave = ::AutosaveNoticeScreen( )
		autosave.SetPosition( 0, 0, 0 )
		//autosave.SetAlphaClamp( 0 )
		images.push( autosave )
		
		if( images.len( ) > 0 )
			AddChild( images[ 0 ] )

		loading = ::LoadingAnimation( )
		loading.SetPosition( vpRect.Left + 16, vpRect.Bottom - 16, 0 )
		loading.SetAlpha( 0 )
		AddChild( loading )
		
		levelName = ::Gui.Text( )
		levelName.SetFontById( FONT_SIMPLE_SMALL )
		levelName.SetRgba( COLOR_CLEAN_WHITE )
		local locString = ::GameApp.LocString( "Loading_LoadScreen_NoDiff" ).Replace( "levelName", ::GameApp.LocString( "FrontEnd" ) )
		levelName.BakeLocString( locString, TEXT_ALIGN_LEFT )
		levelName.SetPosition( vpRect.Left + 42, vpRect.Bottom - levelName.LineHeight, 0 )
		levelName.SetAlpha( 0 )
		AddChild( levelName )

		imageIndex = 0
		finalFadeOut = false

		SetZPos( 0.01 )
	}
	
	function OnTick( dt )
	{
		if( finalFadeOut )
		{
			SetAlphaClamp( timer )
			if( timer <= 0.0 )
				loadScreen.SetCanvasIsFadedOut( )
			timer -= 2.0 * dt
		}
		else if( imageIndex == images.len( ) )
		{
			loading.SetAlphaClamp( timer )
			levelName.SetAlphaClamp( timer )

			if( loadScreen.NewLevelIsLoaded( ) )
			{
				finalFadeOut = true
				timer = 1.0
				return
			}

			timer += 2.0 * dt
		}
		else
		{
			// Realistic settings
			local timePerImage = [ 5.7, 4.7, 6.9, 2.7 ]
			if( images.len( ) == 3 )
				timePerImage = [ 5.7, 6.9, 2.7 ]
			local fadeInTime = 0.35
			local fadeOutTime = 0.35
			
			if( ::GameApp.DebugFastBootupScreens( ) )
			{
				timePerImage = [ 1.0, 1.0, 1.0, 1.0 ]
				fadeInTime = 0.2
				fadeOutTime = 0.2
			}
			
			local image = images[ imageIndex ]
			local time = timePerImage[ imageIndex ]

			if( timer >= time )
			{
				if( "Unload" in image )
					image.Unload( )
				image.DeleteSelf( )
				RemoveChild( image )
				imageIndex += 1
				timer = 0.0

				if( imageIndex == images.len( ) )
				{
					finished = true
					return
				}

				image = images[ imageIndex ]
				time = timePerImage[ imageIndex ]
				
				if( "Start" in image )
					image.Start( )
				
				AddChild( image )
			}

			local alpha = null
			if( timer > time - fadeOutTime )
				alpha = ( time - timer ) / fadeOutTime
			else
				alpha = timer / fadeInTime
			image.SetAlphaClamp( alpha )

			timer += dt
		}

		::Gui.CanvasFrame.OnTick( dt )
	}
	
	function CanProceedToNewLevel( )
	{
		return ( finished ? 1 : 0 )
	}
}



