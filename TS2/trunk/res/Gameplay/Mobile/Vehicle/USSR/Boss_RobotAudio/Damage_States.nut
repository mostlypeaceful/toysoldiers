
sigexport function EntityOnCreate( entity )
{
	local al = AudioLogic( )
	entity.Logic = al
	
	local state1 = al.StateEvents( 1 )
	state1.OnEnter.Event = "Play_Boss_Robot_Critical"
	
	local state2 = al.StateEvents( 2 )
	state2.OnEnter.Event = "Play_Boss_Robot_Critical"
	
	local state3 = al.StateEvents( 3 )
	state3.OnEnter.Event = "Play_Robot_Death"

}
