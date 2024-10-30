sigimport "Levels/Scripts/Common/game_standard.nut"

sigexport function EntityOnCreate( entity )
{
	RegisterLevelLogic( entity, LevelLogic_SpawnTest( ) ) // RENAME to current level (e.g. LevelLogic_Campaign_lvl01)
}

class LevelLogic_SpawnTest extends GameStandardLevelLogic // RENAME to current level (e.g. LevelLogic_Campaign_lvl01)
{
	waveList1 = null

	function OnSpawn( )
	{
		::GameStandardLevelLogic.OnSpawn( )
		
		waveList1 = AddWaveList( "wave_01" )
		waveList1.Activate( )
	}
}
