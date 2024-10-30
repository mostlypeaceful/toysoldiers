
// Requires
sigimport "gui/scripts/controls/animatingcanvas.nut"
sigimport "gui/scripts/controls/popupmessage.nut"
sigimport "gui/scripts/hud/wavelistdisplay.nut"

// Resources
sigimport "Gui/Textures/WaveIcons/blank_g.png"

sigexport function CanvasCreateWaveList( waveList )
{
	return SinglePlayerWaveList( waveList )
}

class FinalWaveText extends AnimatingCanvas
{
	constructor( start, end )
	{
		::AnimatingCanvas.constructor( )
		
		local text = ::Gui.Text( )
		text.SetFontById( FONT_FANCY_MED )
		text.SetRgba( 1, 0, 0, 1 )
		text.BakeLocString( ::GameApp.LocString( "Last_Wave" ), TEXT_ALIGN_CENTER )
		AddChild( text )
		
		SetAlpha( 0 )
		
		// Animate
		local fadeTime = 0.4
		local pauseTime = 3.0
		local flyTime = 0.6
		
		FadeIn( fadeTime )
		AddAction( ::UniformScaleTween( 0.7, 1.0, fadeTime ) )
		
		AddDelayedAction( pauseTime, ::YMotionTween( start, end, flyTime, EasingTransition.Quadratic, EasingEquation.InOut ) )
		AddDelayedAction( pauseTime, ::UniformScaleTween( 1.0, 0.6, flyTime, EasingTransition.Quadratic, EasingEquation.InOut ) )
	}
}

class SinglePlayerWaveList extends AnimatingCanvas
{
	// Display
	display = null 
	displayText = null
	survivalTimerText = null
	launchButton = null
	waveIndexText = null
	waveIndexCanvas = null
	
	// Data
	waveList = null // tSinglePlayerWaveList*
	tintPulse = 0
	showLaunchButton = false
	looping = null
	survival = false

	constructor( _waveList )
	{
		::AnimatingCanvas.constructor( )
		waveList = _waveList
		audioSource = ::GameApp.GetPlayerByUser( waveList.User ).AudioSource
		showLaunchButton = false
		looping = false
		survival = (::GameApp.CurrentLevelLoadInfo.MapType == MAP_TYPE_SURVIVAL)
		
		// Add display
		display = ::WaveListDisplay( null )
		display.audioSource = audioSource
		display.SetPosition( 0, 0, 0 )
		AddChild( display )
		
		// Set position based on the user
		local screenRect = waveList? waveList.User.ComputeScreenSafeRect( ): ::GameApp.FrontEndPlayer.User.ComputeScreenSafeRect( )
		local topOffset = 31 + 24
		SetPosition( Math.Vec3.Construct( screenRect.Center.x, screenRect.Top + topOffset, 0.2 ) )

		displayText = ::Gui.Text( )
		if( survival )
		{
			survivalTimerText = ::Gui.Text( )
			survivalTimerText.SetFontById( FONT_FIXED_SMALL )
			survivalTimerText.SetPosition( Math.Vec3.Construct( 0, 4, 0 ) )
			AddChild( survivalTimerText )
			UpdateSurvivalTimer( 0 )
			
			displayText.SetFontById( FONT_FANCY_MED )
			displayText.SetPosition( Math.Vec3.Construct( 0, -30, 0 ) )
			UpdateSurvivalText( 1 )
		}
		else
		{
			displayText.SetFontById( FONT_FIXED_SMALL )
			displayText.BakeLocString( ::GameApp.LocString( "Waiting" ), TEXT_ALIGN_CENTER )
			displayText.SetPosition( Math.Vec3.Construct( 0, 34, 0 ) )
			
			waveIndexCanvas = ::AnimatingCanvas( )
				waveIndexText = ::Gui.Text( )
				waveIndexText.SetFontById( FONT_FIXED_SMALL )
				waveIndexCanvas.AddChild( waveIndexText )
			waveIndexCanvas.SetPosition( Math.Vec3.Construct( 0, -64, 0 ) )
			waveIndexCanvas.SetAlpha( 0 )
			AddChild( waveIndexCanvas )
		}
		AddChild( displayText )

		launchButton = ControllerButton( "Gui/Textures/Gamepad/button_x_g.png", "Force_Launch", ControllerButtonAlignment.LEFT, FONT_SIMPLE_SMALL )
		launchButton.SetPosition( Math.Vec3.Construct( launchButton.LocalRect.Width * -0.5, displayText.GetPosition( ).y + displayText.LineHeight + 10, 0 ) )
		launchButton.SetAlpha( 0 )
		AddChild( launchButton )
		
		SetAlpha( 0 )
		::TutorialPresenter.RegisterCanvas( this, "wavelist", GetPosition( ), ::GameApp.HudLayer( _waveList.Layer ), "Play_Presenter_Placement_Wave" )
		IgnoreBoundsChange = 1

		if( survival )
		{
			display.DeleteSelf( )
			display = null
		}
	}
	
