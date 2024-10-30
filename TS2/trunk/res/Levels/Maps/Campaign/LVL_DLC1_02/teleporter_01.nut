sigexport function EntityOnCreate( entity )
{
	entity.Logic = cmp_lvl4_Teleporter_01( )
}


class cmp_lvl4_Teleporter_01 extends TeleporterLogic
{
	constructor( )
	{
		TeleporterLogic.constructor( )
		ExitPathName = "teleporter_01_exit"
	}
	function DebugTypeName( )
		return "cmp_lvl4_Teleporter_01"
	
}