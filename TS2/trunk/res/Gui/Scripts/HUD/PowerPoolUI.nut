@[TOP_OF_BAR] { "Top Of Bar Percentage:", 0.75, [ 0.0:1.0 ], "TODO COMMENT" }

// Requires
sigimport "Gui/Scripts/Controls/ProgressBar.nut"
sigimport "gui/scripts/controls/animatingcanvas.nut"

// Resources
sigimport "gui/textures/weapons/overcharge/overcharge_overlay_g.png"
sigimport "gui/textures/weapons/overcharge/overcharge_bar_g.png"
sigimport "effects/fx/gui/turbocharged_activated.fxml"
sigimport "effects/fx/gui/turbocharged_text_glow.fxml"

sigexport function CanvasCreatePowerPoolUI( cUI )
{
	return PowerPoolUI( cUI )
}

class TurbochargedText extends AnimatingCanvas
{
	turboText = null
	jitterCanvas = null
	jitterFactor = null
	jittering = null
	initialScale = 1.0
	flippedForSplitScreen = false
	colorPulseTimer = 0.0
	turnBackToTurboText = -1.0
	
	constructor( locID )
	{
		::AnimatingCanvas.constructor( )
		jitterFactor = 0
		jittering = false
		
		jitterCanvas = ::AnimatingCanvas( )
			turboText = ::Gui.Text( )
			turboText.SetFontById( FONT_FANCY_MED )
			turboText.SetPosition( ::Math.Vec3.Construct( 0, 0, 0.0 ) )
			turboText.BakeLocString( GameApp.LocString( locID ), TEXT_ALIGN_CENTER )
			turboText.SetUniformScale( 0.6 )
			jitterCanvas.AddChild( turboText )
		jitterCanvas.ShiftPivot( ::Math.Vec2.Construct( turboText.Width * 0.5, turboText.Height * 0.5 ) )
		jitterCanvas.SetPosition( 0 - turboText.Width * 0.5, turboText.Height * 0.5, 0 )
		
		turboText.SetRgba( 0, 0.64 * 1.333, 0.67 * 1.333, 1.0 )
		SetAlpha( 0.0 )
		AddChild( jitterCanvas )
	}
	
	function FlipTextForSplitScreen( )
	{
		flippedForSplitScreen  = true
		turboText.BakeLocString( GameApp.LocString( "Overcharge" ), TEXT_ALIGN_RIGHT )
		jitterCanvas.ShiftPivot( ::Math.Vec2.Construct( -turboText.Width, turboText.Height * 0.5 ) )
	}
	function ChangeToBarrageText( )
	{
		turnBackToTurboText = 4.0
		if( flippedForSplitScreen  )
		{
			turboText.BakeLocString( GameApp.LocString( "BarrageReady" ), TEXT_ALIGN_RIGHT )
			jitterCanvas.ShiftPivot( ::Math.Vec2.Construct( -turboText.Width, turboText.Height * 0.5 ) )
		}
		else
		{
			turboText.BakeLocString( GameApp.LocString( "BarrageReady" ), TEXT_ALIGN_CENTER )
			jitterCanvas.ShiftPivot( ::Math.Vec2.Construct( turboText.Width * 0.5, turboText.Height * 0.5 ) )
		}
	}

	function ChangeBackToTurbochargedText( )
	{
		if( flippedForSplitScreen  )
		{
			turboText.BakeLocString( GameApp.LocString( "Overcharge" ), TEXT_ALIGN_RIGHT )
			jitterCanvas.ShiftPivot( ::Math.Vec2.Construct( -turboText.Width, turboText.Height * 0.5 ) )
		}
		else
		{
			turboText.BakeLocString( GameApp.LocString( "Overcharge" ), TEXT_ALIGN_CENTER )
			jitterCanvas.ShiftPivot( ::Math.Vec2.Construct( turboText.Width * 0.5, turboText.Height * 0.5 ) )
		}
	}

	function SetJitter( jitter )
	{
		jitterFactor = jitter
		if( jitterFactor == null )
		{
			//if( jittering )
			{
				jittering = false
				jitterCanvas.ClearActions( )
				jitterCanvas.SetAngle( 0 )
				jitterCanvas.SetUniformScale( initialScale )
			}
		}
		else if( !jittering )
		{
			jittering = true
			DoJitter( )
		}
	}
	
