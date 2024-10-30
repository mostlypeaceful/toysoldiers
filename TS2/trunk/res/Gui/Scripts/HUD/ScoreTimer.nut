// High score / Timer for Trial

// Constants
enum TimerCountDirection
{
	Down = 0, 
	Up = 1
}

// Requires
sigimport "gui/scripts/controls/animatingcanvas.nut"
sigimport "gui/scripts/hud/clocktimer.nut"
sigimport "gui/scripts/hud/minileaderboard.nut"
sigimport "gui/scripts/endgamescreens/scoreutility.nut"
sigimport "gui/scripts/hud/currentplayerui.nut"

// Resources
sigimport "gui/textures/score/score_decoration_g.png"

// Helper Classes
class ScoreTimer_ScoreText extends AnimatingCanvas
{
	// Display
	text = null // Gui.Text
	
	// Data
	points = null
	mult = 1
	
	constructor( startingPoints_ = null )
	{
		::AnimatingCanvas.constructor( )
		mult = 1
		
		text = ::Gui.Text( )
		text.SetFontById( FONT_SIMPLE_SMALL )
		text.SetRgba( ::Math.Vec4.Construct( 1.0, 1.0, 1.0, 1.0 ) )
		AddChild( text )
		
		SetPoints( ( ( startingPoints_ == null )? 0: startingPoints_ ) )
		
		ShiftPivot( ::Math.Vec2.Construct( 0, text.Height * 0.5 ) )
	}
	
	function Score( )
	{
		return points * mult
	}
	
	function SetPoints( p )
	{
		points = p
		
		text.BakeCString( ::StringUtil.AddCommaEvery3Digits( Score( ).tointeger( ).tostring( ) ), TEXT_ALIGN_LEFT )
	}
	
	function AddPoints( p )
	{
		SetPoints( points + p )
		
		// Script Animation
		local end = ::Math.Vec3.Construct( 1.0, 1.0, 0.0 )
		local start = ::Math.Vec3.Construct( 1.0, 1.0, 1.0 )
		
		ClearActions( )
		AddAction( RgbTween( start, end, 0.2 ) )
		AddAction( UniformScaleTween( 1.0, 1.3, 0.2 ) )
		AddDelayedAction( 0.2, UniformScaleTween( 1.3, 1.0, 0.2 ) )
		AddDelayedAction( 0.2, RgbTween( end, start, 0.2 ) )
	}
	
	function AddBonus( multiplier )
	{
		mult *= multiplier
		SetPoints( points )
	}
	
	function MakeBig( )
	{
		text.SetFontById( FONT_FANCY_LARGE )
		if( !::GameApp.IsAsianLanguage( ) )
			text.SetYPos( -text.Height )
		SetPoints( points )
	}
}

class ScoreTimer_TimeLeftText extends AnimatingCanvas
{
	// Display
	text = null // Gui.Text
	
	// Data
	dangerOn = null
	
	constructor( )
	{
		::AnimatingCanvas.constructor( )
		
		dangerOn = false
		
		text = ::Gui.Text( )
		text.SetFontById( FONT_SIMPLE_SMALL )
		text.SetRgba( ::Math.Vec4.Construct( 1.0, 1.0, 1.0, 1.0 ) )
		AddChild( text )
		
		SetTime( 0 )
		
		ShiftPivot( ::Math.Vec2.Construct( 0, text.Height * 0.5 ) )
	}
	
	function SetTime( time )
	{
		if( time == null )
			return
			
		local minutes = ::Math.RoundDown( time / 60 )
		local seconds = time - (minutes * 60)
		
		local secondsString = ( ( seconds < 0 )? (-seconds).tostring( ): seconds.tostring( ) )
		if( seconds >= 10 && secondsString.len( ) >= 4 )
			secondsString = secondsString.slice( 0, 4 )
		else if( secondsString.len( ) >= 3 )
		{
			secondsString = secondsString.slice( 0, 3 )
		}
		if( seconds < 10 )
		{
			secondsString = "0" + secondsString
		}
		local timeString = minutes.tostring( ) + ":" + secondsString
		
		text.BakeCString( timeString )
	}
	
	function SetDanger( danger, dangerCB )
	{
		if( !dangerOn && danger )
		{
			if( dangerCB )
				dangerCB( )
				
			dangerOn = true
			
			local end = ::Math.Vec3.Construct( 1.0, 0.0, 0.0 )
			local start = ::Math.Vec3.Construct( 1.0, 1.0, 1.0 )
			AddAction( RgbPulse( start, end, 0.5 ) )
			AddAction( UniformScalePulse( 1.0, 1.3, 0.5 ) )
		}
		else if( dangerOn && !danger )
		{
			dangerOn = false
			ClearActions( )
		}
	}
}

