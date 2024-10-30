sigexport function EntityOnCreate( entity )
{
	entity.Logic = FollowPathCameraPointLogic( )
	local params = entity.Logic.Params
	
	params.LookAtTarget = "focus_platform"
}
