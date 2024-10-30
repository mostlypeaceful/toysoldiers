// Animating Canvas

// Requires
sigimport "gui/scripts/utility/tweener.nut"

// Preset Tweens
////////////////////////////////////////////////////////////////////////////////

// Constructors:
// constructor( start, end, duration, transition = EasingTransition.Linear, equation = EasingEquation.Out, startCallback = null, endCallback = null )

class MotionTween extends Tweener
{	
	function OnTick( dt, canvas )
	{
		canvas.SetPosition( easer.Evaluate( timer ) )
	}
	
	function _typeof( ) 
		return "MotionTween"
}

class XMotionTween extends Tweener
{
	function OnTick( dt, canvas )
	{
		canvas.SetXPos( easer.Evaluate( timer ) )
	}
	
	function _typeof( ) 
		return "XMotionTween"
}

class YMotionTween extends Tweener
{
	function OnTick( dt, canvas )
	{
		canvas.SetYPos( easer.Evaluate( timer ) )
	}
	
	function _typeof( ) 
		return "YMotionTween"
}

class RgbTween extends Tweener
{	
	function OnTick( dt, canvas )
	{
		canvas.SetRgb( easer.Evaluate( timer ) )
	}
	
	function _typeof( ) 
		return "RgbTween"
}

class RgbaTween extends Tweener
{	
	function OnTick( dt, canvas )
	{
		canvas.SetRgba( easer.Evaluate( timer ) )
	}
	
	function _typeof( ) 
		return "RgbaTween"
}

class AlphaTween extends Tweener
{	
	function OnTick( dt, canvas )
	{
		canvas.SetAlpha( easer.Evaluate( timer ) )
	}
	
	function _typeof( ) 
		return "AlphaTween"
}

class ScaleTween extends Tweener
{	
	function OnTick( dt, canvas )
	{
		canvas.SetScale( easer.Evaluate( timer ) )
	}
	
	function _typeof( ) 
		return "ScaleTween"
}

class UniformScaleTween extends Tweener
{	
	function OnTick( dt, canvas )
	{
		local value = easer.Evaluate( timer )
		canvas.SetUniformScale( value )
	}
	
	function _typeof( ) 
		return "UniformScaleTween"
}

class AngleTween extends Tweener
{	
	function OnTick( dt, canvas )
	{
		canvas.SetAngle( easer.Evaluate( timer ) )
	}
	
	function _typeof( ) 
		return "AngleTween"
}

class TextCountTween extends TweenerWithSound
{
	text = null
	formatter = null
	sound = null
	soundCounter = null
	static threshold = 0.1
	
	constructor( textObj, start, end, duration, formatFunc = null, soundID = null, source = null )
	{
		::Tweener.constructor( start, end, duration, EasingTransition.Linear )
		audioSource = source
		text = textObj
		formatter = formatFunc
		sound = soundID
		soundCounter = 0
	}
	
	function _typeof( ) 
		return "TextCountTween"
	
	function OnTick( dt, canvas )
	{
		// Text
		local value = easer.Evaluate( timer )
		if( formatter )
			formatter( text, value )
		else
			text.BakeCString( value.tostring( ) )
		
		// Sound
		if( sound )
		{
			soundCounter += dt
			if( soundCounter > threshold )
			{
				PlaySound( sound )
			}
		}
	}
}

class SoundRtpcTween extends TweenerWithSound
{
	id = null
	
	constructor( rtpcEventID, start, end, duration, source = null )
	{
		::Tweener.constructor( start, end, duration, EasingTransition.Linear )
		audioSource = source
		id = rtpcEventID
	}
	
	function _typeof( ) 
		return "SoundRtpcTween"
	
	function OnTick( dt, canvas )
	{
		local value = easer.Evaluate( timer )
		SetAudioParam( id, value )
	}
}

class AlphaPulse extends Pulser
{
	function OnTick( dt, canvas )
	{
		canvas.SetAlphaClamp( easer.Evaluate( timer ) )
	}
	
	function _typeof( ) 
		return "AlphaPulse"
}

class RgbPulse extends Pulser
{
	function OnTick( dt, canvas )
	{
		canvas.SetRgb( easer.Evaluate( timer ) )
	}
	
	function _typeof( ) 
		return "RgbPulse"
}

class RgbaPulse extends Pulser
{
	function OnTick( dt, canvas )
	{
		canvas.SetRgba( easer.Evaluate( timer ) )
	}
	
	function _typeof( ) 
		return "RgbaPulse"
}

class UniformScalePulse extends Pulser
{
	function OnTick( dt, canvas )
	{
		local value = easer.Evaluate( timer )
		canvas.SetUniformScale( value )
	}
	
	function _typeof( ) 
		return "UniformScalePulse"
}

class SoundAction extends CanvasAction
{
	audioSource = null
	constructor( duration, startSound, endSound = null, source_ = null )
	{
		audioSource = source_
		local playStartSound = 
			function( canvas ):( startSound ) 
			{ 
				PlaySound( startSound )
			}
			
		local playEndSound = null
		if( endSound != null )
		{
			if( source != null )
				playEndSound = function( canvas ):( endSound ) { source.AudioEvent( endSound ) }
			else
				playEndSound = function( canvas ):( endSound ) { ::GameApp.AudioEvent( endSound ) }
		}
		::CanvasAction.constructor( duration, null, playStartSound, playEndSound )
	}
	
