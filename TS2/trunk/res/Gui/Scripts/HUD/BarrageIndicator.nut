// Indicator for which barrage is currently active

// Requires
sigimport "gui/scripts/controls/animatingcanvas.nut"
sigimport "gui/scripts/controls/radialprogressmeter.nut"

// Resources
sigimport "gui/textures/barrage/barrage_icon_unknown_g.png"
sigimport "gui/textures/gamepad/button_y_g.png"
sigimport "gui/textures/barrage/barrage_ac130_g.png"
sigimport "gui/textures/barrage/barrage_artillery_g.png"
sigimport "gui/textures/barrage/barrage_b52_g.png"
sigimport "gui/textures/barrage/barrage_nuke_g.png"
sigimport "gui/textures/barrage/barrage_rambo_g.png"
sigimport "gui/textures/barrage/barrage_ivan_g.png"
sigimport "gui/textures/barrage/barrage_countdown_overlay_g.png"

sigexport function CanvasCreateBarrageIndicator( cppObj )
{
	return BarrageIndicator( cppObj.User )
}

class SlotMotionTween extends TweenerWithSound
{
	prev = null
	tickSound = false
	
	function OnTick( dt, canvas )
	{
		local z = Math.Wrap( easer.Evaluate( timer ), 0.01, 0.001 )
		if( tickSound && prev != null && prev > z )
			PlaySound( "Play_HUD_Barrage_Scroll" )
		canvas.SetZPos( z )
		prev = z
	}
}

class SlotMachineSlot extends AnimatingCanvas
{
	// Display
	icons = null // array of icons
	iconPaths = null // array of filepaths

	constructor( country, player )
	{
		::AnimatingCanvas.constructor( )
		
		icons = [ ]
		iconPaths = [
			{ name = "BARRAGE_B52", path = "gui/textures/barrage/barrage_b52_g.png", dlc = DLC_COLD_WAR },
			{ name = "BARRAGE_AC130", path = "gui/textures/barrage/barrage_ac130_g.png", dlc = DLC_COLD_WAR },
			{ name = "BARRAGE_ARTILLERY", path = "gui/textures/barrage/barrage_artillery_g.png", dlc = DLC_COLD_WAR },
			{ name = "BARRAGE_NUKE", path = "gui/textures/barrage/barrage_nuke_g.png", dlc = DLC_COLD_WAR },
			{ name = "BARRAGE_RAMBO", path = ( ( country == COUNTRY_USSR )? "gui/textures/barrage/barrage_ivan_g.png": "gui/textures/barrage/barrage_rambo_g.png" ), dlc = DLC_COLD_WAR },
			{ name = "BARRAGE_LAZER", path = "gui/textures/barrage/barrage_lazer_g.png", dlc = DLC_EVIL_EMPIRE },
			{ name = "BARRAGE_NAPALM", path = "gui/textures/barrage/barrage_napalm_g.png", dlc = DLC_NAPALM },
		]
		foreach( iconPath in iconPaths )
		{
			if( "dlc" in iconPath && player && player.HasDLC( iconPath.dlc ) )
			{
				local icon = ::AnimatingCanvas( )
					local iconImage = ::Gui.TexturedQuad( )
					iconImage.SetTexture( iconPath.path )
					icon.AddChild( iconImage )
				AddChild( icon )
				icons.push( icon )
			}
		}
	}
	
	function Start( barrage, spinTime, skipInto )
	{
		// Clear
		ClearActions( )
		foreach( icon in icons )
		{
			icon.ClearActions( )
		}
		
		// Reset icons
		local visibleIconIndex = 0
		local newIconIndex = visibleIconIndex
		foreach( i, path in iconPaths )
		{
			if( path.name == barrage.Name )
			{
				newIconIndex = i
				break
			}
		}

		if( newIconIndex != visibleIconIndex )
		{
			local swapIconPath = iconPaths[ visibleIconIndex ]
			iconPaths[ visibleIconIndex ] = { name = barrage.Name, path = barrage.IconPath }
			iconPaths[ newIconIndex ] = swapIconPath
			
			local swapIcon = icons[ visibleIconIndex ]
			icons[ visibleIconIndex ] = icons[ newIconIndex ]
			icons[ newIconIndex ] = swapIcon
		}
		
		// Start icons spinning
		local start = 0.01
		local spacing = start / icons.len( )
		local totalDistance = 10 * spacing * icons.len( )
		foreach( i, icon in icons )
		{
			local iconStart = start + i * spacing
			local callback = skipInto ? null : function( canvas ) { PlaySound( "Play_HUD_Barrage_Select" ) }
			local tween = ::SlotMotionTween( iconStart, iconStart + totalDistance, spinTime, EasingTransition.Quadratic, EasingEquation.Out, null, callback )
			tween.audioSource = audioSource
			tween.tickSound = !skipInto
			icon.AddAction( tween )
		}
	}
}

