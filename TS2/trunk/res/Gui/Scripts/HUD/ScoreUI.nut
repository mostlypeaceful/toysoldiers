// Main informational HUD

// Requires
sigimport "gui/scripts/controls/animatingcanvas.nut"
sigimport "Gui/Scripts/Controls/ControllerButton.nut"
sigimport "gui/scripts/hud/multistagemessage.nut"
sigimport "gui/scripts/hud/readysetgocounter.nut"
sigimport "gui/scripts/hud/tutorialpresenter.nut"
sigimport "gui/scripts/hud/flaghealthbar.nut"

// Resources
sigimport "gui/textures/misc/hud_decoration_g.png"
sigimport "gui/textures/score/score_decoration_g.png"
sigimport "gui/textures/score/warningexclamation_i.png"

sigvars Slot Machine
@[SPIN_TIME] { "Spin Time", 6.0, [ 0.1:10.0 ], "Max time to spin" }
@[SPIN_RATE_START] { "Spin Rate Start", 0.05, [ 0.0:10.0 ], "Time between changes at the start of the spin" }
@[SPIN_RATE_END] { "Spin Rate End", 1.0, [ 0.0:10.0 ], "Time between changes at the end of the spin" }

sigexport function CanvasCreateScoreUI( scoreUI )
{
	return ScoreUI( scoreUI )
}

class TicketsDisplay extends AnimatingCanvas
{
	// Display 
	scoreText = null
	healthBar = null // Gui.TexturedQuad
	textAlignment = null
	rightAligned = null
	
	constructor( country, rightAligned_ )
	{
		::AnimatingCanvas.constructor( )
		
		rightAligned = rightAligned_
		textAlignment = rightAligned? TEXT_ALIGN_RIGHT: TEXT_ALIGN_LEFT
		
		scoreText = ::Gui.Text( )
		scoreText.SetFontById( FONT_FANCY_MED )
		scoreText.SetPosition( ::Math.Vec3.Construct( 0, 0, 0 ) )
		scoreText.BakeCString( ( 0 ).tostring( ), textAlignment )
		AddChild( scoreText )
		
		local width = 256
		
		healthBar = ::FlagHealthBar( 1.0, country )
		local flagY = 5
		if( rightAligned )
			healthBar.SetPosition( -width, flagY, 0 )
		else
			healthBar.SetPosition( width - healthBar.WorldRect.Width, flagY, 0 )
		AddChild( healthBar )
	}
	
	function SetScore( score, percent )
	{
		local clamped = score
		if( !::GameApp.AAACantLose ) clamped = ::Math.Max( score, 0 )
		scoreText.BakeCString( clamped.tostring( ), textAlignment )
		
		// Set flag
		healthBar.SetPercent( percent )
	}
}

class MoneyDisplay extends AnimatingCanvas
{
	// Display
	moneyText = null
	textAlignment = null
	
	constructor( rightAligned )
	{
		::AnimatingCanvas.constructor( )
		
		textAlignment = rightAligned? TEXT_ALIGN_RIGHT: TEXT_ALIGN_LEFT
		
		moneyText = ::Gui.Text( )
		moneyText.SetFontById( FONT_FANCY_MED )
		moneyText.SetPosition( ::Math.Vec3.Construct( 0, 0, 0 ) )
		moneyText.SetScale( Math.Vec2.Construct( 0.6, 0.6 ) ) 
		local moneyString = LocString.ConstructMoneyString( "0" )
		moneyText.BakeLocString( moneyString, textAlignment )
		AddChild( moneyText )
	}
	
	function SetMoney( money )
	{
		local moneyString = ::LocString.ConstructMoneyString( money.tostring( ) )
		moneyText.BakeLocString( moneyString, textAlignment )
	}
}

class ScoreUI extends Gui.CanvasFrame
{
	// Data
	scoreUIobj = null
	
	// Display
	scoreBG = null
	profileText = null
	protectToyBox = null
	protectToyBoxExclamation = null
	anim = null
	ticketsDisplay = null
	moneyDisplay = null
	
	textAlignment = null
	rightAligned = null
	exclamationStart = null
	exclamationEnd = null
	toyboxDanger = null

