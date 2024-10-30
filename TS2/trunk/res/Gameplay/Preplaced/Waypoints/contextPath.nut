
sigexport function EntityOnCreate( entity )
{
	entity.Logic = WaypointLogic( )
	entity.Logic.PathType = PATH_TYPE_CONTEXT_PATH
	GameApp.CurrentLevel.RegisterContextPathStart( entity )
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
