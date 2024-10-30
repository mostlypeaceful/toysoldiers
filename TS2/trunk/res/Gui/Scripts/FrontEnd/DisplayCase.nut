// Display Case

// Requires
sigimport "gui/scripts/frontend/staticscreens/staticscreen.nut"

class FrontEndDisplayCase extends FrontEndStaticScreen
{
	// Display
	fadingTextCanvas = null // AnimatingCanvas
	descriptionText = null // Gui.Text
	attributeText = null // Gui.Text
	nameText = null // Gui.Text
	
	// Data
	currentIndex = null
	country = null
	
	player = null
	
	constructor( player_ )
	{
		::FrontEndStaticScreen.constructor( )
		
		SetTitle( "Menus_DisplayCase" )
		
		ForwardButtons = 0
		BackButtons = GAMEPAD_BUTTON_B
		currentIndex = 0
		country = COUNTRY_USA
		player = player_
		player.SelectedUnitChangedCallback = SelectedUnitChanged.bindenv( this )
		
		local vpRect = ::GameApp.FrontEndPlayer.ComputeViewportSafeRect( )
		
		controls.padding = 20
		controls.AddControl( GAMEPAD_BUTTON_LTHUMB_MINMAG, "Menus_DisplayCaseSelectUnit" )
		controls.AddControl( GAMEPAD_BUTTON_RTHUMB_MINMAG, "Menus_DisplayCaseRotateUnit" )
		controls.AddControl( GAMEPAD_BUTTON_B, "Menus_Back" )
		
		local descLabel = ::Gui.Text( )
		descLabel.SetFontById( FONT_FANCY_MED )
		descLabel.SetRgba( COLOR_CLEAN_WHITE )
		descLabel.BakeLocString( ::GameApp.LocString( "DisplayCase_Description" ), TEXT_ALIGN_CENTER )
		descLabel.SetPosition( vpRect.Center.x - 400, vpRect.Center.y - 100, 0 )
		AddChild( descLabel )
		
		local descLabel = ::Gui.Text( )
		descLabel.SetFontById( FONT_FANCY_MED )
		descLabel.SetRgba( COLOR_CLEAN_WHITE )
		descLabel.BakeLocString( ::GameApp.LocString( "DisplayCase_Attributes" ), TEXT_ALIGN_CENTER )
		descLabel.SetPosition( vpRect.Center.x + 400, vpRect.Center.y - 100, 0 )
		AddChild( descLabel )
		
		descriptionText = ::Gui.Text( )
		descriptionText.SetFontById( FONT_SIMPLE_SMALL )
		descriptionText.SetRgba( COLOR_CLEAN_WHITE )
		descriptionText.SetPosition( vpRect.Center.x - 400, vpRect.Center.y - 60, 0 )
		AddChild( descriptionText )
		
		attributeText = ::Gui.Text( )
		attributeText.SetFontById( FONT_SIMPLE_SMALL )
		attributeText.SetRgba( COLOR_CLEAN_WHITE )
		attributeText.SetPosition( vpRect.Center.x + 400, vpRect.Center.y - 60, 0 )
		AddChild( attributeText )
		
		nameText = ::Gui.Text( )
		nameText.SetFontById( FONT_SIMPLE_SMALL )
		nameText.SetRgba( COLOR_CLEAN_WHITE )
		nameText.SetPosition( vpRect.Center.x, vpRect.Center.y - 210, 0 )
		AddChild( nameText )
	}
	
	function SelectedUnitChanged( unitLogic )
	{
		if( is_null( unitLogic ) )
		{
			HideText( 1 )
		}
		else
		{
			SetUnitID( unitLogic.UnitIDString, unitLogic.Country )
			HideText( 0 )
		}
	}	

	
	function SetUnitID( unitIDString, country )
	{
		local newUnitID = ((country == COUNTRY_USA)? "USA_": "USSR_") + unitIDString
		
		local unitDesc = ::GameApp.LocString( newUnitID + "_FullDescription" )
		local unitAtt = ::GameApp.LocString( newUnitID + "_PurchaseDescription" )
		
		descriptionText.BakeBoxLocString( 250, unitDesc, TEXT_ALIGN_CENTER )
		attributeText.BakeBoxLocString( 250, unitAtt, TEXT_ALIGN_CENTER )
		nameText.BakeLocString( ::GameApp.LocString( newUnitID + "_Class"), TEXT_ALIGN_CENTER )
	}
	
	function HideText( show )
	{
		descriptionText.Invisible = show
		attributeText.Invisible = show
		nameText.Invisible = show
	}
	
	function OnBackOut( )
	{
		if( ::GameApp.CurrentLevel.CanExitDisplayCase )
		{
			local dialog = ::ModalConfirmationBox( "Quit_ConfirmDisplayCase", player.User, "Ok", "Cancel" )
			dialog.onAPress = function( )
			{
				::ResetGlobalDialogBoxSystem( )
				::GameApp.LoadFrontEnd( )
			}
			return false
		}
		else
			return false
	}
	
	function OnFadeOut( )
	{
		player.SelectedUnitChangedCallback = null
		::FrontEndStaticScreen.OnFadeOut( )
	}
	
	function HandleCanvasEvent( event )
	{
		if( event.Id == ON_PLAYER_SIGN_OUT )
		{
			BackButtons = 0 //prevent 'B' from backing out of menu
			
			local dialogBox = ::ModalErrorBox( "Menus_UserSignedOutReturnToTitleScreen2", ::GameApp.FrontEndPlayer.User )
			dialogBox.onFadedOut = function( )
			{
				::ResetGlobalDialogBoxSystem( )
				::GameApp.LoadFrontEnd( )
				
			}.bindenv( this )
			
			return true
		}
		else if( event.Id == SESSION_INVITE_ACCEPTED )
		{
			::ResetGlobalDialogBoxSystem( )
			::GameApp.LoadFrontEnd( )
			return true
		}
		return false
	}
	
}