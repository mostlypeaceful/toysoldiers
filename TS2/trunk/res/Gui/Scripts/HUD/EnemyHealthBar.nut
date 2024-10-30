sigexport function CanvasCreateControl( worldSpaceControl )
{
	local healthBar = EnemyHealthBar( worldSpaceControl )
	
	return healthBar
}

class EnemyHealthBar extends Gui.CanvasFrame
{
	worldSpaceControl = null
	bg = null
	fg = null

	barWidth = 16
	barHeight = 2

	fadeInSpeed = 4.0
	fadeOutSpeed = -2.0

	fadeTimer = 0
	fadeDelta = 1
	
	constructor( _worldSpaceControl )
	{
		Gui.CanvasFrame.constructor( )

		worldSpaceControl = _worldSpaceControl

		fadeInSpeed = 4.0
		fadeOutSpeed = -2.0
		
		fadeDelta = fadeOutSpeed
		fadeTimer = 0

		bg = ::Gui.ColoredQuad( )
		bg.SetRect( Math.Vec2.Construct( barWidth, barHeight ) )
		bg.SetPosition( Math.Vec3.Construct( 0, 0, 0.3 ) )
		bg.SetRgba( Math.Vec4.Construct( 0.0, 0.0, 0.0, 0.8 ) )
		AddChild( bg )

		fg = ::Gui.ColoredQuad( )
		fg.SetRect( Math.Vec2.Construct( barWidth, barHeight ) )
		fg.SetPosition( Math.Vec3.Construct( 0, 0, 0.29 ) )
		fg.SetRgba( Math.Vec4.Construct( 0.8, 0.0, 0.0, 0.6 ) )
		AddChild( fg )

		CenterPivot( )

		SetAlpha( 0 )
		Disabled = true
	}
	
	function SetSize( width, height )
	{
		if( bg && fg )
		{
			barWidth = width
			barHeight = height
			
			bg.SetRect( Math.Vec2.Construct( barWidth, barHeight ) )
			fg.SetRect( Math.Vec2.Construct( barWidth, barHeight ) )
			
			CenterPivot()
		}
	}
	
	function OnTick( dt )
	{
		local newAlpha = GetAlpha( ) + fadeDelta * dt
		SetAlphaClamp( newAlpha )

		::Gui.CanvasFrame.OnTick( dt )

		// Once we have faded all the way in wait 1 second and then start to fade out.
		// If SetHealthBarPercent is called again (unit took damage) the timer resets
		if( newAlpha >= 1 )
		{
			fadeTimer += dt
			if( fadeTimer > 1 )
				fadeDelta = fadeOutSpeed
		}
		else if( newAlpha <= 0 )
		{
			Disabled = true
		}
		
		
		Parent.IgnoreBoundsChange = 1

	}
	function SetHealthBarPercent( health )
	{
		Disabled = false

		fg.SetRect( Math.Vec2.Construct( barWidth * health, barHeight ) )
		fadeTimer = 0

		if( health > 0 )
			fadeDelta = fadeInSpeed
		else
			fadeDelta = fadeOutSpeed
	}
}

