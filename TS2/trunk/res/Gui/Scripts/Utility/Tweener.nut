// Tweening utilities for animating in script

// Constants
enum EasingTransition
{
	Linear,
	Sine,
	Quadratic,
	Cubic,
	Quartic,
	Quintic,
	Exponential,
	Elastic
}

enum EasingEquation
{
	In,
	Out,
	InOut
}

class CanvasAction
{
	// Data
	delay = null
	timer = null
	runTime = null
	
	// Callbacks
	onTick = null
	onStart = null
	onEnd = null
	
	constructor( duration, tickCallBack = null, startCallback = null, endCallback = null )
	{
		delay = 0
		runTime = duration
		timer = 0
		
		onTick = tickCallBack
		onStart = startCallback
		onEnd = endCallback
	}
	
	function Step( dt )
	{
		timer += dt
	}
	
	function OnTick( dt, canvas = null )
	{
		if( onTick )
			onTick( dt, canvas )
	}	
}

// Classes to manage animations
class Tweener extends CanvasAction
{
	// Data
	easer = null // Easing object
	
	constructor( start, end, duration, transition = null, equation = null, startCallback = null, endCallback = null )
	{
		::CanvasAction.constructor( duration, null, startCallback, endCallback )
		
		if( transition == null )
			transition = EasingTransition.Quadratic
		if( equation == null )
			equation = EasingEquation.Out
		
		switch( transition )
		{
		case EasingTransition.Linear:
			easer = LinearEaser( start, end - start, duration, equation )
			break
		case EasingTransition.Sine:
			easer = SineEaser( start, end - start, duration, equation )
			break
		case EasingTransition.Quadratic:
			easer = QuadraticEaser( start, end - start, duration, equation )
			break
		case EasingTransition.Cubic:
			easer = CubicEaser( start, end - start, duration, equation )
			break
		case EasingTransition.Quartic:
			easer = QuarticEaser( start, end - start, duration, equation )
			break
		case EasingTransition.Quintic:
			easer = QuinticEaser( start, end - start, duration, equation )
			break
		case EasingTransition.Exponential:
			easer = ExponentialEaser( start, end - start, duration, equation )
			break
		case EasingTransition.Elastic:
			easer = ElasticEaser( start, end - start, duration, equation )
			break
		}
		
		onTick = function( dt, canvas )
		{
			return easer.Evaluate( timer )
		}
	}
}

class Looper extends Tweener
{
	function Step( dt )
	{
		::Tweener.Step( dt )
		
		// Run forever
		if( timer > runTime )
		{
			timer -= runTime
			if( onEnd )
				onEnd( this )
		}
	}
}

class Pulser extends Tweener
{
	// Data
	direction = null
	
	constructor( start, end, duration, transition = null, equation = null, startCallback = null, endCallback = null )
	{
		// Defaults are a little different for Pulsers
		if( transition == null )
			transition = EasingTransition.Sine
		if( equation == null )
			equation = EasingEquation.InOut
			
		::Tweener.constructor( start, end, duration, transition, equation, startCallback, endCallback )
		direction = 1
	}
	
	function Step( dt )
	{
		::Tweener.Step( direction * dt )
		
		// Run forever
		if( timer > runTime )
		{
			timer = runTime - (timer - runTime)
			direction = -1
			if( onEnd )
				onEnd( this )
		}
		else if( timer < 0 )
		{
			timer = -timer
			direction = 1
			if( onStart )
				onStart( this )
		}
	}
}

class TweenerWithSound extends Tweener
{
	audioSource = null
	
	function PlaySound( sound )
	{
		if( audioSource != null )
			audioSource.AudioEvent( sound )
		else
			::GameApp.AudioEvent( sound )
	}
	
	function SetAudioParam( id, value )
	{
		if( audioSource != null )
			audioSource.SetAudioParam( id, value )
		else
			::GameApp.SetAudioParam( id, value )
	}
}

// A Squirrel implementation of Robert Penner's easing equations
// Reference: http://www.robertpenner.com/easing/penner_chapter7_tweening.pdf
////////////////////////////////////////////////////////////////////////////////
class Easer
{	
	// Data
	b = null
	c = null
	d = null
	eq = null
	
	// Functions
	fPow = null
	fSin = null
	fCos = null
	
	// begin and change must be scalable, differenceable
	// duration must be a scalar
	constructor( begin, change, duration, equation )
	{
		b = begin
		c = change
		d = duration
		
		fPow = ::Math.Pow
		fSin = ::Math.Sin
		fCos = ::Math.Cos
		
		if( duration == 0 )
		{
			eq = function( t ):(b) { return b }
			return
		}
		
		switch( equation )
		{
		case EasingEquation.In:
			eq = EaseIn.bindenv( this )
			break
		case EasingEquation.Out:
			eq = EaseOut.bindenv( this )
			break
		case EasingEquation.InOut:
			eq = EaseInOut.bindenv( this )
			break
		}
	}
	
	function Evaluate( t )
	{
		return eq( t )
	}
	
