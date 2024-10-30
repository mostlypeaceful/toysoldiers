// Compass minimap

// Resources
sigimport "gui/textures/minimap/indicator_g.png"
sigimport "gui/textures/minimap/tick_g.png"
sigimport "gui/textures/minimap/enemy_g.png"
sigimport "gui/textures/minimap/friendly_g.png"
sigimport "gui/textures/minimap/toybox_g.png"

sigexport function CanvasCreateMiniMap( miniMap )
{
	return MiniMap( miniMap )
}

sigexport function CanvasCreateUnitIcon( unitIcon )
{
	return UnitMiniMapIcon( unitIcon, null )
}

class UnitMiniMapIcon extends Gui.CanvasFrame
{
	// C++ Data
	unitIcon = null
	unitLogic = null
	
	// Display
	image = null
	
	// Functionality
	angle = null
	minimap = null
	offset = null // number of pixels to offset in the y direction
	clampFunc = null
	wrapFunc = null
	lerpFunc = null
	remapFunc = null

	constructor( unitIcon_, user_, offset_ = 0 )
	{
		::Gui.CanvasFrame.constructor( )

		angle = 0
		unitIcon = unitIcon_
		offset = offset_
		local mathLib = ::Math
		clampFunc = mathLib.Clamp
		wrapFunc = mathLib.Wrap
		lerpFunc = mathLib.Lerp
		remapFunc = mathLib.Remap

		image = ::Gui.TexturedQuad( )
		image.SetTexture( "gui/textures/minimap/enemy_g.png" )
		image.SetXPos( -4 )
		AddChild( image )
		
		CenterPivot( )
	}

	function OnTick( dt )
	{
		if( ::GameApp.Paused( ) )
			return
			
		::Gui.CanvasFrame.OnTick( dt )
		
		if( unitLogic == null )
			return
			
		local angle = unitIcon.MiniMapAngle( unitLogic )
		local x = minimap.AngleToPos( angle )
		if( x > 292 && x < 580.5 )
			x = 292
		else if( x > 580.5 )
			x = 0
		SetPosition( x, 15 + offset, -0.01 )
		SetAlpha( clampFunc( AlphaBasedOnPos( x ), 0.5, 1.0 ) )
	}

	function SetUnit( unit )
	{
		unitLogic = unit
		angle = unitIcon.MiniMapAngle( unitLogic )
	}
	
	function AlphaBasedOnPos( x )
	{
		// If it is within the last smallPercent of either edge, map it between 0% and midAlpha
		if( x < 29.2 )
			return lerpFunc( 0.0, 0.5, remapFunc( 0, 29.2, x ) )
			
		if( x > 262.8 )
			return lerpFunc( 0.5, 0.0, remapFunc( 262.8, 292, x ) )
			
		// If it is from the smallPercent to the bigPercent, falloff from 100% to midAlpha
		if( x < 87.6 )
			return lerpFunc( 0.5, 1.0, remapFunc( 29.2, 87.6, x ) )
		
		if( x > 204.4 )
			return lerpFunc( 1.0, 0.5, remapFunc( 204.4, 262.8, x ) )
		
		return 1.0
	}
}

class CompassTick extends Gui.CanvasFrame
{
	// Data
	angle = null
	image = null
	
	constructor( a)
	{
		::Gui.CanvasFrame.constructor( )
		
		angle = a
		
		image = ::Gui.TexturedQuad( )
		image.SetTexture( "gui/textures/minimap/tick_g.png" )
		AddChild( image )
		
		CenterPivot( )
	}
}

class CompassCardinal extends Gui.CanvasFrame
{
	// Data
	angle = null
	text = null
	
	constructor( a, cardinalIndex )
	{
		::Gui.CanvasFrame.constructor( )
		
		angle = a
		
		local numCardinals = 8
		local ids = [ "Direction_S", "Direction_SW", "Direction_W", "Direction_NW", "Direction_N", "Direction_NE", "Direction_E", "Direction_SE" ]
		
		text = ::Gui.Text( )
		text.SetFontById( FONT_SIMPLE_SMALL )
		text.SetRgba( COLOR_CLEAN_WHITE )
		text.BakeLocString( GameApp.LocString( ids[ cardinalIndex ] ) )
		AddChild( text )
		
		CenterPivot( )
	}
}

class MiniMap extends AnimatingCanvas
{
	// Display
	indicator = null // Gui.TexturedQuad, this is essentially the background
	ticks = null // Array of CompassTick and CompassCardinal objects, tickmarks and cardinals on the line
	units = null // Array of UnitMiniMapIcon objects
	
	// Data
	miniMap = null
	visibleDistance = 0
	width = null
	enemyLine = null
	currentAngle = null // number
	totalWidth = null
	wrapFunc = null
	lerpFunc = null
	remapFunc = null

