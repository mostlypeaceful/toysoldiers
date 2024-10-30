// Point Capture UI

// Requires
sigimport "gui/scripts/controls/radialprogressmeter.nut"

// Resources
sigimport "gui/textures/misc/capture_gray_g.png"
sigimport "gui/textures/misc/capture_blue_g.png"
sigimport "gui/textures/misc/capture_red_g.png"

sigexport function CanvasCreatePointCaptureUI( cppObj )
{
	return ::PointCaptureUI( cppObj.User )
}

class PointCaptureUI extends AnimatingCanvas
{
	// Display
	grayCapture = null
	blueCapture = null
	redCapture = null
	status = null
	
	// Data
	user = null
	owner = null
	lastUpdateTimer = null
	
	// Statics
	static Width = 104
	static Height = 104
	static FadeTime = 0.3
	static ShowTime = 4.0
	
	constructor( user_ )
	{
		::AnimatingCanvas.constructor( )
		user = user_
		owner = TEAM_NONE
		lastUpdateTimer = 0
		
		// Background (gray)
		grayCapture = ::RadialProgressMeter( "gui/textures/misc/capture_gray_g.png", DIRECTION_CLOCKWISE )
		grayCapture.CenterPivot( )
		grayCapture.SetPosition( 0, 0, 0.001 )
		grayCapture.SetAngle( -MATH_PI_OVER_2 )
		grayCapture.SetPercent( 0.0 )
		AddChild( grayCapture )
		
		// Blue Capture
		blueCapture = ::RadialProgressMeter( "gui/textures/misc/capture_blue_g.png", DIRECTION_CLOCKWISE )
		blueCapture.CenterPivot( )
		blueCapture.SetPosition( 0, 0, 0.001 )
		blueCapture.SetAngle( -MATH_PI_OVER_2 )
		blueCapture.SetPercent( 0.0 )
		blueCapture.SetAlpha( 0.0 )
		AddChild( blueCapture )
		
		// Red Capture
		redCapture = ::RadialProgressMeter( "gui/textures/misc/capture_red_g.png", DIRECTION_CLOCKWISE )
		redCapture.CenterPivot( )
		redCapture.SetPosition( 0, 0, 0 )
		redCapture.SetAngle( -MATH_PI_OVER_2 )
		redCapture.SetPercent( 0.0 )
		redCapture.SetAlpha( 0.0 )
		AddChild( redCapture )
		
		// Status text
		status = ::Gui.Text( )
		status.SetFontById( FONT_SIMPLE_SMALL )
		status.SetRgba( COLOR_CLEAN_WHITE )
		status.SetPosition( 0, -Height * 0.5 - 32, 0 )
		AddChild( status )
		
		// Add to screen
		local vpRect = user.ComputeViewportRect( )
		::GameApp.HudLayer( "viewport" + user.ViewportIndex.tostring( ) ).AddChild( this )
		SetPosition( vpRect.Center.x, vpRect.Center.y + 120, 0.4 )
		SetAlpha( 0.0 )
		
		if( !user.IsLocal )
			Invisible = true
	}
	
	function OnTick( dt )
	{
		if( lastUpdateTimer > 0.0 )
		{
			lastUpdateTimer -= dt
			if( lastUpdateTimer <= 0.0 )
				Show( false )
		}
		
		::AnimatingCanvas.OnTick( dt )
	}
	
	function SetStatusText( locId )
	{
		 status.BakeLocString( ::GameApp.LocString( locId ), TEXT_ALIGN_CENTER )
	}
	
	function Updated( )
	{
		if( lastUpdateTimer <= 0.0 )
			Show( true )
		lastUpdateTimer = ShowTime
	}
	
	function Show( show )
	{
		ClearActionsOfType( "AlphaTween" )
		AddAction( ::AlphaTween( GetAlpha( ), show ? 1.0 : 0.0, FadeTime ) )
	}
	
	function CurrentOwner( )
	{
		return owner
	}
	
	function SetDisplayPercent( team, percent )
	{
		percent = ::Math.Clamp( percent, 0.00001, 0.9999 )
		if( team == TEAM_RED )
			redCapture.SetPercent( percent )
		else if( team == TEAM_BLUE )
			blueCapture.SetPercent( percent )
		else if( team == TEAM_NONE )
			grayCapture.SetPercent( percent )
	}
	
	function Display( team )
	{
		if( team == TEAM_RED )
			return redCapture
		else if( team == TEAM_BLUE )
			return blueCapture
		else if( team == TEAM_NONE )
			return grayCapture
	}
	
	function SetPercent( team, percent )
	{
		if( team == TEAM_NONE )
			return
		local otherTeam = ( ( team == TEAM_BLUE ) ? TEAM_RED : TEAM_BLUE )
		if( owner == TEAM_NONE )
			otherTeam = owner
		
		// Move progress to top
		Display( team ).SetZPos( 0 )
		Display( otherTeam ).SetZPos( 0.001 )
		
		// Set the progress for the capturing team
		Display( team ).SetDirection( DIRECTION_CLOCKWISE )
		SetDisplayPercent( team, percent )
		Display( otherTeam ).SetDirection( DIRECTION_COUNTERCLOCKWISE )
		SetDisplayPercent( otherTeam, 1.0 - percent )
		Display( team ).SetAlpha( 1 )
		Display( otherTeam ).SetAlpha( 1 )
		
		if( otherTeam != TEAM_NONE )
			Display( TEAM_NONE ).SetAlpha( 0 )
			
		SetStatusText( "beginCapturing" )
		
		Updated( )
	}
	
	function Capture( team )
	{
		// Set the owner
		owner = team
		
		if( owner == TEAM_NONE )
		{
			// Point became neutral (I honestly don't think this can happen)
			SetDisplayPercent( TEAM_BLUE, 0.0 )
			SetDisplayPercent( TEAM_RED, 0.0 )
			SetDisplayPercent( TEAM_NONE, 1.0 )
			Display( TEAM_NONE ).SetAlpha( 0 )
		}
		else
			SetPercent( team, 1.0 )
		
		SetStatusText( "alliedCaptured" )
		
		// TODO Animation
	}
	
	function Lost( toTeam )
	{
		owner = toTeam
		SetPercent( toTeam, 1.0 )
		
		SetStatusText( "enemyCaptured" )
		
		// TODO Animation
	}
	
	function Disputed( isDisputed )
	{
		SetStatusText( "disputeCapturing" )
		Updated( )
		
		// TODO Animation
	}
}