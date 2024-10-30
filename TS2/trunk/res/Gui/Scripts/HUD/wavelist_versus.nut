// Versus wave list

// Requires
sigimport "gui/scripts/hud/wavelist_sp.nut"
sigimport "gui/scripts/controls/animatingcanvas.nut"

// Resources
sigimport "gui/textures/misc/wavelaunch_decoration_g.png"

sigexport function CanvasCreateWaveList( waveList ) 
{
	return VersusWaveList( waveList )
}

class VersusWaveListIcon extends AnimatingCanvas
{
	image = null // Gui.TexturedQuad
	width = null // number
	
	constructor( country, unitID, rightAligned )
	{
		::AnimatingCanvas.constructor( )
		
		width = 64
		local height = 50
		
		image = ::UnitIcon( unitID, country )
		image.SetPosition( ::Math.Vec3.Construct( ( rightAligned )? width / 2: -width / 2, height / 2, 0 ) )
		AddChild( image )
		
		SetAlpha( 0.0 )

		AddAction( AlphaTween( 0.0, 1.0, 0.3, EasingTransition.Sine, EasingEquation.In ) )
	}
}

class VersusWaveListIconContainer extends AnimatingCanvas
{
	// Data
	queue = null // FIFO queue
	width = null // number, height of the object
	rightAligned = null
	
	constructor( rightAligned_ )
	{
		::AnimatingCanvas.constructor( )
		queue = [ ]
		width = 0
		rightAligned = rightAligned_
	}
	
	function Reposition( )
	{
		width = 0
		for( local i = queue.len( ) - 1; i >= 0; --i )
		{
			queue[ i ].SetXPos( rightAligned? width: -width )
			width += queue[ i ].WorldRect.Width + 5
		}
	}
	
	function Push( icon )
	{
		queue.push( icon )
		AddChild( icon )
		Reposition( )
		icon.AddAction( XMotionTween( rightAligned? -300 : 300, 0, 0.5, EasingTransition.Cubic, EasingEquation.Out ) )
	}

	function Pop( )
	{
		if( queue.len( ) > 0 )
		{
			queue[ 0 ].AddAction( AlphaTween( 1.0, 0.0, 0.3, EasingTransition.Sine, EasingEquation.In, null, function( canvas ) {
				queue[ 0 ].DeleteSelf( )
				queue.remove( 0 )
				Reposition( )
				AddAction( XMotionTween( GetXPos( ), rightAligned? -width: width, 0.4, EasingTransition.Cubic, EasingEquation.Out ) )
			}.bindenv( this ) ) )
		}
	}
	
	function StartLaunching( )
	{
		if( queue.len( ) > 0 )
		{
			queue[ 0 ].AddAction( RgbPulse( ::Math.Vec3.Construct( 1.0, 1.0, 1.0 ), ::Math.Vec3.Construct( 0.0, 1.0, 0.0 ), 0.5 ) )
		}
	}
}

class VersusWaveListIconQueue extends AnimatingCanvas
{
	// Data
	icons = null // VersusWaveListIconContainer
	toPush = null // array of PlayerBigMessage objects
	pushTimer = null // number, timer before next push
	rightAligned = null
	
	constructor( rightAligned_ )
	{
		::Gui.CanvasFrame.constructor( )
		
		rightAligned = rightAligned_
		toPush = [ ]
		pushTimer = 0
		
		icons = VersusWaveListIconContainer( rightAligned )
		AddChild( icons )
	}
	
	function OnTick( dt )
	{
		::Gui.CanvasFrame.OnTick( dt )
		
		pushTimer -= dt
		if( pushTimer < 0 )
		{
			if( toPush.len( ) > 0 )
			{
				local icon = toPush[ 0 ]
				icons.Push( icon )
				icons.ClearActions( )
				icons.SetXPos( ( rightAligned )? -icons.width: icons.width )
				
				toPush.remove( 0 )
				pushTimer = 0.5
			}
		}
	}
	
	function Count( )
	{
		return icons.queue.len( )
	}
	
	function AddIcon( icon )
	{
		toPush.push( icon )
	}
	
	function PopIcon( )
	{
		icons.Pop( ) 
	}
	
	function Clear( )
	{
		PopIcon( )
	}
	
	function StartLaunching( )
	{
		icons.StartLaunching( )
	}
}

class VersusWaveList extends Gui.CanvasFrame
{
	waveList = null // C++ wavelist
	
	width = null // number
	background = null // Gui.TexturedQuad
	displayText = null // Gui.Text
	iconQueue = null // VersusWaveListIconQueue
	
	launching = null // bool, true if currently launching next wave
	tintPulse = null // number
	
	toAdd = null // array of icons to add
	addIconTimer = null // float, countdown timer for adding icons
	iconQueue = null // WaveListIconQueues
	static timeBetweenAdd = 0.51
	
	rightAligned = null
	
