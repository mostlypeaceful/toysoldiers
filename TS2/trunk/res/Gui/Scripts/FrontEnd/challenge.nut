
sigimport "Gui/Scripts/FrontEnd/common.nut"
sigimport "Gui/Scripts/FrontEnd/levelselect.nut"
sigimport "Gui/Scripts/FrontEnd/quickmatch.nut"
sigimport "Gui/Scripts/FrontEnd/privatematch.nut"

// I DON'T THINK WE EVEN USE THIS FILE ANYMORE!

class FrontEndChallengeMenu extends FrontEndMenuBase
{
	function FinalizeIconSetup( )
	{
		SetMenuNameText( "Menus_Challenge" )
		
		icons.push( FrontEndMenuEntry( "Menus_ViewChallengeLevelSelect", "Menus_ViewChallengeLevelSelect_HelpText", ShowLevelSelect.bindenv(this) ) )
		icons.push( FrontEndMenuEntry( "Menus_ViewChallengeQuickMatch", "Menus_ViewChallengeQuickMatch_HelpText", ShowQuickMatch.bindenv(this) ) )
		VerticalMenu.FinalizeIconSetup( )
	}
	function ShowLevelSelect( )
		return PushNextMenu( ChallengeLevelSelectMenu( ) )
	function ShowQuickMatch( )
		return PushNextMenu( FrontEndChallengeQuickMatch( ) )
}