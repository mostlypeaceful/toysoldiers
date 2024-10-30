
sigimport "Gui/Scripts/FrontEnd/common.nut"
sigimport "Gui/Scripts/FrontEnd/mainmenu.nut"

class FrontEndTitleScreen extends VerticalMenu
{
	buttonPressedFlag = null
	noInput = null
	
	constructor( )
	{
		::VerticalMenu.constructor( )
		SetPosition( 0, 0, 0.5 )
		local vpRect = ::GameApp.ComputeScreenSafeRect( )
		menuPositionOffset = ::Math.Vec3.Construct( vpRect.Left + 460, vpRect.Top + 320, 0.0 )
		ForwardButtons = 0
		BackButtons = 0
		buttonPressedFlag = false
		
		local gameLogo = ::Gui.TexturedQuad( )
		gameLogo.SetTexture( "gui/textures/frontend/logo_ts2_g.png" )
		gameLogo.SetPosition( 240, 120, 0.0 )
		AddChild( gameLogo )
		
		// Reset the music if they came here from the credits
		::GameApp.AudioEvent( "Stop_UI_Credits" )
	}
	
	function FinalizeIconSetup( )
	{
		icons.push( ::FrontEndMenuEntry( "Press_Start", null, ShowMainMenu.bindenv(this) ) )
		::VerticalMenu.FinalizeIconSetup( )
	}
	
	function QueryUser( )
	{
		local userId = ::Application.FindActiveLocalUser( ForwardButtons )
		if( userId == ~0 )
			return null
			
		if( !::GameApp.SetPlayerLocalUser( 0, userId ) )
			return null
		
		user = ::GameApp.FrontEndPlayer.User
		return user
	}
	
	function VerticalMenuFadeIn( verticalMenuStack )
	{
		verticalMenuStack.ClearUser( )
		::FrontEndMenuBase.VerticalMenuFadeIn( verticalMenuStack )

		DoAfter( 0.5, function( canvas ) { ForwardButtons = GAMEPAD_BUTTON_A | GAMEPAD_BUTTON_START }.bindenv( this ) )
	}
	
	function ShowMainMenu( )
	{
		if( ::GameApp.FrontEndPlayer.User && !::GameApp.FrontEndPlayer.User.SignedIn )
		{
			buttonPressedFlag = true
			::GameApp.FrontEndPlayer.User.ShowSignInUI( )
			return false
		}
		
		// Apply Game Settings
		::GameApp.FrontEndPlayer.ApplyProfileSettings( )
		::GameApp.FrontEndPlayer.ShowFirstPlayMessages( )
		
		if( ::GameApp.PAXDemoMode )
			nextMenu = PAXFrontEndMainMenu( )
		else
			nextMenu = FrontEndMainMenu( )
			
		nextAction = VerticalMenuAction.PushMenu
		return true
	}
	
	function HandleCanvasEvent( event )
	{
		if( event.Id == ON_PLAYER_SIGN_IN && buttonPressedFlag )
		{
			buttonPressedFlag = false
			highlightIndex = 0
			AutoAdvance = true
			return true
		}
		else if( event.Id == ON_SYSTEM_UI_CHANGE )
		{
			if( !::GameApp.SysUiShowing && ::GameApp.FrontEndPlayer.User && !::GameApp.FrontEndPlayer.User.SignedIn )
			{
				buttonPressedFlag = false
				triggerUserReset = true;
				return true
			}
		}
		
		return false
	}
	
	function IgnoreSignOutEvent( )
	{
		return true
	}
}

