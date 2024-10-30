// Overlay for AC130 Barrage

// Requires
sigimport "gui/scripts/controls/animatingcanvas.nut"
sigimport "gui/scripts/hud/scoretimer.nut"
sigimport "gui/scripts/weapons/turrets/turret_at_03_hud.nut"

// Resources
sigimport "gui/textures/weapons/overlays/ac130_overlay_g.png"
sigimport "gui/textures/weapons/overlays/ac130_autogun_g.png"
sigimport "gui/textures/weapons/overlays/ac130_cannon_g.png"
sigimport "gui/textures/weapons/overlays/ac130_minigun_g.png"
sigimport "gui/textures/weapons/overlays/ac130_circle_g.png"
sigimport "gui/textures/weapons/overlays/ac130_triangle_g.png"
sigimport "gui/textures/weapons/overlays/ac130_reticle_g.png"
sigimport "gui/textures/weapons/overlays/ac130_rotator_g.png"
sigimport "gui/textures/weapons/overlays/ac130_20mm_g.png"
sigimport "gui/textures/weapons/overlays/ac130_40mm_g.png"
sigimport "gui/textures/weapons/overlays/ac130_105mm_g.png"

class JitteryUpDown extends AnimatingCanvas
{
	// Display
	image = null
	
	constructor( texturePath )
	{
		::AnimatingCanvas.constructor( )
		
		image = ::AnimatingCanvas( )
		local ret = ::Gui.TexturedQuad( )
		ret.SetTexture( texturePath )
		ret.CenterPivot( )
		ret.SetPosition( ::Math.Vec3.Construct( 0, 0, 0 ) )
		image.AddChild( ret )
		AddChild( image )
		
		AddJitter( )
	}
	
	function AddJitter( )
	{
		local newPos = SubjectiveRand.Float( 0, 160 )
		image.AddAction( ::YMotionTween( image.GetYPos( ), newPos, SubjectiveRand.Float( 0.5, 1.0 ), EasingTransition.Quadratic, EasingEquation.InOut, null, 
			function( canvas ) { AddJitter( ) }.bindenv(this) ) )
	}
}

class RandomNumberText extends AnimatingCanvas
{
	// Display
	text = null
	
	constructor( min, max, changeFrequency )
	{
		::AnimatingCanvas.constructor( )
		
		text = ::Gui.Text( )
		text.SetFontById( FONT_SIMPLE_SMALL )
		
		text.SetScale( ::Math.Vec2.Construct( 1.5, 1.5 ) )
		text.SetAlpha( 0.3 )
		AddChild( text )
		
		RandomNumber( min, max, changeFrequency )
	}
	
	function RandomNumber( min, max, changeFrequency )
	{
		text.BakeCString( ::SubjectiveRand.Int( min, max ).tostring( ) )
		AddAction( ::CanvasAction( changeFrequency, null, null, function( canvas ):(min, max, changeFrequency)
		{
			canvas.RandomNumber( min, max, changeFrequency )
		} ) )
	}
}

class AC130Overlay extends AnimatingCanvas
{
	// Display
	background = null
	barrageTime = null
	northAngleIndicator = null
	dial = null
	dialOrbiterCircle = null
	dialOrbiterDiamond = null
	upDownCircle = null
	upDownTriangle = null
	countUpBarrageTime = null
	distanceText = null
	altitudeText = null
	typeReticle = null
	typeLabel = null
	
	// Data
	vpIndex = null
	countUpTime = null
	mapType = null
	
