// Controller button ring for used units

// Sigvars
sigvars Vanishing Controls
@[VanishAfter] { "Time to show controls", 7.0, [ 0.0:100.0 ], "Time before the controls fade out" }
@[FinalAlpha] { "Final alpha of faded controls", 0.00, [ 0.0:1.0 ], "This is the alpha value for the controls after they have fully faded out" }

// Requires
sigimport "gui/scripts/controls/controllerbutton.nut"
sigimport "gui/scripts/controls/controllerbuttoncontainer.nut"

// Resources
sigimport "gui/textures/weapons/controls/controls_semicircle_g.png"
sigimport "gui/textures/weapons/controls/controls_tickmark_g.png"

enum UnitControlRingConsts
{
	maxTickMarks = 7,
	radius = 67 // (this needs to match the texture) 
}

// Art for the UnitControlRing
////////////////////////////////////////////////////////////////////////////////
class UnitControlRingArt extends Gui.CanvasFrame
{
	// Display
	ring = null // Gui.TexturedQuad
	ticks = null // array of Gui.TexturedQuad objects
	
	constructor( )
	{
		::Gui.CanvasFrame.constructor( )
		
		// Create the ring image
		ring = ::Gui.TexturedQuad( )
		ring.SetTexture( "gui/textures/weapons/controls/controls_semicircle_g.png" )
		ring.SetPivot( ::Math.Vec2.Construct( ring.TextureDimensions( ).x * 0.5, 0 ) )
		ring.SetPosition( 0, 0, 0.02 )
		AddChild( ring )
		
		// Create the tickmarks
		ticks = [ ]
		for( local i = 0; i < UnitControlRingConsts.maxTickMarks; ++i )
		{
			local tick = ::Gui.TexturedQuad( )
			tick.SetTexture( "gui/textures/weapons/controls/controls_tickmark_g.png" )
			tick.SetPivot( ::Math.Vec2.Construct( tick.TextureDimensions( ).x * 0.5, 0 ) )
			tick.SetPosition( 0, 1, 0.01 )
			ticks.push( tick ) // Add to the list of tick marks
			AddChild( tick )
		}

	}
	
	// Essentially hide all the art
	function Clear( )
	{
		SetAlpha( 0.0 )
	}
	
	// Reposition the tick marks according to how many there are
	function SetControlCount( count )
	{
		// If only one control or more than the max, just do a regular reposition and hide the ring
		if( count <= 1 || count > UnitControlRingConsts.maxTickMarks )
		{
			Clear( )
			return
		}
		
		SetAlpha( 1.0 )
		
		// Otherwise, set up the tick marks
		local angleBetweenControls = MATH_PI / (UnitControlRingConsts.maxTickMarks - 1)
		local startAngle = ((UnitControlRingConsts.maxTickMarks - count) * 0.5) * angleBetweenControls
		foreach( i, tick in ticks )
		{
			tick.SetAngle( startAngle + i * angleBetweenControls - MATH_PI_OVER_2 )
			// If the tick index is greater than the current number of items, hide it
			tick.SetAlpha( ( i < count )? 1.0: 0.0 ) 
		}
		
		// And hide some of the ring
		local startHeight = ::Math.Sin( startAngle ) * UnitControlRingConsts.radius
		local pos = ring.GetPosition( )
		ring.SetPosition( ::Math.Vec3.Construct( pos.x, startHeight, pos.z ) )
		ring.SetTextureRect( ::Math.Vec2.Construct( 0, startHeight / ring.TextureDimensions( ).y ), ::Math.Vec2.Construct( 1, 1 ) )
	}
}


// Like ControllerButtonContainer but in a nice semi-circle with some extra art
////////////////////////////////////////////////////////////////////////////////
class UnitControlRing extends ControllerButtonContainer
{
	// Display
	ring = null // UnitControlRingArt
	
	// Vanishing controls
	vanishTimer = null // number, seconds before controls fade out
	vanishFadeTimer = null // number, seconds it takes for controls to fade
	vanishFinalAlpha = null // number, final alpha value for vanishing controls
	vanishAlphaVelocity = null // number
	
	// Data
	level = null
	
	constructor( isSorted = true )
	{
		::ControllerButtonContainer.constructor( FONT_SIMPLE_MED, 10, 1.0, isSorted )
		
		// Add the ring image
		ring = UnitControlRingArt( )
		AddChild( ring )
		
		Clear( )
		
		level = ::GameApp.CurrentLevel
	}
	
	function OnTick( dt )
	{
		::Gui.CanvasFrame.OnTick( dt )
		
		if( !level.TutAlwaysShowUnitControls )
		{
			if( vanishTimer > 0 )
			{
				vanishTimer -= dt
			}
			else
			{
				if( vanishFadeTimer > 0 )
				{
					local alphaDelta = dt * vanishAlphaVelocity
					SetAlphaClamp( GetAlpha( ) - alphaDelta )
					vanishFadeTimer -= dt
				}
			}
		}
	}
	
	function ResetVanishingControls( )
	{
		SetAlpha( 1.0 )
		
		vanishTimer = @[VanishAfter]
		vanishFadeTimer = 1.0
		vanishFinalAlpha = @[FinalAlpha]
		vanishAlphaVelocity = (1.0 - vanishFinalAlpha) / vanishFadeTimer
	}
	
	function Clear( )
	{
		ControllerButtonContainer.Clear( )
		ring.SetControlCount( 0 )
	}
	
	function Reposition( )
	{
		local count = controls.len( )
		
		// If there are 0 controls or less, just use the regular ControllerButtonContainer positioning
		if( count <= 0 )
		{
			ring.Clear( )
			return
		}
		
		// Otherwise, show the ring...
		ring.SetControlCount( count )
		
		// ...and set up the buttons around it
		local angleBetweenControls = MATH_PI / (UnitControlRingConsts.maxTickMarks - 1)
		local startAngle = ((UnitControlRingConsts.maxTickMarks - count) * 0.5) * angleBetweenControls
		local centerIndex = Math.RoundDown( count / 2 )
		local radius = UnitControlRingConsts.radius + 28
		local spacing = 10
		foreach( i, c in controls )
		{
			local xOffset = 0
			local yOffset = -14
			if( i < centerIndex )
			{
				c.SetAlignment( ControllerButtonAlignment.RIGHT )
				xOffset = spacing
				yOffset = 0
			}
			else if( i > centerIndex )
			{
				c.SetAlignment( ControllerButtonAlignment.LEFT )
				xOffset = -spacing
				yOffset = 0
			}
			else if( count % 2 == 0 )
			{
				xOffset = -spacing
				yOffset = 0
			}
			
			local angle = startAngle + (count - 1 - i) * angleBetweenControls
			local sin = ::Math.Sin( angle )
			local cos = ::Math.Cos( angle )
			c.SetPosition( ::Math.Vec3.Construct( cos * radius + xOffset, sin * radius + yOffset, 0.0 ) )
			
			c.ConstrainText( 300 )
		}
		
		if( count % 2 != 0 )
		{
			controls[ centerIndex ].SetAlignment( ControllerButtonAlignment.TOP )
		}
	}
}