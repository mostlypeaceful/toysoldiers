// WaveList Display

// Requires
sigimport "gui/scripts/controls/animatingcanvas.nut"
sigimport "levels/scripts/common/challengescripts.nut"
sigimport "gui/scripts/controls/textchoice.nut"

sigimport "gui/textures/waveicons/blank_g.png"
sigimport "gui/textures/waveicons/blank_usa_g.png"
sigimport "gui/textures/frontend/arrow_left_g.png"
sigimport "gui/textures/frontend/arrow_right_g.png"

class UnitIcon extends Gui.CanvasFrame
{
	unitID = null
	image = null
	removeIconTimer = -1

	constructor( _unitID, country )
	{
		::Gui.CanvasFrame.constructor( )

		unitID = _unitID
		removeIconTimer = -1

		image = ::Gui.TexturedQuad( )
		image.SetTexture( ::GameApp.UnitWaveIconPath( unitID, country ) )
		
		AddChild( image )

		CenterPivot( )
		SetPosition( Math.Vec3.Construct( 0, 0, -0.001 ) )
	}
	
	function Remove( )
	{
		removeIconTimer = 0.5
	}
	
	function Step( dt )
	{
		if( removeIconTimer > 0 )
		{
			removeIconTimer -= dt

			local pos = GetPosition( )
			pos.y += ( 100 * dt ).tointeger( )
			SetPosition( pos )
			SetAlpha( removeIconTimer * 2 )

			if( removeIconTimer < 0 )
				return true //return true to remove
		}
		
		return false
	}
}

class WaveIcon extends AnimatingCanvas
{
	background = null
	images = null
	waveID = 0
	loopNumber = 0
	
	constructor( _waveID )
	{
		::AnimatingCanvas.constructor( )

		images = [ ]
		waveID = _waveID
		loopNumber = 0

		background = ::Gui.TexturedQuad( )
		background.SetTexture( "gui/textures/waveicons/blank_g.png" )
		background.SetPosition( 0, 0, 0.006 )
		AddChild( background )

		CenterPivot( )
		SetZPos( 0.005 )
	}
	
	function OnTick( dt )
	{
		::AnimatingCanvas.OnTick( dt )
		
		for( local i = 0; i < images.len( ); i += 1 )
		{
			if( images[ i ].Step( dt ) )
			{
				images[ i ].DeleteSelf( )
				images.remove( i )
				PositionUnitIcons( )
				i -= 1
			}
		}
	}
	
	function AddUnitIcon( unitID, country )
	{
		// Don't double up on icons
		foreach( icon in images )
			if( icon.unitID == unitID )
				return
		
		background.SetTexture( country == COUNTRY_USA ? "gui/textures/waveicons/blank_usa_g.png" : "gui/textures/waveicons/blank_g.png" )
		
		images.push( ::UnitIcon( unitID, country ) )
		AddChild( images.top( ) )
		PositionUnitIcons( )
	}
	
	function RemoveUnitIcon( unitID )
	{
		for( local i = 0; i < images.len( ); i += 1 )
		{
			if( images[ i ].unitID == unitID )
				images[ i ].Remove( )
		}
	}
	
	function PositionUnitIcons( )
	{	
		if( images.len( ) == 0 )
		{
			background.SetAlpha( 0 )
			return
 		}

		local dims = images[ 0 ].image.TextureDimensions( )
		local pos = Math.Vec3.Construct( 0, 0, 0 )
		local width = dims.x * 0.25
		
		pos.x -= width * ( images.len( ) - 1 ) * 0.5
		for( local i = 0; i < images.len( ); i += 1 )
		{
			images[ i ].SetPosition( pos )
			pos.x += width
			pos.z += 0.0001
		}
	}
}

class WaveListDisplay extends AnimatingCanvas
{
	// Display
	icons = null // array of WaveIcon objects
	textChoice = null // TextChoice
	