	function PlaySound( sound )
	{
		if( audioSource != null )
			audioSource.AudioEvent( sound )
		else
			::GameApp.AudioEvent( sound )
	}
}

// Base Animating Canvas class
////////////////////////////////////////////////////////////////////////////////
class AnimatingCanvas extends Gui.CanvasFrame
{
	// Data
	activeActions = null // array of AnimationAction objects, actions currently happening
	pendingActions = null // array of AnimationAction objects, actions waiting to be taken
	actionsToRemove = [ ]
	actionsToBecomeActive = [ ]
	audioSource = null
	
	constructor( )
	{
		::Gui.CanvasFrame.constructor( )
		
		activeActions = [ ]
		pendingActions = [ ]
	}
	
	function OnTick( dt )
	{
		::Gui.CanvasFrame.OnTick( dt )
		
		actionsToRemove.clear( )
		actionsToBecomeActive.clear( )
		
		// Update pending actions
		foreach( i, action in pendingActions )
		{
			action.delay -= dt
			if( action.delay < 0 )
			{
				if( action.onStart )
					action.onStart( this )
				actionsToBecomeActive.push( action )
				actionsToRemove.push( i )
			}
		}
		
		// Remove actions that are no longer pending
		actionsToRemove.reverse( )
		foreach( i in actionsToRemove )
			pendingActions.remove( i )
		actionsToRemove.clear( )
		
		// Update active actions
		foreach( i, action in activeActions )
		{
			action.Step( dt )
			
			if( action.timer > action.runTime )
			{
				actionsToRemove.push( i ) // (do this first in case they clear during onEnd)
				
				// Do last tick
				action.timer = action.runTime
				action.OnTick( 0, this )
				
				if( action.onEnd )
					action.onEnd( this )
				break
			}
			
			// The important part
			////////////////////////////////////////////////////////////////////
			action.OnTick( dt, this )
			////////////////////////////////////////////////////////////////////
		}
		
		actionsToRemove.reverse( )
		foreach( i in actionsToRemove )
			activeActions.remove( i )
		
		foreach( action in actionsToBecomeActive )
			activeActions.push( action )
	}
	
	// Interface
	function AddAction( action )
	{
		if( action.onStart )
			action.onStart( this )
		activeActions.push( action )
	}
	
	function AddDelayedAction( delay, action )
	{
		action.delay = delay
		pendingActions.push( action )
	}
	
	function ClearActions( )
	{
		pendingActions.clear( )
		activeActions.clear( )
		actionsToRemove.clear( )
		actionsToBecomeActive.clear( )
	}
	
	//internal use only
	function RemoveActionsOfTypeFromArray( type, array )
	{
		for( local i = array.len( ) - 1; i >= 0; --i )
			if( typeof array[ i ] == type )
				array.remove( i )
	}
	
	function ClearActionsOfType( type )
	{
		RemoveActionsOfTypeFromArray( type, pendingActions )
		RemoveActionsOfTypeFromArray( type, activeActions )
		RemoveActionsOfTypeFromArray( type, actionsToRemove )
		RemoveActionsOfTypeFromArray( type, actionsToBecomeActive )
	}
	
	// Common Animations
	function FadeIn( time = 1.0 )
	{
		ClearActions( )
		AddAction( ::AlphaTween( 0.0, 1.0, time, EasingTransition.Quadratic, EasingEquation.Out ) )
	}
	
	function FadeInAnd( time = 1.0, func = null )
	{
		ClearActions( )
		AddAction( ::AlphaTween( 0.0, 1.0, time, EasingTransition.Quadratic, EasingEquation.Out, null, func ) )
	}
	
	function FadeOut( time = 1.0 )
	{
		ClearActions( )
		AddAction( ::AlphaTween( 1.0, 0.0, time, EasingTransition.Quadratic, EasingEquation.Out ) )
	}
	
	function FadeOutAndDie( time = 1.0 )
	{
		ClearActions( )
		AddAction( ::AlphaTween( 1.0, 0.0, time, EasingTransition.Quadratic, EasingEquation.Out, null, function( canvas ) { canvas.DeleteSelf( ) } ) )
	}
	
	function FadeOutAnd( time = 1.0, func = null )
	{
		ClearActions( )
		AddAction( ::AlphaTween( 1.0, 0.0, time, EasingTransition.Quadratic, EasingEquation.Out, null, func ) )
	}
	
	function DoAfter( time, func )
	{
		AddAction( ::CanvasAction( time, null, null, func ) )
	}
	
	function PlaySound( sound )
	{
		 if( sound != null && !ParentIsInvisible( ) )
		 {
			if( audioSource != null )
			{
				audioSource.AudioEvent( sound )
			}
			else
				::GameApp.AudioEvent( sound )
		 }
	}
}
