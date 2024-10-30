sigexport function EntityOnCreate( entity )
{
	entity.Logic = cmp_lvl4_Teleporter_02( )
}


class cmp_lvl4_Teleporter_02 extends TeleporterLogic
{
	constructor( )
	{
		TeleporterLogic.constructor( )
		ExitPathName = "teleporter_02_exit"
	}
	function DebugTypeName( )
		return "cmp_lvl4_Teleporter_02"
	
}