class ScoreTimer_DescText extends AnimatingCanvas
{
	// Display
	text = null // Gui.Text
	
	constructor( locID )
	{
		::AnimatingCanvas.constructor( )

		text = ::Gui.Text( )
		text.SetFontById( FONT_SIMPLE_SMALL )
		text.SetRgba( COLOR_CLEAN_WHITE )
		AddChild( text )
		
		Reset( locID )
	}
	
	function Height( )
	{
		return text.Height
	}
	
	function Reset( locID )
	{
		if( locID == null )
			return
		text.BakeBoxLocString( 256, ::GameApp.LocString( locID ), TEXT_ALIGN_RIGHT )
		AddDelayedAction( 10.0, AlphaTween( 1.0, 0.0, 2.0 ) )
	}
}

// Main Class
////////////////////////////////////////////////////////////////////////////////
class ScoreTimer extends AnimatingCanvas
{
	// Display
	background = null // Gui.TexturedQuad, pretty lines
	timeLabel = null // Gui.Text
	scoreLabel = null // Gui.Text
	timeLeftText = null // ScoreTimer_TimeLeftText
	scoreText = null // ScoreTimer_ScoreText
	clock = null // ClockTimer
	descText = null // ScoreTimer_DescText
	miniLeaderboard = null // MiniLeaderboard
	
	// Data
	countDir = null
	dangerThreshold = null
	currentTime = null
	goalTime = null
	pause = null
	player = null
	minigameId = null
	tickTimer = null
	done = null
	currentLevel = null
	
	// Callbacks
	onStart = null
	onEnd = null
	onDangerStart = null
	
	// player_( Player ), player to use
	// minigameId_( enum ), id of the minigame
	// countDirection_( enum ), default: TimerCountDirection.Down
	// time_( number ), (if down) start time, (if up) goal time, null for no timer
	// dangerThreshold_( number ), default: null (can also be -1 for no danger)
	// showScore_( bool ), default: true
	// showLabels_( bool ), default: true
	// descId_( LocString ), default: null (if null, will not show desc text
	constructor( player_, minigameId_, countDirection_ = null, time_ = null, dangerThreshold_ = null, showScore_ = null, showLabels_ = null, descId_ = null )
	{
		::AnimatingCanvas.constructor( )
		
		player = player_
		audioSource = player.AudioSource
		minigameId = minigameId_
		countDir = ( ( countDirection_ != null )? countDirection_: TimerCountDirection.Down )
		dangerThreshold = ( ( dangerThreshold_ == null || dangerThreshold_ < 0 )? null: dangerThreshold_ )
		
		pause = false
		tickTimer = 0
		done = false
		currentLevel = ::GameApp.CurrentLevel
		currentLevel.MiniGameCurrentScore = 0
		
		local showScore = ( ( showScore_ != null )? showScore_: true )
		local showLabels = ( ( showLabels_ != null )? showLabels_: true )
		local descId = descId_
		local initialTime = 0.0
		
		local vpRect = ::GameApp.CurrentLevel.ControllingPlayer( ).ComputeViewportSafeRect( )
		
		if( time_ != null )
		{
			initialTime = ( ( countDir == TimerCountDirection.Down )? time_: 0.0 )
			currentTime = initialTime
			currentLevel.MiniGameTime = currentTime
			goalTime = ( ( time_ == null || countDir == TimerCountDirection.Down )? 0.0: time_ )
			if( time_ == 0 )
			{
				countDir = TimerCountDirection.Up
				goalTime = null
			}
		}
		else
		{
			currentTime = null
			goalTime = null
		}
		
		// Background Image
		background = ::Gui.TexturedQuad( )
		background.SetTexture( "gui/textures/score/score_decoration_g.png" )
		background.SetPosition( 0, 0, 0 )
		AddChild( background )
		
		if( ::GameApp.SingleScreenCoop )
		{
			local players = [ ::GameApp.GetPlayer( 0 ), ::GameApp.GetPlayer( 1 ) ]
			//local scores = [ ::GetMinigameScore( players[ 0 ] ), ::GetMinigameScore( players[ 1 ] ) ]
			
			descText = ::PreviousPlayerUI( players[ 0 ], 0, players[ 1 ], 0 )
			descText.SetPosition( 256, -vpRect.Center.y + vpRect.Top, 0 )
		}
		else
		{
			// Descriptive Text
			descText = ::ScoreTimer_DescText( descId )
			descText.SetPosition( 0, -descText.Height( ), 0 )
		}
		
		AddChild( descText )
		
		// Consts
		local row1 = 4
		local row2 = 36
		local col1 = 90
		
		if( time_ != null )
		{
			// Labels
			timeLabel = ::Gui.Text( )
			timeLabel.SetFontById( FONT_SIMPLE_SMALL )
			timeLabel.SetRgba( COLOR_CLEAN_WHITE )
			timeLabel.BakeLocString( GameApp.LocString( "Time_Left" ), TEXT_ALIGN_RIGHT )
			timeLabel.SetPosition( ::Math.Vec3.Construct( col1, row1, 0 ) )
			AddChild( timeLabel )
		}

		scoreLabel = ::Gui.Text( )
		scoreLabel.SetFontById( FONT_SIMPLE_SMALL )
		scoreLabel.SetRgba( COLOR_CLEAN_WHITE )
		scoreLabel.BakeLocString( GameApp.LocString( "Score" ), TEXT_ALIGN_RIGHT )
		scoreLabel.SetPosition( ::Math.Vec3.Construct( col1, row2, 0 ) )
		scoreLabel.SetUniformScale( 1.1 )	//scale this and scoreText up just a bit to make it more pronounced!
		AddChild( scoreLabel )
		
		// Timer
		timeLeftText = ::ScoreTimer_TimeLeftText( )
		timeLeftText.SetPosition( ::Math.Vec3.Construct( col1 + 5, row1 + timeLeftText.text.Height * 0.5, 0 ) )
		timeLeftText.SetTime( initialTime )
		AddChild( timeLeftText )
		
		// Score
		scoreText = ::ScoreTimer_ScoreText( 0 )
		scoreText.SetPosition( ::Math.Vec3.Construct( col1 + 5, row2 + scoreText.text.Height * 0.5, 0 ) )
		scoreText.SetUniformScale( 1.1 )//scaled up along with scoreLabel
		AddChild( scoreText )
		
		if( time_ == null )
		{
			scoreText.MakeBig( )
			scoreText.SetYPos( ::GameApp.IsAsianLanguage( ) ? 40 : 36 )
		}
		
		// Clock
		clock = ::ClockTimer( initialTime, countDir )
		clock.SetPosition( ::Math.Vec3.Construct( 256 - 64, row1, -0.01 ) )
		AddChild( clock )
		
		// Leaderboard
		miniLeaderboard = ::MiniLeaderboard( player, minigameId ) // HACK
		miniLeaderboard.SetPosition( ::Math.Vec3.Construct( 0, 72, 0) )
		AddChild( miniLeaderboard )
		
		// Reset
		Reset( countDirection_, time_, dangerThreshold_, showScore_, showLabels_, descId_ )
		
		// Add this to the HUD
		::GameApp.HudRoot.AddChild( this )
		SetPosition( vpRect.Right, vpRect.Center.y, 0.41 )
		SetAlpha( 0.0 )
		FadeIn( )
	}
	
