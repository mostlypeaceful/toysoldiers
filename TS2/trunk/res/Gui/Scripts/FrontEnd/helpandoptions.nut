
sigimport "Gui/Scripts/FrontEnd/common.nut"
sigimport "Gui/Scripts/FrontEnd/howtoplay.nut"
sigimport "Gui/Scripts/FrontEnd/controls.nut"
sigimport "Gui/Scripts/FrontEnd/settings.nut"
sigimport "Gui/Scripts/FrontEnd/credits.nut"
sigimport "gui/scripts/dialogbox/dialogbox.nut"
sigimport "gui/scripts/dialogbox/globalmodaldialogbox.nut"

class FrontEndHelpAndOptionsMenu extends FrontEndMenuBase
{
	function FinalizeIconSetup( )
	{
		if( !::GameApp.GameMode.IsFrontEnd )
		{
			local vpRect = ::GameApp.ComputeScreenSafeRect( )
			menuPositionOffset = ::Math.Vec3.Construct( vpRect.Left + 20, vpRect.Center.y - 150, 0 )
			
			local fade = ::Gui.ColoredQuad( )
			fade.SetRgba( 0.0, 0.0, 0.0, 0.5 )
			fade.SetRect( ::Math.Vec2.Construct( 1280, 720 ) )
			fade.SetPosition( 0, 0, 0.02 )
			AddChild( fade )
		}
		SetMenuNameText( "Menus_HelpAndOptions" )

		icons.push( FrontEndMenuEntry( "Menus_HowToPlay", "Menus_HowToPlay_HelpText", ShowHowToPlay.bindenv(this) ) )
		icons.push( FrontEndMenuEntry( "Menus_Controls", "Menus_Controls_HelpText", ShowControls.bindenv(this) ) )
		if( ::GameApp.GameMode.IsFrontEnd || ::GameApp.FrontEndPlayer.PlayerIndex == ::GameApp.WhichPlayer( user ) || !user.IsLocal )
			icons.push( FrontEndMenuEntry( "Menus_Settings", "Menus_Settings_HelpText", ShowSettings.bindenv(this) ) )
		icons.push( FrontEndMenuEntry( "Menus_Credits", "Menus_Credits_HelpText", ShowCredits.bindenv(this) ) )
		if( ::GameApp.GameMode.IsFrontEnd && ::GameApp.IsFullVersion )
		{
			icons.push( FrontEndMenuEntry( "Menus_SelectStorage", "Menus_SelectStorage_HelpText", ShowDeviceSelection.bindenv(this) ) )
			icons.push( FrontEndMenuEntry( "Menus_ResetProfile", "Menus_ResetProfile_HelpText", ResetProfile.bindenv(this) ) )
		}
		::VerticalMenu.FinalizeIconSetup( )
		
		::GameApp.AudioEvent( "Stop_UI_Credits" )
	}
	
	function ShowHowToPlay( )
	{
		if( ::GameApp.GameMode.IsFrontEnd )
			return PushNextMenu( ::FrontEndHowToPlayMenu( ) )
		else
			return PushNextMenu( ::FrontEndHowToPlayMenu( ) )
	}
	
	function ShowControls( )
	{
		return PushNextMenu( ::FrontEndControlsMenu( ) )
	}
	
	function ShowSettings( )
	{
		return PushNextMenu( ::FrontEndSettingsMenu( ) )
	}
	
	function ShowCredits( )
	{
		return PushNextMenu( ::FrontEndCreditsMenu( ) )
	}
	
	function ShowDeviceSelection( )
	{
		return ::GameApp.FrontEndPlayer.ChooseSaveDeviceId( 1 )
	}
	
	function ResetProfile( )
	{
		// Show Dialog
		local dialog = ::ModalConfirmationBox( 
			"Menus_ResetProfileWarning", 
			::GameApp.FrontEndPlayer.User, 
			"Menus_ResetProfileReset", 
			"Menus_ResetProfileCancel" )
		
		dialog.onCanceled = function( )
		{
		}
		
		dialog.onFadedOut = function( )
		{
			::GameApp.FrontEndPlayer.ResetProfile( )
			::GameApp.FrontEndPlayer.SaveProfile( )
		}
	}
	
	function HandleCanvasEvent( event )
	{
		switch( event.Id )
		{
			case ON_UPGRADE_TO_FULL_VERSION:
				RefinalizeIconSetup( )
				break;
		}
		
		return ::FrontEndMenuBase.HandleCanvasEvent( event )
	}
}