class BarrageIndicator extends AnimatingCanvas
{
	// Display
	icon = null // Gui.TexturedQuad
	button = null // Gui.TexturedQuad, the button to press for a barrage
	nameText = null // Gui.Text
	descText = null // Gui.Text
	alignment = null // bool, true for left, false for right
	hitOneHundred = null // bool
	countdownTimer = null // RadialProgressMeter
	slotMachine = null // SlotMachineSlot
	readyText = null // gui.text
	
	// Data
	activeBarrage = null // Barrage
	user = null
	startPosition = null
	endPosition = null
	
	constructor( user_ )
	{
		::AnimatingCanvas.constructor( )
		
		user = user_
		alignment = (user.ViewportIndex == 0)
		hitOneHundred = false
		
		// Create the icon
		local iconCanvas = ::AnimatingCanvas( )
			icon = ::Gui.TexturedQuad( )
			icon.SetTexture( "gui/textures/barrage/barrage_icon_unknown_g.png" )
			iconCanvas.AddChild( icon )
		iconCanvas.SetPosition( alignment? 0: -64, 0, 0 )
		iconCanvas.AddAction( ::RgbaPulse( ::Math.Vec4.Construct( 0.8, 0.8, 0.8, 0.7 ), ::Math.Vec4.Construct( 1.5, 1.5, 1.5, 1.0 ), 0.5 ) )
		AddChild( iconCanvas )
		
		// Create the slot machine
		local player = ::GameApp.GetPlayer( ::GameApp.WhichPlayer( user ) )
		slotMachine = ::SlotMachineSlot( player.Country, player )
		slotMachine.audioSource = player.AudioSource
		slotMachine.SetPosition( iconCanvas.GetXPos( ), 0, 0 )
		AddChild( slotMachine )
		
		// Create the button
		if( !user.IsViewportVirtual )
		{
			button = ::ControllerButton( GAMEPAD_BUTTON_Y, "ActivateBarrage", alignment? ControllerButtonAlignment.LEFT: ControllerButtonAlignment.RIGHT, FONT_SIMPLE_SMALL )
			button.SetPosition( ::Math.Vec3.Construct( 3, icon.WorldRect.Height + 12, -0.01 ) )
			button.Invisible = true
			button.AddAction( ::RgbaPulse( ::Math.Vec4.Construct( 0.8, 0.8, 0.8, 0.7 ), ::Math.Vec4.Construct( 1.5, 1.5, 1.5, 1.0 ), 0.5 ) )
			AddChild( button )
		}
		
		// Create the text
		local space = 7
		local sign = alignment? 1: -1
		nameText = ::Gui.Text( )
		nameText.SetFontById( FONT_FANCY_MED )
		nameText.SetRgba( COLOR_CLEAN_WHITE )
		nameText.SetPosition( ::Math.Vec3.Construct( sign * (icon.WorldRect.Width + space), 3, 0 ) )
		nameText.BakeCString( "Barrage Ready!", alignment? TEXT_ALIGN_LEFT: TEXT_ALIGN_RIGHT )
		nameText.SetScale( ::Math.Vec2.Construct( 0.8, 0.8 ) )
		AddChild( nameText )
		
		descText = ::Gui.Text( )
		descText.SetFontById( FONT_SIMPLE_SMALL )
		descText.SetRgba( COLOR_CLEAN_WHITE )
		descText.SetPosition( ::Math.Vec3.Construct( sign * (icon.WorldRect.Width + space), 35, 0 ) )
		descText.BakeCString( "", alignment? TEXT_ALIGN_LEFT: TEXT_ALIGN_RIGHT )
		descText.SetScale( ::Math.Vec2.Construct( 0.7, 1.0 ) )
		AddChild( descText )
		
		// Barrage Ready Text
		readyText = ::Gui.Text( )
		readyText.SetFontById( FONT_FANCY_LARGE )
		readyText.SetRgba( COLOR_CLEAN_WHITE )
		readyText.SetPosition( ::Math.Vec3.Construct( sign * (icon.WorldRect.Width + space), 3, 0 ) )
		readyText.BakeLocString( ::GameApp.LocString( "BarrageReady" ), alignment? TEXT_ALIGN_LEFT: TEXT_ALIGN_RIGHT )
		
		local readyTextContainer = ::AnimatingCanvas( )
		readyTextContainer.AddChild( readyText )
		readyTextContainer.AddAction( ::RgbaPulse( ::Math.Vec4.Construct( 0.8, 0.8, 0.8, 0.7 ), ::Math.Vec4.Construct( 1.5, 1.5, 1.5, 1.0 ), 0.5 ) )
		AddChild( readyTextContainer )
		
		// Create the radial countdown timer
		countdownTimer = ::RadialProgressMeter( "gui/textures/barrage/barrage_countdown_overlay_g.png" )
		countdownTimer.SetPosition( iconCanvas.GetPosition( ) )
		countdownTimer.SetZPos( -0.01 )
		countdownTimer.SetAlpha( 0 )
		countdownTimer.SetPercent( 1.0 )
		countdownTimer.CenterPivot( )
		countdownTimer.SetAngle( -MATH_PI_OVER_2 )
		AddChild( countdownTimer )
		
		local safeRect = user.ComputeViewportSafeRect( )
		if( alignment )
			startPosition = ::Math.Vec3.Construct( safeRect.Center.x - LocalRect.Width * 0.5, safeRect.Center.y - LocalRect.Height * 0.5, 0.2 )
		else
			startPosition = ::Math.Vec3.Construct( safeRect.Center.x + LocalRect.Width * 0.5, safeRect.Center.y - LocalRect.Height * 0.5, 0.2 )
		
		SetAlpha( 0 )
		if( alignment )
			endPosition = ::Math.Vec3.Construct( safeRect.Left, safeRect.Top + 100, startPosition.z )
		else
			endPosition = ::Math.Vec3.Construct( safeRect.Right, safeRect.Top + 100, startPosition.z )
		SetPosition( startPosition )
		
		if( user.IsViewportVirtual )
		{
			if( ::GameApp.CurrentLevelLoadInfo.MapType != MAP_TYPE_HEADTOHEAD )
			{
				endPosition.y = safeRect.Top + 30
				startPosition.y = endPosition.y
				startPosition.x = safeRect.Right + 300
				local vpi = ( ( user.ViewportIndex == 0 )? 1: 0 ) // use the other player's viewport
				::GameApp.HudLayer( "viewport" + vpi.tostring( ) ).AddChild( this )
			}
		}
		else
			::GameApp.HudLayer( "alwaysShow" ).AddChild( this )
		
		//if( user.IsViewportVirtual && ::GameApp.CurrentLevelLoadInfo.MapType != MAP_TYPE_HEADTOHEAD )
		//	Invisible = true
	}
	
