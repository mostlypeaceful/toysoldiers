// Hit Indicator

// Requires
sigimport "gui/scripts/controls/animatingcanvas.nut"

// Resources
sigimport "gui/textures/weapons/overlays/hit_indicator_g.png"

class HitIndicator extends AnimatingCanvas
{
	// Display
	image = null // Gui.TexturedQuad
	
	// Data
	//user = null // Entity
	//attacker = null // Entity
	ui = null // WeaponUI
	
	constructor( /*drivingUser, attackerEntity,*/ weaponUI )
	{
		::AnimatingCanvas.constructor( )
		
		//user = drivingUser
		//attacker = attackerEntity
		ui = weaponUI
		
		// Image
		image = ::Gui.TexturedQuad( )
		image.SetTexture( "gui/textures/weapons/overlays/hit_indicator_g.png" )
		image.CenterPivot( )
		image.SetPosition( 0, 0, 0 )
		image.SetAngle( ui.GetAngleToAttacker( ) )
		AddChild( image )
		
		// Fade out and die after 2 seconds
		FadeOutAndDie( 2.0 )
	}
	
	function OnTick( dt )
	{
		image.SetAngle( ui.GetAngleToAttacker( ) )
		::AnimatingCanvas.OnTick( dt )
	}
}