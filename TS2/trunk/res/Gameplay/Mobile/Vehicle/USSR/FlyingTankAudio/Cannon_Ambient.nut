
sigexport function EntityOnCreate( entity )
{
	local al = AudioLogic( )
	entity.Logic = al
	
	local state1 = al.StateEvents( 1 )
	state1.OnEnter.Event = "Play_FlyingTank_WingBreak"
		
	local state2 = al.StateEvents( 2 )
	state2.OnEnter.Event = "Play_FlyingTank_Cannon_Amb"
	
	local state3 = al.StateEvents( 3 )
	state3.OnEnter.Event = "Play_FlyingTank_Cannon_Critical"
	
	local state4 = al.StateEvents( 4 )
	state4.OnEnter.Event = "Stop_FlyingTank_Cannon_Critical"

}
