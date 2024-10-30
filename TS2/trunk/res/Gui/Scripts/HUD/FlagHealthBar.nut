// Flag health bar

// Requires
sigimport "gui/scripts/controls/progressbar.nut"

// Resources
sigimport "gui/textures/score/usa_healthbar_01_g.png"
sigimport "gui/textures/score/usa_healthbar_02_g.png"
sigimport "gui/textures/score/usa_healthbar_03_g.png"
sigimport "gui/textures/score/usa_healthbar_04_g.png"
sigimport "gui/textures/score/usa_healthbar_05_g.png"
sigimport "gui/textures/score/usa_healthbar_06_g.png"
sigimport "gui/textures/score/sov_healthbar_01_g.png"
sigimport "gui/textures/score/sov_healthbar_02_g.png"
sigimport "gui/textures/score/sov_healthbar_03_g.png"
sigimport "gui/textures/score/sov_healthbar_04_g.png"
sigimport "gui/textures/score/sov_healthbar_05_g.png"
sigimport "gui/textures/score/sov_healthbar_06_g.png"
sigimport "gui/textures/score/health_frame_g.png"
sigimport "gui/textures/score/health_background_g.png"

class FlagHealthBar extends AnimatingCanvas
{
	// Display
	flag = null
	healthBar = null
	
	// Data
	flagPathBase = null
	
	// Statics
	static Width = 104
	static Height = 64
	
	constructor( startingPercent, country )
	{
		::AnimatingCanvas.constructor( )
		
		flag = ::Gui.TexturedQuad( )
		AddChild( flag )
		
		flagPathBase = ( ( country == COUNTRY_USSR )? "gui/textures/score/sov_healthbar_0": "gui/textures/score/usa_healthbar_0" )
		
		healthBar = ::ProgressBar( "gui/textures/score/health_frame_g.png", "gui/textures/score/health_background_g.png" )
		healthBar.SetMode( PROGRESSBAR_MODE_TEXTURE )
		healthBar.SetPosition( 0, Height + 4, 0 )
		AddChild( healthBar )
		
		SetPercent( startingPercent )
	}
	
	function SetPercent( percent )
	{
		percent = ::Math.Clamp( percent, 0.0, 1.0 )
		
		// Set Flag
		local flagNumber = ::Math.Lerp( 5.0, 0.0, percent ).tointeger( ) + 1
		local flagPath = flagPathBase + flagNumber.tostring( ) + "_g.png"
		flag.SetTexture( flagPath )
		
		// Set Bar
		healthBar.SetMeterHorizontal( percent )
		local meterColors = [
			::Math.Vec4.Construct( 0.0, 1.0, 0.0, 1.0 ),
			::Math.Vec4.Construct( 0.0, 1.0, 0.0, 1.0 ),
			::Math.Vec4.Construct( 1.0, 1.0, 0.0, 1.0 ),
			::Math.Vec4.Construct( 1.0, 0.5, 0.0, 1.0 ),
			::Math.Vec4.Construct( 1.0, 0.0, 0.0, 1.0 ),
			::Math.Vec4.Construct( 1.0, 0.0, 0.0, 1.0 ),
		]
		healthBar.SetMeterColor( meterColors[ flagNumber - 1 ] )
	}
}