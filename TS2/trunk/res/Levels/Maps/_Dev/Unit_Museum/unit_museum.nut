sigimport "Levels/Scripts/Common/game_standard.nut"

sigexport function EntityOnCreate( entity )
{
	RegisterLevelLogic( entity, LevelLogic_UnitMuseum( ) ) // RENAME to current level (e.g. LevelLogic_Campaign_lvl01)
}

class LevelLogic_UnitMuseum extends GameStandardLevelLogic // RENAME to current level (e.g. LevelLogic_Campaign_lvl01)
{
	function OnSpawn( )
	{
		::GameStandardLevelLogic.OnSpawn( )
	}
}
