
sigexport function EntityOnCreate( entity )
{
	local al = AudioLogic( )
	entity.Logic = al
		
	local state0 = al.StateEvents( 0 )
	state0.OnEnter.Event = "Play_FlyingTank_Engine_State_0"
	
	local state1 = al.StateEvents( 1 )
	state1.OnEnter.Event = "Play_FlyingTank_Engine_State_1"
	
	local state3 = al.StateEvents( 3 )
	state3.OnEnter.Event = "Stop_FlyingTank_Engine_State_1"

}
