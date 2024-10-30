
sigimport "Levels/Scripts/Common/game_standard.nut"

sigexport function EntityOnCreate( entity )
{
	RegisterLevelLogic( entity, JungleLevelTestLogic( ) )
}


class JungleLevelTestLogic extends GameStandardLevelLogic
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


	// LEVEL BOSS
	function OnRegisterBoss( boss )
	{
		boss.Logic.UnitPath.LoopPaths = 1
		//boss.Logic.TakesDamage = false
		boss.Logic.RegisterForLevelEvent( LEVEL_EVENT_REACHED_END_OF_PATH, OnBossReachedEndOfPath.bindenv( this ) )
		boss.Logic.RegisterForLevelEvent( LEVEL_EVENT_UNIT_DESTROYED, OnBossDestroyed.bindenv( this ) )	
		boss.Logic.RegisterForLevelEvent( LEVEL_EVENT_BOSS_STAGE_CHANGED, OnBossStageChanged.bindenv( this ) )	
	}
	
	function OnBossStageChanged( boss )
	{
		print( "boss index: " + boss.stageNum )
		if( boss.stageNum == 1 ) boss.StartNewPath( "path_boss_stage03", 0 )
		else if( boss.stageNum == 2 ) boss.StartNewPath( "path_boss_stage01", 0 )
	}

		
}