	function OnTick( dt )
	{
		::AnimatingCanvas.OnTick( dt )
		
		if( ::GameApp.Paused( ) || pause || done )
			return
		
		if( currentTime != null )
		{
			if( countDir == TimerCountDirection.Down )
			{
				currentTime -= dt
				
				if( currentTime < 0 )
				{
					Done( )
					return
				}
			}
			else
			{
				currentTime += dt
				
				if( goalTime != null && currentTime > goalTime )
				{
					Done( )
					return
				}
			}
			
			timeLeftText.SetTime( currentTime )
			currentLevel.MiniGameTime = currentTime
			
			local danger = false
			if( goalTime != null && dangerThreshold != null )
			{
				if( countDir == TimerCountDirection.Down )
					danger = (currentTime <= dangerThreshold + 1)? true: false
				else
					danger = (currentTime >= dangerThreshold - 1)? true: false

				timeLeftText.SetDanger( danger, onDangerStart )
			}
			
			tickTimer += dt
			if( tickTimer > 1.0 && currentTime > 0.5 )
			{
				tickTimer -= 1.0
				if( danger == true )
					PlaySound( "Play_HUD_StopWatch_NearDone" )
				else
					PlaySound( "Play_HUD_StopWatch_Seconds" )
			}
		}
	}
	
	function ChangePlayer( player_ )
	{
		if( player_ )
		{
			player = player_
			miniLeaderboard.ChangePlayer( player )
		}
	}
	
	function Done( )
	{
		if( onEnd )
			onEnd( )
			
		currentTime = goalTime
		currentLevel.MiniGameTime = currentTime
		done = true
		
		PlaySound( "Play_HUD_StopWatch_Final" )
	
		timeLeftText.SetTime( currentTime )
		timeLeftText.SetDanger( false, onDangerStart )
	}
	
