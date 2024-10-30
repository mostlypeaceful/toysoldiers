sigimport "gui/textures/weapons/ammo/unknownammo_g.png"
sigimport "gui/scripts/hud/flashingtext.nut"
sigimport "gui/textures/weapons/ammo/bullet_tickmark_g.png"
sigimport "gui/textures/weapons/ammo/largeammo_tickmark_g.png"
sigimport "gui/textures/weapons/ammo/reload_symbol_g.png"
sigimport "gui/textures/weapons/ammo/infinity_symbol_g.png"

@[ReloadFadeOut] { "Reload Fadeout", 0.8, [ 0:1 ] }
@[ReloadFlashSpeed] { "Reload Flash Speed", 4.0, [ 0:100 ] }

enum AmmoCounterAlign
{
	Left,
	Right
}

class AmmoCounter extends Gui.CanvasFrame
{
	// Display
	ammoIcon = null // Gui.TexturedQuad, different per type of ammo
	countLabel = null // Gui.Text, displays number of remaining ammo
	reloadingText = null // FlashingText, only displays when reloading
	reloadingSymbol = null // Gui.TexturedQuad
	infiniteSymbol = null // AnimatingCanvas
	bar = null // Gui.TexturedQuad
	
	// Data
	ammoCount = null // number, to keep track if things have changed
	barHeight = null // number, height of bar
	barWidth = null // number
	maxAmmo = null // number, maximum number of projectiles
	tickSize = null // Vec2
	alignment = null // AmmoCounterAlign
	textAlign = null
	
	// Consts
	labelColor = null
	defaultIconPath = null
	textScale = null
	tickMarkSpacing = null
	
	// Fading
	fadingCountdown = null // number, time until fade happens
	fadingTimer = null // number, time until fade stops
	fadingVelocity = null // number, alpha value at end of fade
	
	constructor( iconPath, tickMarkIconPath, maxAmmo_ )
	{
		::Gui.CanvasFrame.constructor( )
		
		// Values
		labelColor = ::Math.Vec4.Construct( 1.0, 1.0, 1.0, 1.0 )
		defaultIconPath = "gui/textures/weapons/ammo/unknownammo_g.png"
		local defaultTickPath = "gui/textures/weapons/ammo/bullet_tickmark_g.png"
		textScale = 0.7
		tickMarkSpacing = 0
		
		ammoCount = 0
		maxAmmo = maxAmmo_
		
		// Create ammo icon
		ammoIcon = ::Gui.TexturedQuad( )
		SetAmmoIcon( iconPath )
		
		AddChild( ammoIcon )
		
		// Create infinite ammo symbol
		infiniteSymbol = ::AnimatingCanvas( )
			local infImage = ::Gui.TexturedQuad( )
			infImage.SetTexture( "gui/textures/weapons/ammo/infinity_symbol_g.png" )
			infiniteSymbol.AddChild( infImage )
		infiniteSymbol.CenterPivot( )
		infiniteSymbol.Invisible = true
		AddChild( infiniteSymbol )
		
		// Create the count label
		countLabel = ::Gui.Text( )
		countLabel.SetFontById( FONT_FANCY_MED )
		countLabel.SetRgba( labelColor )
		SetAmmoCount( 999 )
		countLabel.SetScale( Math.Vec2.Construct( textScale, textScale ) )
		
		AddChild( countLabel )
		
		reloadingSymbol = ::Gui.TexturedQuad( )
		reloadingSymbol.SetTexture( "gui/textures/weapons/ammo/reload_symbol_g.png" )
		reloadingSymbol.CenterPivot( )
		
		reloadingSymbol.SetAlpha( 0.0 )
		AddChild( reloadingSymbol )
		
		if( !GameApp.GameMode.IsSplitScreen || ::GameApp.SingleScreenCoop )
		{
			// Create the ammo count bar
			bar = ::Gui.TexturedQuad( )
			bar.SetTexture( (tickMarkIconPath == null)? defaultTickPath: tickMarkIconPath )
			bar.ColorMap.SetAddressModes( ADDRESS_MODE_WRAP )
			tickSize = bar.TextureDimensions( )
			local maxBarLength = 300 // Empirical
			barWidth = ( ( maxAmmo * tickSize.x > maxBarLength )? maxBarLength: maxAmmo * tickSize.x )
			barHeight = tickSize.y
			AddChild( bar )
		}
		
		SetAlignment( AmmoCounterAlign.Right )

		// Set bar to full
		SetBarValue( 1.0 )
		
		Modified( )
	}
	
