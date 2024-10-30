sigexport function EntityOnCreate( entity )
{
	local al = AudioLogic( )
	entity.Logic = al
	al.OnSpawnEvent.Event = "Play_Object_Firework_100"	
}
