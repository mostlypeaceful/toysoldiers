sigexport function EntityOnCreate( entity )
{
	entity.Logic = cmp_lvl4_Teleporter_03( )
}


class cmp_lvl4_Teleporter_03 extends TeleporterLogic
{
	constructor( )
	{
		TeleporterLogic.constructor( )
		ExitPathName = "teleporter_03_exit"
	}
	function DebugTypeName( )
		return "cmp_lvl4_Teleporter_03"
	
}