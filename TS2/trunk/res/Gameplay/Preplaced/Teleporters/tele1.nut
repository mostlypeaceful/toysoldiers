sigexport function EntityOnCreate( entity )
{
	entity.Logic = Teleporter1Logic( )
}


class Teleporter1Logic extends TeleporterLogic
{
	constructor( )
	{
		TeleporterLogic.constructor( )
		ExitPathName = "tele1_exit"
		DestroyedPathNameDisable = "tele1_entry"
		DestroyedPathNameEnable = "tele_alt"
	}
	function DebugTypeName( )
		return "Teleporter1Logic"
	
}