	// All virtual methods
	function EaseIn( t ) {}
	function EaseOut( t ) {}
	function EaseInOut( t ) {}
}

class LinearEaser extends Easer
{
	function EaseIn( t ) { return c * t / d + b }
	function EaseOut( t ) { return c * t / d + b }
	function EaseInOut( t ) { return c * t / d + b }
}

class SineEaser extends Easer
{
	function EaseIn( t )
	{
		return c * ( 1 - fCos( t/d * MATH_PI_OVER_2 ) ) + b
	}
	
	function EaseOut( t )
	{
		return c * fSin( t/d * MATH_PI_OVER_2 ) + b
	}
	
	function EaseInOut( t )
	{
		return c/2 * ( 1 - fCos( MATH_PI * t/d ) ) + b
	}
}

class QuadraticEaser extends Easer
{
	function EaseIn( t )
	{
		return c * (t /= d) * t + b
	}
	
	function EaseOut( t )
	{
		return -c * ( t /= d ) * ( t - 2 ) + b
	}
	
	function EaseInOut( t )
	{
		t = (t / d) * 2
		
		if ( t < 1 ) // first half
			return ((c / 2) * (t * t)) + b
		else // second half
			return -(c * 0.5) * ((t - 1) * (t - 3) - 1) + b
	}
}

class CubicEaser extends Easer
{
	function EaseIn( t )
	{
		return c * fPow( t / d, 3 ) + b
	}
	
	function EaseOut( t )
	{
		return c * ( fPow( t/d - 1, 3 ) + 1 ) + b
	}
	
	function EaseInOut( t )
	{
		t = (t / d) * 2
		if ( t < 1 )
			return c/2 * fPow( t, 3 ) + b
		return c/2 * ( fPow( t-2, 3 ) + 2 ) + b
	}
}

class QuarticEaser extends Easer
{
	function EaseIn( t )
	{
		return c * fPow( t/d, 4 ) + b
	}
	
	function EaseOut( t )
	{
		return -c * ( fPow( t/d - 1, 4 ) - 1 ) + b
	}
	
	function EaseInOut( t )
	{
		t = (t / d) * 2
		if ( t < 1 )
			return c/2 * fPow( t, 4 ) + b
		return -c/2 * ( fPow( t-2, 4 ) - 2 ) + b
	}
}

class QuinticEaser extends Easer
{
	function EaseIn( t )
	{
		return c * fPow( t/d, 5 ) + b
	}
	
	function EaseOut( t )
	{
		return c * ( fPow( t/d - 1, 5 ) + 1 ) + b
	}
	
	function EaseInOut( t )
	{
		t = (t / d) * 2
		if ( t < 1 )
			return c/2 * fPow( t, 5 ) + b
		return c/2 * ( fPow( t-2, 5 ) + 2 ) + b
	}
}

class ExponentialEaser extends Easer
{
	function EaseIn( t )
	{
		return c * fPow( 2, 10 * (t/d - 1) ) + b
	}
	
	function EaseOut( t )
	{
		return c * ( -fPow( 2, -10 * t/d ) + 1) + b;
	}
	
	function EaseInOut( t )
	{
		t = (t / d) * 2
		if ( t < 1 )
			return c/2 * fPow( 2, 10 * ( t - 1 ) ) + b
		return c/2 * ( -fPow( 2, -10 * --t ) + 2) + b
	}
}

class ElasticEaser extends Easer
{
	function EaseIn( t )
	{
		if( t == 0 )
			return b
		if( ( t /= d ) == 1 )
			return b + c
			
		local p = d * 0.3
		local a = c
		local s = p / 4
		local postFix = a * fPow( 2, 10 * ( t -= 1 ) )
		return -( postFix * fSin( ( t * d - s ) * ( MATH_2_PI ) / p ) ) + b
	}
	
	function EaseOut( t )
	{
		if( t == 0 )
			return b
		if( ( t /= d ) == 1 )
			return b + c
			
		local p = d * 0.3
		local a = c
		local s = p / 4
		return ( a * fPow( 2, -10 * t ) * fSin( ( t * d - s ) * ( MATH_2_PI ) / p ) + c + b );
	}
	
	function EaseInOut( t )
	{
		if( t == 0 )
			return b
		
		t = (t / d) * 2
		if( t == 1 )
			return b + c
			
		local p = d * 0.3
		local a = c
		local s = p / 4
		
		if( t < 1 )
		{
			local postFix = a * fPow( 2, 10 * ( t -= 1 ) )
			//return -0.5 * ( postFix * fSin( ( t * d - s ) * ( MATH_2_PI ) / p ) ) + b
			return (postFix* fSin( (t*d-s)*(MATH_2_PI)/p ))* -0.5 + b;
		}
		
		local postFix = a * fPow( 2, -10 * ( t -= 1 ) )
		//return ( postFix * fSin( ( t * d - s ) * ( MATH_2_PI ) / p ) ) * 0.5 + c + b
		return postFix * fSin( (t*d-s)*(MATH_2_PI)/p )*0.5 + c + b;
	}
}