	constructor( _scoreUIobj )
	{
		::Gui.CanvasFrame.constructor( )

		scoreUIobj = _scoreUIobj
		toyboxDanger = 0
		
		// Hackish way of finding if using the second viewport and coop
		rightAligned = (scoreUIobj.User.ViewportIndex >= 1) && ( ::GameApp.GameMode.IsCoOp || ::GameApp.GameMode.IsVersus ) && ::GameApp.GameMode.IsSplitScreen
		if( scoreUIobj.User.IsViewportVirtual )
			rightAligned = true
		
		local safeRect = scoreUIobj.User.ComputeViewportSafeRect( );
		local offset = Math.Vec3.Construct( 0, 0, 0 )
		textAlignment = rightAligned? TEXT_ALIGN_RIGHT: TEXT_ALIGN_LEFT
		
		// If we are not right aligned, make some pretty lines
		if( !rightAligned )
		{
			local line1 = ::Gui.TexturedQuad( )
			line1.SetTexture( "gui/textures/misc/hud_decoration_g.png" )
			line1.SetPosition( 0, -4, 0.5 )
			AddChild( line1 )
			
			local line2 = ::Gui.TexturedQuad( )
			line2.SetTexture( "gui/textures/misc/hud_decoration_g.png" )
			line2.SetPosition( 0, safeRect.Height + 2, 0.5 )
			AddChild( line2 )
		}

		ticketsDisplay = ::TicketsDisplay( scoreUIobj.Player.Country, rightAligned )
		ticketsDisplay.SetPosition( offset )
		if( !rightAligned )
			::TutorialPresenter.RegisterCanvas( ticketsDisplay, "score", ticketsDisplay.GetPosition( ), this, "Play_Presenter_Placement_Flag" )
		AddChild( ticketsDisplay )
		
		moneyDisplay = ::MoneyDisplay( rightAligned )
		moneyDisplay.SetPosition( offset + ::Math.Vec3.Construct( 0, ticketsDisplay.GetPosition( ).y + ticketsDisplay.scoreText.WorldRect.Height, 0 ) )
		::TutorialPresenter.RegisterCanvas( moneyDisplay, "money", moneyDisplay.GetPosition( ), this, "Play_Presenter_Placement_Money" )
		AddChild( moneyDisplay )

		profileText = ::Gui.Text( )
		profileText.SetFontById( FONT_SIMPLE_SMALL )
		profileText.BakeLocString( scoreUIobj.User.GamerTag, textAlignment )
		AddChild( profileText )
		
		local availableSpace = 152
		if( profileText.Width != 0 && profileText.Width > availableSpace )
			profileText.SetScale( availableSpace / profileText.Width, 1.0 )
		
		profileText.SetPosition( offset + ::Math.Vec3.Construct( 0, moneyDisplay.GetPosition( ).y + moneyDisplay.WorldRect.Height, 0 ) )
		
		// Pretty line underneath
		local smallLine = ::Gui.TexturedQuad( )
		smallLine.SetTexture( "gui/textures/score/score_decoration_g.png" )
		smallLine.SetPosition( 0, profileText.GetPosition( ).y + profileText.LineHeight + 6, 0.2 )
		if( rightAligned )
			smallLine.SetPosition( -smallLine.WorldRect.Width, profileText.GetPosition( ).y + profileText.LineHeight + 6, 0.2 )
		AddChild( smallLine )
		
		// Network fix
		if( ::GameApp.GameMode.IsCoOp && scoreUIobj.User.IsViewportVirtual )
		{
			RemoveChild( ticketsDisplay )
			RemoveChild( moneyDisplay )
			profileText.SetPosition( offset )
			RemoveChild( smallLine )
		}
		
		if( ::GameApp.OneManArmy )
			RemoveChild( moneyDisplay )

		protectToyBox = ::FlashingText( "Protect_Toy_Box", 4.0, 0.8, TEXT_ALIGN_CENTER )
		protectToyBox.SetRgb( 1, 0, 0 )
		protectToyBox.SetPosition( ( rightAligned ? -1 : 1 ) * safeRect.Width / 2.0, safeRect.Height / 2.0 + 100, 0 )
		protectToyBox.SetAlpha( 0 )
		AddChild( protectToyBox )
		
		if( !rightAligned )
		{
			exclamationStart = ::Math.Vec3.Construct( safeRect.Width / 2.0, safeRect.Height / 2.0, 0 )
			exclamationEnd = ::Math.Vec3.Construct( 256 + 20, 32, 0 )
		}
		else
		{
			exclamationStart = ::Math.Vec3.Construct( -safeRect.Width / 2.0, safeRect.Height / 2.0, 0 )
			exclamationEnd = ::Math.Vec3.Construct( -( 256 + 20 ), 32, 0 )
		}
		
		protectToyBoxExclamation = ::AnimatingCanvas( )
			local exclamationImage = ::Gui.TexturedQuad( )
			exclamationImage.SetTexture( "gui/textures/score/warningexclamation_i.png" )
			exclamationImage.CenterPivot( )
			protectToyBoxExclamation.AddChild( exclamationImage )
		protectToyBoxExclamation.SetPosition( exclamationStart )
		protectToyBoxExclamation.SetAlpha( 0 )
		protectToyBoxExclamation.SetUniformScale( 0.25 )
		AddChild( protectToyBoxExclamation )
		
		SetScore( 20, 1.0 )
		
		SetPosition( rightAligned ? safeRect.Right : safeRect.Left, safeRect.Top, 0.25 )
		
		IgnoreBoundsChange = 1
	}
	function OnTick( dt )
	{
		local prevDanger = toyboxDanger
		if( toyboxDanger > 0 )
			toyboxDanger -= dt
			
		if( toyboxDanger <= 0 && prevDanger > 0 )
		{
			// Hide Exclamation
			protectToyBoxExclamation.ClearActions( )
			protectToyBoxExclamation.AddAction( ::AlphaTween( protectToyBoxExclamation.GetAlpha( ), 0, 0.5 ) )
		}
			
		::Gui.CanvasFrame.OnTick( dt )
	}
	function SetScore( score, percent )
	{
		ticketsDisplay.SetScore( score, percent )
	}
	function SetMoney( money )
	{
		if( ::GameApp.GameMode.IsCoOp && scoreUIobj.User.IsViewportVirtual )
		{
			if( ::GameApp.OneManArmy )
				profileText.BakeLocString( scoreUIobj.User.GamerTag, textAlignment )
			else
				profileText.BakeLocString( ::LocString.ConstructMoneyString( money.tointeger( ).tostring( ) ) % " - " % scoreUIobj.User.GamerTag, textAlignment )
		}
		else
			moneyDisplay.SetMoney( money )
	}
	function Show( show )
	{
		SetAlpha( ( show )? 1.0 : 0.0 )
	}
	function ProtectToyBox( )
	{
		if( toyboxDanger <= 0 )
		{
			protectToyBox.ShowCount( 2 )
			
			::GameApp.AudioEvent( "Play_UI_ProtectToyBox" )

			// Show exclamation
			protectToyBoxExclamation.ClearActions( )
			protectToyBoxExclamation.SetPosition( exclamationStart )
			protectToyBoxExclamation.AddAction( ::AlphaPulse( 0.5, 1.3, 0.5 ) )
			protectToyBoxExclamation.AddAction( ::UniformScaleTween( 0.1, 0.7, 0.8 ) )
			protectToyBoxExclamation.AddDelayedAction( 2.0, ::MotionTween( exclamationStart, exclamationEnd, 0.6 ) )
			protectToyBoxExclamation.AddDelayedAction( 2.0, ::UniformScaleTween(0.7, 0.25, 0.6 ) )
		}
		
		toyboxDanger = 4.5
	}
	
