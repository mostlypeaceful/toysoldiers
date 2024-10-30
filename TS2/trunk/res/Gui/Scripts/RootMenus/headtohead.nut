
sigimport "Gui/Scripts/RootMenus/ingame_rootmenu.nut"

class HeadToHeadStandardRootMenu extends BaseInGameRootMenu
{
	function OnTick( dt )
	{
		BaseInGameRootMenu.OnTick( dt )
		AutoHandlePauseMenu( dt )
	}
}


