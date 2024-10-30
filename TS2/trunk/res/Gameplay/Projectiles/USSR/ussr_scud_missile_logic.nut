
sigexport function EntityOnCreate( entity )
{
	entity.Logic = ScudMissileLogic( )
}

class ScudMissileLogic extends ScudMissile
{
	constructor( )
	{
		ScudMissile.constructor( )

		MinTimeMultiplier = 0.6
		MaxTimeMultiplier = 1.6
		UserTimeMultiplier = 0.8
//		ShellCamSteerRate = 0.4
	
	}

	function DebugTypeName( )
		return "ScudMissileLogic"
}