	function ShowMoney( show )
	{
		moneyDisplay.ClearActions( )
		moneyDisplay.AddAction( AlphaTween( moneyDisplay.GetAlpha( ), show? 1.0: 0.0, 0.5 ) )
	}
	function ShowTickets( show )
	{
		ticketsDisplay.ClearActions( )
		ticketsDisplay.AddAction( AlphaTween( ticketsDisplay.GetAlpha( ), show? 1.0: 0.0, 0.5 ) )
	}
	
	// TEST // TODO: Remove ////////////////////////////////////////////////////
	function Test( action, button, player, held )
	{
		if( !held ) // Pressed
		{
			switch( button )
			{
				case GAMEPAD_BUTTON_A:
				break
				
				case GAMEPAD_BUTTON_B:
				break
				
				case GAMEPAD_BUTTON_X:
				break
				
				case GAMEPAD_BUTTON_Y:
				break
				
				case GAMEPAD_BUTTON_RSHOULDER:
				break
				
				case GAMEPAD_BUTTON_LSHOULDER:
				break
			}
		}
		else // Held
		{
			switch( button )
			{
				case GAMEPAD_BUTTON_A:
					::GameApp.CurrentLevel.IncRankProgress( player, 1 )
				break
				
				case GAMEPAD_BUTTON_B:
					::GameApp.CurrentLevel.IncRankProgress( player, 10 )
				break
				
				case GAMEPAD_BUTTON_X:
				break
				
				case GAMEPAD_BUTTON_Y:
				break
				
				case GAMEPAD_BUTTON_RSHOULDER:
				break
				
				case GAMEPAD_BUTTON_LSHOULDER:
				break
			}
		}
	}
}

