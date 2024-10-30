// Meter for the battery

// Requires
sigimport "gui/scripts/controls/animatingcanvas.nut"
sigimport "Gui/Scripts/Controls/ProgressBar.nut"

// Resources
sigimport "gui/textures/weapons/battery/battery_overlay_g.png"
sigimport "gui/textures/weapons/battery/battery_bar_g.png"
sigimport "gui/textures/weapons/battery/low_battery_g.png"

sigexport function CanvasCreateBatteryMeter( ui )
{
	return BatteryMeter( ui )
}

class BatteryMeter extends AnimatingCanvas
{
	// Display
	bar = null // ProgressBar
	lowBatteryWarning = null // AnimatingCanvas
	
	// Data
	owner = null
	low = null
	
	constructor( ownerUI )
	{
		::AnimatingCanvas.constructor( )
		
		owner = ownerUI
		audioSource = ::GameApp.GetPlayerByUser( owner.User ).AudioSource
		
		// Create the bar
		bar = ::ProgressBar( "gui/textures/weapons/battery/battery_overlay_g.png", "gui/textures/weapons/battery/battery_bar_g.png" )
		bar.SetMode( PROGRESSBAR_MODE_TEXTURE )
		bar.background.SetZPos( -0.02 )
		bar.SetPosition( Math.Vec3.Construct( 0, -bar.Size( ).y, 0.0 ) )
		AddChild( bar )
		
		// Position (same as PowerPoolUI)
		local vp = owner.User.ComputeViewportSafeRect( )
		local spacing = 54
		if( !::Player.DebugEnableMinimap( ) )
			spacing = 0
		local pos = Math.Vec3.Construct( vp.Left, vp.Bottom - spacing, 0.3 )
		
		if( GameApp.GameMode.IsSplitScreen && (GameApp.GameMode.IsCoOp || GameApp.GameMode.IsVersus) )
		{
			// Make everything smaller in split screen
			local scale = 0.85
			SetScale( Math.Vec2.Construct( scale, scale ) )
			pos.y = vp.Bottom - spacing * scale
			
			// Left screen user gets it on the other side
			if( owner.User.ViewportIndex == 0 )
			{
				pos.x = vp.Right - bar.Size( ).x * scale
			}
		}
		
		// Create the low battery warning
		lowBatteryWarning = ::AnimatingCanvas( )
			local batteryImage = ::Gui.TexturedQuad( )
			batteryImage.SetTexture( "gui/textures/weapons/battery/low_battery_g.png" )
			batteryImage.CenterPivot( )
			batteryImage.SetPosition( 0, 0, 0 )
			lowBatteryWarning.AddChild( batteryImage )
		lowBatteryWarning.SetPosition( ::Math.Vec3.Construct( vp.Center.x, vp.Bottom - 64, 0 ) - pos )
		lowBatteryWarning.SetAlpha( 0 )
		AddChild( lowBatteryWarning )
		
		low = false
		
		Set( 1.0 )
		SetPosition( pos )
		SetAlpha( 0.0 )
	}
	
	function Set( percent )
	{
		local red = ::Math.Vec4.Construct( 1.0, 0.0, 0.0, 1.0 )
		local yellow = ::Math.Vec4.Construct( 1.0, 1.0, 0.0, 1.0 )
		local green = ::Math.Vec4.Construct( 0.0, 1.0, 0.0, 1.0 )
		local color = null
		
		// From 0.0 to mid, red to yellow
		local mid = 0.8
		if( percent < mid )
		{
			color = ::Math.Vec4.Lerp( red, yellow, ::Math.Remap( 0.0, mid, percent ) )
		}
		else // From mid to 1.0, yellow to green
		{
			color = ::Math.Vec4.Lerp( yellow, green, ::Math.Remap(mid, 1.0, percent ) )
		}
		
		bar.SetMeterColor( color )
		bar.SetMeterHorizontal( percent )
		
		// update low battery warning
		if( !low && percent < 0.1 )
		{
			low = true
			lowBatteryWarning.AddAction( ::RgbaPulse( ::Math.Vec4.Construct( 1.0, 1.0, 1.0, 0.5 ), ::Math.Vec4.Construct( 1.0, 1.0, 1.0, 1.0 ), 0.5 ) )
			PlaySound( "Play_HUD_LowBattery" )
		}
		else if( low && percent >= 0.1 )
		{
			CancelAlert( )
		}
	}
	
	function FadeOut( )
	{
		if( low )
		{
			CancelAlert( )
		}
		
		ClearActions( )
		AddAction( AlphaTween( GetAlpha( ), 0.0, 0.5, EasingTransition.Quadratic, EasingEquation.Out ) )
	}
	
	function FadeIn( )
	{
		ClearActions( )
		AddAction( AlphaTween( GetAlpha( ), 1.0, 0.5, EasingTransition.Quadratic, EasingEquation.In ) )
	}
	
	function CancelAlert( )
	{
		low = false
		lowBatteryWarning.ClearActions( )
		lowBatteryWarning.SetAlpha( 0 )
		PlaySound( "Stop_HUD_LowBattery" )
	}
}