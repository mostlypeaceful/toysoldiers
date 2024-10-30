
sigexport function EntityOnCreate( entity )
{
	local al = AudioLogic( )
	entity.Logic = al
	
	local onSpawn = al.OnSpawnEvent
	onSpawn.Event = "Play_Robot_Heli_Engine"

	local onDelete = al.OnDeleteEvent
	onDelete.Event = "Stop_Robot_Heli_Engine"
}