	// Data
	wavelistObj = null // tWaveList*
	count = null // number, number of waves
	highest = null
	static iconWidth = 82
	static visibleIconCount = 7 // number of icons visible at any time
	static centerIconIndex = 3 // icon that is in the center
	offsets = null // array of floats, x-offsets of visible icons
	alphas = null // array of floats, alpha values of visible icons
	scales = null // array of floats, scales of visible icons
	currentWaveIndex = null // integer, index of the current wave
	iconStrings = null // Array of LocStrings
	previousLoops = null // integer
	
	constructor( wavelist, startingIndex = 0, highestIndex = -1, showText = false, showArrows = false )
	{
		::AnimatingCanvas.constructor( )
		IgnoreBoundsChange = 1
		
		wavelistObj = wavelist
		icons = [ ]
		iconStrings = [ ]
		offsets = [ iconWidth * -2.0, iconWidth * -1.4, iconWidth * -0.8, iconWidth *  0, iconWidth *  0.8, iconWidth *  1.4, iconWidth *  2.0 ]
		alphas = [ 0.0, 0.25, 0.5, 1.0, 0.5, 0.25, 0.0 ]
		scales = [ 0.25, 0.5, 0.75, 1.0, 0.75, 0.5, 0.25 ]
		highest = highestIndex
		count = 0
		previousLoops = 0
		
		SetupIcons( wavelist, startingIndex )
		currentWaveIndex = startingIndex
		GoToWave( currentWaveIndex )
		
		local textHeight = 50
		local textChoiceWidth = ( ::GameApp.IsAsianLanguage( ) ? 200 : 100 )
		
		// Setup text
		if( showText )
		{
			if( count == 0 )
				textChoice = ::TextChoice( [ "Rewind_No_Waves" ], currentWaveIndex, highest, showArrows, textChoiceWidth )
			else
				textChoice = ::TextChoice( iconStrings, currentWaveIndex, highest, showArrows, textChoiceWidth )

			textChoice.SetPosition( 0, textHeight, 0 )
			AddChild( textChoice )
		}
	}
	
	function Clear( )
	{
		foreach( icon in icons )
			icon.DeleteSelf( )
		icons.clear( )
		iconStrings.clear( )
		currentWaveIndex = 0
		previousLoops = 0
		count = 0
	}
	
	function SetupIcons( wavelist, startingIndex = 0 )
	{
		wavelistObj = wavelist
		if( !is_null( wavelist ) && wavelist.WaveCount > 0 )
		{
			Clear( )
			currentWaveIndex = startingIndex
			local loopCount = ::Math.RoundUp( wavelist.TotalWaveIndex( ).tofloat( ) / wavelist.WaveCount.tofloat( ) )
			//::print( "TotalWaveIndex: " + wavelist.TotalWaveIndex( ).tostring( ) + " WaveCount:" + wavelist.WaveCount.tostring( ) + " loopCount: " + loopCount.tostring( ) )
			AddIcons( wavelist, wavelist.TotalWaveIndex( ) == 0 ? 1 : loopCount.tointeger( ) )
		}
	}
	
	function AddIcons( wavelist = null, loopCount = 1 )
	{
		if( wavelist == null )
			wavelist = wavelistObj
		
		local waveCount = wavelist.WaveCount
		
		// Set up initial icons
		for( local loop = 0; loop < loopCount; ++loop )
		{
			for( local i = 0; i < waveCount; ++i )
			{
				local wave = wavelist.Wave( i )
				AddWaveIcon( wave, loop + previousLoops )
			}
		}
		
		previousLoops += loopCount
		count = icons.len( )
	}
	
	function CurrentWaveIndex( )
	{
		return currentWaveIndex
	}
	
	function WaveCount( )
	{
		return count
	}
	
	function SetHighest( index )
	{
		highest = index
		foreach( i in icons )
		{
			if( i > highest || highest < 0 )
				icon.SetRgb( ::Math.Vec3.Construct( 0.3, 0.3, 0.3 ) )
			else
				icon.SetRgb( ::Math.Vec3.Construct( 1.0, 1.0, 1.0 ) )
		}
	}
	
