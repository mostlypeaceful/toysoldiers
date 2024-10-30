sigexport function EntityOnCreate( entity )
{
	entity.Logic = cmp_lvl4_Teleporter_04( )
}


class cmp_lvl4_Teleporter_04 extends TeleporterLogic
{
	constructor( )
	{
		TeleporterLogic.constructor( )
		ExitPathName = "teleporter_04_exit"
	}
	function DebugTypeName( )
		return "cmp_lvl4_Teleporter_04"
	
}