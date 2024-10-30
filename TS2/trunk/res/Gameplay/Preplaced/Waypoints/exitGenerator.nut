
sigexport function EntityOnCreate( entity )
{
	entity.Logic = WaypointLogic( )
	entity.Logic.PathType = PATH_TYPE_EXIT_GENERATOR_PATH
	GameApp.CurrentLevel.RegisterExitGenStart( entity )
}


sigexport function EntityOnSiblingsCreate( entity )
{
	PathEntity.AsPathEntity( entity ).ForEachNextPt( AttachLogic )
}

function AttachLogic( entity )
{
	entity.Logic = WaypointLogic( )
	PathEntity.AsPathEntity( entity ).ForEachNextPt( AttachLogic )
}