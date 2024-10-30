
sigimport "Gameplay/Turrets/Common/TurretLogic.nut"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_Wall_Wire( );
}
class USA_Wall_Wire extends BaseTurretLogic
{
	function DebugTypeName( )
		return "USA_Wall_Wire"

	function OnSpawn( )
	{
		//BaseTurretLogic.OnSpawn( )
	}	
}