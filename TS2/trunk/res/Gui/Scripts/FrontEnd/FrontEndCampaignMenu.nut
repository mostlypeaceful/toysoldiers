// Front end campaign menu

// Continue / New Campaign
// Level Select
// Display Case
// Rations Dump

// Requires
sigimport "gui/scripts/frontend/common.nut"
sigimport "gui/scripts/frontend/levelselect.nut"
sigimport "gui/scripts/frontend/displaycase.nut"
sigimport "gui/scripts/pausemenu/rationsdump.nut"
sigimport "Gui/Scripts/DialogBox/globalmodaldialogbox.nut"

class FrontEndCampaignMenu extends FrontEndMenuBase
{
	dlc = null
	
	constructor( dlc_ )
	{
		dlc = dlc_
		::FrontEndMenuBase.constructor( )
	}
	
	function FinalizeIconSetup( )
	{
		local campaignNames = {
			[ DLC_COLD_WAR ] = "Menus_CampaignColdWar", 
			[ DLC_NAPALM ] = "Menus_CampaignNapalm", 
			[ DLC_EVIL_EMPIRE ] = "Menus_CampaignSpecOps"
		}
		if( dlc in campaignNames )
			SetMenuNameText( campaignNames[ dlc ] )
		else
			SetMenuNameText( "Menus_Campaign" )
		
		local profile = ::GameApp.FrontEndPlayer.GetUserProfile( )
		local highestLevel = ::Math.Min( profile.HighestLevelReached( dlc ), ::GameApp.NumLevelsInTable( MAP_TYPE_CAMPAIGN ) - 1 )
		local campaignComplete = ::GameApp.IsCampaignComplete( dlc, ::GameApp.FrontEndPlayer )
		
		if( ::UserProfile.IsFirstLevelInDlc( highestLevel, dlc ) )
		{
			icons.push( ::FrontEndMenuEntry( "Menus_CampaignNew", "Menus_CampaignNew_HelpText", LaunchLevelFunc( highestLevel ) ) )
		}
		else if( !campaignComplete )
		{
			local levelInfo = ::GameApp.GetLevelLoadInfo( MAP_TYPE_CAMPAIGN, highestLevel )
			local str = ::GameApp.LocString( "Menus_CampaignContinue" )
			local desc = ::GameApp.LocString( "Menus_CampaignContinue_HelpText" ).Replace( "levelNumber", levelInfo.LevelIndex + 1 ).Replace( "levelName", levelInfo.MapDisplayName )
			icons.push( ::FrontEndMenuEntry( str, desc, LaunchLevelFunc( highestLevel ) ) )
		}
		
		icons.push( ::FrontEndMenuEntry( "Menus_CampaignLevelSelect", "Menus_CampaignLevelSelect_HelpText", function( ) { return PushNextMenu( ::CampaignLevelSelectMenu( dlc ) ) }.bindenv(this) ) )
		icons.push( ::FrontEndMenuEntry( "Menus_CampaignRationsDump", "Menus_CampaignRationsDump_HelpText", function( ) { return PushNextMenu( ::RationsDumpScreen( dlc ) ) }.bindenv(this) ) )
		::VerticalMenu.FinalizeIconSetup( )
	}
	
	function LaunchLevelFunc( i )
	{
		return function( ):(i)
		{
			if( ::GameApp.FrontEndPlayer.User.IsOnlineEnabled && !::GameApp.FrontEndPlayer.User.SignedInOnline )
			{
				local dialog = ::ModalConfirmationBox( "Menus_NoXboxLiveNoStats", ::GameApp.FrontEndPlayer.User, "Ok", "Cancel" )
				dialog.onAPress = function( ):(i)
				{
					ActuallyLaunchLevel( i )
					AutoExit = true
					PlaySound( forwardSound )
				}.bindenv(this)
				dialog.onBPress = function( )
				{
					PlaySound( errorSound )
				}.bindenv(this)
			}
			else
			{			
				ActuallyLaunchLevel( i )
				AutoExit = true
				return true
			}
			return false
		}.bindenv(this)
	}
	
	function ActuallyLaunchLevel( i )
	{
		::GameAppSession.HostGame( ::GameApp.FrontEndPlayer.User, CONTEXT_GAME_MODE_CAMPAIGN, CONTEXT_GAME_TYPE_STANDARD )
		::SetFrontEndLevelSelectRestart( MAP_TYPE_CAMPAIGN, dlc, null, false )
		::GameApp.LoadLevel( MAP_TYPE_CAMPAIGN, i, null )
	}
}
