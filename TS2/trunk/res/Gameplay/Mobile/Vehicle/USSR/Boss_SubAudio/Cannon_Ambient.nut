
sigexport function EntityOnCreate( entity )
{
	local al = AudioLogic( )
	entity.Logic = al
		
	local state2 = al.StateEvents( 2 )
	state2.OnEnter.Event = "Play_FlyingTank_Cannon_Amb"
	
	local state3 = al.StateEvents( 3 )
	state3.OnEnter.Event = "Play_FlyingTank_Cannon_Critical"
	
	local state5 = al.StateEvents( 5 )
	state5.OnEnter.Event = "Stop_FlyingTank_Cannon_Critical"

}
