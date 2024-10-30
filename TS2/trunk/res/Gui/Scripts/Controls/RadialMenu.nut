
sigvars Radial Menu
@[Stickiness] { "Stickiness", 0.95, [ 0.0:1.0 ] }

sigimport "Gui/Scripts/Controls/BaseMenu.nut"

class RadialMenuIcon extends BaseMenuIcon
{
	// Callback
	displayStringCallback = null // function
	
	constructor( imagePath, selectCb )
	{
		::BaseMenuIcon.constructor( imagePath, selectCb )
		CenterPivot( )
		transitionTime = 0.13
	}
	
	function OnSelect( )
	{
		// Bounce animation
		ClearActions( )
		local currentScale = GetScale( ).x
		local bounceScale = scaleActive.x * 1.2
		local endScale = scaleActive.x
		AddAction( ::UniformScaleTween( currentScale, bounceScale, 0.08 ) )
		AddDelayedAction( 0.08, ::UniformScaleTween( bounceScale, endScale, 0.3 ) )
		return ::BaseMenuIcon.OnSelect( )
	}
	
	function DisplayString( unit, player )
	{
		if( displayStringCallback )
			return displayStringCallback( unit, player )
		else
			return null
	}
}

class RadialMenu extends BaseMenu
{
	radius = null
	restrictSelection = -1
	
	// Display
	displayText = null
	
	constructor( radius_ = 66 )
	{
		::BaseMenu.constructor( )
		restrictSelection = -1
		radius = radius_
		
		scrollSound = "Play_HUD_WeaponMenu_Scroll"
		forwardSound = "Play_HUD_WeaponMenu_Select"
		backSound = "Play_HUD_WeaponMenu_Close"
		errorSound = "Play_HUD_WeaponMenu_Error"
		
		displayText = ::Gui.Text( )
		displayText.SetFontById( FONT_SIMPLE_SMALL )
		displayText.SetRgba( COLOR_CLEAN_WHITE )
		displayText.SetPosition( 0, 39 + radius, 0 )
		displayText.BakeCString( "", TEXT_ALIGN_CENTER )
		AddChild( displayText )
	}
	
	function FadeIn( )
	{
		PlaySound( "Play_HUD_WeaponMenu_Open" )
		::BaseMenu.FadeIn( )
	}
	
	function FadeOut( )
	{
		PlaySound( "Play_HUD_WeaponMenu_Close" )
		::BaseMenu.FadeOut( )
	}
	
	function SetRadius( _radius )
	{
		radius = _radius
	}
	
	function FinalizeIconSetup( ) // assumes 'icons' array is filled with RadialMenuIcons
	{
		if( icons == null || icons.len( ) == 0 )
			return

		local dtheta = MATH_2_PI / icons.len( )
		local theta = -MATH_PI_OVER_2
		foreach( i in icons )
		{
			i.SetPosition( radius * ::Math.Cos( theta ), radius * ::Math.Sin( theta ), i.GetZPos( ) )
			AddChild( i )
			theta += dtheta
		}		
	}
	
	function TryHotKeys( gamepad )
	{
		return false
	}
	
	function HighlightByAngle( angle, magnitude )
	{
		if( icons.len( ) == 0 )
			return false
		
		if( restrictSelection != -1 )
			return HighlightByIndex( restrictSelection )
			
		angle = MATH_2_PI - angle + MATH_PI_OVER_2
		local newHighlightIndex = magnitude > @[Stickiness] ? ::Math.QuantizeCircle( angle, icons.len( ) ) : -1
		return HighlightByIndex( newHighlightIndex )
	}
}