	constructor( wl )
	{
		::Gui.CanvasFrame.constructor( )
		
		waveList = wl
		tintPulse = 0
		rightAligned = (waveList.User.ViewportIndex == 0) && GameApp.GameMode.IsSplitScreen
		toAdd = [ ]
		addIconTimer = 0
				
		local vpRect = waveList.User.ComputeViewportSafeRect( )
		if( GameApp.GameMode.IsNet )
		{
			// My list
			if( !waveList.User.IsViewportVirtual )
			{
				rightAligned = true
				SetPosition( ::Math.Vec3.Construct( vpRect.Center.x - 10, vpRect.Top, 0.3 ) )
			}
			// Enemy list
			else 
			{
				rightAligned = false
				SetPosition( ::Math.Vec3.Construct( vpRect.Center.x + 10, vpRect.Top, 0.3 ) )
			}
		}
		else
		{
			SetPosition( ::Math.Vec3.Construct( rightAligned? vpRect.Right: vpRect.Left, vpRect.Top, 0.3 ) )
		}
		
		iconQueue = ::VersusWaveListIconQueue( rightAligned )
		iconQueue.SetPosition( ::Math.Vec3.Construct( 0.0, 0.0, 0.0 ) )
		AddChild( iconQueue )
		
		local height = 50
		local padding = 6
		width = 3 * 64 + 2 * padding
		
		// Create background image
		background = Gui.TexturedQuad( )
		background.SetTexture( "gui/textures/misc/wavelaunch_decoration_g.png" )
		background.SetPosition( ::Math.Vec3.Construct( rightAligned? -background.WorldRect.Width : 0, height + 2, 0.01 ) )
		AddChild( background )
		
		// and some text
		displayText = ::Gui.Text( )
		displayText.SetFontById( FONT_FIXED_SMALL )
		displayText.SetRgba( COLOR_CLEAN_WHITE )
		displayText.SetPosition( ::Math.Vec3.Construct( rightAligned? -displayText.Width: 0, height + padding, -0.01 ) )
		displayText.BakeCString( "" )
		AddChild( displayText )
		
		local control = ::ControllerButton( "gui/textures/gamepad/button_x_g.png", "Launc_Offensive_Wave", ControllerButtonAlignment.LEFT, FONT_SIMPLE_SMALL )
		control.SetPosition( ::Math.Vec3.Construct( rightAligned? -control.GetSize( ).Width: 0, displayText.GetPosition( ).y + displayText.Height + padding + 3, -0.01 ) )
		if( !waveList.User.IsViewportVirtual )
		{
			if( GameApp.GameMode.IsNet )
			{
				control.SetPosition( ::Math.Vec3.Construct( -(control.GetSize( ).Width * 0.5) - 10, displayText.GetPosition( ).y + displayText.Height + padding + 3, -0.01 ) )
			}
			AddChild( control )
		}

		IgnoreBoundsChange = 1
	}
	
	function OnTick( dt )
	{
		::Gui.CanvasFrame.OnTick( dt )
		
		// If there are items to add
		if( toAdd.len( ) > 0 )
		{
			if( addIconTimer < 0 )
			{
				addIconTimer = timeBetweenAdd
				iconQueue.AddIcon( toAdd[ 0 ] )
				toAdd.remove( 0 )
			}
			else
			{
				addIconTimer -= dt
			}
		}
	}
	
	function Setup( wavelist )
	{
	}
	
	function AddWaveIcon( wave, unitID )
	{
		toAdd.push( VersusWaveListIcon( wave.Country, unitID, rightAligned ) )
	}
	
	function NextWave( )
	{
		iconQueue.PopIcon( )
	}
	
	function Readying( )
	{
		displayText.BakeLocString( GameApp.LocString( "Readying" ), rightAligned? TEXT_ALIGN_RIGHT: TEXT_ALIGN_LEFT )
		displayText.SetRgba( COLOR_CLEAN_WHITE )
	}
	
	function CountdownTimer( time )
	{
		time = time.tointeger( )
		local minutes = ( time / 60 ).tointeger( )
		local seconds = time % 60
		local formatString = ( minutes < 10 ? "0" : "" ) + minutes + ":" + ( seconds < 10 ? "0" : "" ) + seconds
		
		displayText.BakeCString( formatString, rightAligned? TEXT_ALIGN_RIGHT: TEXT_ALIGN_LEFT )
		displayText.SetRgba( COLOR_CLEAN_WHITE )
	}
	
	function LaunchStart( )
	{
		displayText.BakeLocString( GameApp.LocString( "Launching" ), rightAligned? TEXT_ALIGN_RIGHT: TEXT_ALIGN_LEFT )
		tintPulse = MATH_PI_OVER_8
		iconQueue.StartLaunching( )
	}
	
	function Launching( dt )
	{
		tintPulse += dt
		local tint = ( Math.Sin( tintPulse * 4 ) + 1 ) * 0.5
		local color = Math.Vec4.Construct( 1, tint, tint, 1 )
		displayText.SetRgba( color )
		
		// TODO: Color next wave icon
	}
	
	function Show( show )
	{
		SetAlpha( show ? 1 : 0 )
	}
	
	function Looping( loop )
	{
	}
	
	function Clear( )
	{
		displayText.SetAlpha( 0 )
		iconQueue.Clear( )
	}
}