	constructor( type = "mg" )
	{
		::AnimatingCanvas.constructor( )
		mapType = ::GameApp.CurrentLevelLoadInfo.MapType
		
		background = ::Gui.TexturedQuad( )
		background.SetTexture( "gui/textures/weapons/overlays/ac130_overlay_g.png" )
		background.SetPosition( ::Math.Vec3.Construct( 0, 0, 0.01 ) )
		AddChild( background )
		
		barrageTime = ::ScoreTimer_TimeLeftText( )
		barrageTime.text.SetFontById( FONT_FIXED_SMALL )
		barrageTime.SetPosition( ::Math.Vec3.Construct( 1280 / 2 - 20, 96, 0 ) )
		barrageTime.SetScale( ::Math.Vec2.Construct( 1.5, 1.5 ) )
		barrageTime.SetAlpha( 0.3 )
		AddChild( barrageTime )
		
		northAngleIndicator = ::Gui.Text( )
		northAngleIndicator.SetFontById( FONT_FIXED_SMALL )
		northAngleIndicator.SetAlpha( 0.3 )
		northAngleIndicator.SetScale( ::Math.Vec2.Construct( 1.9, 1.6 ) )
		northAngleIndicator.BakeCString( "N" )
		AddChild( northAngleIndicator )
		
		dial = ::Gui.TexturedQuad( )
		dial.SetTexture( "gui/textures/weapons/overlays/ac130_rotator_g.png" )
		dial.CenterPivot( )
		dial.SetPosition( 209, 521, 0 )
		AddChild( dial )
		
		dialOrbiterCircle = ::Gui.TexturedQuad( )
		dialOrbiterCircle.SetTexture( "gui/textures/weapons/overlays/ac130_circle_g.png" )
		dialOrbiterCircle.CenterPivot( )
		dialOrbiterCircle.SetPosition( 209 + 64, 521, 0 )
		AddChild( dialOrbiterCircle )
		
		dialOrbiterDiamond = ::Gui.TexturedQuad( )
		dialOrbiterDiamond.SetTexture( "gui/textures/weapons/overlays/ac130_diamond_g.png" )
		dialOrbiterDiamond.CenterPivot( )
		dialOrbiterDiamond.SetPosition( 209 + 56, 521, 0 )
		AddChild( dialOrbiterDiamond )
		
		upDownCircle = ::JitteryUpDown( "gui/textures/weapons/overlays/ac130_circle_g.png" )
		upDownCircle.SetPosition( 109, 400, 0 )
		AddChild( upDownCircle )
		
		upDownTriangle = ::Gui.TexturedQuad( )
		upDownTriangle.SetTexture( "gui/textures/weapons/overlays/ac130_triangle_g.png" )
		upDownTriangle.SetPosition( 121, 400, 0 )
		AddChild( upDownTriangle )
		
		AddRandomNumberText( 1087, 625, 1000, 9999, 0.3 )
		AddRandomNumberText( 1157, 625, 100, 999, 1.2 )
		AddRandomNumberText( 1141, 400, 1000, 9999, 0.8 )
		AddRandomNumberText( 1141, 330, 1000, 9999, 0.12 )
		
		countUpTime = 0
		countUpBarrageTime = ::Gui.Text( )
		countUpBarrageTime.SetFontById( FONT_FIXED_SMALL )
		countUpBarrageTime.SetPosition( 1145, 92, 0.011 )
		countUpBarrageTime.SetAlpha( 0.3 )
		AddChild( countUpBarrageTime )
		
		AddRandomNumberText( 236, 365, 0, 6, 2.0 )
		AddRandomNumberText( 105, 365, 20, 35, 1.3 )
		
		distanceText = ::Gui.Text( )
		distanceText.SetFontById( FONT_FIXED_SMALL )
		distanceText.SetPosition( 969, 112, 0.011 )
		distanceText.SetScale( ::Math.Vec2.Construct( 1.3, 1.3 ) )
		distanceText.SetAlpha( 0.3 )
		AddChild( distanceText )
		
		altitudeText = ::Gui.Text( )
		altitudeText.SetFontById( FONT_FIXED_SMALL )
		altitudeText.SetPosition( 1126, 112, 0.011 )
		altitudeText.SetScale( ::Math.Vec2.Construct( 1.3, 1.3 ) )
		altitudeText.SetAlpha( 0.3 )
		AddChild( altitudeText )

		local reticles = {
			cannon = "gui/textures/weapons/overlays/ac130_cannon_g.png", 
			mg = "gui/textures/weapons/overlays/ac130_minigun_g.png", 
			autogun = "gui/textures/weapons/overlays/ac130_autogun_g.png"
		}
		local labels = {
			cannon = "gui/textures/weapons/overlays/ac130_105mm_g.png", 
			mg = "gui/textures/weapons/overlays/ac130_20mm_g.png", 
			autogun = "gui/textures/weapons/overlays/ac130_40mm_g.png"
		}
		
		typeReticle = ::Gui.TexturedQuad( )
		typeReticle.SetTexture( reticles[ type ] )
		typeReticle.CenterPivot( )
		typeReticle.SetPosition( 1280 / 2, 720 / 2, 0.011 )
		AddChild( typeReticle )
		
		typeLabel = ::Gui.TexturedQuad( )
		typeLabel.SetTexture( labels[ type ] )
		typeLabel.SetPosition( 358, 89, 0.011 )
		AddChild( typeLabel )
		
		CenterPivot( )
	}
	