	function Setup( wavelist )
	{
		if( !display )
			return
			
		display.SetupIcons( wavelist, wavelist.CurrentUIWaveListID )
		display.highest = -1
		looping = wavelist.IsLooping( ) || ::GameApp.CurrentLevel.MapType == MAP_TYPE_SURVIVAL
		//SetWaveNumberText( )
	}
	
	function SetWaveNumberText( )
	{
		if( !display )
			return
			
		local waveIndex = display.CurrentWaveIndex( )
		local waveCount = display.WaveCount( )
		local str = ( waveIndex + 1 ).tostring( ) + ( ( looping )? "": ( " / " + waveCount.tostring( ) ) )

		waveIndexCanvas.ClearActions( )
		local delay = 0
		local showTime = 5.0
		if( waveIndexCanvas.GetAlpha( ) > 0.1 )
		{
			waveIndexCanvas.AddAction( ::AlphaTween( waveIndexCanvas.GetAlpha( ), 0.0, 0.2 ) )
			delay += 0.2
		}
		
		waveIndexCanvas.AddDelayedAction( delay, ::AlphaTween( waveIndexCanvas.GetAlpha( ), 1.0, 1.0, null, null,
			function( canvas ):(waveIndexText, str) { waveIndexText.BakeCString( str, TEXT_ALIGN_CENTER ) } ) )
		waveIndexCanvas.AddDelayedAction( delay + showTime, ::AlphaTween( 1.0, 0.0, 1.0 ) )
	}
	
	function AddWaveIcon( wave )
	{
	}
	
	function NextWave( )
	{
		if( display )
		{
			if( looping && display.currentWaveIndex + 3 >= display.count )
				display.AddIcons( )
			display.NextWave( )
			SetWaveNumberText( )
		}
		
		displayText.SetRgba( COLOR_CLEAN_WHITE )
	}
	
	function FinalEnemyWave( )
	{
		// Clear Text
		displayText.SetAlpha( 0 )
		
		// Show Final Wave Text
		local startY = 200
		local endY = 34
		local text = ::FinalWaveText( startY, endY )
		text.SetPosition( 0, startY, 0 )
		AddChild( text )
	}
	
	function UpdateSurvivalText( round )
	{
		displayText.BakeLocString( ::RoundLocString( round ), TEXT_ALIGN_CENTER )
	}
	
	function UpdateSurvivalTimer( time )
	{
		survivalTimerText.BakeLocString( ::LocString.ConstructTimeString( time, 0 ), TEXT_ALIGN_CENTER )
		survivalTimerText.SetRgba( COLOR_CLEAN_WHITE )
	}
	
	function Readying( )
	{
		if( !survival )
			displayText.BakeLocString( ::GameApp.LocString( "Readying" ), TEXT_ALIGN_CENTER )
	}
	
	function CountdownTimer( time )
	{
		if( !survival )
		{	
			if( showLaunchButton == false )
			{
				launchButton.ClearActions( )
				launchButton.FadeIn( 0.5 )
				showLaunchButton = true
				if( ::GameApp.CurrentLevelLoadInfo.MapType == MAP_TYPE_CAMPAIGN )
					PlaySound( "Play_HUD_Wave_LaunchReady" )
			}
			
			displayText.BakeLocString( ::LocString.ConstructTimeString( time, 0 ), TEXT_ALIGN_CENTER )
		}
			
		displayText.SetRgba( COLOR_CLEAN_WHITE )
	}
	
	function LaunchStart( )
	{
		if( !survival )
			displayText.BakeLocString( ::GameApp.LocString( "Launching" ), TEXT_ALIGN_CENTER )
		tintPulse = MATH_PI_OVER_8
		
		
		if( !survival )
		{	
			showLaunchButton = false
			launchButton.ClearActions( )
			launchButton.FadeOut( 0.5 )
		}
	}
	
	function Launching( dt )
	{
		tintPulse += dt
		local tint = ( ::Math.Sin( tintPulse * 4 ) + 1 ) * 0.5
		local color = ::Math.Vec4.Construct( 1, tint, tint, 1 )
		displayText.SetRgba( color )
	}
	
	function Show( show )
	{
		if( ::GameApp.CurrentLevel.MapType == MAP_TYPE_MINIGAME ) // HACK
			Invisible = !show
		else
		{
			ClearActions( )
			AddAction( ::AlphaTween( GetAlpha( ), show ? 1 : 0, 0.5, EasingTransition.Quadratic, EasingEquation.Out ) )
		}
	}
	
	function Looping( loop )
	{
		looping = loop
	}

	function Clear( )
	{
		if( display )
			display.Clear( )
		displayText.SetAlpha( 0 )
	}
}

