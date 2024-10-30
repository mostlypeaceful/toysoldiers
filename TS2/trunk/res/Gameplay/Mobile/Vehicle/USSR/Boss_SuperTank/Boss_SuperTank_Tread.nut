
sigexport function EntityOnCreate( entity )
{
	local al = AudioLogic( )
	entity.Logic = al
		
	local state0 = al.StateEvents( 0 )
	state0.OnEnter.Event = "Play_Boss_SuperTank_Tread"
	
	local state3 = al.StateEvents( 3 )
	state3.OnEnter.Event = "Stop_Boss_SuperTank_Tread"
	

}