	function Reset( countDirection_ = null, time_ = null, dangerThreshold_ = null, showScore_ = null, showLabels_ = null, descId_ = null )
	{
		local showScore = ( ( showScore_ != null )? showScore_: true )
		local showLabels = ( ( showLabels_ != null )? showLabels_: true )
		countDir = ( ( countDirection_ != null )? countDirection_: TimerCountDirection.Down )
		done = false
		
		if( descId_ != null && "Reset" in descText )
		{
			descText.Reset( descId_ )
		}
		
		// Labels
		if( showLabels )
		{
			if( timeLabel )
				timeLabel.SetAlpha( 1 )
			if( showScore )
				scoreLabel.SetAlpha( 1 )
		}
		else
		{
			if( timeLabel )
				timeLabel.SetAlpha( 0 )
			scoreLabel.SetAlpha( 0 )
		}
		
		if( time_ != null )
		{		
			local initialTime = ( ( countDir == TimerCountDirection.Down )? time_: 0.0 )
			dangerThreshold = ( ( dangerThreshold_ == null || dangerThreshold_ < 0 )? null: dangerThreshold_ )
			currentTime = initialTime
			currentLevel.MiniGameTime = currentTime
			clock.Reset( currentTime, countDir )
			goalTime = ( ( countDir == TimerCountDirection.Down )? 0.0: time_ )
			timeLeftText.SetTime( initialTime )
			if( time_ == 0 )
			{
				countDir = TimerCountDirection.Up
				goalTime = null
			}
		}
		else
		{
			goalTime = null
			currentTime = null
			timeLeftText.SetAlpha( 0 )
			clock.SetAlpha( 0 )
			if( timeLabel )
				timeLabel.SetAlpha( 0 )
		}		
		
		if( showScore )
			scoreText.SetAlpha( 1 )
		else
			scoreText.SetAlpha( 0 )
			
		scoreText.SetPoints( 0 )
		
		miniLeaderboard.SetPlayerScore( 0 )
		miniLeaderboard.TryGetUpdatedLeaderboards( )
		
		if( ::GameApp.SingleScreenCoop )
		{
			local vpRect = ::GameApp.CurrentLevel.ControllingPlayer( ).ComputeViewportSafeRect( )
			local players = [ ::GameApp.GetPlayer( 0 ), ::GameApp.GetPlayer( 1 ) ]
			descText.DeleteSelf( )
			descText = ::PreviousPlayerUI( players[ 0 ], 0, players[ 1 ], 0 )
			descText.SetPosition( 256, -vpRect.Center.y + vpRect.Top, 0 )
			AddChild( descText )
		}
			
		if( onStart )
			onStart( )
	}
	
	function AddPoints( p )
	{
		scoreText.AddPoints( p )		
		miniLeaderboard.SetPlayerScore( scoreText.Score( ) )
		currentLevel.MiniGameCurrentScore = scoreText.Score( )
		::SetMinigameBestScore( player, currentLevel.MiniGameCurrentScore )
	}
	
	function AddBonus( multiplier )
	{
		scoreText.AddBonus( multiplier )
		miniLeaderboard.SetPlayerScore( scoreText.Score( ) )
	}
	
	function AddTime( time )
	{
		if( currentTime != null )
			currentTime += time
	}
	
	function Score( )
	{
		return scoreText.Score( )
	}
	
	function FadeOut( )
	{
		ClearActions( )
		AddAction( AlphaTween( GetAlpha( ), 0.0, 0.5, null, null, null, function( canvas ) { canvas.DeleteSelf( ) } ) )
		AddAction( XMotionTween( GetXPos( ), GetXPos( ) + 256, 0.5, EasingTransition.Quadratic, EasingEquation.In ) )
	}
	
	function FadeIn( )
	{
		ClearActions( )
		AddAction( AlphaTween( GetAlpha( ), 1.0, 0.5 ) )
		AddAction( XMotionTween( GetXPos( ), GetXPos( ) - 256, 0.5, EasingTransition.Quadratic, EasingEquation.Out ) )
	}
	
	function Pause( )
	{
		pause = true
		clock.Pause( pause )
	}
	
	function Start( )
	{
		pause = false
		clock.Pause( pause )
	}
	
	function GetResults( )
	{
		local results = {
			score = null,
			personalBest = false,
			xblaBest = false,
			friendsBest = false
		}
		
		local score = scoreText.Score( )
		local previousBest = miniLeaderboard.previousBest.score
		
		results.score = score
		
		if( previousBest == -1 || score > previousBest )
			results.personalBest = true
		
		if( miniLeaderboard.BetterThanTheRest( ) )
			results.xblaBest = true
			
		if( miniLeaderboard.FriendsBest( ) )
			results.friendsBest = true
		
		return results
	}
}
