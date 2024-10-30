// Turret Repair Indicator

// Requires
sigimport "gui/scripts/controls/animatingcanvas.nut"
sigimport "gui/scripts/controls/radialprogressmeter.nut"

// Resources
sigimport "gui/textures/cursor/repair_indicator_g.png"
sigimport "gui/textures/cursor/repair_indicator_overlay_g.png"


sigexport function CanvasCreateControl( worldSpaceControl )
{
	return TurretRepairIndicator( )
}

class TurretRepairIndicator extends AnimatingCanvas
{
	indicatorIcon = null
	indicatorOverlay = null

	constructor( )
	{
		::AnimatingCanvas.constructor( )
		
		indicatorIcon = ::Gui.TexturedQuad( )
		indicatorIcon.SetTexture( "gui/textures/cursor/repair_indicator_g.png" )
		indicatorIcon.CenterPivot( )
		indicatorIcon.SetPosition( 0, 0, 0.001 )
		AddChild( indicatorIcon )
		
		indicatorOverlay = ::RadialProgressMeter( "gui/textures/cursor/repair_indicator_overlay_g.png" )
		indicatorOverlay.CenterPivot( )
		indicatorOverlay.SetPosition( 0, 0, 0 )
		indicatorOverlay.SetAngle( -MATH_PI_OVER_2 )
		AddChild( indicatorOverlay )
		
		Invisible = true
		SetAlpha( 0 )
		SetZPos( 0.7 )
	}

	function Show( )
	{
		AddAction( ::AlphaTween( GetAlpha( ), 1.0, 0.2, null, null, null, 
			function ( canvas )
			{
				canvas.Invisible = false
				canvas.SetAlpha( 1 )
			} ) )
	}
	
	function Hide( )
	{
		Invisible = true
		SetAlpha( 0 )
	}
	
	function SetPercent( percent )
	{
		indicatorOverlay.SetPercent( ::Math.Clamp( 1.0 - percent, 0.0, 1.0 ) )
	}
}