	constructor( _miniMap )
	{
		::AnimatingCanvas.constructor( )

		miniMap = _miniMap
		visibleDistance = 300
		currentAngle = 0
		totalWidth = 868
		local height = 44
		local midline = 24
		enemyLine = midline - 9
		wrapFunc = ::Math.Wrap
		lerpFunc = ::Math.Lerp
		remapFunc = ::Math.Remap
		
		// Indicator
		indicator = ::Gui.TexturedQuad( )
		indicator.SetTexture( "gui/textures/minimap/indicator_g.png" )
		indicator.SetPosition( 0, 0, 0.01 )
		AddChild( indicator )
		
		width = indicator.TextureDimensions( ).x
		
		// MiniMap compass goes in the bottom left corner
		local vpSafeRect = miniMap.User.ComputeViewportSafeRect( )
		local miniMapX = vpSafeRect.Left
		local miniMapY = vpSafeRect.Bottom - height
		
		local gameMode = ::GameApp.GameMode
		if( gameMode.IsSplitScreen && ( gameMode.IsCoOp || gameMode.IsVersus ) )
		{
			// Make everything smaller in split screen
			local scale = 0.85
			SetUniformScale( scale )
			miniMapY = vpSafeRect.Bottom - height * scale
			
			// Left screen user gets it on the other side
			if( miniMap.User.ViewportIndex == 0 )
				miniMapX = vpSafeRect.Right - width * scale
		}
		
		SetPosition( miniMapX, miniMapY, 0.2 )
		
		ticks = [ ]
		units = [ ]
		
		// Tickmarks, 4 per cardinal and diagonal (8) so 4 x 8 = 32. skip every 4th for each direction text
		local ticksPerCardinal = 4
		local numCardinals = 8
		local angleBetweenTicks = MATH_2_PI / (ticksPerCardinal * numCardinals)
		local angleBetweenCardinals = MATH_2_PI / numCardinals
		for( local i = 0; i < ticksPerCardinal * numCardinals; ++i )
		{
			// Create Cardinal
			if( i % 4 == 0 ) continue
			
			// Create Tick
			local tick = ::CompassTick( angleBetweenTicks * i )
			tick.SetPosition( AngleToPos( tick.angle ), midline, 0 )
			ticks.push( tick )
			AddChild( tick )
		}
		for( local i = 0; i < numCardinals; ++i )
		{
			local cardinal = ::CompassCardinal( angleBetweenCardinals * i, i )
			cardinal.SetPosition( AngleToPos( cardinal.angle ), midline, -0.001 )
			ticks.push( cardinal )
			AddChild( cardinal )
		}

		IgnoreBoundsChange = 1
	}
	
	function AngleToPos( angle )
	{
		return wrapFunc( -((angle - currentAngle) / MATH_2_PI * 868) + 146, 0, 868 )
	}
	
	function AlphaBasedOnPos( x )
	{
		// Invisible if outside of the width
		if( x < 0.0 || x > width )
			return 0.0
			
		// If it is within the last smallPercent of either edge, map it between 0% and midAlpha
		if( x < 29.2 )
			return lerpFunc( 0.0, 0.5, remapFunc( 0, 29.2, x ) )
			
		if( x > 262.8 )
			return lerpFunc( 0.5, 0.0, remapFunc( 262.8, 292, x ) )
			
		// If it is from the smallPercent to the bigPercent, falloff from 100% to midAlpha
		if( x < 87.6 )
			return lerpFunc( 0.5, 1.0, remapFunc( 29.2, 87.6, x ) )
		
		if( x > 204.4 )
			return lerpFunc( 1.0, 0.5, remapFunc( 204.4, 262.8, x ) )
		
		return 1.0
	}
	
	function OnTick( dt )
	{
		// Setup positions for all objects
		foreach( icon in ticks )
		{
			local x = AngleToPos( icon.angle )
			icon.SetXPos( x )
			icon.SetAlpha( AlphaBasedOnPos( x ) )
		}
		
		// Tick children
		::AnimatingCanvas.OnTick( dt )
	}
	
	function FadeIn( )
	{
		::AnimatingCanvas.FadeIn( 0.5 )
	}
	
	function FadeOut( )
	{
		::AnimatingCanvas.FadeOut( 0.5 )
	}

	function RotateMap( angle )
	{
		currentAngle = angle
	}
	
	function AddToyBox( toyboxIcon )
	{
		toyboxIcon.offset = 16
		toyboxIcon.image.SetTexture( "gui/textures/minimap/toybox_g.png" )
		AddUnit( toyboxIcon )
	}

	function AddDrivable( drivableUnitIcon )
	{
		drivableUnitIcon.offset = 16
		drivableUnitIcon.image.SetTexture( "gui/textures/minimap/friendly_g.png" )
		AddUnit( drivableUnitIcon )
	}
	
	function AddUnit( unitIcon )
	{
		unitIcon.minimap = this
		AddChild( unitIcon )
	}
}
