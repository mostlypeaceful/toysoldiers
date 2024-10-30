
sigimport "Gui/Scripts/RootMenus/ingame_rootmenu.nut"

class SinglePlayerStandardRootMenu extends BaseInGameRootMenu
{
	function OnTick( dt )
	{
		BaseInGameRootMenu.OnTick( dt )
		AutoHandlePauseMenu( dt )
	}
}