	function DoJitter( )
	{
		local wasBarrageText = false
		if( turnBackToTurboText > 0.0 )
			wasBarrageText = true
		
		if( wasBarrageText )
		{
			turnBackToTurboText -= 0.033333
			if( turnBackToTurboText < 0.0 )
			{
				ChangeBackToTurbochargedText( )
			}
		}
		
		colorPulseTimer += 0.334567
		local t = ::Math.Abs( ::Math.Sin( colorPulseTimer  ) )
		local turboColor = ::Math.Vec4.Construct( 0, 0.64 * 1.333, 0.67 * 1.333, 1.0 )
		if( wasBarrageText )
		{
			turboColor = ::Math.Vec4.Construct( 1.333, 0.24 * 1.333, 0.27 * 1.333, 1.0 )
			colorPulseTimer += 0.334567		//again, but faster this time!
		}
		
		local whiteColor = ::Math.Vec4.Construct( 1.2, 1.2, 1.2, 1.0 )
		local currentColor = ::Math.Lerp4( turboColor, whiteColor, t )
		turboText.SetRgba( currentColor )
		
		local scale01 = 0.6
		local scale02 = 0.6543210
		if( wasBarrageText )
			scale02 = 0.7478238
		local t2 = ::Math.Abs( ::Math.Sin( colorPulseTimer * 2.2345 ) )
		turboText.SetUniformScale( ::Math.Lerp( scale01, scale02, t2 ) )
		
		local angleRange = ::Math.Vec2.Construct( -MATH_PI / 10, MATH_PI / 10 )
		local scaleRange = ::Math.Vec2.Construct( 0.0, 0.0 )
		local timeRange = ::Math.Vec2.Construct( 0.0, 0.0 )

		local newAngle = ::ObjectiveRand.Float( angleRange.x, angleRange.y ) * jitterFactor
		local newScale = initialScale + ( ::ObjectiveRand.Float( scaleRange.x, scaleRange.y ) * jitterFactor )
		local time = ::Math.Lerp( timeRange.x, timeRange.y, ( 1 - jitterFactor ) )
		time = ::ObjectiveRand.Float( time - 0.01, time + 0.01 )
		
		jitterCanvas.AddAction( ::AngleTween( jitterCanvas.GetAngle( ), newAngle, time, EasingTransition.Linear, null, null,  function( canvas ) { DoJitter( ) }.bindenv(this) ) )
		jitterCanvas.AddAction( ::UniformScaleTween( jitterCanvas.GetScale( ).x, newScale, time ) )
	}
	
	function Show( visible )
	{
		if( visible )
		{			
			FadeIn( 0.2 )
			//AddAction( ::RgbaPulse( ::Math.Vec4.Construct( 1, 1, 1, 0.7 ), ::Math.Vec4.Construct( 2, 2, 2, 1.0 ), 1.0 ) )
		}
		else
		{
			FadeOut( 0.2 )
		}
	}
}


class PowerPoolUI extends AnimatingCanvas
{
	// Display
	poolProgress = null
	overchargeText = null
	barrageText = null
	comboText = null
	comboTextAlign = null
	
	// Data
	ownerUIobj = null
	poolPercent = null
	visible = null
	flyPos = null
	isTurbochargeActive = false
	
	// Fx
	fireworks = null
	textGlow = null
	
	fireworksXPosition = null
	fireworksYPosition = null
	
	constructor( _ownerUIobj )
	{
		::AnimatingCanvas.constructor( )
		ownerUIobj = _ownerUIobj
		poolPercent = 0
		visible = true
		comboTextAlign = TEXT_ALIGN_LEFT
		
		local vp = ownerUIobj.User.ComputeViewportSafeRect( )

		poolProgress = ::ProgressBar( "gui/textures/weapons/overcharge/overcharge_overlay_g.png", "gui/textures/weapons/overcharge/overcharge_bar_g.png" )
		poolProgress.SetMode( PROGRESSBAR_MODE_TEXTURE )
		poolProgress.background.SetZPos( -0.001 )
		poolProgress.SetPosition( 0, -poolProgress.Size( ).y, 0.1 )
		AddChild( poolProgress )
		SetAlpha( 0 )
		
		local poolRect = poolProgress.WorldRect
		overchargeText = ::TurbochargedText( "Overcharge" )
		overchargeText.SetPosition( 280, -32, 0 )
		
		AddChild( overchargeText )
		
		comboText = ::Gui.Text( )
		comboText.SetFontById( FONT_FANCY_MED )
		comboText.SetRgba( 0.0, 0.64, 0.67, 1.0 ) // "Turbocharge Blue"
		comboText.SetUniformScale( 0.7 )
		comboText.SetPosition( poolProgress.Size( ).x + 10, -poolProgress.Size( ).y, 0 )
		AddChild( comboText )
		SetCombo( 0 )
		
		/*
		barrageText = ::AnimatingCanvas( )
			local barrage = ::Gui.Text( )
			barrage.SetFontById( FONT_FANCY_MED )
			barrage.SetRgba( COLOR_CLEAN_WHITE )
			barrage.BakeLocString( ::GameApp.LocString( "BarrageReady" ), TEXT_ALIGN_CENTER )
			barrageText.AddChild( barrage )
		barrageText.SetPosition( poolProgress.Size( ).x * 0.5, -poolProgress.Size( ).y - 4, -0.002 )
		barrageText.SetAlpha( 0 )
		AddChild( barrageText )
		*/
		ShowBarrage( false )
		
		local spacing = 54
		if( !::Player.DebugEnableMinimap( ) )
			spacing = 0
		local pos = ::Math.Vec3.Construct( vp.Left, vp.Bottom - spacing, 0.3 )
		flyPos = pos
		fireworksXPosition = 200
		fireworksYPosition = 616 + 54
		
		if( ::GameApp.GameMode.IsSplitScreen && (::GameApp.GameMode.IsCoOp || ::GameApp.GameMode.IsVersus) )
		{
			// Make everything smaller in split screen
			local scale = 0.85
			SetUniformScale( scale )
			//pos.y = vp.Bottom - spacing * scale
			fireworksYPosition += 3 - spacing;
			
			// Left screen user gets it on the other side and flipped
			if( ownerUIobj.User.ViewportIndex == 0 )
			{
				pos.x = vp.Right
				pos.y += poolProgress.Size( ).y * scale - 3
				poolProgress.SetAngle( MATH_PI )
				fireworksXPosition = 505
				//overchargeText.FlipTextForSplitScreen( )
				overchargeText.SetPosition( 55, -59, 0 )
				comboText.SetPosition( -poolProgress.Size( ).x - 10, -poolProgress.Size( ).y * 2, 0 )
				comboTextAlign = TEXT_ALIGN_RIGHT
			}
			else
			{
				fireworksXPosition = 800
			}
			
		}
		
		SetPosition( pos )
	}
	
