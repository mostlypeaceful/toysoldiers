// Reticle for auto-aim, like the Apache

sigvars Auto-Aim
@[AutoAimSize] { "Size", (64,64), [ 0:2000 ] }

// Requires
sigimport "gui/scripts/weapons/reticle.nut"

// Resources
sigimport "Gui/Textures/Weapons/MachineGun/reticle_g.png"
sigimport "Gui/Textures/Weapons/MachineGun/reticle_large_square_g.png"

class AutoAimReticle extends Reticle
{
	// Display
	autoAimBox = null // Gui.TexturedQuad
	
	// Data
	size = null // Math.Vec2
	
	constructor( size_ = null )
	{
		::Reticle.constructor( "Gui/Textures/Weapons/MachineGun/reticle_g.png" )
	
		size = size_? size_: @[AutoAimSize]
		
		autoAimBox = ::Gui.TexturedQuad( )
		autoAimBox.SetTexture( "Gui/Textures/Weapons/MachineGun/reticle_large_square_g.png" )
		autoAimBox.CenterPivot( )
		autoAimBox.SetPosition( 0, 0, 0.01 )
		AddChild( autoAimBox )
	}
	
	function SetReticlePos( pos )
	{
		local offset = pos - GetPosition( )
		image.SetPosition( offset )
	}
	
	function AutoAimBoxSize( )
	{
		return size
	}
	
	function SetReticleSpread( spread ) { }
} 