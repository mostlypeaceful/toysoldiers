
// Requires 
sigimport "gui/scripts/controls/progressbar.nut"

// Resources
sigimport "gui/textures/bosshealthbar/bosshealthbar_bg_g.png"
sigimport "gui/textures/bosshealthbar/bosshealthbar_g.png"

sigexport function CanvasCreateScreenSpaceHealthBarList( healthBarList )
{
	return ScreenSpaceHealthBar( )
}

class ScreenSpaceHealthBar extends AnimatingCanvas
{
	image = null
	displayText = null
	bar = null
	goalValue = 0.0
	goalColor = null
	currentColor = null
	fillTime = 0.0
	fillDuration = 0.0
	
	// Statics
	static barWidth = 256
	static barHeight = 16

	constructor( text = null, imagePath = null )
	{
		::AnimatingCanvas.constructor( )
		goalValue = 1.0
		fillTime = 0.0
		fillDuration = 0.0
		goalColor = ::Math.Vec4.Construct( 1.0, 1.0, 1.0, 1.0 )
		currentColor = goalColor

		image = ::Gui.TexturedQuad( )
		if( imagePath )
			image.SetTexture( imagePath )
		image.SetPosition( -barWidth - image.TextureDimensions( ).x, -image.TextureDimensions( ).y * 0.5 + 38, 0 )
		AddChild( image )

		displayText = ::Gui.Text( )
		displayText.SetFontById( FONT_SIMPLE_MED )
		displayText.SetPosition( -barWidth * 0.5, 0, 0 )
		if( text )
			displayText.BakeLocString( text, TEXT_ALIGN_CENTER )
		AddChild( displayText )
		
		bar = ::ProgressBar( "gui/textures/bosshealthbar/bosshealthbar_bg_g.png", "gui/textures/bosshealthbar/bosshealthbar_g.png" )
		bar.background.SetZPos( -0.01 )
		bar.SetMode( PROGRESSBAR_MODE_TEXTURE )
		bar.SetPosition( -barWidth, 30, 0 )
		AddChild( bar )

		SetAlpha( 0 )
		SetHealthBarPercent( null, 1.0 )
		local vpRect = ::GameApp.ComputeScreenSafeRect( )
		SetPosition( vpRect.Center.x + barWidth * 0.5 + 38, vpRect.Top + 10, 0.2 )
	}
	
	function OnTick( dt )
	{
		::AnimatingCanvas.OnTick( dt )
		if( fillDuration != 0.0 )
		{
			fillTime += dt
			if( fillTime < fillDuration )
			{
				local percent = fillTime / fillDuration
				bar.SetMeterHorizontal( goalValue * percent )
				
				local flashes = 6
				local flash = ::Math.Cos( MATH_2_PI * percent * flashes ) * 0.5 + 0.5
				flash = ::Math.RemapMinimum( flash, 0.4 )
				local color = ::Math.Lerp4( currentColor, goalColor, percent )
				bar.SetMeterColor( color * flash )
			}
			else
			{				
				bar.SetMeterHorizontal( goalValue )
				bar.SetMeterColor( goalColor )
				currentColor = goalColor
				
				SetFillTime( 0.0 )
			}
		}
	}
	
	function AddHealthBar( text, imagePath )
	{
		if( text )
			displayText.BakeLocString( text, TEXT_ALIGN_CENTER )
		
		if( imagePath )
		{
			image.SetTexture( imagePath )
			image.SetPosition( -barWidth - image.TextureDimensions( ).x, -image.TextureDimensions( ).y * 0.5 + 38, 0 )
		}
		
		FadeIn( )
		
		// Hide regular list
		::GameApp.CurrentLevel.ShowWaveListUI( false )
	}
	
	function SetHealthBarPercent( index, percent )
	{
		goalValue = percent
		if( fillDuration == 0.0 )
			bar.SetMeterHorizontal( goalValue )
	}
	
	function SetColor( index, color )
	{
		goalColor = color
		if( fillDuration == 0.0 )
		{
			currentColor = goalColor
			bar.SetMeterColor( goalColor )
		}
	}
	
	function SetFlashAndFill( index, flash, fill )
	{
		SetFillTime( fill )
	}

	function SetFillTime( time )
	{
		fillDuration = time
		fillTime = 0.0	
	}
}