	function Set( meterPercent, timerPercent )
	{
		poolPercent = meterPercent
		
		if( meterPercent < 1.0 )
			poolProgress.SetMeterHorizontal( poolPercent * timerPercent * @[TOP_OF_BAR] )
		else
			poolProgress.SetMeterHorizontal( 0.75 + (poolPercent - 1.0) * timerPercent * (1.0 - 0.75) )
	}
	
	function SetColor( color )
	{
		poolProgress.SetMeterColor( color )
	}

	function OverChargeActive( active )
	{
		if( active )
		{
			overchargeText.Show( true )
			
			if( fireworks == null )
			{
				fireworks = ::Gui.ScreenSpaceFxSystem( )
				fireworks.SetSystem( "effects/fx/gui/turbocharged_activated.fxml", -1, true )	//path, playcount(-1=loop), localSystem
				fireworks.SetPosition( fireworksXPosition, fireworksYPosition, 0.75 )		//not sure why my positioning doesn't line up with the overchargeText....hafta pixel-place it!!! urg!
				fireworks.SetDelay( 0.0 )
				AddChild( fireworks )
							
				textGlow = ::Gui.ScreenSpaceFxSystem( )
				textGlow.SetSystem( "effects/fx/gui/turbocharged_text_glow.fxml", -1, true )	//path, playcount(-1=loop), localSystem
				textGlow.SetPosition( fireworksXPosition, fireworksYPosition, 0.0 )		//not sure why my positioning doesn't line up with the overchargeText....hafta pixel-place it!!! urg!
				textGlow.SetDelay( 0.0 )
				AddChild( textGlow )			
				
				if( ::GameApp.GameMode.IsSplitScreen && (::GameApp.GameMode.IsCoOp || ::GameApp.GameMode.IsVersus) )
				{
					fireworks.SetUniformScale( 0.75 )
					textGlow.SetUniformScale( 0.75 )
				}
			}
			
			if( isTurbochargeActive == false )
			{
				overchargeText.ChangeBackToTurbochargedText( )
			}

			if( fireworks != null )
			{
				// begin the shaking!!!
				overchargeText.SetJitter( 0.025 )
				fireworks.SetEmissionRates( 1.0 )
				textGlow.SetEmissionRates( 1.0 )
				fireworks.OverrideSystemAlphas( 0.8 )
			}
		}
		else
		{
			overchargeText.SetJitter( null )
			overchargeText.Show( false )
			if( fireworks != null )
			{
				fireworks.SetEmissionRates( 0 )
				textGlow.SetEmissionRates( 0 )
			}
		}
		
		isTurbochargeActive = active
	}
	
	function Show( show )
	{
		isTurbochargeActive = false
		if( visible && !show )
			AddAction( ::AlphaTween( GetAlpha( ), 0.0, 0.5 ) )
		visible = show
		if( visible )
		{
			AddAction( ::AlphaTween( GetAlpha( ), 1.0, 0.5 ) )
			ShowBarrage( false )
			SetCombo( 0 )
		}
	}
	
	function ShowBarrage( show )
	{
		//barrageText.ClearActions( )
		if( show )
		{
			overchargeText.ChangeToBarrageText( )
			//barrageText.AddAction( ::AlphaPulse( 0.6, 1.0, 0.4 ) )
		}
		else
		{
			//barrageText.AddAction( ::AlphaTween( barrageText.GetAlpha( ), 0.0, 0.5 ) )
		}
	}	
	
	function SetCombo( combo )
	{
		if( combo < 1 )
			comboText.SetAlpha( 0 )
		else
			comboText.SetAlpha( 1 )
		
		comboText.BakeCString( combo.tostring( ) + "x", comboTextAlign )
	}
	
	function GetFlyToPosition( )
	{
		if( flyPos )
			return flyPos
		else
			return ::Math.Vec3.Construct( 0, 0, 0 )
	}
}
