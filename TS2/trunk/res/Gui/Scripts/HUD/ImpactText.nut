// Impact Text

class ImpactText extends AnimatingCanvas
{
	// Display
	animator = null
	
	constructor( locID, source = null, autoClear = false )
	{
		::AnimatingCanvas.constructor( )		
		audioSource = source
		
		// Text
		animator = ::AnimatingCanvas( )
		animator.audioSource = AudioSource
		
		local text = ::Gui.Text( )
		text.SetFontById( FONT_FANCY_LARGE )
		text.SetRgba( COLOR_CLEAN_WHITE )
		text.BakeLocString( ::GameApp.LocString( locID ), TEXT_ALIGN_CENTER )
		text.SetPosition( 0, -text.Height * 0.5, 0 )
		animator.AddChild( text )
		
		AddChild( animator )
		
		// Animate
		animator.SetAlpha( 0 )
		animator.AddAction( ::AlphaTween( 0.0, 1.0, 0.2 ) )
		animator.AddAction( ::UniformScaleTween( 0.3, 1.0, 0.3, null, EasingEquation.In ) )
		animator.AddDelayedAction( 0.5, ::YMotionTween( 0, 20, 1.0 ) )
		
		// Audio
		PlaySound( "Play_HUD_Impact01" )
		
		if( autoClear )
		{
			AddAction( ::CanvasAction( 4.0, null, null, function( canvas ) { canvas.Clear( ) } ) )
		}
	}
	
	function Clear( )
	{
		ClearActions( )
		animator.ClearActions( )
		animator.AddAction( ::UniformScaleTween( animator.GetScale( ).x, 0.3, 0.3 ) )
		animator.AddAction( ::YMotionTween( animator.GetYPos( ), 0, 0.3 ) )
		animator.AddDelayedAction( 0.2, ::AlphaTween(  animator.GetAlpha( ), 0.0, 0.1 ) )
	}
}

function DisplayImpactText( locID )
{
	local playerCount = ::GameApp.PlayerCount;
	for( local p = 0; p < playerCount; ++p )
	{
		local player = ::GameApp.GetPlayer( p )
		::StandardImpactText( locID, player )
	}
}

function StandardImpactText( locID, player )
{
	local text = ::ImpactText( locID, player.AudioSource, true )
	if( !player.User.IsLocal )
		text.Invisible = true
	local vpSafeRect = player.User.ComputeViewportSafeRect( )
	text.SetPosition( vpSafeRect.Left + vpSafeRect.Width / 2, vpSafeRect.Top + 264, 0.3 )
	::GameApp.HudLayer( "alwaysHide" ).AddChild( text )
}