	function BarrageBegin( )
	{
		// Hide the Y button
		if( button )
			button.Invisible = true
				
		// Show the countdown timer
		countdownTimer.SetAlpha( 1 )
		countdownTimer.SetPercent( 1.0 )
	}
	
	function BarrageEnd( )
	{
		// Hide the meter
		FadeOut( )
		SetBarrageUsable( false )
		
		// Hide the countdownTimer
		countdownTimer.SetAlpha( 0 )
	}
	
	function StartSpinning( barrage, spinTime, skipInto )
	{
		activeBarrage = barrage
		
		FadeIn( )
		
		slotMachine.SetAlpha( 1 )
		slotMachine.Start( barrage, spinTime, skipInto )
		
		readyText.SetAlpha( 1 )
		
		icon.SetAlpha( 0 )
		nameText.SetAlpha( 0 )
		descText.SetAlpha( 0 )
		
		if( button )
		{
			button.SetText( "ActivateBarrage" )
			button.Invisible = true
		}
		
		// Tutorial present every time
		SetPosition( startPosition )
		AddDelayedAction( 1.0, ::MotionTween( startPosition, endPosition, 1.0, null, EasingEquation.InOut ) )
	}
	
	function SetAvailable( )
	{
		if( button )
			button.Invisible = false
		icon.SetTexture( activeBarrage.IconPath )
		
		nameText.BakeLocString( ::GameApp.LocString( activeBarrage.Name ), alignment? TEXT_ALIGN_LEFT: TEXT_ALIGN_RIGHT )
		descText.BakeLocString( ::GameApp.LocString( activeBarrage.Name + "_DESC" ), alignment? TEXT_ALIGN_LEFT: TEXT_ALIGN_RIGHT )
		nameText.SetAlpha( 1 )
		descText.SetAlpha( 1 )
		icon.SetAlpha( 1 )
		
		readyText.SetAlpha( 0 )
		slotMachine.SetAlpha( 0 )
		
		ClearActionsOfType( "MotionTween" )
		SetPosition( endPosition )
	}
	
	function SetBarrageUsable( isUsable )
	{
		if( button )
		{
			if( isUsable )
			{
				button.Invisible = false
				button.SetText( "UseBarrage" )
			}
			else
			{
				button.Invisible = true
				button.SetText( "ActivateBarrage" )
			}
		}
	}
	
	function UpdateTimer( percent )
	{
		if( percent == 0 )
			countdownTimer.SetAlpha( 0 )
		countdownTimer.SetPercent( percent )
	}
	
	function FadeOut( )
	{
		ClearActions( )
		AddAction( ::AlphaTween( GetAlpha( ), 0.0, 0.5, EasingTransition.Sine, EasingEquation.Out ) )
	}
	
	function IsSpinning( )
	{
		return spinning
	}
}