// Default reticle

class Reticle extends Gui.CanvasFrame
{
	// Display
	image = null // Gui.TexturedQuad
	alphaDelta = 0
	alphaIn = 0
	alphaOut = 0
	
	constructor( imagePath = null )
	{
		::Gui.CanvasFrame.constructor( )
		
		if( imagePath )
		{
			image = ::Gui.TexturedQuad( )
			image.SetTexture( imagePath )
			image.CenterPivot( )
			image.SetPosition( ::Math.Vec3.Construct( 0, 0, 0 ) )
			AddChild( image )
		}
			
		alphaIn = 1.0
		alphaOut = -1.0
		alphaDelta = alphaIn 
	}
	
	function SetReticlePos( pos ) {} // Virtual
	
	function ReticleSize( ) { return (image)? image.TextureDimensions( ): ::Math.Vec2.Construct( 0, 0 ) }
	
	function SetReticleSpread( spread ) {} // Virtual
	
	function SetReticleOverTarget( over )
	{
		if( over )
			SetRgb( Math.Vec3.Construct( 1, 0, 0 ) )
		else
			SetRgb( Math.Vec3.Construct( 1, 1, 1 ) )
	}
	
	function AutoAimBoxSize( ) { return ::Math.Vec2.Construct( 0, 0 ) } // Virtual
	
	function OnTick( dt ) 
	{
		SetAlphaClamp( GetAlpha( ) + alphaDelta * dt )
		::Gui.CanvasFrame.OnTick( dt )
	}
	
	function ShowHide( show ) 
	{
		alphaDelta = show ? alphaIn : alphaOut
	}
} 