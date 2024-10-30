
sigimport "Gui/Scripts/FrontEnd/common.nut"
sigimport "Gui/Scripts/FrontEnd/playgame.nut"
sigimport "Gui/Scripts/FrontEnd/leaderboards.nut"
sigimport "Gui/Scripts/FrontEnd/helpandoptions.nut"
sigimport "Gui/Scripts/FrontEnd/debugmenu.nut"
sigimport "gui/scripts/frontend/levelselect.nut"
sigimport "gui/scripts/frontend/watchtrailerscreen.nut"
sigimport "gui/scripts/dialogbox/globalmodaldialogbox.nut"

class PAXFrontEndMainMenu extends FrontEndMenuBase
{
	function FinalizeIconSetup( )
	{
		SetMenuNameText( "Menus_PAXMainMenu" )
		
		icons.push( ::FrontEndMenuEntry( "Menus_PAXPlayTheTrial", "Menus_PAXPlayTheTrial_HelpText", PlayTrial.bindenv(this) ) )
		icons.push( ::FrontEndMenuEntry( "Menus_PAXPlayTheSurvival", "Menus_PAXPlayTheSurvival_HelpText", PlaySurvival.bindenv(this) ) )
		icons.push( ::FrontEndMenuEntry( "Menus_PAXPlayTheMinigame", "Menus_PAXPlayTheMinigame_HelpText", PlayMiniGame.bindenv(this) ) )
		
		::VerticalMenu.FinalizeIconSetup( )
	}
	function PlayTrial( )
	{		
		::GameApp.LoadLevel( MAP_TYPE_CAMPAIGN, 0, null )
		nextAction = VerticalMenuAction.ExitStack
		return true
	}
	function PlayMiniGame( )
	{		
		::GameApp.LoadLevel( MAP_TYPE_MINIGAME, 10, null )
		nextAction = VerticalMenuAction.ExitStack
		return true
	}
	function PlaySurvival( )
	{		
		//GameApp.LoadLevel( MAP_TYPE_SURVIVAL, 0, null )
		//nextAction = VerticalMenuAction.ExitStack
		PushNextMenu( ::SurvivalLevelSelectMenu( ) )
		return true
	}
	function WatchTheTrailer( )
	{
		PushNextMenu( ::WatchTrailerScreen( ) )
		return true
	}
	function ShowPaxDemoOptions( )
	{
		PushNextMenu( ::FrontEndHelpAndOptionsMenu( ) )
		return true
	}
}

class FrontEndMainMenu extends FrontEndMenuBase
{
	function FinalizeIconSetup( )
	{
		SetMenuNameText( "Menus_MainMenu" )
		
		icons.push( ::FrontEndMenuEntry( "Menus_PlayTheGame", "Menus_PlayTheGame_HelpText", ShowPlayGameMenu.bindenv(this)  ) )
		icons.push( ::FrontEndMenuEntry( "Menus_ViewLeaderboards", "Menus_ViewLeaderboards_HelpText", ShowLeaderboards.bindenv(this) ) )
		icons.push( ::FrontEndMenuEntry( "Menus_ViewAchievements", "Menus_ViewAchievements_HelpText", ShowAchievements.bindenv(this) ) )
		icons.push( ::FrontEndMenuEntry( "Menus_ViewHelpAndOptions", "Menus_ViewHelpAndOptions_HelpText", ShowHelpAndOptions.bindenv(this) ) )

		local trialGame = !::GameApp.IsFullVersion
		if( trialGame )
			icons.push( ::FrontEndMenuEntry( "Menus_UnlockFullGame", "Menus_UnlockFullGame_HelpText", UnlockFullGame.bindenv(this) ) )
		else
			icons.push( ::FrontEndMenuEntry( "Menus_DownloadContent", "Menus_DownloadContent_HelpText", DownloadContent.bindenv(this) ) )

		icons.push( ::FrontEndMenuEntry( "Menus_ExitGame", "Menus_ExitGame_HelpText", ExitGame.bindenv(this) ) )
		
		// TODO remove this for release builds
		if( ::GameApp.DebugMainMenuEnabled( ) )
			icons.push( ::FrontEndMenuEntry( "Debug Main Menu", null, ShowDebugMenu.bindenv(this), true ) )
		//icons.push( ::FrontEndMenuEntry( "Menus_PAXTestMenu", null, ShowPaxMenu.bindenv(this) ) )
		
		::VerticalMenu.FinalizeIconSetup( )
	}
	function ShowPaxMenu( )
	{
		return PushNextMenuFromType( ::PAXFrontEndMainMenu )
	}
	function ShowPlayGameMenu( )
	{
		WarnIfNoWavelistRewindDeviceSelected( )
		return PushNextMenuFromType( ::FrontEndPlayGameMenu )
		//else
			//return false;
	}
	function ShowLeaderboards( )
	{
		return PushNextMenuFromType( ::FrontEndLeaderboardsMenu )
	}
	function ShowAchievements( )
	{
		::GameApp.FrontEndPlayer.User.ShowAchievementsUI( )
		return false
	}
	function ShowHelpAndOptions( )
	{
		return PushNextMenuFromType( ::FrontEndHelpAndOptionsMenu )
	}
	function UnlockFullGame( )
	{
		if( !user.SignedInOnline )
			::ModalInfoBox( "Menus_Error08", user )
		else
			user.ShowMarketplaceUI( false )
		return false
	}
	function DownloadContent( )
	{
		if( !::GameApp.FrontEndPlayer.User.SignedInOnline )
			::ModalInfoBox( "Menus_Error08", user )
		else
			::GameApp.FrontEndPlayer.User.ShowMarketplaceUI( true )
		return false
	}
	function ExitGame( )
	{
		if( ::GameApp.IsFullVersion )
		{
			local dialog = ::ModalConfirmationBox( "ExitGame_Confirm", ::GameApp.FrontEndPlayer.User, "Ok", "Cancel" )
			dialog.onFadedOut = function( )
			{
				::GameApp.ExitGame( )
			}.bindenv( this )
		}
		else
		{
			return PushNextMenu( ::TrialBuyGameScreen( ) )
		}
		return false
	}
	function WarnIfNoWavelistRewindDeviceSelected( )
	{
		if( ::GameApp.FrontEndPlayer.NeedsToChooseSaveDevice )
		{
			::GameApp.FrontEndPlayer.ChooseSaveDeviceId( 0 )
			return false
		}
		return true;
	}
	function ShowDebugMenu( )
	{
		return PushNextMenuFromType( FrontEndDebugMenu )
	}
		
	function PurchaseComplete( )
	{
		if( GameApp.IsFullVersion )
		{
			icons[ 4 ].Reset( "Menus_DownloadContent", "Menus_DownloadContent_HelpText", DownloadContent.bindenv(this) )
		}
	}
	
	function VerticalMenuFadeIn( verticalMenuStack )
	{
		::VerticalMenu.VerticalMenuFadeIn( verticalMenuStack )
		//PurchaseComplete( )
	}
	
	function HandleCanvasEvent( event )
	{
		switch( event.Id )
		{
			case ON_UPGRADE_TO_FULL_VERSION:
				PurchaseComplete( )
				break;
		}
		
		return ::FrontEndMenuBase.HandleCanvasEvent( event )
	}
}