	function AddRandomNumberText( x, y, min, max, frequency )
	{
		local text = ::RandomNumberText( min, max, frequency )
		text.SetPosition( x, y, 0.011 )
		AddChild( text )
	}
	
	function SetViewportIndex( index )
	{
		vpIndex = index
		local vpRect = ::GameApp.ComputeViewportRect( vpIndex )
		SetPosition( vpRect.Center.x, vpRect.Center.y, 0.45 )
		SetScissorRect( vpRect )
	}
	
	function OnTick( dt )
	{
		::AnimatingCanvas.OnTick( dt )
		
		if( vpIndex == null )
			return
		
		// Get Data
		local data = ::GameApp.AC130BarrageData( vpIndex )
		
		if( data )
		{
			if( mapType != MAP_TYPE_MINIGAME )
			{
				// Time remaining in Barrage
				if( "barrageData" in data && "progress" in data )
				{
					local timeLeft = data.barrageData.Duration * ( 1 - data.progress )
					barrageTime.SetTime( timeLeft )
				}
				else
					barrageTime.SetTime( 0.0 )
			}
			else
			{
				// Time remaining in Minigame
				if( "timeLeft" in data )
				{
					local timeLeft = data.timeLeft
					barrageTime.SetTime( timeLeft )
				}
				else
					barrageTime.SetTime( 0.0 )
			}

			// North Indicator
			local angle = data.northAngle
			local radius = 200
			local center = ::Math.Vec2.Construct( 1280 / 2, 720 / 2 )
			northAngleIndicator.SetPosition( center.x + Math.Cos( angle ) * radius, center.y + Math.Sin( angle ) * radius, 0 )

			// Position of camera
			local pos = data.position
			local l = pos.Length( ).tointeger( )
			local y = pos.y.tointeger( )
			distanceText.BakeCString( l.tostring( ) )
			altitudeText.BakeCString( y.tostring( ) )
		}
		
		// Dial
		dial.SetAngle( dial.GetAngle( ) + (MATH_PI_OVER_2 * dt) )
		
		local a = dialOrbiterCircle.GetAngle( )
		local r = 68
		dialOrbiterCircle.SetAngle( a - (MATH_PI_OVER_4 * dt) )
		dialOrbiterCircle.SetPosition( 209 + Math.Cos( a ) * r, 521 + Math.Sin( a ) * r, 0 )
		
		a = dialOrbiterDiamond.GetAngle( )
		r = 46
		dialOrbiterDiamond.SetAngle( a + (MATH_PI_OVER_4 * dt) * 1.3 )
		dialOrbiterDiamond.SetPosition( 209 + Math.Cos( a ) * r, 521 + Math.Sin( a ) * r, 0 )
		
		// Up down thingy
		local newPos = upDownTriangle.GetYPos( )
		newPos -= 121 / 4 * dt //  }
		if( newPos < 400 )     //   > MAGIC NUMBERS ARE AWESOME
			newPos = 560       //  }
		upDownTriangle.SetYPos( newPos )
		
		// Count up time
		countUpTime += dt
		local s = countUpTime.tointeger( ).tostring( )
		while( s.len( ) < 3 )
			s = "0" + s
		countUpBarrageTime.BakeCString( s )
	}
}