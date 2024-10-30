

class FadeInFadeOutTexture extends Gui.CanvasFrame
{
	time = 0
	inDelta = 0
	outDelta = 0
	currentDelta = 0
	forceShow = false
	
	image = null
	
	constructor( path )
	{
		::Gui.CanvasFrame.constructor( )
		
		image = ::Gui.TexturedQuad( )
		image.SetTexture( path )
		image.CenterPivot( )
		image.SetPosition( ::Math.Vec3.Construct( 0, 0, 0 ) )
		AddChild( image )
		
		time = 0
		inDelta = 2.0
		outDelta = -2.0
		currentDelta = outDelta
		forceShow = false
		SetAlpha( 0 )
	}
	
	// < 0 for always on, > 1 for time before fade out
	// force if you dont want a FadeOut to clear until the timer expires
	function SetTime( newTime, force )
	{
		time = newTime
		currentDelta = inDelta
		forceShow = force
	}
	
	function OnTick( dt )
	{
		if( time > 0 )
		{
			time -= dt
			if( time <= 0 )
			{
				currentDelta = outDelta
				forceShow = false
			}
		}
		
		SetAlphaClamp( GetAlpha( ) + currentDelta * dt )
		
		::Gui.CanvasFrame.OnTick( dt )
	}
	
	function FadeOut( )
	{
		if( !forceShow )
			currentDelta = outDelta
	}
	
	function ForceHide( )
	{
		currentDelta = 0
		SetAlpha( 0 )
	}
}
