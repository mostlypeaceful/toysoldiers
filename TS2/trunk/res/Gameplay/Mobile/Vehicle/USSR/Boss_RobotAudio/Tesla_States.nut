
sigexport function EntityOnCreate( entity )
{
	local al = AudioLogic( )
	entity.Logic = al
	
	local state1 = al.StateEvents( 1 )
	state1.OnEnter.Event = "Play_Robot_Tesla_Amb"
	
	local state2 = al.StateEvents( 2 )
	state2.OnEnter.Event = "Stop_Robot_Tesla_Amb"
	
}
