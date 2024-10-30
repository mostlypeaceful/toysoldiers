
sigimport "Levels/Scripts/Common/game_standard.nut"

sigexport function EntityOnCreate( entity )
{
	RegisterLevelLogic( entity, WarfareLevelTestLogic( ) )
}


class WarfareLevelTestLogic extends GameStandardLevelLogic
{
	function OnSpawn( )
	{
		::GameStandardLevelLogic.OnSpawn( )


		AddWaveList( "WaveListExample01" )
		AddWaveList( "AllyWaveListExample01" )
//		AddWaveList( "WaveList2" )
		
		WaveList( 0 ).Activate( )
		WaveList( 1 ).Activate( )
	}
}
