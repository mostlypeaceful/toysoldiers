
sigexport function EntityOnCreate( entity )
{
	entity.Logic = SubMissileLogic( )
}

class SubMissileLogic extends SubMissile
{
	constructor( )
	{
		SubMissile.constructor( )
	}

	function DebugTypeName( )
		return "SubMissileLogic"
}