	function Reposition( )
	{
		local spaceForText = 37
		local zFront = -0.07
		local zMid = -0.05
		local iconX
		local countX
		local symbolX
		local barX
		local barAngle
		
		if( alignment == AmmoCounterAlign.Right )
		{
			iconX = -ammoIcon.WorldRect.Width
			countX = iconX - 8 - spaceForText
			symbolX = iconX - 5 - spaceForText * 0.5
			barX = countX - 6
			barAngle = MATH_PI
		}
		else if( alignment == AmmoCounterAlign.Left )
		{
			iconX = 0
			countX = iconX + ammoIcon.WorldRect.Width + 8, -countLabel.WorldRect.Height * 0.5
			symbolX = countX - 3 + spaceForText * 0.5
			barX = countX + spaceForText + 6
			barAngle = 0
		}
		
		ammoIcon.SetPosition( Math.Vec3.Construct( iconX, -ammoIcon.WorldRect.Height * 0.5, zFront ) )
		countLabel.SetPosition( Math.Vec3.Construct( countX, -countLabel.WorldRect.Height * 0.5, zFront ) )
		reloadingSymbol.SetPosition( Math.Vec3.Construct( symbolX, 0.0, zMid ) )
		infiniteSymbol.SetPosition( Math.Vec3.Construct( symbolX, 0.0, zMid ) )
		if( !GameApp.GameMode.IsSplitScreen || ::GameApp.SingleScreenCoop )
		{
			bar.SetPosition( Math.Vec3.Construct( barX, barHeight / 2, zMid ) )
			bar.SetAngle( barAngle )
		}
	}
	
	function OnTick( dt )
	{
		//reloadingText.OnTick( dt )
		reloadingSymbol.SetAngle( reloadingSymbol.GetAngle( ) + dt * MATH_PI )
		
		if( fadingCountdown > 0 )
		{
			fadingCountdown -= dt
		}
		else
		{
			if( fadingTimer > 0 ) 
			{
				fadingTimer -= dt
				SetAlpha( GetAlpha( ) - fadingVelocity * dt )
			}
		}
	}
	
	function SetAlignment( a )
	{
		if( alignment == a )
			return
			
		alignment = a
		Reposition( )
		textAlign = ( alignment == AmmoCounterAlign.Left )? TEXT_ALIGN_LEFT: TEXT_ALIGN_RIGHT
	}
	
	function Modified( )
	{
		fadingCountdown = 1.0 
		fadingTimer = 0.5
		fadingVelocity = (1.0 - 0.5) / fadingTimer // (start - end) / duration
		SetAlpha( 1.0 )
	}
	
	function SetAmmoIcon( iconPath )
	{
		if( iconPath ) 	ammoIcon.SetTexture( iconPath )
		else			ammoIcon.SetTexture( defaultIconPath )
	}
	
	function SetBarValue( value ) // [0:1]
	{
		if( GameApp.GameMode.IsSplitScreen && !::GameApp.SingleScreenCoop )
			return
			
		value = ::Math.Clamp( value, 0.0, 1.0 )
		local barSize = ::Math.Vec2.Construct( ::Math.RoundDown(barWidth * value / tickSize.x) * tickSize.x, barHeight )
		bar.SetRect( barSize )
		local sign = ( alignment == AmmoCounterAlign.Right )? -1: 1
		bar.SetTextureRect( ::Math.Vec2.Construct( 0, 0 ), ::Math.Vec2.Construct( sign * barSize.x / tickSize.x, sign * barSize.y / tickSize.y ) )
	}
	
	function SetAmmoCount( value )
	{
		ammoCount = value
		if( value < 0 )
		{
			// Infinite ammo
			countLabel.Invisible = true
			infiniteSymbol.Invisible = false
		}
		else
		{
			countLabel.Invisible = false
			infiniteSymbol.Invisible = true
			countLabel.BakeCString( value.tostring( ), TEXT_ALIGN_LEFT )
		}
	}
	
	function SetAmmoValues( percent, count, reloading, forceRefresh ) // percent: number [0:1], count: number, reloading: bool
	{
		// hide/show numbers and reload based on flag
		//reloadingText.Show( reloading )
		reloadingSymbol.SetAlpha( (reloading)? 1.0: 0.0 )
		countLabel.SetAlpha( (reloading)? 0.0: 1.0 )
		
		if( count != ammoCount || reloading || count < 0 || forceRefresh )
		{
			// Set the bar
			SetBarValue( percent )

			// Trigger the ammo counter modified
			Modified( )
		}
		
		// Set the count
		if( count != ammoCount ) SetAmmoCount( count )
	}
}
