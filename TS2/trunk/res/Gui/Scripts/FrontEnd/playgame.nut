
sigimport "Gui/Scripts/FrontEnd/common.nut"
sigimport "gui/scripts/frontend/levelselect.nut"
sigimport "gui/scripts/frontend/frontendcampaignmenu.nut"
sigimport "gui/scripts/frontend/displaycase.nut"
sigimport "gui/scripts/frontend/headtohead.nut"

class FrontEndPlayGameMenu extends FrontEndMenuBase
{
	showPartySessions = null
	
	function FinalizeIconSetup( )
	{
		SetMenuNameText( "Menus_PlayGame" )
		
		local player = ::GameApp.FrontEndPlayer
		local fullGame = ::GameApp.IsFullVersion
		if( fullGame )
		{
			icons.push( ::FrontEndMenuEntry( "Menus_ViewColdWarCampaignMenu", "Menus_ViewColdWarCampaignMenu_HelpText", function( ) { return PushNextMenu( ::FrontEndCampaignMenu( DLC_COLD_WAR ) ) }.bindenv(this) ) )
			if( player.HasDLC( DLC_EVIL_EMPIRE ) && !::GameApp.E3Mode )
				icons.push( ::FrontEndMenuEntry( "Menus_ViewSpecOpsCampaignMenu", "Menus_ViewSpecOpsCampaignMenu_HelpText", function( ) { return PushNextMenu( ::FrontEndCampaignMenu( DLC_EVIL_EMPIRE ) ) }.bindenv(this) ) )
			if( player.HasDLC( DLC_NAPALM ) && !::GameApp.E3Mode )
				icons.push( ::FrontEndMenuEntry( "Menus_ViewNapalmCampaignMenu", "Menus_ViewNapalmCampaignMenu_HelpText", function( ) { return PushNextMenu( ::FrontEndCampaignMenu( DLC_NAPALM ) ) }.bindenv(this) ) )

			if( !::GameApp.E3Mode )
				icons.push( ::FrontEndMenuEntry( "Menus_ViewVersusMenu", "Menus_ViewVersusMenu_HelpText", function( ) { return PushNextMenu( ::FrontEndHeadToHeadMenu( ) ) }.bindenv(this) ) )
		}
		else
		{
			icons.push( ::FrontEndMenuEntry( "Menus_PlayTrial", "Menus_PlayTrial_HelpText", PlayTrial.bindenv( this ) ) )
		}
		
		icons.push( ::FrontEndMenuEntry( "Menus_ViewSurvivalMenu", "Menus_ViewSurvivalMenu_HelpText", function( ) { return PushNextMenu( ::SurvivalLevelSelectMenu( ) ) }.bindenv(this) ) )
		icons.push( ::FrontEndMenuEntry( "Menus_ViewMinigameMenu", "Menus_ViewMinigameMenu_HelpText", function( ) { return PushNextMenu( ::MinigameLevelSelectMenu( ) ) }.bindenv(this) ) )

		if( !::GameApp.E3Mode )
		{
			showPartySessions = ::FrontEndMenuEntry( "Menus_ShowPartySessions", "Menus_ShowPartySessions_HelpText", function( ) { ::GameApp.FrontEndPlayer.User.ShowCommunitySessionsUI( ); return null; }.bindenv(this) )
			icons.push( showPartySessions )
		}
		icons.push( ::FrontEndMenuEntry( "Menus_CampaignDisplayCase", "Menus_CampaignDisplayCase_HelpText", DisplayCase.bindenv( this ) ) )
			
		::VerticalMenu.FinalizeIconSetup( )
		
		UpdateRankedOptionMenu( )
	}
	
	function UpdateRankedOptionMenu( )
	{
		if( !showPartySessions )
			return
			
		if( !::GameApp.FrontEndPlayer.User.SignedInOnline )
		{
			showPartySessions.SetLocked( true )
			showPartySessions.descriptor = "Menus_Error08"
		}
		else if( !::GameApp.FrontEndPlayer.User.HasPrivilege( PRIVILEGE_MULTIPLAYER ) )
		{
			showPartySessions.SetLocked( true )
			showPartySessions.descriptor = "Menus_ViewVersusRankedMatch_HelpText_Locked"
		}
		else if( !::GameApp.IsFullVersion )
		{
			showPartySessions.SetLocked( true )
			showPartySessions.descriptor = "Menus_ViewVersusRankedMatch_HelpText_Locked"
		}
		else
		{
			showPartySessions.SetLocked( false )
			showPartySessions.descriptor = "Menus_ViewVersusRankedMatch_HelpText"
		}
		
		if( highlightIndex != -1 )
			SetDescriptorText( icons[ highlightIndex ].descriptor )
	}
	
	function HandleCanvasEvent( event )
	{
		if( event.Id == ON_PLAYER_NO_LIVE || 
			event.Id == ON_PLAYER_YES_LIVE || 
			event.Id == ON_PLAYER_SIGN_OUT )
		{
			UpdateRankedOptionMenu( )
		}
		return false
	}

	function PlayTrial( )
	{
		::SetFrontEndLevelSelectRestart( MAP_TYPE_CAMPAIGN, DLC_COLD_WAR )
		GameApp.LoadLevel( MAP_TYPE_CAMPAIGN, 0, FillTrialLoadLevelInfo.bindenv( this ) )
		nextAction = VerticalMenuAction.ExitStack
		return true
	}
	
	function FillTrialLoadLevelInfo( info )
	{
		info.Difficulty = DIFFICULTY_CASUAL
	}
	
	function DisplayCase( )
	{
		::SetFrontEndDisplayCaseRestart( )
		GameApp.LoadLevel( MAP_TYPE_DEVSINGLEPLAYER, 0, null )
		nextAction = VerticalMenuAction.ExitStack
		return true
	}
	
	function ResetOptionsToFullVersion( )
	{
		finalized = false
		foreach( icon in icons )
		{
			RemoveChild( icon )
			icon.DeleteSelf( )
		}
		icons = [ ]
		FinalizeIconSetup( )
		if( icons.len( ) > 0 )
		{
			HighlightByIndex( 0 )
			icons[ 0 ].OnHighlight( true )
		}
	}
	
	function HandleCanvasEvent( event )
	{
		switch( event.Id )
		{
			case ON_UPGRADE_TO_FULL_VERSION:
				ResetOptionsToFullVersion( )
				break;
		}
		
		return ::FrontEndMenuBase.HandleCanvasEvent( event )
	}
}
