
sigimport "Gameplay/Turrets/Common/TurretLogic.nut"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Wall_Wire( );
}
class USSR_Wall_Wire extends BaseTurretLogic
{
	function DebugTypeName( )
		return "USSR_Wall_Wire"

	function OnSpawn( )
	{
		//BaseTurretLogic.OnSpawn( )
	}	
}