sigexport function EntityOnCreate( entity )
{
	local al = AudioLogic( )
	entity.Logic = al
		
	al.OnSpawnEvent.Event = "Play_World_Emitter_Pond_Berlin"
	al.OnDeleteEvent.Event = "Stop_World_Emitter_Pond_Berlin"
	
}