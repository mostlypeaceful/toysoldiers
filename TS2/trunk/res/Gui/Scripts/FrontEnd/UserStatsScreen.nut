// User Stats Screen

// Requires
sigimport "gui/scripts/controls/animatingcanvas.nut"
sigimport "gui/scripts/frontend/staticscreens/staticscreen.nut"
sigimport "gui/scripts/endgamescreens/statsdisplay.nut"

class UserStatsScreen extends FrontEndStaticScreen
{
	display = null	
	reader = null
	
	constructor( userID, userGamertag )
	{
		::FrontEndStaticScreen.constructor( )
		inputHook = true
		
		SetTitle( ::LocString.FromCString( userGamertag ) )
		SetSubtitle( ::GameApp.LocString( "EndGame_Stats" ) )
		
		local vpSafeRect = ::GameApp.ComputeScreenSafeRect( )
		
		// Create Stats
		display = ::StatsDisplay( )
		display.audioSource = ::GameApp.FrontEndPlayer.AudioSource
		display.SetPosition( vpSafeRect.Center.x, decoration.WorldRect.Bottom + 65, 0 )
		AddChild( display )
		
		// Controls
		controls.AddControl( [ "gui/textures/gamepad/button_lshoulder_g.png", "gui/textures/gamepad/button_rshoulder_g.png" ], "LB_Pages" )
		controls.AddControl( "gui/textures/gamepad/button_b_g.png", "Menus_Back" )
		
		// Get Stats
		reader = ::ArbitraryStatReader( )
		reader.Create( userID )
	}
	
	function OnTick( dt )
	{
		::FrontEndStaticScreen.OnTick( dt )
		
		if( reader )
		{
			if( reader.Update( ) )
			{
				display.AddStandardStats( reader, null )
				display.ShowPage( 0 )
				reader = null
			}
		}
	}
	
	function HandleInput( gamepad )
	{
		if( !reader && display.HandleInput( gamepad ) )
			return true
			
		return FrontEndStaticScreen.HandleInput( gamepad )
	}
}