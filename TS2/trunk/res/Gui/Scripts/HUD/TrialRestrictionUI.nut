// Trial Restriction UI

class TrialRestrictionUI extends AnimatingCanvas
{
	// Display
	text = null
	
	// Data
	currentValue = null
	profile = null
	mapType = null
	
	constructor( player )
	{
		::AnimatingCanvas.constructor( )
		profile = player.GetUserProfile( )
		mapType = ::GameApp.CurrentLevelLoadInfo.MapType
		
		// Text
		text = ::Gui.Text( )
		text.SetFontById( FONT_SIMPLE_SMALL )
		text.SetRgba( 1, 0, 0, 1 )
		AddChild( text )
		
		// Add to Hud
		::GameApp.HudRoot.AddChild( this )
		local vpRect = ::GameApp.ComputeScreenSafeRect( )
		SetPosition( vpRect.Center.x, vpRect.Top, 0 )
	}
	
	function SetValue( value )
	{
		if( ::GameApp.IsFullVersion )
			return
			
		currentValue = value.tointeger( )
			
		local loc = null
		
		if( mapType == MAP_TYPE_SURVIVAL )
		{
			if( value < 0.0 )
				loc = ::GameApp.LocString( "TrialRestrict_SurvivalExpired" )
			else
				loc = ::GameApp.LocString( "TrialRestrict_Survival" ).Replace( "time", ::LocString.ConstructTimeString( value.tointeger( ).tofloat( ), 0 ) )
		}
		else if( mapType == MAP_TYPE_MINIGAME )
		{
			loc = ::GameApp.LocString( "TrialRestrict_Minigame" ).Replace( "current", value.tointeger( ) ).Replace( "total", TRIAL_MAX_MINIGAME_PLAYS.tointeger( ) )
		}
		
		if( loc )
			text.BakeLocString( loc, TEXT_ALIGN_CENTER )
	}
	
	function OnTick( dt )
	{
		if( mapType == MAP_TYPE_SURVIVAL )
		{
			local timeLeft = profile.SurvivalTimeRemaining
			if( timeLeft.tointeger( ) != currentValue )
				SetValue( timeLeft )
		}
		else if( mapType == MAP_TYPE_MINIGAME )
		{
			local playsLeft = profile.MinigameTriesRemaining
			if( playsLeft != currentValue )
				SetValue( playsLeft )
		}
		
		::AnimatingCanvas.OnTick( dt )
	}
	
	function HandleCanvasEvent( event )
	{
		switch( event.Id )
		{
			case ON_UPGRADE_TO_FULL_VERSION:
				if( mapType == MAP_TYPE_SURVIVAL || mapType == MAP_TYPE_MINIGAME )
					FadeOutAndDie( )
				break;
		}
		
		return ::AnimatingCanvas.HandleCanvasEvent( event )
	}
}