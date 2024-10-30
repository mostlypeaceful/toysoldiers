
sigexport function EntityOnCreate( entity )
{
	local al = AudioLogic( )
	entity.Logic = al
		
	local state0 = al.StateEvents( 0 )
	state0.OnEnter.Event = "Play_Boss_Homer_Rotor_Right_State0"
	
	local state1 = al.StateEvents( 1 )
	state1.OnEnter.Event = "Play_Boss_Homer_Rotor_Right_State1"
	
	local state2 = al.StateEvents( 2 )
	state2.OnEnter.Event = "Play_Boss_Homer_Rotor_Right_State2"
	

}
