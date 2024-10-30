


class TargetIcon extends Gui.TexturedQuad
{
	constructor( )
	{
		::Gui.TexturedQuad.constructor( )
		SetTexture( "Gui/Textures/Weapons/MachineGun/reticle_square01_g.png" )
		CenterPivot( )
		SetRgba( 1, 0, 0, 0.25 )
	}
}

class LockIcon extends Gui.TexturedQuad
{
	constructor( )
	{
		::Gui.TexturedQuad.constructor( )
		SetTexture( "Gui/Textures/Weapons/MachineGun/reticle_circular04_g.png" )
		CenterPivot( )
		SetRgba( 1, 1, 0, 0.75 )
	}
}