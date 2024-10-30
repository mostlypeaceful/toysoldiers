
sigimport "Gui/Scripts/RootMenus/ingame_rootmenu.nut"

class CoOpStandardRootMenu extends BaseInGameRootMenu
{
	function OnTick( dt )
	{
		BaseInGameRootMenu.OnTick( dt )
		AutoHandlePauseMenu( dt )
	}
}


