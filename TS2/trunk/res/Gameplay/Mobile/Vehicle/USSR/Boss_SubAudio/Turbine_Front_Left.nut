
sigexport function EntityOnCreate( entity )
{
	local al = AudioLogic( )
	entity.Logic = al
	
	local state3 = al.StateEvents( 3 )
	state3.OnEnter.Event = "Play_BossSub_AirTurbine"
	
	local state4 = al.StateEvents( 4 )
	state4.OnEnter.Event = "Stop_BossSub_AirTurbine"

}