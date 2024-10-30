// Minigame Time UI

class MinigameTimeUI extends AnimatingCanvas
{
	// Display
	time = null
	
	// Data
	level = null
	currentTime = null
	
	constructor( level_, player )
	{
		::AnimatingCanvas.constructor( )
		level = level_
		
		time = ::Gui.Text( )
		time.SetFontById( FONT_FANCY_LARGE )
		time.SetRgba( COLOR_CLEAN_WHITE )
		AddChild( time )
		
		local vpRect = player.User.ComputeViewportSafeRect( )
		SetPosition( vpRect.Center.x, vpRect.Top, 0.3 )
		::GameApp.HudLayer( "alwaysShow" ).AddChild( this )
	}
	
	function OnTick( dt )
	{
		local newTime = level.MiniGameTime
		if( currentTime != newTime )
		{
			currentTime = newTime
			time.BakeLocString( ::LocString.ConstructTimeString( newTime, true ), TEXT_ALIGN_CENTER )
		}
	}
}