// Unit Pickup Indicator

// Requires
sigimport "gui/scripts/controls/animatingcanvas.nut"

// Resources
sigimport "gui/textures/cursor/specialsoldierstar_g.png"

UnitPickupIndicator_Art <- {
	[ PICKUPS_BARRAGE_ROLL ] = "gui/textures/cursor/specialsoldierstar_g.png",
}

sigexport function CanvasCreateControl( worldSpaceControl )
{
	return ::UnitPickupIndicator( ) 
}

class UnitPickupIndicator extends AnimatingCanvas
{
	// Display
	icon = null
	
	constructor( )
	{
		::AnimatingCanvas.constructor( )
		
		icon = ::Gui.TexturedQuad( )
		AddChild( icon )
		
		SetZPos( 0.7 )
		Invisible = true
		SetAlpha( 0 )
	}
	
	function Show( )
	{
		Invisible = false
		AddAction( ::AlphaTween( 0.0, 1.0, 0.2, null, null, null, 
			function ( canvas )
			{
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
		local pickupID = percent.tointeger( )
		local texturePath = null
		
		if( pickupID in ::UnitPickupIndicator_Art )
			texturePath = UnitPickupIndicator_Art[ pickupID ]
		
		if( texturePath )
		{
			icon.SetTexture( texturePath )
			icon.CenterPivot( )
			icon.SetPosition( 0, 0, 0 )
		}
	}
}