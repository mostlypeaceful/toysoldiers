
sigimport "Gui/Scripts/FrontEnd/common.nut"
sigimport "Gui/Scripts/FrontEnd/levelselect.nut"
sigimport "gui/scripts/frontend/quickmatch.nut"
sigimport "gui/scripts/frontend/privatematch.nut"
sigimport "gui/scripts/frontend/versuscustommatch.nut"

class FrontEndHeadToHeadMenu extends FrontEndMenuBase
{
	rankedOptionMenu = null
	
	function FinalizeIconSetup( )
	{
		SetMenuNameText( "Menus_HeadToHead" )		
		
		rankedOptionMenu = ::FrontEndMenuEntry( "Menus_ViewVersusRankedMatch", "Menus_ViewVersusRankedMatch_HelpText", ShowQuickMatch.bindenv(this) )
		
		icons.push( rankedOptionMenu )
		icons.push( ::FrontEndMenuEntry( "Menus_ViewVersusCustomMatch", "Menus_ViewVersusCustomMatch_HelpText", ShowLevelSelect.bindenv(this) ) )
		
		::VerticalMenu.FinalizeIconSetup( )
		
		UpdateRankedOptionMenu( )
	}
	
	function UpdateRankedOptionMenu( )
	{
		if( !::GameApp.FrontEndPlayer.User.SignedInOnline )
		{
			rankedOptionMenu.SetLocked( true )
			rankedOptionMenu.descriptor = "Menus_Error08"
		}
		else if( !::GameApp.FrontEndPlayer.User.HasPrivilege( PRIVILEGE_MULTIPLAYER ) )
		{
			rankedOptionMenu.SetLocked( true )
			rankedOptionMenu.descriptor = "Menus_ViewVersusRankedMatch_HelpText_Locked"
		}
		else
		{
			rankedOptionMenu.SetLocked( false )
			rankedOptionMenu.descriptor = "Menus_ViewVersusRankedMatch_HelpText"
		}
		
		//SetDescriptorText( rankedOptionMenu.descriptor )
		if( highlightIndex != -1 )
			SetDescriptorText( icons[ highlightIndex ].descriptor )
	}
	
	function ShowLevelSelect( )
	{
		return PushNextMenu( ::HeadToHeadLevelSelectMenu( false, PlayerRestriction.None ) )
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
	
	function VerticalMenuFadeIn( stack )
	{
		::FrontEndMenuBase.VerticalMenuFadeIn( stack )
		UpdateRankedOptionMenu( )
	}
	
	function ShowQuickMatch( )
	{
		local user = ::GameApp.FrontEndPlayer.User
		local meNoDlc = (user.AddOnsInstalled == 0)
	
		if( meNoDlc )
		{
			WarnIncompatibleDlc( )
			return false
		}
		
		::SetFrontEndRankedMatchRestart( )
		return PushNextMenu( ::FrontEndVersusQuickMatch( ) )
	}
}