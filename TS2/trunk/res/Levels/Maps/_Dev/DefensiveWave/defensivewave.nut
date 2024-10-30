sigimport "Levels/Scripts/Common/game_standard.nut"

sigexport function EntityOnCreate( entity )
{
	RegisterLevelLogic( entity, LevelLogic_DefensiveWave( ) )
}

class LevelLogic_DefensiveWave extends GameStandardLevelLogic
{
	enemyWave = null
	playerWave = null
	
	function OnSpawn( )
	{
		::GameStandardLevelLogic.OnSpawn( )
		
		enemyWave = AddWaveList( "EnemyWave" )
		enemyWave.Activate( )
		
		playerWave = AddWaveList( "PlayerWave" )
		playerWave.Activate( )
	}
}