	function AddSurvivalWaveIcon( wave, loop )
	{
	}
	
	function AddWaveIcon( wave, loop )
	{
		if( wave.IsFunction( ) )
			return 
			
		// Try to find a wave icon that already exists
		for( local i = 0; i < icons.len( ); ++i )
		{
			if( icons[ i ].waveID == wave.WaveID( ) && icons[ i ].loopNumber == loop )
			{
				AddUnitsToWave( icons[ i ], wave )
				return
			}
		}
		
		local icon = ::WaveIcon( wave.WaveID( ) )
		icon.loopNumber = loop
		AddUnitsToWave( icon, wave )
		icons.push( icon )
		count = icons.len( )
		
		local i = icons.len( ) - 1
		
		if( wave.WaveID( ) == 0 )
			iconStrings.push( ::GameApp.LocString( "Restart" ) )
		else
			iconStrings.push( ::WaveLocString( i + 1 ) )
		
		if( i > highest )
			icon.SetRgb( ::Math.Vec3.Construct( 0.3, 0.3, 0.3 ) )
		if( highest < 0 )
			icon.SetRgb( ::Math.Vec3.Construct( 1.0, 1.0, 1.0 ) )
		
		local visibleIndex = VisibleIndex( i )
		if( IsIndexVisible( visibleIndex ) )
		{
			icon.SetPosition( offsets[ visibleIndex ], 0, 0 )
			icon.SetAlpha( alphas[ visibleIndex ] )
			icon.SetScale( ::Math.Vec2.Construct( scales[ visibleIndex ], scales[ visibleIndex ] ) )
		}
		else
		{
			icon.SetPosition( 0, 0, 0 )
			icon.SetAlpha( 0 )
			icon.SetScale( ::Math.Vec2.Construct( 0.25, 0.25 ) )
		}
		
		AddChild( icon )
	}
	
	function AddUnitsToWave( icon, wave )
	{
		for( local i = 0; i < wave.UnitDescCount( ); ++i )
		{
			icon.AddUnitIcon( wave.UIUnitID( i ), wave.Country( i ) )
		}
	}
	
	function VisibleIndex( index )
	{
		return centerIconIndex - currentWaveIndex + index
	}
	
	function IsIndexValid( index )
	{
		if( highest < 0 )
			return ( index < count && index >= 0 )
		else
			return ( index < count && index >= 0 && index <= highest )
	}
	
	function IsIndexVisible( index )
	{
		return ( index < visibleIconCount && index >= 0 )
	}
	
	function NextWave( )
	{
		ShiftIcons( 1 )
	}
	
	function PreviousWave( )
	{
		ShiftIcons( -1 )
	}
	
	function ShiftIcons( offset )
	{
		GoToWave( currentWaveIndex + offset )
	}
	
	function GoToWave( index )
	{
		local offset = index - currentWaveIndex;
		if( IsIndexValid( index ) && index != currentWaveIndex )
			currentWaveIndex = index
		else
			return
		
		// Move icons
		foreach( i, icon in icons )
		{
			icon.ClearActions( )
			
			local nextIndex = VisibleIndex( i )
			if( !IsIndexVisible( nextIndex ) )
			{
				icon.SetAlpha( 0 )
				continue
			}
			
			local currentState = { x = icon.GetXPos( ), a = icon.GetAlpha( ), s = icon.GetScale( ).x }
			local nextState = { x = offsets[ nextIndex ], a = alphas[ nextIndex ], s = scales[ nextIndex ] }
			icon.AddAction( ::XMotionTween( currentState.x, nextState.x, 0.4 ) )
			icon.AddAction( ::AlphaTween( currentState.a, nextState.a, 0.4 ) )
			icon.AddAction( ::UniformScaleTween( currentState.s, nextState.s, 0.4 ) )
		}
		
		// Change Text
		UpdateText( currentWaveIndex )
	}

	function UpdateText( index )
	{
		if( textChoice )
			textChoice.GoToIndex( index )
	}
}