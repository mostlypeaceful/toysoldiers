

sigimport "Levels/Scripts/Common/game_standard.nut"

sigexport function EntityOnCreate( entity )
{
	RegisterLevelLogic( entity, LevelLogic_survivalleveltest( ) ) // RENAME to current level
}

class LevelLogic_survivalleveltest extends GameStandardLevelLogic // RENAME to current level
{
	wave_01 = null

	function OnSpawn( )
	{
		::GameStandardLevelLogic.OnSpawn( )

		wave_01 = AddWaveList( "WaveListExample01" )
		wave_01.Activate( )
		wave_01.SetLooping( false )
	}
}