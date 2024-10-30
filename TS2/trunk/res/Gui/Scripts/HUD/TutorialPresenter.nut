// Class to handle presenting elements to the player in a tutorial

// Requires
sigimport "gui/scripts/controls/animatingcanvas.nut"

class TutorialPresentationManager extends Gui.CanvasFrame
{
	// Data
	canvases = null // Table of elements
	endAction = null
	shown = false
	data = null
	p = null
	pivot = null
	canvas = null
	sound = null
	
	constructor( )
	{
		::Gui.CanvasFrame.constructor( )
		
		// Add to the HUD root
		::GameApp.HudRoot.AddChild( this )
		SetPosition( 0, 0, 0.1 )
		
		// Table of canvases to animate
		canvases = { }
		shown = false
		data = null
		p = null
		pivot = null
		canvas = null
	}
	
	function RegisterCanvas( canvas, name, position, parentCanvas, sound = null )
	{
		// Add canvas and data to table
		canvases[ name ] <- {
			["name"] = name,
			["canvas"] = canvas,
			["position"] = ::Math.Vec3.Construct( position.x, position.y, position.z ),
			["parentCanvas"] = parentCanvas,
			["sound"] = sound
		}
	}
	
	function Present( name, user, pauseTime = null, noPivot = false, positionOffset = ::Math.Vec2.Construct( 0,0 ) )
	{
		if( name in canvases )
		{
			data = canvases[ name ]
			local vpRect = user.ComputeViewportSafeRect( )
			
			canvas = data.canvas
			p = data.parentCanvas
			sound = data.sound
			
			// Hide initially
			canvas.ClearActions( )
			canvas.SetAlpha( 0 )
			
			// Do this before we remove from the parent
			local screenSpacePosition = canvas.CalculateScreenPosition( ) // TODO
			screenSpacePosition.z = 0 // Always on top
			
			// Remove from parent
			if( canvas.HasParent( ) )
				canvas.Parent.RemoveChild( canvas )
			
			// Add as child of this
			AddChild( canvas )
			
			// Center on Screen
			local lr = canvas.LocalRect
			pivot = ( (noPivot)? ::Math.Vec2.Construct( 0, 0 ): ::Math.Vec2.Construct( lr.Left + lr.Width * 0.5, lr.Top + lr.Height * 0.5 ) )
			
			canvas.SetPosition( ::Math.Vec3.Construct( vpRect.Center.x + positionOffset.x - pivot.x, vpRect.Center.y + positionOffset.y - pivot.y, 0 ) )
			canvas.AddAction( ::MotionTween( canvas.GetPosition( ), canvas.GetPosition( ), 0.1 ) ) // HACK!?!?
			
			// Fade & Scale in
			canvas.AddAction( ::AlphaTween( 0.0, 1.0, 1.0 ) )
			
			// Wait for a few seconds then fly to proper position (in screen space)
			endAction = ::MotionTween( canvas.GetPosition( ), screenSpacePosition /*+ pivot*/, 1.0, EasingTransition.Quartic, EasingEquation.InOut, null, 
						AfterTween.bindenv(this) )
						
			if( pauseTime )
				canvas.AddDelayedAction( pauseTime, endAction )
			
			shown = true
		}
	}
	
	function MoveToEnd( )
	{
		if( shown )
		{
			canvas.AddAction( endAction )
			if( sound )
				::GameApp.AudioEvent( sound )
			shown = false
		}
	}
	
	function AfterTween( canvas )
	{
		// Insert into original parent... in proper position
		RemoveChild( canvas )
		p.AddChild( canvas )
		
		// in proper position and uncenter pivot
		canvas.SetPosition( data.position )
	}
}

// Global singleton
TutorialPresenter <- TutorialPresentationManager( )
